## 1. Implementación

- [x] 1.1 `ChatMessage.quotedChatId` y `quotedFilePath` en `protocol.h`, pasados por `wmchat.cpp` a `CWmSendMessage`.
- [x] 1.2 `WmSendMessage`: con `status@broadcast`, `ContextInfo.RemoteJID` y cita como imagen o video con `JPEGThumbnail`; si no, como hoy.
- [x] 1.3 `OnKeySendMsg` en Status: validaciones (sin selección, propio, vencido, edición), envío al autor con la cita y aviso "Reply sent to <nombre>.".
- [x] 1.4 `UiHistoryView`: buscar la cita en Status si no está en el chat.
- [x] 1.5 README: responder estados.

## 2. Verificación

- [x] 2.1 Sandbox sin red: sin selección, propio y vencido muestran su aviso y no envían; con un estado vigente el log muestra el envío al autor con `RemoteJID status@broadcast` y la miniatura.
- [ ] 2.2 En vivo: responder un estado real de un contacto; en el teléfono del contacto (o en el tuyo, en su chat) la respuesta aparece citando el estado, y en nchat la cita muestra el texto o archivo del estado.
