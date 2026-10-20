## Why

In WhatsApp, every chat has a header at the top with the contact's photo, name and presence. Also, a ring around the photo signals whether the contact has statuses, and tapping it opens them.

In nchat the bottom bar only shows the name and presence on one line. There is no photo, and to see a contact's statuses you have to go to Status, filter with Alt-u and go back manually.

## What Changes

- **3-row banner at the top of the history**, in contact chats and in groups:
  - inline profile photo, in terminals with image support (Ghostty, kitty). Without image support, the banner is shown as text only;
  - contact: `name · number`, presence (typing, online, last seen) and statuses, one row each;
  - group: name and number of participants;
  - no separator line: the banner is distinguished by its own color, `history_banner_color`.
- **Contact statuses** on the third row of the banner, only for contacts, counting those from the last 24 h:
  - `● 3 new status updates · Alt-u` if there are unreviewed statuses (with the `●` mark from `status-seen-indicator`);
  - `◯ 5 status updates · Alt-u` if you have already reviewed all of them;
  - empty row if the contact has no statuses from the last 24 h.
- **Presence colors** in the presence row, configurable:
  - typing: green;
  - online: green;
  - away (`away`): yellow;
  - last seen (`seen ...`): gray;
  - if the chat is muted, ` · muted` in gray at the end.
- **Ctrl-v (`open`) without a selected message** opens the chat's profile photo with `attachment_open_command`, like other photos.
  - The photo is opened at full size. It is requested from WhatsApp only when opened, and stored in `<profile>/avatars/<chatId>.full.jpg`.
  - If the photo changed (the preview id changed), the full copy is deleted and requested again the next time.
  - Without a photo, "No profile photo." is shown.
  - With a selected message, Ctrl-v still opens its attachment as it does today.
- **Alt-u in a contact's chat** switches to Status filtered by that contact. Alt-u again closes it and returns to the contact's chat. Inside the filter everything works as today: selecting clears the mark, `open` marks it as seen, and you can reply.
- **Profile photo**:
  - requested from WhatsApp when the chat is opened, at reduced size (preview), like the app does;
  - stored in `<profile>/avatars/`, and queried again at most once every 24 h per chat, sending the stored photo id so it is not downloaded if unchanged;
  - if the contact hides their photo or has none, this is remembered and not asked again for 24 h.
- `chat_banner_enabled=1` in `ui.conf` so it can be disabled.
- Status has no banner: its list row and the filter already identify it.

## Capabilities

### New Capabilities
- `chat-banner`: chat header with photo, name, number or participants, presence and the contact's statuses, and access to their statuses with Alt-u.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/common/src/protocol.h`: `GetAvatarRequest` (preview or full) and `NewAvatarNotify`.
- `lib/wmchat/go/gowm.go`, `cgowm.go` and `lib/wmchat/src/wmchat.cpp`: `WmGetAvatar` with `GetProfilePictureInfo` (preview and `ExistingID`), download to `avatars/` and notification to the UI.
- `src/uimodel.{h,cpp}`: request the photo when opening a chat, presence by type, status count per contact, Alt-u in contact chats and return, Ctrl-v without selection to open the photo.
- `src/uihistoryview.cpp`: draw the banner and the photo in the first rows of the history.
- `src/uiconfig.cpp`, `src/uicolorconfig.cpp`, `src/uihelpdialog.cpp` and `README.md`: `chat_banner_enabled`, `history_banner_color` and presence colors, Ctrl-v without selection, Alt-u in chats.
