## Purpose

Allow seeing only one contact's statuses inside the statuses chat, choosing the contact from a selected status or from a dialog.

## ADDED Requirements

### Requirement: Filter by the author of the selected status
In the statuses chat, with a status selected and no filter active, the `status_filter` command SHALL filter the history to show only the statuses of the author of the selected status. The selected status MUST remain selected after filtering.

#### Scenario: Filter from the selection
- **WHEN** the user selects a status from Jane Doe and presses `Alt-u`
- **THEN** the history shows only Jane Doe's statuses and the same status stays selected

### Requirement: Contacts dialog with statuses
In the statuses chat, with no selection and no filter active, the `status_filter` command SHALL open a dialog with the contacts that have statuses, sorted by the date of their most recent status. The dialog MUST allow searching by typing, like the other list dialogs. Choosing a contact MUST apply the filter, and cancelling MUST leave everything unchanged.

#### Scenario: Choose a contact
- **WHEN** the user opens the dialog, types "jane" and chooses Jane Doe
- **THEN** the history shows only Jane Doe's statuses, from the most recent

#### Scenario: Cancel
- **WHEN** the user opens the dialog and presses `Esc`
- **THEN** no filter is applied

#### Scenario: Contacts with old statuses
- **WHEN** a contact only has statuses that have not yet been loaded from the history
- **THEN** they still appear in the dialog

### Requirement: Remove the filter
With a filter active, the `status_filter` command SHALL remove the filter and show all statuses. Leaving the statuses chat MUST remove the filter.

#### Scenario: Same command
- **WHEN** a filter is active and the user presses `Alt-u`
- **THEN** the history shows all statuses again, from the most recent

#### Scenario: Change chat
- **WHEN** a filter is active, the user goes to another chat and returns to statuses
- **THEN** all statuses are shown

### Requirement: Filter indicator
While the filter is active, the statuses row in the list SHALL show the filtered contact's name after the row name.

#### Scenario: Row with filter
- **WHEN** the filter for Jane Doe is active
- **THEN** the row shows `◉ Status · Jane Doe`

### Requirement: Navigation and actions on the filtered list
With the filter active, message selection, `PgUp`/`PgDn` and the actions on the selected message SHALL operate on the filtered statuses. If the filtered list does not fill the history and there are more statuses in the cache, the system MUST load more statuses until it is filled or none are left.

#### Scenario: Move up through the filtered list
- **WHEN** the filter for Jane Doe is active and the user moves up with `↑`
- **THEN** the selection only goes through Jane Doe's statuses

#### Scenario: Open a filtered status
- **WHEN** the user opens the attachment of the selected status with the filter active
- **THEN** that status's attachment opens, not another one's

#### Scenario: Few statuses loaded
- **WHEN** the filtered contact has old statuses that are not yet in memory
- **THEN** they are loaded from the cache and appear in the history

#### Scenario: New status from another contact
- **WHEN** the filter for Jane Doe is active and a status from another contact arrives
- **THEN** it does not appear in the filtered history

### Requirement: Only in the statuses chat
Outside the statuses chat, the `status_filter` command SHALL NOT have any effect.

#### Scenario: Alt-u in a normal chat
- **WHEN** the user presses `Alt-u` in a chat that is not the statuses chat
- **THEN** nothing happens
