## Why

Responder un estado desde nchat publicaba el texto como un estado nuevo, porque nchat envía la respuesta al chat actual, que en Status es `status@broadcast`. Desde `status-expired-local-only`, enviar en Status está bloqueado con un aviso, así que hoy no se puede responder un estado.

En WhatsApp, responder un estado es enviar un mensaje privado al autor citando el estado. El autor lo ve en su chat con la miniatura o el texto del estado.

## What Changes

- En Status, con un estado seleccionado, escribir y dar Enter envía la respuesta al chat privado del autor, citando el estado como lo hace WhatsApp.
- Al enviar:
  - nchat pasa al chat del autor, donde se ve la respuesta con la cita, como en la app. Si todavía no hay chat con el autor, se queda en Status con el aviso "Reply sent to <nombre>.";
  - si falla, aparece "Reply to <nombre> failed, please try again from Status Updates." y el texto vuelve a la entrada de Status;
  - la respuesta aparece en el chat del autor como cualquier mensaje enviado.
- La cita incluye el texto del estado y, si es foto o video, su miniatura incluida en el mensaje (`<id>.thumb.jpg`). No se descarga nada.
- Siguen bloqueados, con un aviso:
  - enviar sin estado seleccionado: publicaría un estado nuevo;
  - responder un estado propio;
  - responder un estado vencido (más de 24 h), igual que en la app;
  - enviar archivos, editar o usar el editor externo en Status, como hoy.
- En el chat del autor, la cita de una respuesta a estado muestra el texto o el archivo del estado. Hoy se ve vacía, porque el estado no está en ese chat. Vale también para las respuestas que otros hacen a tus estados.

## Capabilities

### New Capabilities
- `status-reply`: responder un estado con un mensaje privado al autor que lo cita.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/common/src/protocol.h`: `ChatMessage.quotedChatId` (chat del mensaje citado, vacío si es el mismo chat).
- `lib/wmchat/src/wmchat.cpp` y `lib/wmchat/go/gowm.go`: `WmSendMessage` recibe el chat de la cita y la ruta de la miniatura; con `status@broadcast` arma `ContextInfo.RemoteJID` y la cita como imagen o video con miniatura.
- `src/uimodel.cpp`: `OnKeySendMsg` en Status envía al autor en vez de bloquear; validaciones y aviso.
- `src/uihistoryview.cpp`: buscar la cita en Status cuando no está en el chat.
- `README.md`: responder estados.
