## 1. Modelo

- [x] 1.1 Agregar `ChatSectionStatus=0` al enum y renumerar (Count=4). `GetChatSection` devuelve Status para `status@broadcast`. Ajustar arrays por sección y `SetSectionCollapsed`.
- [x] 1.2 `IsChatHidden`: nunca oculta Status, y excluye Status del conteo de otras secciones.
- [x] 1.3 `OnKeyPinChat`/`OnKeyArchiveChat`: ignorar sobre `status@broadcast`.

## 2. Vista y configuración

- [x] 2.1 Opciones `list_status_name=◉ Status`, `list_color_status_fg=0x6fcfa8`, `list_color_status_bg=0x173029`, `list_attr_status=bold`.
- [x] 2.2 `UiListView::Draw`: títulos solo si hay Pinned o Archived. Fila de Status sin título, con su nombre, estilo, selección y `*`.
- [x] 2.3 README: documentar las opciones nuevas.

## 3. Verificación

- [x] 3.1 Sandbox offline:
  - [x] la fila de estados aparece primera, antes de `PINNED`;
  - [x] `Alt-o` colapsa "Others" sin ocultar los estados, y el conteo no los incluye;
  - [x] `Tab`/`Shift-Tab` pasan por la fila, y el historial muestra los estados;
  - [x] seleccionada se distingue de no seleccionada;
  - [x] `Alt-p`/`Alt-a` sobre la fila no hacen nada;
  - [x] con `status_broadcast=0` no aparece.
- [ ] 3.2 En vivo en Ghostty: un estado nuevo pone `*` sin notificación, y verlo lo quita.
