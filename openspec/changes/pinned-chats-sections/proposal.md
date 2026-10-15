## Why

nchat ya permite fijar chats con `Alt-p` y archivarlos con `Alt-a`, pero:
- **La sincronización estaba rota:** la copia local del estado `regular_low` de WhatsApp (pins y archivados) quedó corrupta (`mismatching LTHash`), y el snapshot del servidor también. Por eso `Alt-p` fallaba con `409 conflict` y no llegaban los pins ni los archivados del teléfono. Ya está corregido en el commit `3f567a41`.
- **No hay distinción visual:** la lista no indica qué chats están fijados.
- **Los archivados son invisibles:** nchat los oculta por completo y descarta sus mensajes nuevos en la UI. Con la sincronización arreglada, son 74 chats que desaparecieron de la lista.

## What Changes

- **Recuperación del estado de WhatsApp (hecho):** ante `mismatching LTHash`, nchat hace un resync completo desde el servidor. Si el snapshot del servidor también es inconsistente, pide una copia de recuperación al teléfono, como WhatsApp Web, y reintenta el cambio.
- **Tres secciones en la lista, en este orden:**
  - `Fijados`: chats fijados.
  - `Otros`: chats ni fijados ni archivados.
  - `Archivados`: chats archivados, ordenados por último mensaje.

  Cada título se muestra solo si su sección tiene chats. Con solo la sección `Otros`, la lista se ve como hoy, sin títulos.
- **Secciones colapsables:** `Alt-o` colapsa o expande `Otros` y `Alt-h` hace lo mismo con `Archivados`. Una sección colapsada muestra solo su título, con el número de chats y `*` si alguno tiene no leídos. `Archivados` empieza colapsada. El estado se recuerda entre reinicios.
- **Chats archivados:**
  - Se pueden abrir y leer.
  - Reciben mensajes nuevos en la UI, pero no generan notificación, igual que hoy.
  - Siguen archivados al recibir mensajes, como en WhatsApp con "Mantener chats archivados".
  - `Alt-a` mueve un chat entre `Otros` y `Archivados` en vivo.
- **Configuración:**
  - `ui.conf`: `list_pinned_header`, `list_others_header`, `list_archived_header`, `list_others_collapsed` y `list_archived_collapsed`.
  - `key.conf`: `toggle_others` y `toggle_archived`.
  - `color.conf`: `list_color_header`.

Fuera de alcance: fijado solo local, orden manual entre fijados, y el límite de 3 chats fijados de WhatsApp.

## Capabilities

### New Capabilities
- `chat-list-pinned-sections`: secciones `Fijados`, `Otros` y `Archivados` en la lista de chats, colapsables, y sincronización confiable de fijados y archivados aunque el estado de WhatsApp se haya corrompido.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- **Go (`lib/wmchat/go/gowm.go`, hecho):** `ResyncAppState`, `RequestAppStateRecovery` y `SendAppStateWithResync`, más el manejo de `events.AppStateSyncError` y de `AppStateSyncComplete` con `Recovery`.
- **UI:**
  - `src/uimodel.{h,cpp}`: los archivados entran en `m_ChatVec`, orden por sección, estado colapsado, navegación entre chats visibles y teclas nuevas.
  - `src/uilistview.cpp`: filas de título y omisión de chats colapsados.
  - `src/uiconfig.cpp`, `src/uikeyconfig.cpp`, `src/uicolorconfig.cpp`: opciones nuevas.
- Sin cambios de protocolo C++ ni de esquema (`chats2.isArchived` ya existe).
