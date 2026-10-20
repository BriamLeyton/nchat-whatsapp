## Context

- **Serialized requests:** `WmChat` processes requests on a single thread (`m_RequestsQueue`), and `WmDownloadFile` downloads synchronously. Queuing 50 downloads delays any message send until they finish.
- **Downloads:** `UiModel::Impl::DownloadAttachment` is the only download point, and it already rejects expired statuses. The result arrives through `NewMessageFileNotify`, which updates `fileInfo` in memory and in the cache.
- **Arrival of statuses:** `NewMessagesNotify` arrives both for new messages (`cached == false`) and for cache reads (`cached == true`).
- **Video size:** available in `<msgId>.meta` (`file_length`) thanks to `media-preview`.
- **Updating an attachment in the cache:** `MessageCache::UpdateMessageFileInfo` exists.

## Goals / Non-Goals

**Goals:**
- Current statuses ready when opened, without blocking message sends.
- Bounded disk: old videos are deleted, and the visual parts (frame, thumbnail) are kept.

**Non-Goals:**
- Prefetching attachments of normal chats (that is `attachment_prefetch`).
- Cleaning up images or messages.

## Decisions

### 1. Triggering the prefetch
- It is triggered in `NewMessagesNotify` of the statuses chat, with `cached` either `true` or `false`. This way, at startup, current statuses in the cache that are not downloaded are also prefetched when they are loaded.
- Requirements for queuing:
  - `status_prefetch=1`;
  - the status is not expired;
  - the attachment is not downloaded and is downloadable;
  - it is an image, or a video with `file_length <= status_prefetch_video_max_mb * 1024 * 1024`.
- Duplicates are discarded with a set of queued or in-progress ids.

### 2. One-at-a-time queue
`m_StatusPrefetchQueue` (deque of `{profileId, chatId, msgId}`) and `m_StatusPrefetchInFlight`:
- When queuing, if none is in progress, the first is sent with `DownloadAttachment(..., DownloadFileActionNone)`.
- On `NewMessageFileNotify` of the in-progress message, the next one is sent.
- Before sending, expiry and already-downloaded are checked again, for example because the user opened it. If so, it is skipped.
- **Without a connection:** the queue is kept and nothing is sent until online (`Status::FlagOnline`), because downloads would fail and be marked as failed.
- **Fallback:** if the notification does not arrive within 120 s, it is considered finished and the queue moves on, so a lost download does not block the queue.

**Rejected alternative:** sending everything at once. It blocks sends for several minutes.

### 3. Cleanup at startup
- When the profile finishes loading, if `status_video_retention_days > 0`, a background thread:
  1. queries the cache, with `MessageCache::FetchFileInfosSync(profileId, chatId, beforeTime)`, for statuses with `fileInfo` and `timeSent < now - N days`;
  2. for each video (`UiImage::IsVideoFile`) with an existing file, deletes the file and stores `fileInfo` with status `FileStatusNotDownloaded` via `UpdateMessageFileInfo`;
  3. updates the messages in memory, under the model's mutex, if they are loaded.
- **Kept:** `.poster.jpg`, `.thumb.jpg` and `.meta`.
- Runs once per startup. The log records the number of files and MB freed.

### 4. Interaction with expired statuses
A cleaned video always belongs to an expired status, because N ≥ 1 day. Therefore it is never downloaded again: the view shows the frame or the thumbnail, and opening shows "Expired status, attachment not available.".

## Risks / Trade-offs

- **Mobile data:** prefetching uses data as the official app does. Setting `status_prefetch=0` disables it.
- **Frame:** if the frame was not generated before the video was deleted, only the embedded thumbnail remains. In practice it is generated when viewed or prefetched, if the view draws it.
- **In-memory queue:** lost on exit. On restart, reading the cache queues the current statuses again.
