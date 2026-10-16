## 1. Implementación

- [x] 1.1 `UiImage::Prepare` con `p_Upscale` (escala sin tope de 1) y flag en la clave de caché.
- [x] 1.2 `UiHistoryView`: `p_Upscale=true` para `meta->thumbPath` en adjuntos y vista previa de link; vista previa de link con el mismo máximo `inline_image_max_height`.
- [x] 1.3 Default `inline_image_max_height=12` en `uiconfig.cpp` y README.

## 2. Verificación

- [x] 2.1 Prueba aislada de `Prepare`: miniatura de 90×160 con y sin agrandar da distintas filas, y la imagen grande no cambia.
- [ ] 2.2 En vivo en Ghostty, tras cambiar `inline_image_max_height=12` en `~/.config/nchat/ui.conf` con nchat cerrado: video no descargado del mismo alto que una imagen.
