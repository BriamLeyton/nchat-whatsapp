## Context

- **Peticiones serializadas:** `WmChat` procesa las peticiones en un solo hilo (`m_RequestsQueue`), y `WmDownloadFile` descarga de forma síncrona. Encolar 50 descargas retrasa cualquier envío de mensaje hasta que terminen.
- **Descargas:** `UiModel::Impl::DownloadAttachment` es el único punto de descarga, y ya rechaza estados vencidos. El resultado llega por `NewMessageFileNotify`, que actualiza `fileInfo` en memoria y caché.
- **Llegada de estados:** `NewMessagesNotify` llega tanto para mensajes nuevos (`cached == false`) como para lecturas de caché (`cached == true`).
- **Tamaño del video:** disponible en `<msgId>.meta` (`file_length`) gracias a `media-preview`.
- **Actualizar adjunto en caché:** `MessageCache::UpdateMessageFileInfo` existe.

## Goals / Non-Goals

**Goals:**
- Estados vigentes listos al abrirlos, sin bloquear el envío de mensajes.
- Disco acotado: los videos viejos se borran, y lo visual (fotograma, miniatura) se conserva.

**Non-Goals:**
- Precargar adjuntos de chats normales (eso es `attachment_prefetch`).
- Limpiar imágenes o mensajes.

## Decisions

### 1. Disparo de la precarga
- Se dispara en `NewMessagesNotify` del chat de estados, con `cached` en `true` o en `false`. Así, al iniciar, los estados vigentes en caché sin descargar también se precargan cuando se cargan.
- Requisitos para encolar:
  - `status_prefetch=1`;
  - el estado no está vencido;
  - el adjunto no está descargado y es descargable;
  - es imagen, o es video con `file_length <= status_prefetch_video_max_mb * 1024 * 1024`.
- Se descartan duplicados con un set de ids encolados o en curso.

### 2. Cola de una descarga a la vez
`m_StatusPrefetchQueue` (deque de `{profileId, chatId, msgId}`) y `m_StatusPrefetchInFlight`:
- Al encolar, si no hay una en curso, se envía la primera con `DownloadAttachment(..., DownloadFileActionNone)`.
- En `NewMessageFileNotify` del mensaje en curso, se envía la siguiente.
- Antes de enviar se vuelve a chequear si venció o si ya se descargó, por ejemplo porque el usuario la abrió. Si es así, se salta.
- **Sin conexión:** la cola se conserva y no se envía nada hasta estar en línea (`Status::FlagOnline`), porque las descargas fallarían y quedarían marcadas como fallidas.
- **Respaldo:** si no llega la notificación en 120 s, se considera terminada y se sigue, para que una descarga perdida no trabe la cola.

**Alternativa descartada:** enviar todo de una vez. Bloquea los envíos varios minutos.

### 3. Limpieza al iniciar
- Al terminar de cargar el perfil, si `status_video_retention_days > 0`, un hilo en segundo plano:
  1. consulta en la caché, con `MessageCache::FetchFileInfosSync(profileId, chatId, beforeTime)`, los estados con `fileInfo` y `timeSent < now - N días`;
  2. para cada video (`UiImage::IsVideoFile`) con archivo existente, borra el archivo y guarda `fileInfo` con estado `FileStatusNotDownloaded` mediante `UpdateMessageFileInfo`;
  3. actualiza los mensajes en memoria, bajo el mutex del modelo, si están cargados.
- **Se conservan:** `.poster.jpg`, `.thumb.jpg` y `.meta`.
- Corre una vez por inicio. Registra en el log la cantidad de archivos y MB liberados.

### 4. Interacción con estados vencidos
Un video limpiado es siempre de un estado vencido, porque N ≥ 1 día. Por eso nunca se re-descarga: la vista muestra el fotograma o la miniatura, y abrir muestra "Expired status, attachment not available."

## Risks / Trade-offs

- **Datos móviles:** la precarga gasta datos como la app oficial. Con `status_prefetch=0` se desactiva.
- **Fotograma:** si el fotograma no se generó antes de borrar el video, queda solo la miniatura incrustada. En la práctica se genera al verlo o al precargarlo, si la vista lo dibuja.
- **Cola en memoria:** se pierde al salir. Al reiniciar, la lectura de caché vuelve a encolar lo vigente.
