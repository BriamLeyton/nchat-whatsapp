## Why

La fila de estados muestra todos los estados mezclados, y hay más de 1500 en la caché. Para ver los estados de una persona hay que recorrer todo el historial. WhatsApp agrupa los estados por contacto.

## What Changes

- Nuevo comando `status_filter` (por defecto `Alt-u`), que solo actúa en el chat de estados:
  - **Con un estado seleccionado:** filtra el historial para mostrar solo los estados de quien lo publicó.
  - **Sin selección:** abre un diálogo con los contactos que tienen estados, ordenados por el estado más reciente, con búsqueda al escribir. Elegir uno aplica el filtro.
  - **Con un filtro activo:** el mismo comando quita el filtro y vuelven a verse todos los estados.
- Mientras el filtro está activo, la fila de estados muestra el contacto: `◉ Status · Jane Doe`.
- El filtro se quita al salir del chat de estados.
- Con el filtro activo, la selección, `PgUp`/`PgDn` y las acciones sobre el estado seleccionado (abrir, guardar, reaccionar, …) funcionan sobre la lista filtrada.
- Fuera del chat de estados, `Alt-u` no hace nada.

Fuera de alcance: filtrar desde el chat de un contacto, filtrar otros chats o grupos por remitente, y el filtro de 24 horas.

## Capabilities

### New Capabilities
- `status-contact-filter`: filtro del historial de estados por contacto, desde el estado seleccionado o desde un diálogo.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: estado del filtro, lista de mensajes mostrados (filtrada o completa) para el historial y las acciones, carga de más estados cuando la lista filtrada no alcanza, y el comando.
- `src/uistatuscontactdialog.{h,cpp}` (nuevo): diálogo de contactos con estados, basado en `UiListDialog`.
- `src/uihistoryview.cpp`, `src/uilistview.cpp`: usan la lista mostrada y el nombre del filtro.
- `src/uikeyconfig.cpp`, `src/uihelpdialog.cpp`, `README.md`: tecla nueva y su ayuda.
- Sin cambios de protocolo ni de caché.
