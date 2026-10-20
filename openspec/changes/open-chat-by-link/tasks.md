## 1. Detection

- [x] 1.1 `ParseWhatsAppLink`: `wa.me`, `api.whatsapp.com/send`, `whatsapp://send` links and 8 to 15 digit numbers; decode `text` (`+`, `%XX`).
- [x] 1.2 `UiContactListDialog`: row `Open chat with +<number>` at the top when the filter is a number or link; `UiContactListItem` with `phone` and `text`.

## 2. Open or create

- [x] 2.1 `protocol.h`: `CreateChatRequest.phone`; `CreateChatNotify.phone` and `result`.
- [x] 2.2 `wmchat.cpp`, `cgowm.go`, `gowm.go`: `WmCreateChat` with `IsOnWhatsApp` in a goroutine (10 s timeout) and `WmCreateChatNotify`.
- [x] 2.3 `uimodel`: `OpenChatByPhone` (existing chat without network, notice if no connection, otherwise `CreateChatRequest`); handling of `CreateChatNotify` with an already existing chat and error notices.
- [x] 2.4 Pending text: apply it to the box if empty, or the notice "Draft kept, link text not inserted.".

- [x] 2.5 `WmCreateChat`: create the chat with the number (`PhoneNumber`), not with the LID returned by `IsOnWhatsApp`. Compiles; verify live with 4.3.

## 3. Documentation

- [x] 3.1 Help (`uihelpdialog.cpp`) and README: numbers and links in Alt-n.

## 4. Verification

- [x] 4.1 Offline sandbox: parsing of the link and number formats; row in the dialog; existing chat opens with the text; existing draft is kept; new number without connection shows the notice.
- [x] 4.2 Live in Ghostty: an ACME link creates the chat with the text ready; an unregistered number shows the notice.
- [x] 4.3 Live: new chat by number and first message; the log shows `chat <number>@s.whatsapp.net`, the message appears in the open chat, and no second chat appears.
