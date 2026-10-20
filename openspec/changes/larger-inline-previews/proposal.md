## Why

Thumbnails of videos and of images that are not downloaded yet look smaller than downloaded images:
- the thumbnail included in the message is small: 90×160 px for a vertical video, and 96×96 px for some images;
- `UiImage::Prepare` never enlarges, so that thumbnail takes about 6 to 8 rows instead of the 10 of the maximum.

In addition, the user wants all previews a bit larger.

## What Changes

- **Thumbnails included in the message:** they are enlarged until they fill the same box as a downloaded image. Applies to images and videos not downloaded yet, and to expired statuses.
  - They will look somewhat blurry, the same as in the official app before download.
  - Downloaded images and video frames (640 px) do not change: they already fill the box.
- **General size:** `inline_image_max_height` changes from 10 to 12 rows by default.
- **Link preview:** its thumbnail goes from at most 4 rows to using the same `inline_image_max_height` as images.

## Capabilities

### New Capabilities
- `larger-inline-previews`: thumbnails enlarged to the size of images, and larger default sizes.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uiimage.{h,cpp}`: parameter `p_Upscale` in `Prepare`, included in the cache key.
- `src/uihistoryview.cpp`: enlarge when `meta->thumbPath` is used; link preview with the same maximum.
- `src/uiconfig.cpp` and `README.md`: default 12.
- User config: `~/.config/nchat/ui.conf` has `inline_image_max_height=10` written, so the new default does not apply to it. It must be changed by hand with nchat closed.
