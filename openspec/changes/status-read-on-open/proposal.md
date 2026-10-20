## Why

When entering Status, every status drawn on screen is marked as seen, and for current ones a read receipt is sent to whoever posted them. That is nchat's normal behavior for any chat, but in statuses it amounts to saying "I saw all your statuses" just by passing through the list.

The user wants a status to stay unseen until they open it.

## What Changes

- In the statuses chat, drawing a status no longer marks it as seen.
- A status is marked as seen only when it is opened with:
  - `open`: opens the attachment in the external viewer;
  - `open_msg`: views the text in the external viewer, for text statuses.
- When marked as seen:
  - if it is current, the read receipt is sent, as today;
  - if it is expired, it is marked only locally, without a receipt, as today.
- Statuses not opened still count as unread in the Status row.
- Other chats do not change: they are marked when seen.

## Capabilities

### New Capabilities
- `status-read-on-open`: statuses marked as seen only when opened.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: `MarkRead` ignores the statuses chat except on explicit opening; `OnKeyOpenAttachment` and `OnKeyOpenMsg` mark the selected status.
- `README.md`: seen behavior for statuses.
