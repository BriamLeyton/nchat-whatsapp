## Why

Today `Ctrl-g` only shows or hides the bottom help bar. That bar shows only a few commands per page (you have to rotate them with `Ctrl-o`) and uses unclear notation (`M-p`, `^S`). There is no view with all commands at once.

## What Changes

- `Ctrl-g` (`toggle_help`) opens a centered help modal with the full list of commands, grouped by category. Each command shows its shortcut and a description in Spanish.
- The modal closes with `Esc`, with `Ctrl-g` or with `Ctrl-c`. If it does not fit on screen, it scrolls with `↑`/`↓` and `PgUp`/`PgDn`.
- Shortcuts are shown as `Alt+p` and `Ctrl+s`, both in the modal and in the bottom bar. To do that, the defaults of `help_prefix_meta` and `help_prefix_ctrl` change to `Alt+` and `Ctrl+`. If the user keeps the old values (`M-`, `^`), they are migrated.
- The modal reflects the actual `key.conf` configuration. Functions with no assigned key (`KEY_NONE`) do not appear.
- The bottom bar is still controlled by `help_enabled` in `ui.conf`, but `Ctrl-g` no longer toggles it.

## Capabilities

### New Capabilities
- `help-modal`: help modal with all commands and readable key notation.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- **New UI:** `src/uihelpdialog.{h,cpp}`, based on `UiDialog`.
- **Existing UI:**
  - `src/uimodel.cpp`: `OnKeyToggleHelp` opens the modal.
  - `src/uihelpview.{h,cpp}`: `GetKeyDisplay` becomes public and reusable, and gains more keys (PgUp/PgDn, Home/End, Del, Ctrl+[, KEY_F*).
  - `src/uiconfig.cpp`: new defaults and migration of the prefixes.
- **Build:** `CMakeLists.txt` includes the new file.
