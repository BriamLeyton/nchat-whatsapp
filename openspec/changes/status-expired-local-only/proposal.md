## Why

The official WhatsApp app does not let you view statuses older than 24 hours. nchat keeps them in its local cache and lets you browse them, which is useful, but showing them generates traffic that no official client produces:
- it sends read confirmations for statuses from days ago (there are 602 unread in the user's cache);
- it tries to download photos and videos of expired statuses.

In addition, the statuses chat accepts text: sending from there sends the message to `status@broadcast`, which can be published as a status of your own.

The user wants to avoid any pattern that could draw WhatsApp's attention to their account.

## What Changes

- **Expired statuses** (posted more than 24 hours ago) are local-only:
  - when viewed they are marked as read only in nchat, without sending the confirmation to WhatsApp;
  - their attachments are never downloaded: not by automatic download, not inline images, and not when opening or saving. If the file is already on disk, it is used; otherwise the `📎` line stays and, when opening or saving, a notice appears;
  - you cannot react to them.
- **Sending blocked in the statuses chat:** sending text, sending files and editing do nothing in Status and show a notice. This way a status is never published by accident.
- Statuses less than 24 hours old keep working as today: read confirmation, downloads and reactions.

Out of scope: publishing statuses and replying to a status through a private chat.

## Capabilities

### New Capabilities
- `status-expired-local-only`: expired statuses with no traffic to WhatsApp, and a read-only statuses chat.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: expired-status check, and blocks in `MarkRead`, `DownloadAttachment` (the single download point), open/save, react, send text, send file and edit.
- `README.md`: documents the behavior.
- No protocol or cache changes.
