## Why

`media-preview` takes the duration and size from the data the message carries. That only exists for messages received since that change. The following have no data:
- **Old messages**, received or sent from the phone: the data was in the message, but nchat discarded it.
- **Files sent from nchat:** they do not go through the receive handler. Their `fileInfo` points to the local file, already "downloaded".

For every video or audio that is already on disk, the duration and size can be obtained from the file, locally and without network.

## What Changes

- **Local backfill:** when showing a downloaded video or audio (from any chat and direction, old or sent from nchat) with no known duration, it is obtained in the background:
  - the duration, with a configurable command (`media_probe_command`, default `ffprobe`);
  - the size, from the file on disk.
- **Persistence:** the result is stored in `<msgId>.meta`, with the same format as `media-preview`, so it is not recalculated.
- Uses the same worker thread as the video frame, one at a time, and redraws when done.
- A file that fails is not retried in the session.
- When a video or audio that had no data is downloaded, it is also backfilled when shown.

Out of scope:
- attachments not downloaded (there is no local data nor data in the stored message);
- thumbnails of old images not downloaded;
- link previews for old or sent links (requires querying the web; would be a separate change).

## Capabilities

### New Capabilities
- `media-backfill`: duration and size of already downloaded videos and audios, computed locally.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/mediainfo.{h,cpp}`:
  - probe task on the existing worker thread;
  - writing `.meta`;
  - updating the in-memory cache.
- `src/uihistoryview.cpp`: request the backfill for downloaded video or audio without `seconds`.
- `src/uiconfig.cpp`: `media_probe_command`.
- `README.md`.
