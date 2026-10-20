## Purpose

Show in the history the delivery status of sent messages (sent, delivered, read) with WhatsApp's symbols and colors, correctly aggregating group receipts and keeping the status across restarts.

## ADDED Requirements

### Requirement: Delivery statuses of sent messages
The system SHALL assign to each message sent by the user one of these statuses, in increasing order: `sent` (accepted by the server), `delivered` (delivered to the recipient) and `read` (read or played). A message's status MUST NOT go back: a receipt with a status lower than the current one MUST be ignored.

#### Scenario: Newly sent message
- **WHEN** the user sends a message and the server accepts it
- **THEN** the message is in the `sent` status

#### Scenario: Delivery receipt in an individual chat
- **WHEN** a delivery receipt from the recipient arrives for a message in `sent`
- **THEN** the message moves to `delivered`

#### Scenario: Read receipt in an individual chat
- **WHEN** a read or played receipt from the recipient arrives
- **THEN** the message moves to `read`

#### Scenario: Late delivery receipt
- **WHEN** a message is already in `read` and a delivery receipt arrives
- **THEN** the message stays in `read`

#### Scenario: Message sent from another device
- **WHEN** the user sends a message from their phone and nchat receives it through sync
- **THEN** the message appears in the `sent` status and advances with the receipts that arrive later

### Requirement: Aggregation in groups
In group chats, the system SHALL mark a sent message as `delivered` only when all other group participants have delivered (or read) it, and as `read` only when all other participants have read it. Receipts from the user's own other devices MUST NOT count as participants.

#### Scenario: Partially read group
- **WHEN** in a group of 5 participants (the user and 4 others) 3 participants read the message and the fourth only received it
- **THEN** the message is in `delivered`

#### Scenario: Group read by everyone
- **WHEN** the 4 other participants read the message
- **THEN** the message moves to `read`

#### Scenario: Restart with partial reads
- **WHEN** 3 of 4 participants read the message, the user restarts nchat and then the fourth reads it
- **THEN** the message moves to `read`

### Requirement: Status persistence
The system SHALL store the delivery status of each sent message in the local cache, so that when nchat restarts the last known status is shown. The cache update MUST preserve all existing messages and data.

#### Scenario: Restart
- **WHEN** a message was in `delivered` and the user restarts nchat
- **THEN** the message is shown as `delivered`

#### Scenario: Update from the previous version
- **WHEN** the user starts this version with an existing cache
- **THEN** the cache is updated without losing messages, and existing queries on `messages` and `contacts2` keep working

### Requirement: Inferred status for previous messages
For sent messages that have no stored delivery status (those from before this change), the system SHALL show `read` if the message is marked as read and `delivered` otherwise. A later receipt MUST be able to advance that status.

#### Scenario: Old read message
- **WHEN** a sent message from before the update is marked as read
- **THEN** it is shown as `read`

#### Scenario: Old unread message
- **WHEN** a sent message from before the update is not marked as read
- **THEN** it is shown as `delivered`

### Requirement: Visual representation
The history SHALL show the status indicator at the end of the header (name and time) of each sent message: `✓` for `sent`, `✓✓` for `delivered` and `✓✓` for `read`. By default, `sent` and `delivered` MUST use WhatsApp's gray (`#8696a0`) and `read` WhatsApp's blue (`#53bdeb`). In terminals that cannot redefine colors but have 256 colors, the nearest color in the xterm-256 palette MUST be used; with 8 colors, `blue` for `read` and the default color for the rest. Received messages MUST NOT show a status indicator. Messages pending send MUST NOT show an indicator.

#### Scenario: Delivered message
- **WHEN** a sent message is in `delivered`
- **THEN** its header ends with `✓✓` in gray

#### Scenario: Read message
- **WHEN** a sent message is in `read`
- **THEN** its header ends with `✓✓` in blue

#### Scenario: Received message that was read
- **WHEN** the user read a received message
- **THEN** its header does not show `✓`

#### Scenario: Selected message
- **WHEN** a sent message is selected
- **THEN** the indicator keeps its symbol and color, with the selection attribute

### Requirement: Symbol and color configuration
The system SHALL read the symbols from `ui.conf` (`receipt_sent_indicator`, `receipt_delivered_indicator`, `receipt_read_indicator`) and the colors from `color.conf` (`history_receipt_sent_color_fg/_bg`, `history_receipt_delivered_color_fg/_bg`, `history_receipt_read_color_fg/_bg`). The keys MUST be added with their default value when they do not exist, without modifying existing keys.

#### Scenario: First run after updating
- **WHEN** the user starts this version with existing `ui.conf` and `color.conf`
- **THEN** both files include the new keys with their default values and the rest stays the same

#### Scenario: Custom color
- **WHEN** `color.conf` contains `history_receipt_read_color_fg=green`
- **THEN** read messages show `✓✓` in green
