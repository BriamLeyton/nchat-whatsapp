## 1. Protocolo (Go)

- [x] 1.1 `ExtensionByType`: preferir `.mp4`.
- [x] 1.2 Helper `WriteMediaSidecar(connId, msgId, thumb, meta)`: escribe `.thumb.jpg` y `.meta` si no existen, sin red.
- [x] 1.3 Llamarlo desde los manejadores de imagen, video, audio y texto extendido, para todos los chats. Los colores solo se escriben en `status@broadcast`.

## 2. UI

- [x] 2.1 `MediaInfo`: lectura y caché de `.meta` y miniatura, solo para mensajes con adjunto, link o del chat de estados.
- [x] 2.2 `UiImage::IsVideoFile`, `video_thumbnail_command` en `ui.conf`, y un hilo de trabajo para generar fotogramas con redibujo al terminar.
- [x] 2.3 `UiHistoryView::Draw`: elegir la imagen inline por prioridad, línea `▶ duración · tamaño` o `🎤 duración`, y miniatura de respaldo sin descargar.
- [x] 2.4 Vista previa de link (título, descripción, miniatura de 4 filas como máximo).
- [x] 2.5 `UiColorConfig::GetRgbColorPair` y color de estados de texto.
- [x] 2.6 README: `video_thumbnail_command` y vista previa de medios.

## 3. Verificación

- [x] 3.1 Sandbox offline (con `.meta` simulados, imágenes inline desactivadas en tmux):
  - [x] build sin warnings nuevos;
  - [x] mensajes viejos sin metadatos se ven como hoy;
  - [x] chats con texto plano no leen disco por mensaje;
  - [x] línea `▶ 1:15 · 4.2 MB`, `🎤 0:42`, vista previa de link con recorte `…`, y fondo morado de estado de texto;
  - [x] el log no muestra peticiones nuevas.
  - [x] el comando de fotograma por defecto genera el `.jpg` de un `.f4v` real en 0.1 s.
- [x] 3.2 Unidad de Go o prueba manual: escribir y leer `.meta` y `.thumb.jpg` desde un mensaje simulado.
- [ ] 3.3 En vivo en Ghostty:
  - [ ] un video nuevo, en un chat y en estados, llega como `.mp4` con `▶ duración · tamaño`;
  - [ ] una nota de voz muestra `🎤 duración`;
  - [ ] un video ya descargado genera `.poster.jpg` sin bloquear la UI y el fotograma se ve inline (en tmux las imágenes inline están desactivadas);
  - [ ] un link en un chat o grupo muestra su vista previa;
  - [ ] un estado de texto se ve con su color;
  - [ ] un estado vencido no descargado muestra la miniatura.
