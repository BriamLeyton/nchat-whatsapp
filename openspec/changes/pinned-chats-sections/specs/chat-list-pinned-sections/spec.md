## Purpose

Reliably synchronize pinned and archived chats with WhatsApp, and organize the chat list into collapsible sections: pinned, others and archived.

## ADDED Requirements

### Requirement: WhatsApp state recovery
If sending a pin or archive change, or an incoming sync, fails because the local WhatsApp state is corrupt (`mismatching LTHash`), the system SHALL do a full resync of that collection from the server. If the server snapshot also fails verification, the system SHALL request a recovery copy from the phone, once per session and collection. After a successful resync or recovery, pinned, archived and muted chats MUST be reflected in the list without restarting nchat.

#### Scenario: Alt-p with corrupt state
- **WHEN** the `regular_low` state is corrupt, the user presses `Alt-p` on a chat and the phone is online
- **THEN** nchat recovers the state and the next `Alt-p` pins the chat in nchat and on the phone

#### Scenario: Recovery from the phone
- **WHEN** the recovery copy from the phone arrives
- **THEN** the chats pinned and archived on the phone appear that way in nchat without restarting

### Requirement: List sections
The chat list SHALL group chats into three sections, in this order: `Fijados` (pinned), `Otros` (neither pinned nor archived) and `Archivados` (archived). Within each section the current order is kept. When more than one section has chats, each section with chats SHALL show a title row. Title rows MUST NOT be selectable. When only `Otros` has chats, the list MUST NOT show titles.

#### Scenario: List with pinned and archived chats
- **WHEN** there are 2 pinned chats, 10 neither pinned nor archived, and 74 archived, and no section is collapsed
- **THEN** the list shows `Fijados`, the 2 pinned chats, `Otros`, the 10, `Archivados` and the 74

#### Scenario: Navigation between sections
- **WHEN** the selected chat is the last pinned one and the user moves to the next chat
- **THEN** the selection skips the title and lands on the first chat of `Otros`

#### Scenario: Only others
- **WHEN** there are no pinned or archived chats
- **THEN** the list looks as it did before this change

### Requirement: Collapse sections
The `toggle_others` key (default `Alt-o`) SHALL collapse or expand `Otros`, and `toggle_archived` (default `Alt-h`) `Archivados`. `Fijados` does not collapse. A collapsed section:
- shows only its title, followed by the number of chats and `unread_indicator` if any has unread messages;
- has its chats skipped when moving to the next or previous chat;
- if the selected chat becomes hidden by collapsing it, the selection MUST move to the nearest visible chat, or to none if there are no visible chats.

Opening a hidden chat by another route (go to unread, go to chat, select contact) MUST expand its section. The state of each section SHALL be saved in `list_others_collapsed` and `list_archived_collapsed`, and restored at startup. `Archivados` starts collapsed by default.

#### Scenario: Collapse others
- **WHEN** `Otros` is expanded and the user presses `Alt-o`
- **THEN** the section shows only `Otros (N)` and Tab cycles through the chats of the expanded sections

#### Scenario: New message in a collapsed section
- **WHEN** `Otros` is collapsed and a message arrives in one of its chats
- **THEN** the title shows `Otros (N) *`

#### Scenario: Go to a hidden unread chat
- **WHEN** the unread chat that `unread_chat` leads to is in a collapsed `Otros`
- **THEN** that section expands and the chat is opened

#### Scenario: Archived unread chats
- **WHEN** `Archivados` is collapsed and an archived chat has unread messages
- **THEN** `unread_chat` skips it

#### Scenario: Restart
- **WHEN** the user closes nchat with `Otros` collapsed and `Archivados` expanded
- **THEN** on reopening, the sections keep that state

### Requirement: Visible archived chats
Archived chats SHALL appear in the `Archivados` section, sorted by last message. They can be opened, read and replied to. New messages in an archived chat MUST be shown in its history and mark it as unread, but MUST NOT generate a notification or unarchive it. Archiving or unarchiving with `Alt-a` or from the phone SHALL move the chat between sections live.

#### Scenario: Message in an archived chat
- **WHEN** a message arrives in an archived chat
- **THEN** the chat stays in `Archivados`, shows `*` and no notification sounds

#### Scenario: Archive with Alt-a
- **WHEN** the user presses `Alt-a` on a chat in `Otros` and `Archivados` is collapsed
- **THEN** the chat moves to `Archivados` and the selection moves to the nearest visible chat

#### Scenario: Unarchive from the phone
- **WHEN** the user unarchives a chat on the phone
- **THEN** the chat moves to `Otros` in nchat without restarting

### Requirement: Configuration
The system SHALL read:
- from `ui.conf`: `list_pinned_header` (default `Fijados`), `list_others_header` (`Otros`), `list_archived_header` (`Archivados`), `list_others_collapsed` (`0`) and `list_archived_collapsed` (`1`);
- from `key.conf`: `toggle_others` (default `Alt-o`) and `toggle_archived` (`Alt-h`);
- from `color.conf`: `list_color_header_fg/_bg`.

The keys MUST be added with their default value when they do not exist, without modifying existing ones.

#### Scenario: First run after updating
- **WHEN** the user starts this version with their existing configuration files
- **THEN** the files include the new keys with their default values and the rest stays the same
