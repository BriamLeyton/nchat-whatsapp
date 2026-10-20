## Why

Businesses and web pages share links like `https://api.whatsapp.com/send/?phone=15551234567&text=Hello...` or `https://wa.me/15551234567`. In the app, opening the link goes straight to the chat with the text already written.

In nchat, the contacts dialog (Alt-n) only lists synced contacts. To write to a number that is not saved, it has to be saved on the phone first, and then the text has to be copied by hand.

## What Changes

- **The filter of the contacts dialog (Alt-n) accepts a number or a WhatsApp link**:
  - international number with country code: `+1 555 123 4567`, `15551234567`, `(+1) 555-123-4567`;
  - `https://wa.me/<number>[?text=...]`;
  - `https://api.whatsapp.com/send/?phone=<number>&text=...` (also `/send?`, without `https://`, and `whatsapp://send?phone=...`).
- **New row at the top of the list**: `Open chat with +15551234567`, followed by the contacts that match as today. Enter on it opens the chat.
  - If there is already a chat with that number, it is opened (and unarchived, as when choosing a contact). WhatsApp is not queried.
  - If there is no chat, nchat asks WhatsApp once whether the number is registered, as the app does when opening the link. If it is, the chat is created and opened. If not, "+15551234567 is not on WhatsApp." appears.
  - Without a connection, "Not connected, please try again later." appears and nothing is created.
- **The link's text is left written, not sent**: the `text=` is decoded (`+` is a space, `%C3%A1` is `á`) and placed in the chat's message box. The user reviews it, edits it and sends it with Enter.
  - If the chat already had a draft, it is not replaced, and "Draft kept, link text not inserted." appears.
- A number without a country code cannot be guessed: it is taken as is (for example, `5551234567` would be `+55...`). The row shows the number with `+` so the user sees it before pressing Enter.
- Out of scope: `nchat --open <link>` and registering nchat as the system handler for `wa.me` links.

## Capabilities

### New Capabilities
- `open-chat-by-link`: open or create a chat by typing or pasting a WhatsApp number or link in the contacts dialog, with the link's text ready to send.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uicontactlistdialog.{h,cpp}`: detect a number or link in the filter and add the `Open chat with +N` row.
- `src/uimodel.{h,cpp}`: open the existing chat or request its creation by number, pending text for the message box, notices.
- `lib/common/src/protocol.h`: `CreateChatRequest` with `phone`; `CreateChatNotify` with the number in case of error.
- `lib/wmchat/src/wmchat.cpp`, `lib/wmchat/go/cgowm.go`, `lib/wmchat/go/gowm.go`: `WmCreateChat` with `IsOnWhatsApp`, in a goroutine.
- `src/uihelpdialog.cpp`, `README.md`: document numbers and links in Alt-n.
