## 1. Precarga

- [x] 1.1 Configs `status_prefetch=1` y `status_prefetch_video_max_mb=16` en `ui.conf`.
- [x] 1.2 Cola de precarga en `UiModel::Impl`: encolar desde `NewMessagesNotify` del chat de estados, avanzar en `NewMessageFileNotify`, respaldo de 120 s, y re-chequeo de vencido o ya descargado.

## 2. Limpieza

- [x] 2.1 `MessageCache::FetchFileInfosSync(profileId, chatId, beforeTime)`.
- [x] 2.2 Hilo de limpieza al iniciar: borrar videos, actualizar `fileInfo` en caché y memoria, conservar archivos laterales, y registrar en el log.
- [x] 2.3 Config `status_video_retention_days=0` (default: no borra).
- [x] 2.4 README.

## 3. Verificación

- [x] 3.1 Sandbox offline (copia con rutas reescritas a archivos falsos, sin tocar `tmp` real):
  - [x] con el default 0 no se borra nada;
  - [x] con 7, la limpieza borra videos de más de 7 días (273), conserva videos recientes (226) e imágenes (925);
  - [x] la caché queda en "no descargado";
  - [x] la vista muestra la línea del video sin indicador ni petición de descarga.
- [x] 3.2 Sandbox offline: un video vigente de 5 MB con metadatos se encola una sola vez; uno de 40 MB, los vencidos y los videos sin metadatos no; sin conexión la cola no envía nada.
- [ ] 3.3 En vivo: un estado nuevo llega ya descargado, y enviar un mensaje durante la precarga no espera la cola completa.
