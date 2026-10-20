## Context

- **`UiHelpView`:**
  - Arma listas de `"<tecla> <Desc>"` por contexto y las pagina en la barra inferior.
  - `GetKeyDisplay` convierte `KEY_CTRLx` en `help_prefix_ctrl + x` y `\33\ooo` en `help_prefix_meta + c`.
  - Hay unos pocos especiales: `⏎`, `Tab`, flechas y `⌫`.
- **`UiDialog`:**
  - Ya resuelve el marco centrado con título y pie.
  - Suspende las imágenes inline mientras está abierto.
  - `UiMessageDialog` es el ejemplo de bucle modal con `UiController::GetKey`.
- **`Esc` en ncurses:** llega como `27` después de `ESCDELAY`, que por defecto es 1000 ms porque nchat define secuencias `\33x` con `define_key`.

## Goals / Non-Goals

**Goals:** ver todos los comandos de una vez, cerrar rápido con `Esc`, notación legible.

**Non-Goals:** ejecutar comandos desde el modal, buscar en él, traducir el resto de la UI.

## Decisions

### 1. `UiHelpDialog` basado en `UiDialog`
- **Tamaño:** `wReq=0.8`, `hReq=0.8`, con título ` Ayuda ` y pie `Esc cerrar  ↑↓ desplazar`.
- **Contenido:** una tabla estática `{categoría, [{función, descripción}]}`. Al abrirse, se convierte en líneas con el atajo alineado a la izquierda (ancho = máximo atajo + 2) y la descripción.
- **Columnas:** si caben dos columnas de al menos 36 caracteres, las categorías se reparten en dos columnas para aprovechar el ancho.
- **Bucle:** como `UiMessageDialog`. Cierra con `27`, `toggle_help` o `cancel`; desplaza con `up`/`down`/`prev_page`/`next_page`; redibuja con `terminal_resize`.

### 2. `Esc` rápido
Mientras el modal está abierto se usa `set_escdelay(50)`, y se restaura el valor previo (`get_escdelay()`) al cerrarlo. Ghostty envía `Alt+x` como `ESC x` en un solo write, así que 50 ms no rompe los atajos Alt dentro del modal.

*Alternativa descartada:* bajar `ESCDELAY` globalmente. Puede afectar a terminales remotas lentas donde `Alt+x` llega partido.

### 3. Notación
- **Defaults:** `help_prefix_meta=Alt+` y `help_prefix_ctrl=Ctrl+`.
- **Migración:** en `UiConfig::Init`, si el valor es exactamente `M-` o `^`, se reemplaza por el nuevo.
- **`GetKeyDisplay`:** pasa a ser `public static`. Agrega `PgUp`, `PgDn`, `Home`, `End`, `Del`, `Esc` (`KEY_CTRL[`) y `F1`–`F12`. Para `Ctrl` usa la letra en minúscula (`Ctrl+s`), igual que en `Alt+p`.

### 4. `toggle_help`
`OnKeyToggleHelp` abre el modal en vez de alternar la barra, y la barra queda solo bajo `help_enabled`.

### 5. Categorías y funciones

| Categoría | Funciones |
|---|---|
| Chats | `next_chat`, `prev_chat`, `unread_chat`, `goto_chat`, `pin`, `archive_chat`, `delete_chat`, `toggle_others`, `toggle_archived` (estas dos si existen) |
| Mensajes | `send_msg`, `transfer`, `select_emoji`, `select_contact`, `select_mention`, `ext_edit`, `ext_call`, `spell`, `auto_compose`, `find`, `find_next`, `jump_pinned` |
| Mensaje seleccionado | `up`/`down`, `edit_msg`, `delete_msg`, `react`, `forward_msg`, `jump_quoted`, `open`, `open_link`, `open_msg`, `save` |
| Edición de texto | `cut`, `copy`, `paste`, `begin_line`, `end_line`, `backward_word`, `forward_word`, `kill_word`, `backward_kill_word`, `delete_line_after_cursor`, `delete_line_before_cursor` |
| Vista | `toggle_help`, `toggle_list`, `toggle_top`, `toggle_emoji`, `decrease_list_width`, `increase_list_width`, `quit` |

## Risks / Trade-offs

- **Si `key.conf` asigna `Esc` a otra función, el modal la ignora y cierra igual.** Es el comportamiento buscado para un modal.
- **Las descripciones quedan en español y el resto de la UI en inglés.** Lo pidió el usuario y es local a este fork.
