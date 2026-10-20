## 1. Implementation

- [x] 1.1 `UiImage::IsAudioFile` and config `media_probe_command`.
- [x] 1.2 `MediaInfo::RequestProbe`: task on the worker thread, `popen` and `stat`, writing or merging `.meta`, cache update and redraw flag.
- [x] 1.3 `UiHistoryView::Draw`: request the probe for downloaded video or audio without `seconds`.
- [x] 1.4 README.

## 2. Verification

- [x] 2.1 Sandbox: an old downloaded video shows duration and size without blocking, `.meta` is created, and after restart it is not probed again (log).
- [x] 2.2 Sandbox: with an invalid `media_probe_command`, only the size is shown and there are no retries.
- [ ] 2.3 Live: a video sent from nchat shows duration and size.
