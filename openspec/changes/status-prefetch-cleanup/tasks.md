## 1. Prefetch

- [x] 1.1 Configs `status_prefetch=1` and `status_prefetch_video_max_mb=16` in `ui.conf`.
- [x] 1.2 Prefetch queue in `UiModel::Impl`: queue from `NewMessagesNotify` of the statuses chat, advance on `NewMessageFileNotify`, 120 s fallback, and re-check for expired or already downloaded.

## 2. Cleanup

- [x] 2.1 `MessageCache::FetchFileInfosSync(profileId, chatId, beforeTime)`.
- [x] 2.2 Cleanup thread at startup: delete videos, update `fileInfo` in cache and memory, keep sidecar files, and log.
- [x] 2.3 Config `status_video_retention_days=0` (default: does not delete).
- [x] 2.4 README.

## 3. Verification

- [x] 3.1 Offline sandbox (copy with paths rewritten to fake files, without touching the real `tmp`):
  - [x] with the default 0 nothing is deleted;
  - [x] with 7, cleanup deletes videos older than 7 days (273), keeps recent videos (226) and images (925);
  - [x] the cache is left at "not downloaded";
  - [x] the view shows the video line without an indicator or download request.
- [x] 3.2 Offline sandbox: a current 5 MB video with metadata is queued only once; a 40 MB one, expired ones and videos without metadata are not; without a connection the queue sends nothing.
- [ ] 3.3 Live: a new status arrives already downloaded, and sending a message during prefetch does not wait for the full queue.
