## Context

- **`UiHelpView`:**
  - Builds lists of `"<key> <Desc>"` per context and pages them in the bottom bar.
  - `GetKeyDisplay` converts `KEY_CTRLx` into `help_prefix_ctrl + x` and `\33\ooo` into `help_prefix_meta + c`.
  - There are a few special cases: `⏎`, `Tab`, arrows and `⌫`.
- **`UiDialog`:**
  - Already resolves the centered frame with a title and a footer.
  - Suspends inline images while it is open.
  - `UiMessageDialog` is the example of a modal loop with `UiController::GetKey`.
- **`Esc` in ncurses:** it arrives as `27` after `ESCDELAY`, which defaults to 1000 ms because nchat defines `\33x` sequences with `define_key`.

## Goals / Non-Goals

**Goals:** see all commands at once, close quickly with `Esc`, readable notation.

**Non-Goals:** running commands from the modal, searching inside it, translating the rest of the UI.

## Decisions

### 1. `UiHelpDialog` based on `UiDialog`
- **Size:** `wReq=0.8`, `hReq=0.8`, with title ` Ayuda ` and footer `Esc cerrar  ↑↓ desplazar`.
- **Content:** a static table `{category, [{function, description}]}`. When opened, it is converted into lines with the shortcut left-aligned (width = longest shortcut + 2) and the description.
- **Columns:** if two columns of at least 36 characters fit, the categories are split into two columns to use the width.
- **Loop:** like `UiMessageDialog`. Closes with `27`, `toggle_help` or `cancel`; scrolls with `up`/`down`/`prev_page`/`next_page`; redraws with `terminal_resize`.

### 2. Quick `Esc`
While the modal is open, `set_escdelay(50)` is used, and the previous value (`get_escdelay()`) is restored on close. Ghostty sends `Alt+x` as `ESC x` in a single write, so 50 ms does not break Alt shortcuts inside the modal.

*Rejected alternative:* lowering `ESCDELAY` globally. It may affect slow remote terminals where `Alt+x` arrives split.

### 3. Notation
- **Defaults:** `help_prefix_meta=Alt+` and `help_prefix_ctrl=Ctrl+`.
- **Migration:** in `UiConfig::Init`, if the value is exactly `M-` or `^`, it is replaced with the new one.
- **`GetKeyDisplay`:** becomes `public static`. It adds `PgUp`, `PgDn`, `Home`, `End`, `Del`, `Esc` (`KEY_CTRL[`) and `F1`–`F12`. For `Ctrl` it uses the lowercase letter (`Ctrl+s`), the same as `Alt+p`.

### 4. `toggle_help`
`OnKeyToggleHelp` opens the modal instead of toggling the bar, and the bar is shown only under `help_enabled`.

### 5. Categories and functions

| Category | Functions |
|---|---|
| Chats | `next_chat`, `prev_chat`, `unread_chat`, `goto_chat`, `pin`, `archive_chat`, `delete_chat`, `toggle_others`, `toggle_archived` (these two if they exist) |
| Messages | `send_msg`, `transfer`, `select_emoji`, `select_contact`, `select_mention`, `ext_edit`, `ext_call`, `spell`, `auto_compose`, `find`, `find_next`, `jump_pinned` |
| Selected message | `up`/`down`, `edit_msg`, `delete_msg`, `react`, `forward_msg`, `jump_quoted`, `open`, `open_link`, `open_msg`, `save` |
| Text editing | `cut`, `copy`, `paste`, `begin_line`, `end_line`, `backward_word`, `forward_word`, `kill_word`, `backward_kill_word`, `delete_line_after_cursor`, `delete_line_before_cursor` |
| View | `toggle_help`, `toggle_list`, `toggle_top`, `toggle_emoji`, `decrease_list_width`, `increase_list_width`, `quit` |

## Risks / Trade-offs

- **If `key.conf` assigns `Esc` to another function, the modal ignores it and closes anyway.** This is the intended behavior for a modal.
- **The descriptions stay in Spanish and the rest of the UI in English.** The user asked for this, and it is local to this fork.
