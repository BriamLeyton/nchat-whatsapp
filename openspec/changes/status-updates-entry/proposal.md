## Why

Los estados de WhatsApp ya llegan a nchat como el chat `status@broadcast` ("Status Updates"), pero se ordenan como un chat más, silenciado, mezclado en "Others". No se distinguen de las conversaciones, y si "Others" está colapsado, quedan ocultos. En WhatsApp los estados viven aparte, arriba y fuera de la lista de chats.

## What Changes

- Los estados se muestran como una fila fija al inicio de la lista, antes de la sección "Pinned" y fuera de todas las secciones.
- La fila tiene un estilo propio (por defecto, texto verde `#6fcfa8` sobre un verde oscuro discreto `#173029`, en negrita), distinto de los títulos de sección.
- La fila es seleccionable como un chat: con `Tab`/`Shift-Tab`, ir a chat o con la flecha en la lista. Al seleccionarla, el historial muestra los estados como hoy.
- La fila muestra `*` cuando hay estados no vistos, igual que un chat no leído.
- La fila no se cuenta en ninguna sección y nunca se oculta al colapsar "Others" o "Archived".
- Fijar o archivar (`Alt-p`, `Alt-a`) sobre la fila no hace nada, porque WhatsApp no lo permite.
- `status_broadcast=0` sigue ocultando los estados por completo. Con `1` (por defecto) siguen silenciados.
- Nuevas opciones: `list_status_name` en `ui.conf`, y `list_color_status_fg/_bg` y `list_attr_status` en `color.conf`.

Fuera de alcance: filtrar estados de más de 24 horas, agrupar estados por contacto, ver estados en un visor aparte y publicar estados.

## Capabilities

### New Capabilities
- `status-updates-entry`: entrada fija de estados al inicio de la lista de chats, con estilo propio y fuera de las secciones.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: sección nueva para los estados, ordenada primero. No cuenta en los títulos de sección ni en el colapso. `Alt-p`/`Alt-a` se ignoran sobre ella.
- `src/uilistview.cpp`: dibuja la fila de estados con su estilo y nombre.
- `src/uiconfig.cpp`, `src/uicolorconfig.cpp`: opciones nuevas.
- `README.md`: documenta las opciones.
- Sin cambios de protocolo ni de caché.
