## Context

La línea de adjunto se arma en `UiHistoryView::Draw` (`src/uihistoryview.cpp`): `attachmentIndicator + nombre + estado + mediaStr`. `attachmentIndicator` es un `static std::wstring` con `attachment_indicator` de `ui.conf`. Más abajo, al dibujar, la línea recibe el color de adjunto si empieza con ese mismo prefijo (`wline->rfind(attachmentIndicator, 0) == 0`). Con un icono por categoría esa comparación deja de servir.

Ya hay clasificación por extensión en `UiImage::IsImageFile`, `IsVideoFile` e `IsAudioFile` (`src/uiimage.cpp`), que deciden la miniatura inline y el probe de duración. `MediaMeta::isPtt` (sidecar de metadatos) marca las notas de voz, y hoy el sufijo de duración agrega `🎤`. Los mensajes recibidos solo traen la ruta del archivo; `FileInfo::fileType` (MIME) solo se llena al enviar, así que no sirve para clasificar.

Los tests unitarios (`tests/`) enlazan solo `libncutil`, así que la lógica probable debe vivir ahí.

## Goals / Non-Goals

**Goals:**
- Clasificación extensión → categoría pura y con test unitario.
- Cambiar solo la línea de adjunto del historial; mismo layout, mismo color.

**Non-Goals:**
- Reemplazar `UiImage::IsImageFile` y compañía: siguen controlando la miniatura inline y el probe. Unificarlos queda para otro cambio.
- Pasar los iconos por `StrUtil::Textize` cuando `emoji_enabled=0`: el `📎` actual tampoco se convierte. El usuario puede poner iconos ASCII en `ui.conf`.

## Decisions

### Emoji por categoría, no Nerd Font por extensión
Se usan emoji estándar con presentación emoji por defecto (`📷 🎬 🎵 🎤 📕 📄 📊 📦`). Ocupan dos columnas en cualquier terminal moderna y no requieren una fuente especial. Se evitan emoji con presentación de texto por defecto (`🖼 U+1F5BC`, `🗜 U+1F5DC`), porque sin `U+FE0F` su ancho varía entre terminales y descuadra el wrap.
Alternativa descartada: glyphs Nerd Font por extensión (como eza o lf). Dan más detalle, pero se ven rotos sin la fuente. Igual se pueden poner a mano en las opciones por categoría.
El usuario confirmó emoji por categoría y el set de iconos por defecto. También probó los sets Nerd Font `fa` y `md` y los descartó por verse planos.

### Clasificación en `FileUtil`
Nuevo `enum FileCategory` y `FileUtil::GetFileCategory(const std::string& p_Path)` en `lib/ncutil/src/fileutil.h/.cpp`. Toma la extensión de `BaseName(p_Path)` (así un punto en un directorio no cuenta), la pasa a minúsculas y la busca en una tabla `static const std::map<std::string, FileCategory>`. Devuelve `FileCategoryOther` si no la encuentra.
Alternativa descartada: una función nueva en `UiImage`. No se puede probar sin enlazar la UI.

### Icono resuelto en la vista
En `uihistoryview.cpp`, una función local `GetAttachmentIndicator(FileCategory, bool isPtt)` devuelve el `std::wstring` del icono más espacio. Los iconos se cachean en un `static std::map<FileCategory, std::wstring>` armado una vez desde `UiConfig`, igual que los otros indicadores `static` del archivo. Si `isPtt` y la función está activa, gana la categoría voz. Si el icono de la categoría está vacío o `attachment_type_icons_enabled=0`, se usa `attachment_indicator`.

### Sufijo de nota de voz
En la rama `meta->isPtt` de `mediaStr` se omite `🎤 ` cuando la función está activa, y queda `" " + duración`. Con la función desactivada no cambia nada.

### Icono del preview de links
El icono va en la primera línea del preview (título, o primera línea de la descripción), después de `▏ `, y no en la línea del texto con la URL. El preview es lo único que identifica el mensaje como link sin parsear URLs, y un mensaje puede tener texto antes de la URL. Es una opción aparte, `link_preview_indicator` (por defecto `🌐`, el equivalente emoji de `nf-md-web` que eligió el usuario), porque un link no es un adjunto y `attachment_type_icons_enabled` no lo controla. Si no hay título, la descripción se envuelve con el ancho del preview menos el ancho del icono, para que la primera línea no se pase del panel.
Alternativa descartada: icono al inicio de la línea de texto que contiene la URL. Habría que detectar URLs en el texto envuelto, y los mensajes sin preview quedarían igual que hoy de todas formas.

### Color por índice de línea, no por prefijo
La línea de adjunto siempre se inserta en `wlines.begin()`, y después de eso no se inserta nada antes (el quote se inserta antes que ella, y la miniatura, el preview y las reacciones van después). Se reemplaza la comparación de prefijo por `isAttachment = hasAttachmentLine && (lineIndex == 0)`, con `hasAttachmentLine` puesto al insertar `fileStr`. Así cualquier icono recibe el color, y un caption que empieza con `📷` ya no lo recibe por error.
Alternativa descartada: comparar contra el conjunto de todos los iconos. Mantiene el falso positivo con el caption.

## Risks / Trade-offs

- [Un futuro cambio inserta una línea antes del adjunto] → El índice 0 deja de ser correcto. Mitigación: comentario junto a la inserción y en la verificación del color.
- [El ancho del emoji difiere entre terminales] → Solo se usan emoji de presentación emoji por defecto. Los iconos se pueden cambiar en `ui.conf`.
- [Un sticker se ve como imagen `📷`] → Aceptado. Los stickers son `.webp` y no hay metadato que los distinga sin tocar el protocolo.

## Migration Plan

No hay migración: las opciones nuevas tienen valores por defecto y `ui.conf` existentes las reciben al guardarse. Para volver al comportamiento anterior: `attachment_type_icons_enabled=0`.
