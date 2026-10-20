## Why

Hoy `Ctrl-g` solo muestra u oculta la barra de ayuda inferior. Esa barra enseña unos pocos comandos por página (hay que rotarlos con `Ctrl-o`) y usa una notación poco clara (`M-p`, `^S`). No hay una vista con todos los comandos a la vez.

## What Changes

- `Ctrl-g` (`toggle_help`) abre un modal de ayuda centrado con la lista completa de comandos, agrupados por categoría. Cada comando muestra su atajo y una descripción en español.
- El modal se cierra con `Esc`, con `Ctrl-g` o con `Ctrl-c`. Si no cabe en pantalla, se desplaza con `↑`/`↓` y `PgUp`/`PgDn`.
- Los atajos se muestran como `Alt+p` y `Ctrl+s`, tanto en el modal como en la barra inferior. Para eso cambian los defaults de `help_prefix_meta` y `help_prefix_ctrl` a `Alt+` y `Ctrl+`. Si el usuario conserva los valores viejos (`M-`, `^`), se migran.
- El modal refleja la configuración real de `key.conf`. Las funciones sin tecla asignada (`KEY_NONE`) no aparecen.
- La barra inferior sigue controlada por `help_enabled` en `ui.conf`, pero `Ctrl-g` ya no la alterna.

## Capabilities

### New Capabilities
- `help-modal`: modal de ayuda con todos los comandos y notación de teclas legible.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- **UI nueva:** `src/uihelpdialog.{h,cpp}`, basada en `UiDialog`.
- **UI existente:**
  - `src/uimodel.cpp`: `OnKeyToggleHelp` abre el modal.
  - `src/uihelpview.{h,cpp}`: `GetKeyDisplay` pasa a ser pública y reutilizable, y suma más teclas (PgUp/PgDn, Home/End, Del, Ctrl+[, KEY_F*).
  - `src/uiconfig.cpp`: defaults nuevos y migración de los prefijos.
- **Build:** `CMakeLists.txt` incluye el archivo nuevo.
