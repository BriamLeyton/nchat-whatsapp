## 1. Protocol (Go)

- [x] 1.1 `ExtensionByType`: prefer `.mp4`.
- [x] 1.2 Helper `WriteMediaSidecar(connId, msgId, thumb, meta)`: writes `.thumb.jpg` and `.meta` if they do not exist, without network.
- [x] 1.3 Call it from the image, video, audio and extended text handlers, for all chats. Colors are only written in `status@broadcast`.

## 2. UI

- [x] 2.1 `MediaInfo`: reading and caching of `.meta` and thumbnail, only for messages with an attachment, link or from the statuses chat.
- [x] 2.2 `UiImage::IsVideoFile`, `video_thumbnail_command` in `ui.conf`, and a worker thread to generate frames with a redraw when done.
- [x] 2.3 `UiHistoryView::Draw`: choose the inline image by priority, line `▶ duration · size` or `🎤 duration`, and fallback thumbnail without downloading.
- [x] 2.4 Link preview (title, description, thumbnail of 4 rows at most).
- [x] 2.5 `UiColorConfig::GetRgbColorPair` and text status color.
- [x] 2.6 README: `video_thumbnail_command` and media preview.

## 3. Verification

- [x] 3.1 Offline sandbox (with simulated `.meta`, inline images disabled in tmux):
  - [x] build without new warnings;
  - [x] old messages without metadata look as they do today;
  - [x] chats with plain text do not read the disk per message;
  - [x] line `▶ 1:15 · 4.2 MB`, `🎤 0:42`, link preview with `…` truncation, and purple background of a text status;
  - [x] the log shows no new requests.
  - [x] the default frame command generates the `.jpg` of a real `.f4v` in 0.1 s.
- [x] 3.2 Go unit test or manual test: write and read `.meta` and `.thumb.jpg` from a simulated message.
- [ ] 3.3 Live in Ghostty:
  - [ ] a new video, in a chat and in statuses, arrives as `.mp4` with `▶ duration · size`;
  - [ ] a voice note shows `🎤 duration`;
  - [ ] an already downloaded video generates `.poster.jpg` without blocking the UI, and the frame is shown inline (in tmux inline images are disabled);
  - [ ] a link in a chat or group shows its preview;
  - [ ] a text status is shown with its color;
  - [ ] an expired, not downloaded status shows the thumbnail.
