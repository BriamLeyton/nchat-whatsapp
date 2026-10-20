## Context

- **`MediaInfo` (`media-preview`):**
  - `Get(profileId, msgId)` lee `<perfil>/tmp/<msgId>.meta` y lo cachea en memoria;
  - un hilo de trabajo genera fotogramas con `video_thumbnail_command` y avisa con `TakePosterUpdated()`.
- **Vista:** sabe si el adjunto está descargado (`FileStatusDownloaded`) y su ruta. Para archivos enviados desde nchat, la ruta es el archivo original del usuario, fuera de `tmp`.
- **Herramientas:** `ffprobe` está instalado junto con `ffmpeg`.

## Goals / Non-Goals

**Goals:**
- Cubrir lo existente y lo enviado desde nchat, sin red.
- No bloquear la vista.

**Non-Goals:**
- Datos de adjuntos no descargados.
- Vista previa web de links.

## Decisions

### 1. Disparo
En `UiHistoryView::Draw` (pasada final), para un adjunto descargado de video o audio cuya `MediaMeta` no tiene `seconds`, se llama `MediaInfo::RequestProbe(profileId, msgId, filePath)`.
- **Audio:** `oga`, `ogg`, `opus`, `mp3`, `m4a`, `aac`, `wav`, `amr`, con `UiImage::IsAudioFile`.
- La nota de voz (`ptt`) no se puede deducir del archivo, así que esos audios viejos muestran `▶`.

### 2. Sondeo
- Se encola en el mismo hilo de los fotogramas. Una tarea es un fotograma o un sondeo.
- Ejecuta `media_probe_command` reemplazando `%1`, y lee la salida (`popen`). La duración es el número en segundos, redondeado.
- El tamaño sale de `stat` del archivo.
- Default: `ffprobe -v error -show_entries format=duration -of csv=p=0 '%1' </dev/null 2>/dev/null`.

### 3. Guardado
- Si existe `.meta`, se agregan solo `seconds` y `file_length` que falten. Si no existe, se crea.
- Se actualiza la entrada en la caché de `MediaInfo` y se marca para redibujar, con el mismo flag de los fotogramas.
- Si falla, o si el comando está vacío, no se reintenta en la sesión. Si el comando falla pero el tamaño se conoce, se guarda solo el tamaño.

### 4. Costo
- `ffprobe` tarda unos 50 ms por archivo, y solo corre para lo que se dibuja.
- Al guardarse en `.meta`, cada archivo se sondea una sola vez.

## Risks / Trade-offs

- **Sin `ffprobe`:** se guarda solo el tamaño.
- **Archivos enviados desde nchat:** si el usuario los borra o mueve, ya hoy se ven como no disponibles, y no se sondean.
- **`.meta` en `tmp` para mensajes enviados:** pesa menos de 1 KB cada uno.
