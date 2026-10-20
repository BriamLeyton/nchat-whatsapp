## Why

Hoy todos los adjuntos del historial empiezan con el mismo icono genérico (`📎 factura.pdf`, `📎 IMG-2026.jpg`, `📎 backup.zip`). Para saber de qué tipo es un adjunto hay que leer la extensión del nombre. Un icono por tipo de archivo deja reconocer de un vistazo si un mensaje trae una foto, un video, un audio, una nota de voz o un documento, como pasa en WhatsApp.

## What Changes

- La línea de adjunto del historial muestra un icono según la categoría del archivo, en lugar de `attachment_indicator` para todos:
  - imagen `📷`, video `🎬`, audio `🎵`, nota de voz `🎤`, PDF `📕`, documento `📄`, hoja de cálculo `📊`, archivo comprimido `📦`.
  - Cualquier otro tipo (o sin extensión) sigue usando `attachment_indicator` (`📎`).
- La categoría se decide por la extensión del archivo (sin distinguir mayúsculas). La nota de voz se reconoce por el metadato `isPtt` del mensaje.
- En las notas de voz el sufijo de duración deja de repetir el micrófono: `🎤 PTT-01.ogg 0:42` en vez de `📎 PTT-01.ogg 🎤 0:42`.
- La primera línea del preview de un link (el título, o la descripción si no hay título) empieza con `🌐`: `▏ 🌐 Título de la página`. El icono se configura con `link_preview_indicator`; vacío no muestra icono.
- El color de la línea de adjunto (`history_text_attachment_color`) se sigue aplicando con cualquier icono.
- Nuevas opciones en `ui.conf`: `attachment_type_icons_enabled` (por defecto `1`) y un icono configurable por categoría (`attachment_indicator_image`, `attachment_indicator_video`, `attachment_indicator_audio`, `attachment_indicator_voice`, `attachment_indicator_pdf`, `attachment_indicator_document`, `attachment_indicator_spreadsheet`, `attachment_indicator_archive`). Con `attachment_type_icons_enabled=0` todos los adjuntos vuelven a usar `attachment_indicator`.

Fuera de alcance: iconos Nerd Font por extensión, detección por contenido (magic bytes) o por MIME, iconos en otras vistas (lista de chats, diálogos de archivos), y distinguir stickers de imágenes.

## Capabilities

### New Capabilities
- `attachment-file-type-icons`: icono de la línea de adjunto según la categoría del tipo de archivo, icono del preview de links, configuración por categoría y fallback al indicador genérico.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- **Código UI:** `src/uihistoryview.cpp` (elegir el icono de la línea de adjunto, sufijo de nota de voz, icono del preview de links y detección de la línea de adjunto para el color) y `src/uiconfig.cpp` (opciones nuevas).
- **Código nuevo:** clasificación extensión → categoría en `lib/ncutil/src/fileutil.cpp/.h`, para poder probarla con los tests unitarios existentes.
- **Tests:** nuevo test unitario en `tests/` para la clasificación.
- **Docs:** `README.md` (opciones nuevas de `ui.conf`).
- **Protocolo y base de datos:** sin cambios en `wmchat`, en el esquema SQLite ni en la caché.
