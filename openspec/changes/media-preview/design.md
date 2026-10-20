## Context

- **Receiving (Go):**
  - `HandleImageMessage`, `HandleVideoMessage`, `HandleAudioMessage` and the extended text handler call `CWmNewMessagesNotify` with text, `fileId`, `filePath` and `fileStatus`.
  - `filePath` is `<profile>/tmp/<msgId><ext>`, and the extension comes from `ExtensionByType`.
  - The protos carry data that is discarded today:
    - `GetJPEGThumbnail()` on image, video and extended text;
    - `GetSeconds()` and `GetFileLength()` on video and audio, and `GetPTT()` on audio (voice note);
    - `GetTitle()`, `GetDescription()`, `GetBackgroundArgb()` and `GetTextArgb()` on extended text.
- **C interface:** `CWmNewMessagesNotify` has a fixed signature, and the cache stores `fileInfo` as hex with status, id, path and type. Adding fields there would force changes to the protocol and the schema.
- **View:** `UiHistoryView::Draw` decides inline with `UiImage::IsImageFile(filePath)` and `UiImage::Prepare`. It reserves a single image per message (`imageLineIndex`, `imageInfo`).
- **Colors:** `UiColorConfig::GetColorId(hex)` creates colors on demand, and `init_pair` uses incremental ids (`colorPairId`).
- **User's local tools:** `ffmpegthumbnailer`, `ffmpeg` and `mpv` installed.

## Goals / Non-Goals

**Goals:**
- Zero new network requests: only message data and local processes.
- No changes to the C interface or to the cache.
- The view never blocks: reading metadata is cached, and the frame is generated in the background.

**Non-Goals:**
- Playing video inside the terminal.
- Recovering metadata of messages already received.
- Link previews by querying the web.

## Decisions

### 1. Sidecar files in `tmp`
For messages from any chat, Go writes next to `filePath`, with the same base name `<profile>/tmp/<msgId>`:
- `<msgId>.thumb.jpg`: bytes of `GetJPEGThumbnail()`, if not empty;
- `<msgId>.meta`: `key=value` lines, and only the keys that have a value:
  - `seconds`, `file_length` (video and audio), `ptt=1` (voice note);
  - `title`, `description` (extended text with a link preview);
  - `background_argb`, `text_argb` (extended text, only in `status@broadcast`).

A message with no thumbnail and no keys with a value generates no files. For example, a plain text writes nothing.

They are written only if they do not exist, because an edited or repeated message must not rewrite them. Line breaks in the title and description are replaced with a space.

For text without an attachment, the base path is built the same way (`GetPath(connId) + "/tmp/" + msgId`). The UI derives it from `FileUtil::GetApplicationDir() + "/profiles/" + profileId + "/tmp/" + msgId`.

**Rejected alternative:** passing the fields through `CWmNewMessagesNotify` and the cache. It changes the C interface, the SQLite schema and the user's `nchat-status` script.

### 2. `.mp4` extension
`ExtensionByType` adds `.mp4` to `preferredExts`. Already downloaded `.f4v` files stay as they are; mpv opens them the same way.

### 3. Reading in the UI
New `MediaInfo` (`src/mediainfo.{h,cpp}`):
- `GetInfo(profileId, msgId)` returns `{ thumbPath, seconds, fileLength, isPtt, title, description, backgroundArgb, textArgb }`.
- Reads `.meta` with `Config` or a simple parser, and checks whether `.thumb.jpg` exists.
- Stores the result in an in-memory map. Sidecar files do not change after being written, so they are not reread.
- It is only queried for messages with an attachment, with a link in the text, or from the statuses chat (background color), so that the disk is not touched for every plain text. An empty result is also cached.

### 4. Video frame
- `UiImage::IsVideoFile(path)`: `mp4`, `f4v`, `m4v`, `mov`, `webm`, `3gp`, `mkv`.
- For a downloaded video from any chat, the frame is `<path without extension>.poster.jpg`.
- If it does not exist and `video_thumbnail_command` is not empty:
  - it is queued on a single worker thread that runs the command, replacing `%1` with the input and `%2` with the output;
  - when done, it requests a redraw of the history.
  - A video that fails is not retried in the session.
- Default of `video_thumbnail_command`: `ffmpegthumbnailer -i '%1' -o '%2' -s 640 -q 8 >/dev/null 2>&1`.

### 5. Inline image per message
The view chooses a single inline path:
1. downloaded image attachment (as today);
2. video: the frame if it exists; otherwise the embedded thumbnail;
3. image not downloaded: the embedded thumbnail;
4. link: the embedded thumbnail, with a maximum height of 4 rows.

It remains subject to `inline_images_enabled`, and nothing changes in downloads or in the expired-status check.

### 6. Video and audio attachment line
- **Video and audio:** `📎 <file> ▶ m:ss · N.N MB`, with the part that has data: duration from `seconds` and size from `file_length`.
- **Voice note** (`ptt=1`): `📎 <file> 🎤 m:ss`.
- Without metadata it stays as it is today.

### 7. Link preview
If the message has `title` or `description`, the following are added below the text:
- `▏<title>`, in bold;
- `▏<description>`, cut to 2 lines.

The message's normal color is used, and then the thumbnail.

### 8. Text status color
- Applies to a status without an attachment that has `background_argb`. Only statuses carry it.
- The text lines are drawn with a color pair: background from ARGB (alpha is ignored) and text from `text_argb`. If there is no `text_argb`, the text is white or black depending on the background's luminance.
- `UiColorConfig::GetRgbColorPair(fg, bg)` caches up to 32 pairs. Beyond the limit, or if the terminal does not support colors, it is drawn without color.
- Only the text lines change; the header and the time stay the same.

## Risks / Trade-offs

- **Disk:** a few KB per message with an attachment or link. Thumbnails weigh a few KB each, and the `.meta` less than 1 KB.
  - The `tmp` directory gains up to 2 files per message with media.
  - The `status-prefetch-cleanup` change keeps them when cleaning up videos.
- **History sync messages:** they go through the same handlers, so they also generate sidecar files.
- **Frame:**
  - `ffmpegthumbnailer` missing: the command fails, the thumbnail is used and it is not retried.
  - CPU cost: one process per downloaded video, one at a time.
- **Blurry thumbnail:** they are about 100 px, so they look pixelated when scaled. It is acceptable as a fallback, and that is why it has lower priority than the frame or the real image.
- **Colors:** terminals without 256 colors or truecolor show the approximate color or none.
