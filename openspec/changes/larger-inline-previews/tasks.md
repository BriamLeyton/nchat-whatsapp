## 1. Implementation

- [x] 1.1 `UiImage::Prepare` with `p_Upscale` (scale without the cap of 1) and the flag in the cache key.
- [x] 1.2 `UiHistoryView`: `p_Upscale=true` for `meta->thumbPath` in attachments and link preview; link preview with the same `inline_image_max_height` maximum.
- [x] 1.3 Default `inline_image_max_height=12` in `uiconfig.cpp` and README.

## 2. Verification

- [x] 2.1 Isolated test of `Prepare`: a 90×160 thumbnail with and without upscaling gives different rows, and the large image does not change.
- [x] 2.2 Live in Ghostty, after changing `inline_image_max_height=12` in `~/.config/nchat/ui.conf` with nchat closed: a non-downloaded video has the same height as an image.
