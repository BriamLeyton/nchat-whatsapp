## Purpose

Prevent notifications for messages already read on the phone from ringing all at once when reconnecting after a suspension or a network drop. Only chats that are still unread when the offline sync ends are notified.

## ADDED Requirements

### Requirement: Retention during the offline sync
When WhatsApp announces that it is going to deliver pending events for a profile, nchat SHALL enter retention for that profile. During retention, incoming messages MUST NOT produce a bell or a desktop notification when they arrive. Messages SHALL be stored and shown in the list and the history as usual, including the unread marker.

#### Scenario: Reconnect after suspend
- **WHEN** the machine resumes from suspend, nchat reconnects and WhatsApp delivers 20 pending messages in 5 chats
- **THEN** no message rings or generates a desktop notification while they are received

#### Scenario: Reconnect without pending events
- **WHEN** nchat reconnects and WhatsApp does not announce pending events
- **THEN** there is no retention and new messages notify as usual

### Requirement: Notification when the sync ends
When the offline sync ends, after a 2-second margin, nchat SHALL check the chats that would have notified during retention. For each one, if its last incoming message is still unread, it SHALL send a single desktop notification with that message. If it was read on another device during the sync, it MUST NOT notify. The bell SHALL ring at most once per end of retention. This applies even if `notify_every_unread` is enabled.

#### Scenario: Messages already read on the phone
- **WHEN** the sync delivers 3 messages from Alice and then the read receipt for those messages arrives from the phone
- **THEN** at the end there is no bell and no notification for Alice

#### Scenario: Chat that is still unread
- **WHEN** the sync delivers 4 messages from Bob without a read receipt
- **THEN** at the end a single notification from Bob appears with his last message, and the bell rings once

#### Scenario: Several unread chats
- **WHEN** at the end the chats of Bob and Carol are still unread
- **THEN** one notification appears per chat and the bell rings only once

### Requirement: Existing notification rules
The deferred notification MUST follow the same rules as the immediate one: muted chats (except mentions or `muted_notify_unread`), archived and restricted chats do not notify. `terminal_bell_active`/`terminal_bell_inactive` and `desktop_notify_*` SHALL be evaluated at the moment of notifying. Messages sent before nchat's initial connection MUST NOT notify, as today.

#### Scenario: Muted chat
- **WHEN** a muted chat without a mention is still unread at the end of the sync
- **THEN** it does not notify

#### Scenario: nchat startup
- **WHEN** nchat starts and WhatsApp delivers old pending messages
- **THEN** they do not notify, the same as before this change

### Requirement: Maximum retention time
If the sync does not end within 60 seconds of starting, nchat SHALL end the retention and apply the same check as when it ends normally. A disconnection during retention MUST NOT leave retention active forever.

#### Scenario: Lost end of sync
- **WHEN** an offline sync starts and its end does not arrive within 60 seconds
- **THEN** nchat ends the retention and notifies the chats that are still unread

### Requirement: Immediate notification unchanged
Outside retention, new messages SHALL notify on arrival, as before this change.

#### Scenario: Message with nchat connected
- **WHEN** nchat is connected with no offline sync in progress and a message arrives
- **THEN** it notifies immediately according to the current configuration
