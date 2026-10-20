## 1. Modelo

- [x] 1.1 `IsExpiredStatus` y visto local en `MarkRead` (memoria + `MessageCache::UpdateMessageIsRead`, sin petición).
- [x] 1.2 `DownloadAttachment` no envía nada para estados vencidos. `GetMessageAttachmentPath` deja aviso pendiente si no hay archivo, y abrir/guardar/ver lo muestran.
- [x] 1.3 `PreReact` rechaza estados vencidos con aviso.
- [x] 1.4 Enviar texto, enviar archivo y editar en Status: aviso, sin enviar, conservando la entrada.

## 2. Vista y documentación

- [x] 2.1 `UiHistoryView::Draw`: sin descarga ni indicador de descargable para estados vencidos.
- [x] 2.2 README.

## 3. Verificación

- [x] 3.1 Sandbox offline, revisando en el log que no aparezcan `download file`, `mark message read` ni `send reaction` para estados vencidos:
  - [x] recorrer estados viejos no leídos los marca leídos y sobrevive al reinicio;
  - [x] fotos y videos vencidos sin archivo muestran `📎` sin descargar;
  - [x] abrir y guardar un adjunto vencido muestran el aviso;
  - [x] `Alt-s` sobre un estado vencido muestra el aviso;
  - [x] `Enter` y enviar archivo en Status muestran el aviso y conservan el texto.
- [ ] 3.2 En vivo en Ghostty: un estado de hoy envía visto y descarga como antes, y una foto vencida ya descargada se ve inline (en tmux las imágenes inline están desactivadas).
