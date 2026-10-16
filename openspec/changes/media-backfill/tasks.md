## 1. Implementación

- [x] 1.1 `UiImage::IsAudioFile` y config `media_probe_command`.
- [x] 1.2 `MediaInfo::RequestProbe`: tarea en el hilo de trabajo, `popen` y `stat`, escritura o mezcla de `.meta`, actualización de la caché y flag de redibujo.
- [x] 1.3 `UiHistoryView::Draw`: pedir el sondeo para video o audio descargado sin `seconds`.
- [x] 1.4 README.

## 2. Verificación

- [x] 2.1 Sandbox: un video viejo descargado muestra la duración y el tamaño sin bloquear, se crea `.meta`, y tras reiniciar no se vuelve a sondear (log).
- [x] 2.2 Sandbox: con `media_probe_command` inválido, solo el tamaño y sin reintentos.
- [ ] 2.3 En vivo: un video enviado desde nchat muestra la duración y el tamaño.
