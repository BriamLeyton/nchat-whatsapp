## Context

- Replying in nchat: with a selected message, `SendMessage` calls `AddQuoteFromSelectedMessage`, which fills `quotedId`, `quotedText` and `quotedSender`, and sends to the current chat.
- `WmSendMessage` builds `ContextInfo{StanzaID, Participant, QuotedMessage{Conversation: quotedText}}` and does not set `RemoteJID`, because a quote always belongs to the same chat.
- WhatsApp replies to a status like this: the message goes to the author's JID with `ContextInfo{StanzaID: status id, Participant: author, RemoteJID: "status@broadcast", QuotedMessage: status content}`.
- `GetUserId` already stores a status author with their number when WhatsApp gives the equivalence with their `@lid`. Otherwise the `@lid` remains, and whatsmeow can also send to that JID.
- Today, sends in Status are blocked in `OnKeySendMsg`, in file transfer, in the external editor and in editing.

## Decisions

### 1. Destination and validations
In `OnKeySendMsg`, if the current chat is Status:
- no selected status, or editing active: today's notice is kept, and the text stays in the input;
- own status: notice "Replying to own status updates is not supported.";
- expired status (`IsExpiredStatus`): notice "Replying to expired status updates is not supported.";
- otherwise the reply is built:
  - `chatId` = the status's `senderId`;
  - quote of the selected status, with `quotedChatId = "status@broadcast"`;
  - `quotedFilePath`: the status thumbnail (`meta->thumbPath`), if it exists and is a photo or video;
  - then the input is cleared and the selection is left;
  - the author's chat is opened with `GotoChat` if it already exists, without unarchiving it; if it does not exist (first message), with `OpenCreateChat`, which creates it as when opening a contact;
  - with the send result (`SendMessageNotify`): if still in Status, "Reply sent to <name>."; if it failed, "Reply to <name> failed, please try again from Status Updates." and the text returns to the Status input if it is empty.

Moving to the author's chat keeps the success notice from interrupting browsing in Status, and shows the sent reply, as in the app.

### 2. Protocol
- `ChatMessage` gains `quotedChatId` and `quotedFilePath`. They only travel in `SendMessageRequest`; they are not stored in the cache.
- `CWmSendMessage` and `WmSendMessage` receive the two new values.
- In Go, if `quotedChatId == "status@broadcast"`:
  - `ContextInfo.RemoteJID = "status@broadcast"`;
  - the quote is `ImageMessage{JPEGThumbnail, Caption}` or `VideoMessage{JPEGThumbnail, Caption}` depending on the type of the quoted file, with the bytes of `quotedFilePath`. If there is no thumbnail, `Conversation` with the text, as today.
- The type is decided by the extension of the quoted file, which nchat already knows from the status's `fileInfo`. It is passed as an `image:` or `video:` prefix in `quotedFilePath`, to avoid adding a third parameter.

### 3. Quote in the author's chat
- In `UiHistoryView`, if `quotedId` is not among the messages of the chat, it is searched in the loaded Status messages before asking the cache.
- This serves replies sent and replies others make to your statuses.

## Risks / Trade-offs

- **Quote format:** if the author's phone does not show the quote as expected, the message still arrives as normal text. It is verified live with a real status from a contact.
- **Status not loaded:** if the quoted status is not loaded in Status, the quote shows empty, as today.
