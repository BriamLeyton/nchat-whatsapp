## Context

- `UiHistoryView::Draw` calls `MarkReadLocked` for every unread received message it draws.
- `UiModel::Impl::MarkRead` applies the conditions of `mark_read_*`. If the status is expired, it marks it locally (memory and cache, no request). Otherwise it sends `MarkMessageReadRequest`.
- `open` (`OnKeyOpenAttachment`) opens the attachment of the selected message. If it is not downloaded, it first downloads it and opens it when it arrives. For expired statuses the download is blocked.
- `open_msg` (`OnKeyOpenMsg`) shows the text of the selected message in the viewer.

## Decisions

### 1. Block on drawing
- `MarkRead` gains a parameter `p_Opened = false`.
- If the chat is the statuses chat and `p_Opened` is false, it returns without doing anything. Drawing never passes `true`.

### 2. Marking on open
- New function `MarkStatusOpened()`: takes the selected message of the current chat. If it is a received unread status, it calls `MarkRead(profile, chat, msgId, true, true)`.
- It is called from `OnKeyOpenAttachment` (only when the action comes from the user, not from the download notice) and from `OnKeyOpenMsg`.
- **Timing:** it is marked when the key is pressed, even if the attachment still needs to be downloaded, because the intention to view it already exists. If it is an expired status without a file, it cannot be opened; it is still marked locally, without a notice.
- The other conditions of `MarkRead` are kept: active terminal, etc.

## Risks / Trade-offs

- **Inline images:** with `inline_images_enabled` the status image is seen in the list without opening it, and it is not marked. This is what was requested: seen = opened.
- **Status row:** it keeps the unread indicator while there are unopened statuses. This already happens today with statuses off screen.
