## Context

- Responder en nchat: con un mensaje seleccionado, `SendMessage` llama a `AddQuoteFromSelectedMessage`, que llena `quotedId`, `quotedText` y `quotedSender`, y envía al chat actual.
- `WmSendMessage` arma `ContextInfo{StanzaID, Participant, QuotedMessage{Conversation: quotedText}}` y no pone `RemoteJID`, porque la cita siempre es del mismo chat.
- WhatsApp responde un estado así: el mensaje va al JID del autor con `ContextInfo{StanzaID: id del estado, Participant: autor, RemoteJID: "status@broadcast", QuotedMessage: contenido del estado}`.
- `GetUserId` ya guarda al autor de un estado con su número cuando WhatsApp da la equivalencia con su `@lid`. Si no, queda el `@lid`, y whatsmeow también puede enviarle a ese JID.
- Hoy los envíos en Status están bloqueados en `OnKeySendMsg`, en la transferencia de archivos, en el editor externo y en la edición.

## Decisions

### 1. Destino y validaciones
En `OnKeySendMsg`, si el chat actual es Status:
- sin estado seleccionado, o con edición activa: se mantiene el aviso de hoy, y el texto queda en la entrada;
- estado propio: aviso "Replying to own status updates is not supported.";
- estado vencido (`IsExpiredStatus`): aviso "Replying to expired status updates is not supported.";
- si no, se arma la respuesta:
  - `chatId` = `senderId` del estado;
  - cita del estado seleccionado, con `quotedChatId = "status@broadcast"`;
  - `quotedFilePath`: la miniatura del estado (`meta->thumbPath`), si existe y es foto o video;
  - después se limpia la entrada y se sale de la selección;
  - se pasa al chat del autor con `GotoChat` si ya existe, sin desarchivarlo; si no existe (primer mensaje), con `OpenCreateChat`, que lo crea como al abrir un contacto;
  - con el resultado del envío (`SendMessageNotify`): si sigue en Status, "Reply sent to <nombre>."; si falló, "Reply to <nombre> failed, please try again from Status Updates." y el texto vuelve a la entrada de Status si está vacía.

Pasar al chat del autor evita que el aviso de éxito interrumpa la navegación por Status, y muestra la respuesta enviada, como en la app.

### 2. Protocolo
- `ChatMessage` gana `quotedChatId` y `quotedFilePath`. Solo viajan en `SendMessageRequest`; no se guardan en la caché.
- `CWmSendMessage` y `WmSendMessage` reciben los dos valores nuevos.
- En Go, si `quotedChatId == "status@broadcast"`:
  - `ContextInfo.RemoteJID = "status@broadcast"`;
  - la cita es `ImageMessage{JPEGThumbnail, Caption}` o `VideoMessage{JPEGThumbnail, Caption}` según el tipo del archivo citado, con los bytes de `quotedFilePath`. Si no hay miniatura, `Conversation` con el texto, como hoy.
- El tipo se decide con la extensión del archivo citado, que nchat ya conoce del `fileInfo` del estado. Se pasa como prefijo `image:` o `video:` en `quotedFilePath`, para no agregar un tercer parámetro.

### 3. Cita en el chat del autor
- En `UiHistoryView`, si `quotedId` no está en los mensajes del chat, se busca en los mensajes cargados de Status antes de pedirlo a la caché.
- Sirve para las respuestas enviadas y para las que otros hacen a tus estados.

## Risks / Trade-offs

- **Formato de la cita:** si el teléfono del autor no muestra la cita como espera, el mensaje igual llega como texto normal. Se verifica en vivo con un estado real de un contacto.
- **Estado no cargado:** si el estado citado no está cargado en Status, la cita se ve vacía, como hoy.
