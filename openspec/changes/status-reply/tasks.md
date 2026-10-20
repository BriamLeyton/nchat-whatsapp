## 1. Implementation

- [x] 1.1 `ChatMessage.quotedChatId` and `quotedFilePath` in `protocol.h`, passed by `wmchat.cpp` to `CWmSendMessage`.
- [x] 1.2 `WmSendMessage`: with `status@broadcast`, `ContextInfo.RemoteJID` and the quote as an image or video with `JPEGThumbnail`; otherwise as today.
- [x] 1.3 `OnKeySendMsg` in Status: validations (no selection, own, expired, editing), sending to the author with the quote, moving to the author's chat and notices according to the result.
- [x] 1.4 `UiHistoryView`: look up the quote in Status if it is not in the chat.
- [x] 1.5 README: replying to statuses.

## 2. Verification

- [x] 2.1 Offline sandbox without network: without selection, own and expired show their notice and do not send; with a current status the log shows the send to the author with `RemoteJID status@broadcast` and the thumbnail.
- [ ] 2.2 Live: reply to a real status from a contact; on the contact's phone (or yours, in their chat) the reply appears quoting the status, and in nchat the quote shows the status's text or file.
