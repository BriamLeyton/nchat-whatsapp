## Context

Motivation is in proposal.md (Why). Relevant current state:

- The decision to notify is in `UiModel::Impl::UpdateChatInfoIsUnread` (`src/uimodel.cpp`). It is called when messages are added and when their read state changes. It sets `m_TriggerTerminalBell` (the bell rings in `Process()`) and calls `DesktopNotify` immediately.
- It notifies only if the chat goes from read to unread (or always with `notify_every_unread`) and if `timeSent > m_ConnectTime[profileId]`. `m_ConnectTime` is only set by the `ConnectNotify` of `WmChat::Login()`.
- whatsmeow processes socket nodes in order in `handlerQueueLoop` (one node at a time). The offline queue arrives like this: node `ib/offline_preview` (`events.OfflineSyncPreview`), pending messages and receipts (including `read-self`), and node `ib/offline` (`events.OfflineSyncCompleted`). If nothing is pending, no preview arrives.
- `gowm.go` already handles `OfflineSyncCompleted` (it calls `HandleSyncContacts`), but not `OfflineSyncPreview`.
- Go→C++: events go through a C callback (`CWm*Notify` in `cgowm.go`, `Wm*Notify` in `wmchat.cpp`), which enqueues a `DeferNotifyRequest` with the `ServiceMessage`. The protocol thread delivers them to the UI in FIFO order, so the UI receives the end of the sync after all previous messages and receipts.
- `ReceiptTypeReadSelf` is already converted into `NewMessageStatusNotify(isRead=true)`, which calls `UpdateChatInfoIsUnread` and marks the chat as read.
- `ProcessTimers()` runs every ~200 ms on the UI thread.

## Goals / Non-Goals

**Goals:**
- Defer the bell and the desktop notification for messages that arrive during the offline sync, and decide at the end with the read state already updated.
- Reuse the current notification logic and rules, without duplicating them.

**Non-Goals:**
- No change to `m_ConnectTime` or to the filter of old messages at startup.
- No grouping of several chats into a single desktop notification.
- The `markChatAsRead` action of the app state is not processed. The phone sends `read-self` per message, and that is enough.
- No new configuration option: the retention is always active.

## Decisions

### 1. A single protocol message `OfflineSyncNotify { bool isStarted; }`
Go sends `isStarted=true` on `events.OfflineSyncPreview` and `isStarted=false` on `events.OfflineSyncCompleted`, through a new `CWmOfflineSyncNotify(connId, isStarted)`.
- Alternative: reuse `FlagSyncing` of the status (`CWmSetStatus`). Rejected: the status is a bar of flags for the view, it does not travel in the notification queue, and it does not guarantee the order with the messages.
- Alternative: a silence timer after `events.Connected`. Rejected: it does not know when the receipts end, and it would also silence new messages.

### 2. Retention in the UI, per profile
New state in `UiModel::Impl`:
- `m_OfflineSyncStartTime[profileId]` (ms), present while the retention lasts;
- `m_OfflineSyncEndTime[profileId]` (ms), set when the end is received;
- `m_OfflineSyncPendingChats[profileId]` (`std::set<std::string>`), the chats that would have notified.

In `UpdateChatInfoIsUnread`, in the branch that notifies today, if the profile is in retention the bell is not activated and `DesktopNotify` is not called: the chat is added to the pending ones. The computation of `isUnread` and the update of `profileChatInfos[...].isUnread` do not change, so the list looks the same.

To avoid duplicating the logic, the block that decides bell + desktop is extracted to a method `NotifyChatMessage(profileId, chatId, chatMessage)`. Both the immediate path and the flush use it.

### 3. Flush in `ProcessTimers` with a 2 s margin and a 60 s cap
`ProcessOfflineSyncFlush()` is called from `ProcessTimers()`. It flushes a profile when:
- 2 s have passed since `m_OfflineSyncEndTime`, or
- 60 s have passed since `m_OfflineSyncStartTime` without an end.

The 2 s margin covers the events that whatsmeow dispatches in goroutines (downloads, decryption retries) and those that arrive right after the `offline` node. During the margin the retention stays active.

When flushing, for each pending chat:
1. if the chat no longer exists in `m_ChatInfos`, is archived or is restricted, it is skipped;
2. the last message is taken (`GetLastMessageId`). If it is outgoing or already read, it is skipped;
3. the mute rules (`muted_notify_unread`, `hasMention`) and `receivedAfterConnect` are applied;
4. `NotifyChatMessage` is called with the bell limited to once (in practice it already is, because `m_TriggerTerminalBell` is a boolean that is consumed in `Process()`).

Then the three states of the profile are deleted.

- Alternative: flush in the same `case OfflineSyncNotifyType`. Rejected because it leaves no margin for late events.

### 4. A new preview during retention extends it
If another `isStarted=true` arrives (fast reconnection) while retention is active, the pending chats are kept, `m_OfflineSyncStartTime` is reset and `m_OfflineSyncEndTime` is cleared. An `isStarted=false` without active retention is ignored (for example, an end without a preview because nothing was pending).

### 5. `events.Disconnected` changes nothing
Retention ends with the end event or with the 60 s cap. No other signal is added: the cap is enough so that it never stays active forever.

## Risks / Trade-offs

- [A really new message that arrives during the sync notifies a few seconds late] → Acceptable: the sync usually lasts a few seconds, and the notification still arrives.
- [whatsmeow dispatches `OfflineSyncCompleted` before processing some message (goroutine of a node that exceeded the handler time)] → 2 s margin. If it still arrives late, that message notifies immediately, as today: an isolated case, not an avalanche.
- [The phone does not send `read-self` because the chat was marked as read without opening the messages (marking as read from the list)] → In that case a `markChatAsRead` arrives through the app state, which nchat does not process: the chat would notify. One notification per chat instead of one per message. Left as a future improvement if it becomes a problem.
- [nchat startup: a preview also arrives] → Retention applies, but `receivedAfterConnect` still filters old messages at flush. The result is the same as today.

## Migration Plan

No migration: no persisted data or configuration changes. To revert, just remove the commit.

## Open Questions

- Are the 2 s margin and the 60 s cap adequate with long suspensions? They will be adjusted as constants after real testing.
