## Why

- **Waiting when opening:** today a status is downloaded only when it is seen on screen (inline images) or when it is opened (videos). The official app downloads them as soon as they arrive, so they are ready when viewed.
- **Disk:** status attachments already take 1.1 GB in `tmp`, and 972 MB of that are videos (≈4.6 MB each).
  - About 55 videos arrive per day. Prefetching all of them would add about 250 MB/day with no limit.
  - Expired videos can no longer be downloaded, under the rule in `status-expired-local-only`. Keeping them forever only accumulates disk.

## What Changes

- **Status prefetch** (`status_prefetch=1`): when a current status (less than 24 h old) arrives with an attachment that is not downloaded, it is downloaded in the background.
  - Images are always prefetched.
  - Videos are prefetched if their size, taken from the `media-preview` metadata, does not exceed `status_prefetch_video_max_mb` (default 16). Without size metadata they are not prefetched.
  - Prefetches go through their own queue, one at a time: the next one is sent when the previous one finishes. This way sending messages does not wait behind 50 videos, because the protocol processes requests one by one.
  - Expired statuses are never prefetched.
- **Video cleanup** (`status_video_retention_days`, default 0 = never; the user enables it later):
  - with a value N greater than 0, at startup, in the background, the video files of statuses older than N days are deleted;
  - their `fileInfo` in the cache becomes "not downloaded";
  - images, thumbnails, frames and metadata are kept, so the status is still seen with its frame or thumbnail.

Out of scope: cleaning up images, and deleting status messages from the cache.

## Capabilities

### New Capabilities
- `status-prefetch-cleanup`: prefetching of current statuses and limited retention of status videos.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`:
  - prefetch queue fed from `NewMessagesNotify` of the statuses chat, which advances with `NewMessageFileNotify`;
  - cleanup at profile startup.
- `lib/ncutil/src/messagecache.{h,cpp}`: synchronous query of statuses with attachments older than a date.
- `src/uiconfig.cpp`: `status_prefetch`, `status_prefetch_video_max_mb`, `status_video_retention_days`.
- `README.md`.
- Depends on `media-preview` (video size and frame).
