## Purpose

Reply to a status by sending a private message to its author that quotes the status, as in WhatsApp, without publishing anything in Status.

## ADDED Requirements

### Requirement: Private reply to the author
In Status, with a current status from another person selected, sending text SHALL deliver it to the author's private chat, quoting the status with `RemoteJID` `status@broadcast`. Sending in Status MUST NOT publish a status.

#### Scenario: Reply to a status
- **WHEN** the user selects a current status from a contact, types text and presses Enter
- **THEN** the text reaches the contact's private chat, quoting the status
- **AND** nchat moves to the contact's chat, creating it if this is the first message with them

#### Scenario: No status selected
- **WHEN** the user types in Status without a selected status and presses Enter
- **THEN** nothing is sent, the text stays in the input, and the notice that it cannot be sent to Status appears

### Requirement: Disallowed replies
Replying to your own status or to an expired one MUST NOT send anything and SHALL show a notice. The text SHALL stay in the input.

#### Scenario: Expired status
- **WHEN** the user tries to reply to a status older than 24 h
- **THEN** "Replying to expired status updates is not supported." appears and nothing is sent

#### Scenario: Own status
- **WHEN** the user tries to reply to their own status
- **THEN** "Replying to own status updates is not supported." appears and nothing is sent

### Requirement: Quote with thumbnail
If the status is a photo or video with an embedded thumbnail, the quote SHALL carry that thumbnail and its text. Replying MUST NOT download the status's attachment.

#### Scenario: Reply to a video
- **WHEN** the user replies to a video status that is not downloaded
- **THEN** the quote carries the embedded thumbnail and the video is not downloaded

### Requirement: Quote visible in the author's chat
When the quoted message is not in the chat, the quote SHALL be looked up among the loaded statuses and show their text or file.

#### Scenario: Reply to an own status received
- **WHEN** a contact replies to your own status
- **THEN** in their chat the quote shows the status's text or file
