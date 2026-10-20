## Why

Replying to a status from nchat published the text as a new status, because nchat sends the reply to the current chat, which in Status is `status@broadcast`. Since `status-expired-local-only`, sending in Status is blocked with a notice, so today it is not possible to reply to a status.

In WhatsApp, replying to a status means sending a private message to the author quoting the status. The author sees it in their chat with the status's thumbnail or text.

## What Changes

- In Status, with a status selected, typing and pressing Enter sends the reply to the author's private chat, quoting the status as WhatsApp does.
- When sending:
  - nchat moves to the author's chat, where the reply is shown with the quote, as in the app. If there is no chat with the author yet (first message), it is created, as when opening a contact;
  - if it fails, "Reply to <name> failed, please try again from Status Updates." appears and the text returns to the Status input;
  - the reply appears in the author's chat like any sent message.
- The quote includes the status text and, if it is a photo or video, its thumbnail included in the message (`<id>.thumb.jpg`). Nothing is downloaded.
- These remain blocked, with a notice:
  - sending without a selected status: it would publish a new status;
  - replying to your own status;
  - replying to an expired status (more than 24 h), as in the app;
  - sending files, editing or using the external editor in Status, as today.
- In the author's chat, the quote of a reply to a status shows the status's text or file. Today it shows empty, because the status is not in that chat. This also applies to replies others make to your statuses.

## Capabilities

### New Capabilities
- `status-reply`: reply to a status with a private message to the author that quotes the status.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/common/src/protocol.h`: `ChatMessage.quotedChatId` (chat of the quoted message, empty if it is the same chat).
- `lib/wmchat/src/wmchat.cpp` and `lib/wmchat/go/gowm.go`: `WmSendMessage` receives the quote's chat and the thumbnail path; with `status@broadcast` it builds `ContextInfo.RemoteJID` and the quote as an image or video with a thumbnail.
- `src/uimodel.cpp`: `OnKeySendMsg` in Status sends to the author instead of blocking; validations and notices.
- `src/uihistoryview.cpp`: look up the quote in Status when it is not in the chat.
- `README.md`: replying to statuses.
