## 1. Notación

- [x] 1.1 Cambiar los defaults `help_prefix_meta=Alt+` y `help_prefix_ctrl=Ctrl+`, y migrar los valores `M-` y `^` en `UiConfig::Init`. Hacer `GetKeyDisplay` pública, con letra minúscula para Ctrl y teclas extra. Verificar en una copia de `ui.conf` que los valores se migran.

## 2. Modal

- [x] 2.1 Crear `src/uihelpdialog.{h,cpp}` (tabla de categorías, una o dos columnas, desplazamiento, cierre con Esc, `toggle_help` o `cancel`, redibujo al cambiar el tamaño, `set_escdelay(50)` temporal) y agregarlo a `CMakeLists.txt`.
- [x] 2.2 `OnKeyToggleHelp` abre el modal. Documentar en `README.md`.

## 3. Verificación

- [x] 3.1 Sandbox offline:
  - [x] `Ctrl-g` abre el modal con `Alt+p  Fijar/desfijar chat`;
  - [x] `Esc` cierra rápido;
  - [x] desplazamiento en una terminal baja;
  - [x] la barra inferior muestra `Alt+`/`Ctrl+`;
  - [x] una tecla reasignada se refleja en el modal.
