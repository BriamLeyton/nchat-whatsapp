## Context

- `UiHistoryView` draws the history from bottom to top in `m_PaddedWin`, and collects inline images in `imagePlacements` so they can be placed after the refresh with `UiImage`.
- The bottom bar (`UiStatusView`) shows the name, the presence (`GetChatStatusLocked`) and, optionally, the number (`GetContactPhoneLocked`).
- Group members are already requested and stored in `m_GroupMembers` (`RequestGroupMembers`, `GetGroupMembers`).
- The Status filter by sender (`SetStatusFilter`, `ClearStatusFilter`) loads from the DB with `MessageCache::FetchSenderMessagesSync`.
- whatsmeow offers `GetProfilePictureInfo(jid, {Preview, ExistingID})`: it returns the URL and the photo id, `nil` if the id did not change, and known errors if the photo is hidden or does not exist.

## Decisions

### 1. Banner inside the history
- The first rows of `m_PaddedWin` are reserved for the banner, so the layout of the other views does not change. The history is drawn in the remaining height.
- Height: 3 rows in total, no separator. The photo takes the 3 rows, with the width in columns it needs, to the left of the text.
- Text rows:
  - contact: `name · number`; presence; statuses (empty if none);
  - group: name; `N participants`; empty.
- The 3 rows are painted with `history_banner_color` and `history_banner_attr`, to separate them from the messages without spending a row. Default: background like the status bar and the name in bold.
- The photo is added to `imagePlacements` like any inline image. Without a photo, or without image support, the text starts at column 0.
- Status has no banner. With `chat_banner_enabled=0` nothing is reserved.

### 2. Profile photo
- Files: `<profile>/avatars/<chatId>.jpg` (photo), `<chatId>.id` (photo id or `none`).
- When opening a chat (`OnCurrentChatChanged`), and on connecting for the open chat, if `.id` does not exist or is older than 24 h (mtime), `GetAvatarRequest{chatId, basePath}` is sent. Go reads the stored id from `.id`. Nothing is sent without a connection. It is requested at most once per chat per session, even if it fails.
- In Go, `WmGetAvatar(connId, chatId, existingId, path)`:
  - `GetProfilePictureInfo` with `Preview: true` and `ExistingID`;
  - `nil`: unchanged, only the mtime of `.id` is renewed;
  - error for a hidden or nonexistent photo: `.id = none` and the photo is deleted;
  - URL: download with HTTP (10 s timeout), atomic write of the photo and then of `.id`;
  - then `NewAvatarNotify{chatId}` to redraw. A network error writes nothing and is retried in the next session.
- `UiImage` caches by path, size and cell. So that a new photo replaces the previous one, the cache key includes the file mtime (for all inline images).

### 3. Presence with color
- `GetChatPresence(profileId, chatId)` returns the type (`typing`, `online`, `away`, `seen`, none) and the text, with the same logic `GetChatStatus` uses today. `GetChatStatus` switches to it, so the bottom bar does not change.
- Colors in `color.conf`: `history_banner_typing_color` (green), `history_banner_online_color` (green), `history_banner_away_color` (yellow), `history_banner_seen_color` (gray). ` · muted` uses `history_banner_seen_color`.

### 4. Opening the photo with Ctrl-v
- `OnKeyOpenAttachment` without a selection, in a chat with a banner, opens the full photo:
  - if `<chatId>.full.jpg` exists, it is opened with `attachment_open_command`, like any attachment;
  - if not, and the preview exists, `GetAvatarRequest{chatId, full: true}` is sent. When `NewAvatarNotify{chatId, full: true}` arrives, it is opened if the chat is still the current one;
  - if `.id` is `none`, "No profile photo." is shown.
- In Go, `full` uses `Preview: false` and writes `<chatId>.full.jpg`. When saving a preview with a different id, `<chatId>.full.jpg` is deleted.

### 5. Contact statuses
- When opening a contact's chat, and when a new status from them arrives, their statuses from the last 24 h are counted with `FetchSenderMessagesSync` (block of 50, from now). They are stored as `{total, new}` per contact, using `IsStatusNew` for the new ones.
- It is also recalculated when returning from the Status filter, to reflect what was reviewed.
- Nothing is downloaded: they are only counted.

### 6. Alt-u in a contact's chat
- `OnKeyStatusFilter` in a 1:1 chat (not a group, not Status):
  - saves `m_StatusReturnChat` = current chat;
  - switches to Status with `GotoChat` and applies `SetStatusFilter(contactId)`.
- In Status with `m_StatusReturnChat` saved, Alt-u clears the filter and returns with `GotoChat(m_StatusReturnChat)`.
- Leaving Status in another way (switching chats) forgets `m_StatusReturnChat`, the same way the filter is cleared today.
- If the contact has no saved statuses, "No status updates from <name>." is shown and the chat does not change.
- In groups, Alt-u does nothing.

## Risks / Trade-offs

- **Requests to WhatsApp:** the photo is requested with the same query the app uses when opening a chat, at most once every 24 h per chat, and with `ExistingID` nothing is downloaded if it did not change.
- **History height:** the banner takes 3 rows. It can be disabled with `chat_banner_enabled=0`.
- **Duplication with the bottom bar:** name and presence appear at the top and at the bottom. The bottom bar is not touched in this change, because it also shows the profile and the developer mode.
