## Purpose

Show a chat header at the top of the history with photo, details and the contact's statuses, as in WhatsApp, and open their statuses with Alt-u from the chat.

## ADDED Requirements

### Requirement: Chat banner
With `chat_banner_enabled=1`, the history of a contact or group chat SHALL show in its first 3 rows a banner with the profile photo (if image support and a photo are available) and the text to its right, with its own color and no separator line. For a contact it SHALL show `name · number`, presence and statuses, one row each; for a group, the name and the number of participants. The banner MUST NOT take more than 3 rows. The Status chat MUST NOT show a banner.

#### Scenario: Contact chat
- **WHEN** the user opens a contact's chat in Ghostty
- **THEN** the first 3 rows of the history show their photo, `name · number`, their presence and their statuses

#### Scenario: Group
- **WHEN** the user opens a group
- **THEN** the banner shows the group photo, its name and `N participants`

#### Scenario: Without images
- **WHEN** the terminal does not support inline images
- **THEN** the banner shows only the text

### Requirement: Presence colors
The presence in the banner SHALL use a configurable color per type: typing and online in green, away in yellow, last seen in gray. A muted chat SHALL add ` · muted` in gray.

#### Scenario: Contact typing
- **WHEN** the contact starts typing
- **THEN** the presence row shows `typing` in green

#### Scenario: Last seen
- **WHEN** the contact goes offline
- **THEN** the row shows `seen <time>` in gray

### Requirement: Open the profile photo
In a chat with a banner and no selected message, `open` (Ctrl-v) SHALL open the profile photo at full size with `attachment_open_command`, requesting it from WhatsApp only the first time and caching it. Without a photo, "No profile photo." SHALL be shown. With a selected message, `open` MUST keep opening its attachment.

#### Scenario: Open the photo
- **WHEN** the user presses Ctrl-v in a contact's chat without a selected message
- **THEN** the full profile photo opens in the external viewer

#### Scenario: Photo changed
- **WHEN** the contact changes their photo and the banner receives the new preview
- **THEN** the next opening requests and shows the new photo

### Requirement: Statuses in the banner
In a contact's chat, the banner SHALL show their statuses from the last 24 h: `● N new status updates · Alt-u` if there are unreviewed ones, or `◯ N status updates · Alt-u` if all are reviewed. Without statuses from the last 24 h the row SHALL be empty. Counting MUST NOT download attachments.

#### Scenario: Unreviewed statuses
- **WHEN** the contact posted 3 statuses today and none has been reviewed
- **THEN** the banner shows `● 3 new status updates · Alt-u`

#### Scenario: Reviewed statuses
- **WHEN** the user reviews all of the contact's statuses and returns to their chat
- **THEN** the banner shows `◯ 3 status updates · Alt-u`

### Requirement: Alt-u from the chat
In a contact's chat, Alt-u SHALL switch to Status filtered by that contact, and Alt-u in that filter SHALL return to the contact's chat. If the contact has no saved statuses, "No status updates from <name>." SHALL be shown without changing chat.

#### Scenario: View and close
- **WHEN** the user presses Alt-u in a contact's chat and then Alt-u again
- **THEN** first they see only that contact's statuses, and then they return to their chat

### Requirement: Cached profile photo
The photo SHALL be requested from WhatsApp at reduced size when the chat is opened, stored in `<profile>/avatars/`, and queried again at most once every 24 h per chat, sending the stored id so it is not downloaded if unchanged. A hidden or nonexistent photo SHALL be remembered for 24 h.

#### Scenario: Reopen the chat
- **WHEN** the user opens the same chat twice within 24 h
- **THEN** only the first time is WhatsApp queried

#### Scenario: Hidden photo
- **WHEN** the contact hides their photo
- **THEN** the banner is shown without a photo and is not queried again within 24 h
