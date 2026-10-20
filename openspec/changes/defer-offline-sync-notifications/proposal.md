## Why

When the machine comes back from suspend, nchat reconnects and whatsmeow delivers the offline queue all at once: the messages received during the suspension and, behind them, the `read-self` receipts for those already read on the phone. Every message rings (bell and desktop notification) as soon as it arrives, before its read receipt arrives, so dozens of messages that were already read ring at the same time.

The cause: only messages with `timeSent > m_ConnectTime` are notified (`src/uimodel.cpp:3501`). `m_ConnectTime` is set only once, by the `ConnectNotify` of `WmChat::Login()` (`lib/wmchat/src/wmchat.cpp:236`). On reconnect, `events.Connected` (`lib/wmchat/go/gowm.go:909`) does not update it, so everything arriving in the offline queue is treated as a new message.

## What Changes

- nchat forwards to the UI the start (`events.OfflineSyncPreview`) and the end (`events.OfflineSyncCompleted`) of whatsmeow's offline sync.
- While a profile's offline sync is running, incoming messages do not produce a bell or a desktop notification. A list of the chats that would have notified is kept.
- When the sync ends (after a short margin so the last events are processed), nchat checks those chats:
  - if they are still unread (no `read-self` arrived for their last message), it notifies once per chat, with the last message;
  - if they were read on the phone, it does not notify;
  - the bell rings at most once in total.
- If `OfflineSyncCompleted` never arrives, a maximum safety time ends the retention and applies the same check.
- Current rules stay the same: muted, archived and restricted chats, `notify_every_unread`, `terminal_bell_*` and `desktop_notify_*`.
- Outside the offline sync, notification of new messages does not change.

## Capabilities

### New Capabilities
- `offline-sync-notifications`: hold notifications during the offline sync on reconnect, and afterwards notify only the chats that are still unread.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`, `cgowm.go`: handle `events.OfflineSyncPreview` and forward both events through a new C callback.
- `lib/common/src/protocol.h`, `lib/wmchat/src/wmchat.{h,cpp}`: new `OfflineSyncNotify` (start/end).
- `src/uimodel.{h,cpp}`: per-profile retention state, `UpdateChatInfoIsUnread` keeps the chats pending instead of notifying, flush in `ProcessTimers`.
- No change to the database schema or the configuration format.
