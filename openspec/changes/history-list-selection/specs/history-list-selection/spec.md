## Purpose

Select messages in the history like in a normal list: the view stays fixed and the selection moves, scrolling only when the selection leaves the view.

## ADDED Requirements

### Requirement: Fixed view while moving the selection
While message selection is active, moving the selection with `↑`/`↓` to a message that is already fully visible MUST NOT scroll the history. The selected message SHALL be highlighted at its current position.

#### Scenario: Enter selection
- **WHEN** the history shows the last 6 messages and the user presses `↑` from the empty text box
- **THEN** the last message is selected and the history does not move

#### Scenario: Move up within the view
- **WHEN** the last message is selected and the user presses `↑` 3 times
- **THEN** the fourth message from the bottom is selected and the 6 messages stay in the same position

### Requirement: Scrolling at the edges
If the selection moves to a message that is not fully visible, the system SHALL scroll the history the minimum needed to show it fully:
- **Upward:** the message ends up at the top of the view.
- **Downward:** the message ends up at the bottom of the view.

If the selected message does not fit in the view, it is shown truncated with `[...]`, as today.

#### Scenario: Pass the top edge
- **WHEN** the topmost fully visible message is selected and the user presses `↑`
- **THEN** the history scrolls and the previous message is selected and fully visible at the top

#### Scenario: Move back down
- **WHEN** the history was scrolled up and the user presses `↓` until passing the bottom edge
- **THEN** the history scrolls as needed to show the selected message at the bottom

### Requirement: Exit selection
When leaving the selection with `↓` from the most recent message, with `Esc`/cancel, or by sending, the history SHALL show the most recent messages again, as today.

#### Scenario: Exit with down arrow
- **WHEN** the most recent message is selected and the user presses `↓`
- **THEN** the selection ends and the history shows the end of the chat

### Requirement: Jumps to messages
Actions that select a specific message (go to quoted, find, go to pinned) SHALL show it fully, scrolling the view only if it is not visible. `PgUp`/`PgDn` MUST keep their current behavior.

#### Scenario: Go to a visible quoted message
- **WHEN** the quoted message is already fully visible on screen and the user uses `jump_quoted`
- **THEN** the selection moves to that message without scrolling the history

#### Scenario: Go to a quoted message out of view
- **WHEN** the quoted message is above the view
- **THEN** the history scrolls and the message is selected and fully visible
