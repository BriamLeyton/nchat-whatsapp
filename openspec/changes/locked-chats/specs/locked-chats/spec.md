## Purpose

Hide WhatsApp's locked chats (Chat Lock) in nchat, without notifications, and show them only with the secret code, as in WhatsApp Web.

## ADDED Requirements

### Requirement: Locked chat synchronization
nchat SHALL process the `lock` and `setting_chatLock` actions of the app state and persist the locked chats and the secret code hash in the profile. Once per profile it SHALL download the full state of `regular_low` to learn the current state. It MUST NOT repeat that download after completing it.

#### Scenario: First connection with the feature
- **WHEN** nchat connects and the profile does not have the download mark
- **THEN** it downloads the full state of `regular_low`, stores the locked chats and creates the mark

#### Scenario: Change on the phone
- **WHEN** the user locks or unlocks a chat on the phone
- **THEN** nchat hides or shows it without restarting

### Requirement: Locked chats hidden
While locked chats are hidden, nchat MUST NOT show them in the list, in Tab navigation, in the jump to unread, in `goto_chat` or in Alt-n. Their messages MUST NOT generate a desktop notification, a bell or an unread count, whether hidden or visible. A number or link of a hidden locked chat SHALL show "This chat is locked.".

#### Scenario: Message in a locked chat
- **WHEN** a message arrives in a locked chat
- **THEN** it is stored, but does not appear in the list and does not notify

#### Scenario: Link to a locked chat
- **WHEN** the user pastes in Alt-n the link of a hidden locked chat
- **THEN** "This chat is locked." appears and the chat is not opened

### Requirement: Show with the secret code
Alt-l (`toggle_locked`) with locked chats hidden SHALL ask for the secret code with masked input and compare it locally with WhatsApp's hash. The code MUST NOT be sent, stored or logged. With the correct code it SHALL show the `▾ LOCKED` section at the end of the list until the user presses Alt-l again or closes nchat. With an incorrect code it SHALL show "Wrong secret code.". With no code configured it SHALL show "Set a secret code for locked chats in WhatsApp on your phone.".

#### Scenario: Correct code
- **WHEN** the user presses Alt-l and types their WhatsApp secret code
- **THEN** `▾ LOCKED` appears with the locked chats

#### Scenario: Hide again
- **WHEN** the section is visible and the user presses Alt-l
- **THEN** the section disappears and, if the open chat was locked, the first chat in the list is opened

#### Scenario: Restart
- **WHEN** the user closes and reopens nchat
- **THEN** the locked chats are hidden
