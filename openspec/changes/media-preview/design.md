## Context

- **Recepción (Go):**
  - `HandleImageMessage`, `HandleVideoMessage`, `HandleAudioMessage` y el manejador de texto extendido llaman `CWmNewMessagesNotify` con texto, `fileId`, `filePath` y `fileStatus`.
  - `filePath` es `<perfil>/tmp/<msgId><ext>`, y la extensión sale de `ExtensionByType`.
  - Los protos traen datos que hoy se descartan:
    - `GetJPEGThumbnail()` en imagen, video y texto extendido;
    - `GetSeconds()` y `GetFileLength()` en video y audio, y `GetPTT()` en audio (nota de voz);
    - `GetTitle()`, `GetDescription()`, `GetBackgroundArgb()` y `GetTextArgb()` en texto extendido.
- **Interfaz C:** `CWmNewMessagesNotify` tiene firma fija, y la caché guarda `fileInfo` como hex con estado, id, ruta y tipo. Agregar campos ahí obliga a cambiar el protocolo y el esquema.
- **Vista:** `UiHistoryView::Draw` decide inline con `UiImage::IsImageFile(filePath)` y `UiImage::Prepare`. Reserva una sola imagen por mensaje (`imageLineIndex`, `imageInfo`).
- **Colores:** `UiColorConfig::GetColorId(hex)` crea colores a demanda, y `init_pair` usa ids incrementales (`colorPairId`).
- **Herramientas locales del usuario:** `ffmpegthumbnailer`, `ffmpeg` y `mpv` instalados.

## Goals / Non-Goals

**Goals:**
- Cero peticiones de red nuevas: solo datos del mensaje y procesos locales.
- Sin cambios en la interfaz C ni en la caché.
- La vista nunca bloquea: leer metadatos se cachea, y el fotograma se genera en segundo plano.

**Non-Goals:**
- Reproducir video en la terminal.
- Recuperar metadatos de mensajes ya recibidos.
- Vista previa de links consultando la web.

## Decisions

### 1. Archivos laterales en `tmp`
Para mensajes de cualquier chat, Go escribe junto a `filePath`, con el mismo nombre base `<perfil>/tmp/<msgId>`:
- `<msgId>.thumb.jpg`: bytes de `GetJPEGThumbnail()`, si no está vacío;
- `<msgId>.meta`: líneas `clave=valor`, y solo las claves con valor:
  - `seconds`, `file_length` (video y audio), `ptt=1` (nota de voz);
  - `title`, `description` (texto extendido con vista previa de link);
  - `background_argb`, `text_argb` (texto extendido, solo en `status@broadcast`).

Un mensaje sin miniatura ni claves con valor no genera archivos. Por ejemplo, un texto plano no escribe nada.

Se escriben solo si no existen, porque un mensaje editado o repetido no debe reescribirlos. Los saltos de línea en título y descripción se reemplazan por espacio.

Para textos sin adjunto, la ruta base se arma igual (`GetPath(connId) + "/tmp/" + msgId`). La UI la deriva de `FileUtil::GetApplicationDir() + "/profiles/" + profileId + "/tmp/" + msgId`.

**Alternativa descartada:** pasar los campos por `CWmNewMessagesNotify` y la caché. Cambia la interfaz C, el esquema SQLite y el script `nchat-status` del usuario.

### 2. Extensión `.mp4`
`ExtensionByType` agrega `.mp4` a `preferredExts`. Los `.f4v` ya descargados quedan como están; mpv los abre igual.

### 3. Lectura en la UI
Nuevo `MediaInfo` (`src/mediainfo.{h,cpp}`):
- `GetInfo(profileId, msgId)` devuelve `{ thumbPath, seconds, fileLength, isPtt, title, description, backgroundArgb, textArgb }`.
- Lee el `.meta` con `Config` o un parser simple, y comprueba si existe el `.thumb.jpg`.
- Guarda el resultado en un mapa en memoria. Los archivos laterales no cambian después de escritos, así que no se relee.
- Solo se consulta para mensajes con adjunto, con un link en el texto, o del chat de estados (color de fondo), para no tocar el disco por cada texto plano. Un resultado vacío también se cachea.

### 4. Fotograma de video
- `UiImage::IsVideoFile(path)`: `mp4`, `f4v`, `m4v`, `mov`, `webm`, `3gp`, `mkv`.
- Para un video descargado de cualquier chat, el fotograma es `<ruta sin extensión>.poster.jpg`.
- Si no existe y `video_thumbnail_command` no está vacío:
  - se encola en un único hilo de trabajo que ejecuta el comando, reemplazando `%1` por la entrada y `%2` por la salida;
  - al terminar, pide redibujar el historial.
  - Un video que falla no se reintenta en la sesión.
- Default de `video_thumbnail_command`: `ffmpegthumbnailer -i '%1' -o '%2' -s 640 -q 8 >/dev/null 2>&1`.

### 5. Imagen inline por mensaje
La vista elige una sola ruta inline:
1. adjunto de imagen descargado (como hoy);
2. video: el fotograma si existe; si no, la miniatura incrustada;
3. imagen no descargada: la miniatura incrustada;
4. link: la miniatura incrustada, con alto máximo de 4 filas.

Sigue condicionado a `inline_images_enabled`, y no cambia nada de descargas ni del chequeo de vencidos.

### 6. Línea del adjunto de video y audio
- **Video y audio:** `📎 <archivo> ▶ m:ss · N.N MB`, con la parte que tenga datos: duración desde `seconds` y tamaño desde `file_length`.
- **Nota de voz** (`ptt=1`): `📎 <archivo> 🎤 m:ss`.
- Sin metadatos queda como hoy.

### 7. Vista previa de link
Si el mensaje tiene `title` o `description`, debajo del texto se agregan:
- `▏<título>`, con atributo bold;
- `▏<descripción>`, recortada a 2 líneas.

Se usa el color normal del mensaje, y luego la miniatura.

### 8. Color de estados de texto
- Aplica a un estado sin adjunto con `background_argb`. Solo los estados lo traen.
- Las líneas de texto se dibujan con un par de color: fondo desde ARGB (se ignora alfa) y texto desde `text_argb`. Si no hay `text_argb`, el texto es blanco o negro según la luminancia del fondo.
- `UiColorConfig::GetRgbColorPair(fg, bg)` cachea hasta 32 pares. Pasado el límite, o si la terminal no soporta colores, se dibuja sin color.
- Solo cambian las líneas del texto; encabezado y hora quedan igual.

## Risks / Trade-offs

- **Disco:** unos KB por mensaje con adjunto o link. Las miniaturas pesan unos KB cada una, y el `.meta` menos de 1 KB.
  - El directorio `tmp` gana hasta 2 archivos por mensaje con medios.
  - El change `status-prefetch-cleanup` los conserva al limpiar videos.
- **Mensajes de sincronización de historial:** pasan por los mismos manejadores, así que también generan archivos laterales.
- **Fotograma:**
  - `ffmpegthumbnailer` ausente: el comando falla, se usa la miniatura y no se reintenta.
  - Costo de CPU: un proceso por video descargado, uno a la vez.
- **Miniatura borrosa:** son unos 100 px, así que se ve pixelada al escalar. Es aceptable como respaldo, y por eso tiene menos prioridad que el fotograma o la imagen real.
- **Colores:** terminales sin 256 colores ni truecolor muestran el color aproximado o ninguno.
