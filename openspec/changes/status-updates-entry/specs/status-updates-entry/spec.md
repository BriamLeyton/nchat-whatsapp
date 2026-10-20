## Purpose

Show WhatsApp statuses as a fixed, eye-catching, selectable entry at the top of the chat list, separate from conversations.

## ADDED Requirements

### Requirement: Fixed statuses entry
When the statuses chat (`status@broadcast`) exists and `status_broadcast` is not `0`, the list SHALL show it as the first row, before any section title and any chat. The row MUST NOT belong to any section: it is not counted in titles, it is not hidden when sections collapse, and it does not change when titles are shown.

#### Scenario: List with pinned chats
- **WHEN** there are pinned chats and the statuses chat exists
- **THEN** the first row is the statuses row and below it the `PINNED` title appears

#### Scenario: Others collapsed
- **WHEN** the "Others" section is collapsed
- **THEN** the statuses row remains visible and "Others" does not count the statuses

#### Scenario: No pinned or archived chats
- **WHEN** there are only chats in "Others" and the statuses chat exists
- **THEN** no section titles are shown and the statuses row is first

#### Scenario: Statuses hidden
- **WHEN** `status_broadcast=0`
- **THEN** the statuses row is not shown

### Requirement: Own style
The statuses row SHALL be drawn with the name from `list_status_name` and with the color and attribute from `list_color_status` and `list_attr_status`, different from section titles. By default it MUST use `#6fcfa8` text on a `#173029` background in bold. When selected it MUST also use the list's selection attribute.

#### Scenario: Row not selected
- **WHEN** the statuses row is not selected
- **THEN** it is shown with a dark green background and green bold text, spanning the full width of the list

#### Scenario: Row selected
- **WHEN** the statuses row is selected
- **THEN** it is shown with the selection attribute and is distinguishable from when it is not selected

#### Scenario: Custom color
- **WHEN** `color.conf` contains `list_color_status_bg=yellow`
- **THEN** the statuses row is shown with a yellow background

### Requirement: Selection
The statuses row SHALL be selectable like any chat: with next and previous chat, with go to chat, and at startup if it was the current chat. When selected, the history MUST show the statuses.

#### Scenario: Previous chat from the first chat
- **WHEN** the first chat below the statuses row is selected and the user goes to the previous chat
- **THEN** the statuses row becomes selected

#### Scenario: Statuses history
- **WHEN** the statuses row is selected
- **THEN** the history shows the statuses with the name of whoever posted each one

### Requirement: Unseen statuses
The statuses row SHALL show the unread indicator when there are unseen statuses, like a chat. Viewing the statuses MUST remove the indicator.

#### Scenario: New status
- **WHEN** a contact posts a status
- **THEN** the statuses row shows `*` and, with `status_broadcast=1`, there is no notification

### Requirement: Not pinnable or archivable
Pinning or archiving with the statuses row selected SHALL NOT have any effect, and MUST NOT send any change to WhatsApp.

#### Scenario: Alt-p on statuses
- **WHEN** the statuses row is selected and the user presses `Alt-p`
- **THEN** nothing changes and nothing is sent to WhatsApp
