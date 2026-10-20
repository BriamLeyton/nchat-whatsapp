## Context

- **`MediaInfo` (`media-preview`):**
  - `Get(profileId, msgId)` reads `<profile>/tmp/<msgId>.meta` and caches it in memory;
  - a worker thread generates frames with `video_thumbnail_command` and signals with `TakePosterUpdated()`.
- **View:** knows whether the attachment is downloaded (`FileStatusDownloaded`) and its path. For files sent from nchat, the path is the user's original file, outside `tmp`.
- **Tools:** `ffprobe` is installed together with `ffmpeg`.

## Goals / Non-Goals

**Goals:**
- Cover existing files and files sent from nchat, without network.
- Do not block the view.

**Non-Goals:**
- Data for attachments not downloaded.
- Web link previews.

## Decisions

### 1. Trigger
In `UiHistoryView::Draw` (final pass), for a downloaded video or audio attachment whose `MediaMeta` has no `seconds`, `MediaInfo::RequestProbe(profileId, msgId, filePath)` is called.
- **Audio:** `oga`, `ogg`, `opus`, `mp3`, `m4a`, `aac`, `wav`, `amr`, using `UiImage::IsAudioFile`.
- A voice note (`ptt`) cannot be deduced from the file, so those old audios show `▶`.

### 2. Probe
- It is queued on the same thread as the frames. A task is either a frame or a probe.
- It runs `media_probe_command` with `%1` replaced, and reads the output (`popen`). The duration is the number of seconds, rounded.
- The size comes from `stat` on the file.
- Default: `ffprobe -v error -show_entries format=duration -of csv=p=0 '%1' </dev/null 2>/dev/null`.

### 3. Storage
- If `.meta` exists, only the missing `seconds` and `file_length` are added. If not, it is created.
- The entry in the `MediaInfo` cache is updated and marked for redraw, with the same flag used for frames.
- If it fails, or if the command is empty, it is not retried in the session. If the command fails but the size is known, only the size is stored.

### 4. Cost
- `ffprobe` takes about 50 ms per file, and only runs for what is drawn.
- Once saved in `.meta`, each file is probed only once.

## Risks / Trade-offs

- **Without `ffprobe`:** only the size is stored.
- **Files sent from nchat:** if the user deletes or moves them, they already show as unavailable today, and they are not probed.
- **`.meta` in `tmp` for sent messages:** each one weighs less than 1 KB.
