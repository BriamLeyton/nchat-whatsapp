## 1. Clasificación por tipo de archivo

- [x] 1.1 Agregar `enum FileCategory` (`Other`, `Image`, `Video`, `Audio`, `Pdf`, `Document`, `Spreadsheet`, `Archive`) y `FileUtil::GetFileCategory(const std::string& p_Path)` en `lib/ncutil/src/fileutil.h/.cpp`, con la tabla de extensiones del spec, usando la extensión de `BaseName` en minúsculas. Verificar que `./make.sh` compila sin warnings nuevos.
- [x] 1.2 Agregar `tests/test003.cpp` (registrado en `tests/CMakeLists.txt`) que cubra: cada categoría, extensión en mayúsculas (`Factura.PDF`), extensión desconocida (`firmware.bin`), sin extensión (`LEEME`) y punto solo en el directorio (`/tmp/a.zip/LEEME`). Verificar con `./make.sh tests` que pasa.

## 2. Configuración

- [x] 2.1 Agregar a los defaults de `src/uiconfig.cpp` `attachment_type_icons_enabled=1` y `attachment_indicator_image/video/audio/voice/pdf/document/spreadsheet/archive` con los emoji del spec, como escapes UTF-8 igual que `attachment_indicator`. Verificar que tras arrancar y salir `~/.config/nchat/ui.conf` contiene las claves nuevas y el resto queda igual (`diff` contra un respaldo).
- [x] 2.2 Documentar las opciones nuevas en `README.md`, en la lista de `ui.conf` y con una sección `### attachment_type_icons_enabled` junto a `### attachment_indicator`. Verificar leyendo el diff.

## 3. Historial

- [x] 3.1 En `src/uihistoryview.cpp`, agregar el mapa `static` categoría → icono y elegir el icono de la línea de adjunto con `GetFileCategory` y `meta->isPtt`, con fallback a `attachment_indicator` si el icono está vacío o la función está desactivada. Verificar en un chat real que una foto, un video, un PDF y un `.zip` muestran `📷`, `🎬`, `📕` y `📦`.
- [x] 3.2 Omitir `🎤 ` del sufijo de duración de las notas de voz cuando la función está activa. Verificar que una nota de voz se ve `🎤 <archivo>.ogg 0:42` y, con `attachment_type_icons_enabled=0`, `📎 <archivo>.ogg 🎤 0:42`.
- [x] 3.3 Reemplazar la detección de la línea de adjunto por prefijo (`rfind(attachmentIndicator, 0)`) por un flag `hasAttachmentLine` y `lineIndex == 0`, con un comentario sobre el orden de inserción. Verificar que todas las líneas de adjunto usan `history_text_attachment_color` y que un caption que empieza con `📷` usa el color normal.

- [x] 3.4 Agregar `link_preview_indicator` (por defecto `🌐`) a `src/uiconfig.cpp` y `README.md`, y anteponerlo a la primera línea del preview de links en `src/uihistoryview.cpp`, achicando el ancho de esa línea según el ancho del icono. Verificar en un chat real un preview con título (`▏ 🌐 Título`), uno sin título y, con `link_preview_indicator=`, que el preview queda como antes.

## 4. Verificación integral

- [x] 4.1 Probar con la sesión real: adjunto no descargado (`+`), descargando (`⇄`) y fallido (`✗`) conservan el sufijo de estado; la miniatura inline sigue debajo de la línea de adjunto; abrir y guardar adjuntos sigue funcionando. Probar un icono personalizado (`attachment_indicator_pdf=[pdf]`) y uno vacío (`attachment_indicator_image=`).
