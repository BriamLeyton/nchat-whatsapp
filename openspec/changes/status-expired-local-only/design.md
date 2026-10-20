## Context

- **Lectura:** `UiHistoryView::Draw` llama `MarkReadLocked` por cada mensaje recibido no leído que dibuja. `UiModel::Impl::MarkRead` marca `isRead` en memoria y envía `MarkMessageReadRequest`. La caché se actualiza cuando vuelve la notificación del protocolo.
- **Descargas:** todas las descargas de adjuntos pasan por `UiModel::Impl::DownloadAttachment`, que envía `DownloadFileRequest`. Lo usan la vista (prefetch, inline, selección) vía `DownloadAttachmentLocked`, y abrir, guardar y ver vía `GetMessageAttachmentPath`. En Go, `DownloadFromFileId` solo se llama desde `WmDownloadFile`.
- **Reacciones:** `PreReact` prepara y el diálogo de emojis envía `SendReactionRequest`.
- **Envío:** `OnKeySendMsg`, `OnKeyTransfer` (y su diálogo) y la edición no revisan el chat. Para `status@broadcast`, whatsmeow puede publicar el mensaje como estado.

## Goals / Non-Goals

**Goals:**
- Un solo chequeo `IsExpiredStatus(chatId, msg)`, aplicado en los puntos de salida hacia el protocolo, no en cada llamador.

**Non-Goals:**
- Publicar estados o responder por privado.

## Decisions

### 1. Chequeo
`IsExpiredStatus(p_ChatId, p_ChatMessage)`: `IsStatusChat(p_ChatId) && (now - timeSent > 24 h)`, con `now` de `TimeUtil::GetCurrentTimeMSec()`.

### 2. Visto local
En `MarkRead`, si el mensaje es un estado vencido: se pone `isRead = true` en memoria, se llama `MessageCache::UpdateMessageIsRead(profile, chat, msgId, true)`, se actualizan no leídos y la lista, y se sale sin enviar la petición.

### 3. Descargas
- **Punto único:** `DownloadAttachment` sale sin enviar nada si el mensaje es un estado vencido. Así ningún camino futuro puede descargar.
- **Vista:** en `UiHistoryView::Draw` no se llama a la descarga para estados vencidos, y el indicador de descargable no se muestra, para que no parezca que se va a bajar.
- **Abrir, guardar y ver:** `GetMessageAttachmentPath` devuelve el archivo si ya está descargado. Si no lo está y el estado está vencido, marca un aviso pendiente, y el llamador público lo muestra con `MessageDialog` sin el mutex del modelo.

### 4. Reacciones
`PreReact` devuelve `false` para estados vencidos, y el llamador muestra el aviso.

### 5. Envío en Status
`UiModel::OnKeySendMsg`, `OnKeyTransfer` y la edición revisan `IsStatusChat` del chat actual antes de hacer nada. Si es Status, muestran el aviso y salen sin tocar la caja de entrada.

## Risks / Trade-offs

- **Reloj del sistema adelantado o atrasado:** el límite de 24 h usa la hora local. Un reloj desfasado mueve el corte unas horas, sin otro efecto.
- **Estados sin leer que vencen sin haberlos visto:** al verlos ya no se envía confirmación. Es lo que el usuario pidió, y la app oficial tampoco la enviaría.
