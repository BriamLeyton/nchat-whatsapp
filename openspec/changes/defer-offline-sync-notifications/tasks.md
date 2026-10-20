## 1. Go y protocolo

- [x] 1.1 `protocol.h`: `OfflineSyncNotifyType` y `OfflineSyncNotify{isStarted}`; `wmchat.{h,cpp}`: `WmOfflineSyncNotify(connId, isStarted)` que encola un `DeferNotifyRequest`; `cgowm.go`: `extern` y `CWmOfflineSyncNotify`. Verificar: `cd build && make -j$(nproc)` compila sin errores.
- [x] 1.2 `gowm.go`: `case *events.OfflineSyncPreview` llama `CWmOfflineSyncNotify(connId, 1)` y registra en el log `Total`/`Messages`/`Receipts`; `case *events.OfflineSyncCompleted` llama `CWmOfflineSyncNotify(connId, 0)` y mantiene `HandleSyncContacts()`. Verificar: compila y, al arrancar nchat, el log muestra el preview y el fin.

## 2. UI

- [x] 2.1 `uimodel.cpp`: extraer de `UpdateChatInfoIsUnread` el bloque de campana y escritorio a `NotifyChatMessage(profileId, chatId, chatMessage)`, sin cambiar el comportamiento. Verificar: compila y un mensaje recibido con nchat conectado sigue notificando.
- [x] 2.2 `uimodel.{h,cpp}`: estado por perfil `m_OfflineSyncStartTime`, `m_OfflineSyncEndTime`, `m_OfflineSyncPendingChats`; `case OfflineSyncNotifyType` según design.md (decisión 4: el preview inicia o prolonga, el fin sin retención se ignora). Verificar: el log muestra el inicio y el fin de la retención.
- [x] 2.3 `UpdateChatInfoIsUnread`: en retención, añadir el chat a los pendientes en vez de llamar `NotifyChatMessage`. Verificar: el log muestra los chats retenidos y no suena nada durante la sincronización.
- [x] 2.4 `ProcessOfflineSyncFlush()` desde `ProcessTimers()`: vaciar 2 s después del fin o 60 s después del inicio; por chat, revisar archivado/restringido, último mensaje entrante sin leer, silenciado/mención y `receivedAfterConnect`, y llamar `NotifyChatMessage`; registrar en el log los pendientes y los notificados. Verificar: compila y el log muestra el vaciado con los conteos.

## 3. Verificación

- [ ] 3.1 En vivo en Ghostty: con nchat abierto, cortar la red (`nmcli networking off`) unos minutos; recibir mensajes en 2 chats y leer uno en el teléfono; volver a conectar. Esperado: no suena nada durante la sincronización; al final, una notificación solo del chat no leído y una sola campana.
- [x] 3.2 En vivo: suspender el equipo (`systemctl suspend`) con mensajes que se leen en el teléfono durante la suspensión. Esperado: al volver no hay notificaciones.
- [ ] 3.3 En vivo: con nchat conectado y sin sincronización en curso, un mensaje nuevo notifica al instante como antes. Al arrancar nchat, los mensajes antiguos no notifican.
