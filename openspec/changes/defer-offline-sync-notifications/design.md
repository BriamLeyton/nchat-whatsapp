## Context

Motivación en proposal.md (Why). Estado actual relevante:

- La decisión de notificar está en `UiModel::Impl::UpdateChatInfoIsUnread` (`src/uimodel.cpp`). Se llama al añadir mensajes y al cambiar su estado de leído. Activa `m_TriggerTerminalBell` (la campana suena en `Process()`) y llama `DesktopNotify` en el momento.
- Notifica solo si el chat pasa de leído a no leído (o siempre con `notify_every_unread`) y si `timeSent > m_ConnectTime[profileId]`. `m_ConnectTime` solo se fija con el `ConnectNotify` de `WmChat::Login()`.
- whatsmeow procesa los nodos del socket en orden en `handlerQueueLoop` (un nodo a la vez). La cola offline llega así: nodo `ib/offline_preview` (`events.OfflineSyncPreview`), mensajes y recibos pendientes (incluidos `read-self`) y nodo `ib/offline` (`events.OfflineSyncCompleted`). Si no hay pendientes, no llega preview.
- `gowm.go` ya maneja `OfflineSyncCompleted` (llama `HandleSyncContacts`), pero no `OfflineSyncPreview`.
- Go→C++: los eventos van por un callback C (`CWm*Notify` en `cgowm.go`, `Wm*Notify` en `wmchat.cpp`), que encola un `DeferNotifyRequest` con el `ServiceMessage`. El hilo del protocolo los entrega a la UI en orden FIFO, así que la UI recibe el fin de la sincronización después de todos los mensajes y recibos anteriores.
- `ReceiptTypeReadSelf` ya se convierte en `NewMessageStatusNotify(isRead=true)`, que llama `UpdateChatInfoIsUnread` y marca el chat como leído.
- `ProcessTimers()` se ejecuta cada ~200 ms en el hilo de la UI.

## Goals / Non-Goals

**Goals:**
- Diferir la campana y la notificación de escritorio de los mensajes que llegan durante la sincronización offline, y decidir al final con el estado de leído ya actualizado.
- Reutilizar la lógica y las reglas actuales de notificación, sin duplicarlas.

**Non-Goals:**
- No se cambia `m_ConnectTime` ni el filtro de mensajes antiguos al arrancar.
- No se agrupan varios chats en una sola notificación de escritorio.
- No se procesa la acción `markChatAsRead` del app state. El teléfono envía `read-self` por mensaje, y eso basta.
- No hay opción de configuración nueva: la retención está siempre activa.

## Decisions

### 1. Un solo mensaje de protocolo `OfflineSyncNotify { bool isStarted; }`
Go envía `isStarted=true` en `events.OfflineSyncPreview` y `isStarted=false` en `events.OfflineSyncCompleted`, con un nuevo `CWmOfflineSyncNotify(connId, isStarted)`.
- Alternativa: reutilizar `FlagSyncing` del estado (`CWmSetStatus`). Se descarta: el estado es una barra de flags para la vista, no viaja en la cola de notificaciones, y no garantiza el orden con los mensajes.
- Alternativa: un timer de silencio tras `events.Connected`. Se descarta: no sabe cuándo terminan los recibos y silenciaría también mensajes nuevos.

### 2. Retención en la UI, por perfil
Estado nuevo en `UiModel::Impl`:
- `m_OfflineSyncStartTime[profileId]` (ms), presente mientras dura la retención;
- `m_OfflineSyncEndTime[profileId]` (ms), fijado al recibir el fin;
- `m_OfflineSyncPendingChats[profileId]` (`std::set<std::string>`), los chats que habrían notificado.

En `UpdateChatInfoIsUnread`, en la rama que hoy notifica, si el perfil está en retención no se activa la campana ni se llama `DesktopNotify`: se añade el chat a los pendientes. El cálculo de `isUnread` y la actualización de `profileChatInfos[...].isUnread` no cambian, así que la lista se ve igual.

Para no duplicar la lógica, el bloque que decide campana + escritorio se extrae a un método `NotifyChatMessage(profileId, chatId, chatMessage)`. Lo usan la ruta inmediata y el vaciado.

### 3. Vaciado en `ProcessTimers` con margen de 2 s y tope de 60 s
`ProcessOfflineSyncFlush()` se llama desde `ProcessTimers()`. Vacía un perfil cuando:
- han pasado 2 s desde `m_OfflineSyncEndTime`, o
- han pasado 60 s desde `m_OfflineSyncStartTime` sin fin.

El margen de 2 s cubre los eventos que whatsmeow despacha en goroutines (descargas, reintentos de descifrado) y los que llegan justo después del nodo `offline`. Durante el margen la retención sigue activa.

Al vaciar, para cada chat pendiente:
1. si el chat ya no existe en `m_ChatInfos`, está archivado o restringido, se omite;
2. se toma el último mensaje (`GetLastMessageId`). Si es saliente o está leído, se omite;
3. se aplican las reglas de silenciado (`muted_notify_unread`, `hasMention`) y `receivedAfterConnect`;
4. se llama `NotifyChatMessage` con la campana limitada a una vez (en la práctica ya lo está, porque `m_TriggerTerminalBell` es un booleano que se consume en `Process()`).

Luego se borran los tres estados del perfil.

- Alternativa: vaciar en el mismo `case OfflineSyncNotifyType`. Se descarta porque no deja margen para los eventos atrasados.

### 4. Un preview nuevo durante la retención la prolonga
Si llega otro `isStarted=true` (reconexión rápida) con la retención activa, se conservan los pendientes, se reinicia `m_OfflineSyncStartTime` y se borra `m_OfflineSyncEndTime`. Un `isStarted=false` sin retención activa se ignora (por ejemplo, un fin sin preview porque no había pendientes).

### 5. `events.Disconnected` no cambia nada
La retención termina por el fin o por el tope de 60 s. No se añade otra señal: el tope basta para que no quede activa para siempre.

## Risks / Trade-offs

- [Un mensaje realmente nuevo que llega durante la sincronización notifica unos segundos tarde] → Aceptable: la sincronización suele durar pocos segundos, y la notificación llega igual.
- [whatsmeow despacha `OfflineSyncCompleted` antes de procesar algún mensaje (goroutine de un nodo que superó el tiempo del handler)] → Margen de 2 s. Si aun así llega tarde, ese mensaje notifica en el momento, como hoy: un caso aislado, no la avalancha.
- [El teléfono no envía `read-self` porque el chat se marcó como leído sin abrir los mensajes (marcar como leído en la lista)] → En ese caso llega `markChatAsRead` por app state, que nchat no procesa: el chat notificaría. Una sola notificación por chat en vez de una por mensaje. Queda como mejora futura si molesta.
- [Arranque de nchat: también llega un preview] → La retención se aplica, pero `receivedAfterConnect` sigue filtrando lo antiguo en el vaciado. El resultado es igual que hoy.

## Migration Plan

No hay migración: no cambian datos persistidos ni configuración. Para revertir basta con quitar el commit.

## Open Questions

- ¿El margen de 2 s y el tope de 60 s son adecuados con suspensiones largas? Se ajustan como constantes tras probarlo de verdad.
