## Why

Al volver de suspender el equipo, nchat reconecta y whatsmeow entrega de golpe la cola offline: los mensajes recibidos durante la suspensión y, detrás, los recibos `read-self` de los que ya se leyeron en el teléfono. Cada mensaje suena (campana y notificación de escritorio) en cuanto llega, antes de que llegue su recibo de leído, así que suenan a la vez decenas de mensajes que ya se leyeron.

La causa: solo se notifican los mensajes con `timeSent > m_ConnectTime` (`src/uimodel.cpp:3501`). `m_ConnectTime` se fija una sola vez, con el `ConnectNotify` de `WmChat::Login()` (`lib/wmchat/src/wmchat.cpp:236`). Al reconectar, `events.Connected` (`lib/wmchat/go/gowm.go:909`) no lo actualiza, así que todo lo que llega en la cola offline se trata como mensaje nuevo.

## What Changes

- nchat reenvía a la UI el inicio (`events.OfflineSyncPreview`) y el fin (`events.OfflineSyncCompleted`) de la sincronización offline de whatsmeow.
- Mientras dura la sincronización offline de un perfil, los mensajes entrantes no generan campana ni notificación de escritorio. Se guarda una lista de los chats que habrían notificado.
- Al terminar la sincronización (tras un margen corto para que se procesen los últimos eventos), nchat revisa esos chats:
  - si siguen sin leer (no llegó `read-self` para su último mensaje), notifica una sola vez por chat, con el último mensaje;
  - si ya se leyeron en el teléfono, no notifica;
  - la campana suena como mucho una vez en total.
- Si nunca llega `OfflineSyncCompleted`, un tiempo máximo de seguridad termina la retención y aplica la misma revisión.
- Las reglas actuales siguen igual: chats silenciados, archivados y restringidos, `notify_every_unread`, `terminal_bell_*` y `desktop_notify_*`.
- Fuera de la sincronización offline, la notificación de mensajes nuevos no cambia.

## Capabilities

### New Capabilities
- `offline-sync-notifications`: retener las notificaciones durante la sincronización offline al reconectar, y notificar después solo los chats que siguen sin leer.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`, `cgowm.go`: manejar `events.OfflineSyncPreview` y reenviar los dos eventos con un nuevo callback C.
- `lib/common/src/protocol.h`, `lib/wmchat/src/wmchat.{h,cpp}`: nuevo `OfflineSyncNotify` (inicio/fin).
- `src/uimodel.{h,cpp}`: estado de retención por perfil, `UpdateChatInfoIsUnread` deja los chats pendientes en vez de notificar, vaciado en `ProcessTimers`.
- No cambia el esquema de la base de datos ni el formato de configuración.
