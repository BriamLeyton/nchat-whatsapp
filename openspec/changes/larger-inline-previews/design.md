## Context

- `UiImage::Prepare(path, maxCols, maxRows, info)` fits the image into a `maxCols × maxRows` cell box, with scale `min(1, maxW/w, maxH/h)`, so it never enlarges.
- `UiHistoryView` picks the inline path in this order: downloaded image, downloaded video frame, and last, `meta->thumbPath`, the thumbnail included in the message.
- Thumbnails measured in the user's profile: 320×568, 90×160, 96×96, 72×72 and 320×168.

## Decisions

### 1. Upscale only thumbnails
- `Prepare` gains `bool p_Upscale = false`. With `true` the scale is `min(maxW/w, maxH/h)`, without the cap of 1.
- The cache key includes the flag, so the same path with and without upscaling does not collide.
- `UiHistoryView` passes `true` only when the path is `meta->thumbPath`, both for attachments and for link previews.
- Scaling uses `stbir_resize_uint8_srgb`, the same as today. The upscaled image is transmitted to the terminal at its final size, so it does not depend on how the terminal scales.
- Downloaded images are not enlarged. A sticker or a small image is shown at its real size.

### 2. Sizes
- `inline_image_max_height`: default 12.
- Link preview thumbnail: the same `inline_image_max_height`.

## Risks / Trade-offs

- **Blurry:** a 90×160 thumbnail enlarged to 12 rows looks blurry. It is the same effect as the official app before download, and once downloaded it is replaced by the real image or frame.
- **Taller:** with 12 rows fewer messages fit on screen. It can be lowered with `inline_image_max_height`.
