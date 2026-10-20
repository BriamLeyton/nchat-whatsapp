## Context

- **Reading:** `UiHistoryView::Draw` calls `MarkReadLocked` for every unread received message it draws. `UiModel::Impl::MarkRead` marks `isRead` in memory and sends `MarkMessageReadRequest`. The cache is updated when the protocol notification comes back.
- **Downloads:** every attachment download goes through `UiModel::Impl::DownloadAttachment`, which sends `DownloadFileRequest`. The view uses it (prefetch, inline, selection) via `DownloadAttachmentLocked`, and open, save and view use it via `GetMessageAttachmentPath`. In Go, `DownloadFromFileId` is only called from `WmDownloadFile`.
- **Reactions:** `PreReact` prepares, and the emoji dialog sends `SendReactionRequest`.
- **Sending:** `OnKeySendMsg`, `OnKeyTransfer` (and its dialog) and editing do not check the chat. For `status@broadcast`, whatsmeow can publish the message as a status.

## Goals / Non-Goals

**Goals:**
- A single check `IsExpiredStatus(chatId, msg)`, applied at the exit points towards the protocol, not in each caller.

**Non-Goals:**
- Publishing statuses or replying privately.

## Decisions

### 1. Check
`IsExpiredStatus(p_ChatId, p_ChatMessage)`: `IsStatusChat(p_ChatId) && (now - timeSent > 24 h)`, with `now` from `TimeUtil::GetCurrentTimeMSec()`.

### 2. Local read marking
In `MarkRead`, if the message is an expired status: `isRead = true` is set in memory, `MessageCache::UpdateMessageIsRead(profile, chat, msgId, true)` is called, unread counts and the list are updated, and it returns without sending the request.

### 3. Downloads
- **Single point:** `DownloadAttachment` returns without sending anything if the message is an expired status. This way no future path can download it.
- **View:** in `UiHistoryView::Draw`, download is not called for expired statuses, and the downloadable indicator is not shown, so it does not look like it will be fetched.
- **Open, save and view:** `GetMessageAttachmentPath` returns the file if it is already downloaded. If it is not and the status is expired, it sets a pending notice, and the public caller shows it with `MessageDialog` without the model's mutex.

### 4. Reactions
`PreReact` returns `false` for expired statuses, and the caller shows the notice.

### 5. Sending in Status
`UiModel::OnKeySendMsg`, `OnKeyTransfer` and editing check `IsStatusChat` for the current chat before doing anything. If it is Status, they show the notice and return without touching the input box.

## Risks / Trade-offs

- **System clock ahead or behind:** the 24 h limit uses local time. A clock that is off moves the cutoff by some hours, with no other effect.
- **Unread statuses that expire without having been seen:** once they are seen, no confirmation is sent. This is what the user asked for, and the official app would not send it either.
