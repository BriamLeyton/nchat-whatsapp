## Purpose

Show all nchat commands in a readable modal, with shortcuts in the `Alt+p` / `Ctrl+s` notation.

## ADDED Requirements

### Requirement: Help modal
The `toggle_help` key (default `Ctrl-g`) SHALL open a centered modal with the list of commands grouped by category:
- Chats;
- Messages;
- Selected message;
- Text editing;
- View.

Each entry MUST show the configured shortcut and a description in Spanish. Functions with no assigned key MUST NOT appear.

#### Scenario: Open help
- **WHEN** the user presses `Ctrl-g` in the main view
- **THEN** a modal appears with the commands, including `Alt+p  Fijar/desfijar chat`

#### Scenario: Reassigned key
- **WHEN** `key.conf` has `pin=\33\153`
- **THEN** the modal shows `Alt+k  Fijar/desfijar chat`

### Requirement: Close and scroll
The modal SHALL close with `Esc`, `toggle_help` or `cancel` without running any other action. If the list does not fit in the modal, `↑`/`↓` MUST scroll it one line and `PgUp`/`PgDn` one page. When the terminal size changes, the modal MUST redraw with the new size.

#### Scenario: Close with Esc
- **WHEN** the modal is open and the user presses `Esc`
- **THEN** the modal closes in less than 100 ms and the view returns as it was

#### Scroll long list
- **WHEN** the terminal is shorter than the command list
- **THEN** the modal shows an indicator that there is more content, and `↓` scrolls it

### Requirement: Key notation
The modal and the bottom help bar SHALL show keys as `Alt+<key>` and `Ctrl+<key>` (for example `Alt+p`, `Ctrl+s`). The defaults of `help_prefix_meta` and `help_prefix_ctrl` MUST be `Alt+` and `Ctrl+`. The previous values `M-` and `^` MUST be migrated to the new ones. Any other custom value MUST be respected.

#### Scenario: Existing configuration
- **WHEN** `ui.conf` has `help_prefix_meta=M-` and `help_prefix_ctrl=^`
- **THEN** after startup, `ui.conf` has `help_prefix_meta=Alt+` and `help_prefix_ctrl=Ctrl+`, and help shows `Alt+p`

### Requirement: Bottom bar
The bottom help bar SHALL keep being shown or hidden according to `help_enabled` in `ui.conf`. `toggle_help` MUST NOT toggle it.

#### Scenario: Bar enabled
- **WHEN** `help_enabled=1` and the user opens and closes the modal
- **THEN** the bottom bar stays visible
