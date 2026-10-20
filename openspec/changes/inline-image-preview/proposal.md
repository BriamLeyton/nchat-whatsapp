## Why

Hoy nchat muestra las imágenes de WhatsApp solo como una línea de adjunto (`📎 3EB0C4….jpg`). Para verlas hay que abrirlas en un programa externo. Ghostty implementa el protocolo gráfico de kitty, así que la terminal ya puede dibujar la imagen dentro del historial. Ver fotos y stickers en contexto acerca nchat a la experiencia de WhatsApp sin salir de la terminal.

## What Changes

- Los adjuntos de imagen (fotos y stickers: JPEG, PNG, WebP, GIF estático) se renderizan como miniatura inline en el historial, debajo de la línea `📎` del mensaje.
- La miniatura ocupa como máximo `inline_image_max_height` filas (por defecto 10). Mantiene la proporción y no supera el ancho del panel.
- La línea `📎 nombre-archivo` se sigue mostrando siempre, encima de la imagen. Así se puede seguir seleccionando y abriendo el adjunto como hoy.
- Fallback: si la terminal no soporta el protocolo gráfico de kitty, si la imagen no se puede decodificar o si la función está desactivada, se muestra solo la línea `📎` (comportamiento actual, sin filas reservadas).
- Las imágenes visibles en el historial se descargan automáticamente aunque `attachment_prefetch` esté en `1` (solo seleccionado). Los demás tipos de adjunto siguen respetando `attachment_prefetch`.
- Las imágenes se ocultan mientras hay un diálogo abierto encima del historial y se vuelven a dibujar al cerrarlo, al hacer scroll, al cambiar de chat y al redimensionar.
- Nuevas opciones en `ui.conf`: `inline_images_enabled` (por defecto `1`) e `inline_image_max_height` (por defecto `10`).

Fuera de alcance: URLs http(s) a imágenes dentro del texto, animación de GIF/stickers animados, video, soporte para Sixel o iTerm2, y passthrough de tmux.

## Capabilities

### New Capabilities
- `inline-image-preview`: renderizado inline de adjuntos de imagen en el historial con el protocolo gráfico de kitty, detección de soporte de la terminal, auto-descarga de imágenes visibles, configuración y fallback a la línea de adjunto.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- **Código UI:** `src/uihistoryview.cpp/.h` (reserva de filas y registro de posiciones), `src/uiview.cpp`, `src/uidialog.cpp` y `src/uilistdialog.cpp` (ocultar y redibujar alrededor de diálogos), `src/uiconfig.cpp` (opciones nuevas) y `src/ui.cpp` (detección al inicio y limpieza al salir).
- **Código nuevo:** un módulo `src/uiimage.cpp/.h` para la detección de la terminal, la decodificación, el escalado, la caché de IDs y la emisión de secuencias kitty.
- **Dependencias:** `stb_image` y `stb_image_resize2` (header-only, dominio público/MIT) vendorizados en `ext/`. `libwebp` del sistema para stickers WebP. Hay que actualizar `CMakeLists.txt`.
- **Protocolo y base de datos:** sin cambios en `wmchat`, en el esquema SQLite ni en la sesión de WhatsApp.
- **Rendimiento:** cada imagen se decodifica y transmite una sola vez por sesión y archivo; los redibujados solo reposicionan.
