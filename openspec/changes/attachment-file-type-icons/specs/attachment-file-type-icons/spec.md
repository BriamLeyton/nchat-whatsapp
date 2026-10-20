## Purpose

Mostrar en la línea de adjunto del historial un icono según el tipo de archivo (imagen, video, audio, nota de voz, PDF, documento, hoja de cálculo, archivo comprimido), y un icono en el preview de links, para reconocer el contenido de un mensaje sin leer su extensión o su URL.

## ADDED Requirements

### Requirement: Icono de adjunto por categoría de archivo
El sistema SHALL anteponer a la línea de adjunto del historial el icono de la categoría del archivo, seguido de un espacio, en lugar del indicador genérico. La categoría MUST decidirse por la extensión del nombre del archivo, sin distinguir mayúsculas de minúsculas. Las categorías y sus iconos por defecto SHALL ser:

| Categoría | Extensiones | Icono por defecto |
|---|---|---|
| imagen | jpg, jpeg, png, webp, gif, heic, heif, bmp | `📷` |
| video | mp4, f4v, m4v, mov, webm, 3gp, mkv, avi | `🎬` |
| audio | oga, ogg, opus, mp3, m4a, aac, wav, amr, flac | `🎵` |
| PDF | pdf | `📕` |
| documento | doc, docx, odt, rtf, txt, md, ppt, pptx, odp | `📄` |
| hoja de cálculo | xls, xlsx, ods, csv | `📊` |
| archivo comprimido | zip, rar, 7z, tar, gz, tgz, bz2, xz | `📦` |

#### Scenario: Foto
- **WHEN** el historial muestra un mensaje con el adjunto `IMG-2026.jpg`
- **THEN** la línea de adjunto es `📷 IMG-2026.jpg`

#### Scenario: Extensión en mayúsculas
- **WHEN** el historial muestra un mensaje con el adjunto `Factura.PDF`
- **THEN** la línea de adjunto es `📕 Factura.PDF`

#### Scenario: Video con duración y tamaño
- **WHEN** el historial muestra un video `VID-01.mp4` con duración 1:15 y tamaño 4.2 MB
- **THEN** la línea de adjunto es `🎬 VID-01.mp4 ▶ 1:15 · 4.2 MB`

#### Scenario: Estado de descarga
- **WHEN** el adjunto `backup.zip` todavía no está descargado
- **THEN** la línea de adjunto es `📦 backup.zip` seguida del `downloadable_indicator`, igual que antes de este cambio salvo el icono

### Requirement: Icono de nota de voz
El sistema SHALL mostrar el icono de nota de voz (por defecto `🎤`) en la línea de adjunto de un mensaje marcado como nota de voz (push-to-talk), sin importar la extensión del archivo. En ese caso el sufijo de duración MUST mostrarse sin el micrófono, para no repetir el icono.

#### Scenario: Nota de voz con duración
- **WHEN** el historial muestra una nota de voz `PTT-01.ogg` de 0:42
- **THEN** la línea de adjunto es `🎤 PTT-01.ogg 0:42`

#### Scenario: Audio que no es nota de voz
- **WHEN** el historial muestra un audio `cancion.mp3` que no es nota de voz y tiene duración 3:10
- **THEN** la línea de adjunto es `🎵 cancion.mp3 ▶ 3:10` (con el tamaño si se conoce)

### Requirement: Fallback al indicador genérico
El sistema SHALL usar `attachment_indicator` (por defecto `📎`) como icono cuando el archivo no tiene extensión o su extensión no pertenece a ninguna categoría.

#### Scenario: Extensión desconocida
- **WHEN** el historial muestra un mensaje con el adjunto `firmware.bin`
- **THEN** la línea de adjunto es `📎 firmware.bin`

#### Scenario: Sin extensión
- **WHEN** el historial muestra un mensaje con el adjunto `LEEME`
- **THEN** la línea de adjunto es `📎 LEEME`

### Requirement: Configuración de los iconos
El sistema SHALL leer de `ui.conf` la opción `attachment_type_icons_enabled` (por defecto `1`) y un icono por categoría: `attachment_indicator_image`, `attachment_indicator_video`, `attachment_indicator_audio`, `attachment_indicator_voice`, `attachment_indicator_pdf`, `attachment_indicator_document`, `attachment_indicator_spreadsheet` y `attachment_indicator_archive`, con los valores por defecto de los requisitos anteriores. Con `attachment_type_icons_enabled=0` todos los adjuntos MUST usar `attachment_indicator`, y el sufijo de la nota de voz MUST conservar el micrófono como antes de este cambio. Si el icono de una categoría está vacío, esa categoría MUST usar `attachment_indicator`.

#### Scenario: Icono personalizado
- **WHEN** `ui.conf` tiene `attachment_indicator_pdf=[pdf]` y el historial muestra `factura.pdf`
- **THEN** la línea de adjunto es `[pdf] factura.pdf`

#### Scenario: Función desactivada
- **WHEN** `ui.conf` tiene `attachment_type_icons_enabled=0` y el historial muestra `IMG-2026.jpg` y una nota de voz `PTT-01.ogg` de 0:42
- **THEN** las líneas de adjunto son `📎 IMG-2026.jpg` y `📎 PTT-01.ogg 🎤 0:42`

#### Scenario: Icono de categoría vacío
- **WHEN** `ui.conf` tiene `attachment_indicator_image=` y el historial muestra `IMG-2026.jpg`
- **THEN** la línea de adjunto es `📎 IMG-2026.jpg`

### Requirement: Icono del preview de links
El sistema SHALL anteponer el icono `link_preview_indicator` de `ui.conf` (por defecto `🌐`), seguido de un espacio, a la primera línea del preview de un link: el título, o la primera línea de la descripción si no hay título. El resto de las líneas del preview MUST NOT llevar icono. Con `link_preview_indicator` vacío el preview MUST verse como antes de este cambio. Las líneas del preview con icono MUST NOT superar el ancho del panel.

#### Scenario: Preview con título
- **WHEN** el historial muestra un mensaje con un link cuyo preview tiene el título `Ejemplo` y una descripción de una línea
- **THEN** el preview se ve `▏ 🌐 Ejemplo` y debajo `▏ <descripción>`

#### Scenario: Preview sin título
- **WHEN** el preview de un link tiene solo descripción
- **THEN** la primera línea de la descripción empieza con `▏ 🌐 ` y las demás con `▏ `

#### Scenario: Icono de link desactivado
- **WHEN** `ui.conf` tiene `link_preview_indicator=` y el historial muestra un preview con título `Ejemplo`
- **THEN** el preview se ve `▏ Ejemplo`

#### Scenario: Mensaje con link sin preview
- **WHEN** el historial muestra un mensaje con un link sin título ni descripción de preview
- **THEN** el mensaje se ve igual que antes de este cambio, sin icono

### Requirement: Estilo de la línea de adjunto
La línea de adjunto SHALL seguir mostrándose con el color `history_text_attachment_color` con cualquier icono. Una línea de texto del mensaje que empiece con un emoji de icono MUST NOT recibir el color de adjunto.

#### Scenario: Color con icono de categoría
- **WHEN** el historial muestra `📕 factura.pdf`
- **THEN** la línea usa el color `history_text_attachment_color`

#### Scenario: Texto que empieza con el mismo emoji
- **WHEN** el caption de una foto empieza con `📷 en la playa`
- **THEN** la línea del caption usa el color normal del texto, no el de adjunto
