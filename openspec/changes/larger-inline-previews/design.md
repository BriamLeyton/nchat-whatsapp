## Context

- `UiImage::Prepare(path, maxCols, maxRows, info)` ajusta la imagen a un recuadro de `maxCols × maxRows` celdas, con escala `min(1, maxW/w, maxH/h)`, así que nunca agranda.
- `UiHistoryView` elige la ruta inline en este orden: imagen descargada, fotograma del video descargado, y por último `meta->thumbPath`, la miniatura incluida en el mensaje.
- Miniaturas medidas en el perfil del usuario: 320×568, 90×160, 96×96, 72×72 y 320×168.

## Decisions

### 1. Agrandar solo miniaturas
- `Prepare` gana `bool p_Upscale = false`. Con `true` la escala es `min(maxW/w, maxH/h)`, sin el tope de 1.
- La clave de caché incluye el flag, para que la misma ruta con y sin agrandar no choque.
- `UiHistoryView` pasa `true` solo cuando la ruta es `meta->thumbPath`, tanto en adjuntos como en vistas previas de link.
- El escalado usa `stbir_resize_uint8_srgb`, el mismo de hoy. La imagen agrandada se transmite al terminal con su tamaño final, así que no depende de cómo escale el terminal.
- Las imágenes descargadas no se agrandan. Un sticker o una imagen chica se ve en su tamaño real.

### 2. Tamaños
- `inline_image_max_height`: default 14.
- Miniatura de vista previa de link: `min(6, inline_image_max_height)`.

## Risks / Trade-offs

- **Borroso:** una miniatura de 90×160 agrandada a 14 filas se ve borrosa. Es el mismo efecto que la app oficial antes de descargar, y una vez descargado se reemplaza por la imagen o el fotograma real.
- **Más alto:** con 14 filas entran menos mensajes por pantalla. Se puede bajar con `inline_image_max_height`.
