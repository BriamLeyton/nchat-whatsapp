## Why

Las miniaturas de videos e imágenes no descargadas se ven más chicas que las imágenes descargadas:
- la miniatura incluida en el mensaje es pequeña: 90×160 px para un video vertical, y 96×96 px para algunas imágenes;
- `UiImage::Prepare` nunca agranda, así que esa miniatura ocupa unas 6 a 8 filas en vez de las 10 del máximo.

Además, el usuario quiere todas las vistas previas un poco más grandes.

## What Changes

- **Miniaturas incluidas en el mensaje:** se agrandan hasta llenar el mismo recuadro que una imagen descargada. Aplica a imágenes y videos no descargados, y a estados vencidos.
  - Se verán algo borrosas, igual que en la app oficial antes de descargar.
  - Las imágenes descargadas y los fotogramas de video (640 px) no cambian: ya llenan el recuadro.
- **Tamaño general:** `inline_image_max_height` pasa de 10 a 12 filas por defecto.
- **Vista previa de link:** su miniatura pasa de 4 filas como máximo a usar el mismo `inline_image_max_height` que las imágenes.

## Capabilities

### New Capabilities
- `larger-inline-previews`: miniaturas agrandadas al tamaño de las imágenes, y tamaños por defecto mayores.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uiimage.{h,cpp}`: parámetro `p_Upscale` en `Prepare`, incluido en la clave de caché.
- `src/uihistoryview.cpp`: agrandar cuando se usa `meta->thumbPath`; vista previa de link con el mismo máximo.
- `src/uiconfig.cpp` y `README.md`: default 12.
- Config del usuario: `~/.config/nchat/ui.conf` tiene `inline_image_max_height=10` escrito, así que el nuevo default no le aplica. Hay que cambiarlo a mano con nchat cerrado.
