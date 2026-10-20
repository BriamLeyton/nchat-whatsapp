## Purpose

Show which statuses are new and remove that mark when they are browsed with the cursor, without marking them as seen, and show the same mark on the Status row.

## ADDED Requirements

### Requirement: New status mark
An incoming status that has not been opened, was received after the baseline and has not been reviewed SHALL be shown with `status_new_indicator` (default `●`) before the contact's name. Opened, reviewed or own statuses MUST NOT carry the mark.

#### Scenario: New status
- **WHEN** a contact's status arrives and the user enters Status
- **THEN** the status line shows `●` before the name

#### Scenario: Opened status
- **WHEN** the user opens a status with `open` or `open_msg`
- **THEN** the status line does not show `●`

### Requirement: Review on selection
Selecting a status with the cursor SHALL remove its mark. Selecting MUST NOT mark it as seen or send a read receipt.

#### Scenario: Browse with the cursor
- **WHEN** the selection cursor passes over a new status
- **THEN** the status loses the mark
- **AND** it remains unseen until it is opened with `open` or `open_msg`

#### Scenario: Status not browsed
- **WHEN** a new status is shown on screen but not selected
- **THEN** it keeps the mark

### Requirement: Mark on the Status row
The Status row in the list SHALL show `status_new_indicator` instead of `unread_indicator`, if and only if there is at least one new status among the loaded ones.

#### Scenario: All reviewed
- **WHEN** the user selects all the new statuses
- **THEN** the mark on the Status row disappears

#### Scenario: A new one arrives
- **WHEN** a new status arrives with Status off screen
- **THEN** the Status row shows `●`, without a notification

### Requirement: Persistent reviewed statuses
Reviewed statuses SHALL be stored in `<profile>/status_seen`. Without that file, statuses existing at the first start MUST count as reviewed.

#### Scenario: Restart
- **WHEN** the user reviewed statuses, closes nchat and reopens it
- **THEN** those statuses do not show the mark

#### Scenario: First start
- **WHEN** nchat starts without `status_seen`
- **THEN** no existing status shows the mark
