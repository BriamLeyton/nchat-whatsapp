## 1. Notation

- [x] 1.1 Change the defaults `help_prefix_meta=Alt+` and `help_prefix_ctrl=Ctrl+`, and migrate the values `M-` and `^` in `UiConfig::Init`. Make `GetKeyDisplay` public, with a lowercase letter for Ctrl and extra keys. Verify in a copy of `ui.conf` that the values are migrated.

## 2. Modal

- [x] 2.1 Create `src/uihelpdialog.{h,cpp}` (category table, one or two columns, scrolling, close with Esc, `toggle_help` or `cancel`, redraw on resize, temporary `set_escdelay(50)`) and add it to `CMakeLists.txt`.
- [x] 2.2 `OnKeyToggleHelp` opens the modal. Document in `README.md`.

## 3. Verification

- [x] 3.1 Offline sandbox:
  - [x] `Ctrl-g` opens the modal with `Alt+p  Fijar/desfijar chat`;
  - [x] `Esc` closes quickly;
  - [x] scrolling in a short terminal;
  - [x] the bottom bar shows `Alt+`/`Ctrl+`;
  - [x] a reassigned key is reflected in the modal.
