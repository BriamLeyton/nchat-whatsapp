## Purpose

Mark a status as seen, and notify whoever posted it, only when the user opens it, not when passing through the statuses list.

## ADDED Requirements

### Requirement: Statuses unseen when displayed
Displaying a status in the statuses chat MUST NOT mark it as seen or send the read receipt. Other chats SHALL keep being marked when seen.

#### Scenario: Passing through the list
- **WHEN** the user enters Status and sees 10 new statuses without opening any
- **THEN** all 10 remain unseen, no receipt is sent and the Status row keeps the unread indicator

#### Scenario: Normal chat
- **WHEN** the user opens a contact's chat with new messages
- **THEN** the messages are marked as read as before

### Requirement: Seen on open
When an unread received status is opened with `open` or `open_msg`, the system SHALL mark it as seen. If it is current it SHALL send the read receipt. If it is expired it SHALL mark it only locally, without a receipt.

#### Scenario: Open a current image status
- **WHEN** the user selects a current image status and presses `open`
- **THEN** the image opens, the status is marked as seen and the poster receives the receipt

#### Scenario: Open a text status
- **WHEN** the user selects a text status and presses `open_msg`
- **THEN** the status is marked as seen

#### Scenario: Select without opening
- **WHEN** the user only moves the cursor over a status
- **THEN** the status remains unseen

#### Scenario: Expired status
- **WHEN** the user presses `open` on an unread expired status
- **THEN** the status is marked as seen locally, and no receipt is sent and nothing is downloaded
