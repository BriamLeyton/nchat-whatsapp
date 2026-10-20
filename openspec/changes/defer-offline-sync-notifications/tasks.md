## 1. Go and protocol

- [x] 1.1 `protocol.h`: `OfflineSyncNotifyType` and `OfflineSyncNotify{isStarted}`; `wmchat.{h,cpp}`: `WmOfflineSyncNotify(connId, isStarted)` that enqueues a `DeferNotifyRequest`; `cgowm.go`: `extern` and `CWmOfflineSyncNotify`. Verify: `cd build && make -j$(nproc)` compiles without errors.
- [x] 1.2 `gowm.go`: `case *events.OfflineSyncPreview` calls `CWmOfflineSyncNotify(connId, 1)` and logs `Total`/`Messages`/`Receipts`; `case *events.OfflineSyncCompleted` calls `CWmOfflineSyncNotify(connId, 0)` and keeps `HandleSyncContacts()`. Verify: it compiles and, on nchat startup, the log shows the preview and the end.

## 2. UI

- [x] 2.1 `uimodel.cpp`: extract from `UpdateChatInfoIsUnread` the bell and desktop block into `NotifyChatMessage(profileId, chatId, chatMessage)`, without changing behavior. Verify: it compiles and a message received with nchat connected still notifies.
- [x] 2.2 `uimodel.{h,cpp}`: per-profile state `m_OfflineSyncStartTime`, `m_OfflineSyncEndTime`, `m_OfflineSyncPendingChats`; `case OfflineSyncNotifyType` according to design.md (decision 4: the preview starts or extends, an end without retention is ignored). Verify: the log shows the start and end of the retention.
- [x] 2.3 `UpdateChatInfoIsUnread`: during retention, add the chat to the pending ones instead of calling `NotifyChatMessage`. Verify: the log shows the retained chats and nothing rings during the sync.
- [x] 2.4 `ProcessOfflineSyncFlush()` from `ProcessTimers()`: flush 2 s after the end or 60 s after the start; per chat, check archived/restricted, last incoming message unread, muted/mention and `receivedAfterConnect`, and call `NotifyChatMessage`; log the pending and notified chats. Verify: it compiles and the log shows the flush with the counts.

## 3. Verification

- [ ] 3.1 Live in Ghostty: with nchat open, cut the network (`nmcli networking off`) for a few minutes; receive messages in 2 chats and read one on the phone; reconnect. Expected: nothing rings during the sync; at the end, one notification only for the unread chat and a single bell.
- [x] 3.2 Live: suspend the machine (`systemctl suspend`) with messages that are read on the phone during the suspension. Expected: on resume there are no notifications.
- [ ] 3.3 Live: with nchat connected and no sync in progress, a new message notifies instantly as before. On nchat startup, old messages do not notify.
