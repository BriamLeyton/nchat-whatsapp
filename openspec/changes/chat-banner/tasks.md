## 1. Profile photo

- [x] 1.1 `GetAvatarRequest` and `NewAvatarNotify` in `protocol.h`; handling in `wmchat.cpp` and `CWmGetAvatar` in `cgowm.go`.
- [x] 1.2 `WmGetAvatar`: `GetProfilePictureInfo` (preview, `ExistingID`), atomic download, `.id` with `none` for hidden or nonexistent, notification to the UI.
- [x] 1.3 `uimodel`: request the photo when opening a chat (24 h by the mtime of `.id`, once per session) and redraw with `NewAvatarNotify`.
- [x] 1.4 Full photo: `full` in the request, `<chatId>.full.jpg` (deleted if the preview changes), Ctrl-v without selection opens it or requests it and opens it on arrival; "No profile photo." if there is none.

## 2. Banner

- [x] 2.1 `UiHistoryView`: reserve 3 rows with `history_banner_color`, draw contact or group text, and place the photo with `imagePlacements` (cache key with mtime).
- [x] 2.2 Status count per contact (24 h, `{total, new}`) when opening the chat, when a status arrives and when returning from the filter; statuses line in the banner.
- [x] 2.3 `GetChatPresence` (type and text), used by `GetChatStatus`; colored presence and ` · muted` in the banner.
- [x] 2.4 `chat_banner_enabled=1` in `uiconfig.cpp`, `history_banner_color`/`history_banner_attr` and presence colors in `uicolorconfig.cpp`, and README.

## 3. Alt-u from the chat

- [x] 3.1 `OnKeyStatusFilter` in a contact chat: save return, go to Status and filter; Alt-u in the filter returns; switching chats forgets the return; notice if there are no statuses.
- [x] 3.2 Help (`uihelpdialog.cpp`) and README: Alt-u in contact chats and Ctrl-v without selection.

## 4. Verification

- [x] 4.1 Sandbox without network: contact and group banner with text, status count with known values, Alt-u round trip, notice without statuses; without network the photo is not requested; Ctrl-v without photo shows "No profile photo.". Presence colors and photos move to 4.2, because they require network.
- [x] 4.2 Live in Ghostty: presence colors (typing, online, seen); contact and group photos; Ctrl-v opens the full photo; contact with a hidden photo; reopening the chat does not query again (log); new and reviewed statuses in the banner.
