## Why

Today nchat shows WhatsApp images only as an attachment line (`📎 3EB0C4….jpg`). To see them you have to open them in an external program. Ghostty implements the kitty graphics protocol, so the terminal can already draw the image inside the history. Seeing photos and stickers in context brings nchat closer to the WhatsApp experience without leaving the terminal.

## What Changes

- Image attachments (photos and stickers: JPEG, PNG, WebP, static GIF) are rendered as an inline thumbnail in the history, below the message's `📎` line.
- The thumbnail takes at most `inline_image_max_height` rows (default 10). It keeps its aspect ratio and does not exceed the panel width.
- The `📎 file-name` line is always still shown, above the image. That way the attachment can still be selected and opened as today.
- Fallback: if the terminal does not support the kitty graphics protocol, if the image cannot be decoded, or if the feature is disabled, only the `📎` line is shown (current behavior, no reserved rows).
- Images visible in the history are downloaded automatically even when `attachment_prefetch` is `1` (selected only). Other attachment types still respect `attachment_prefetch`.
- Images are hidden while a dialog is open on top of the history, and drawn again when it closes, on scroll, on chat change and on resize.
- New options in `ui.conf`: `inline_images_enabled` (default `1`) and `inline_image_max_height` (default `10`).

Out of scope: http(s) image URLs inside text, GIF/animated sticker animation, video, Sixel or iTerm2 support, and tmux passthrough.

## Capabilities

### New Capabilities
- `inline-image-preview`: inline rendering of image attachments in the history with the kitty graphics protocol, terminal support detection, automatic download of visible images, configuration and fallback to the attachment line.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- **UI code:** `src/uihistoryview.cpp/.h` (row reservation and position recording), `src/uiview.cpp`, `src/uidialog.cpp` and `src/uilistdialog.cpp` (hiding and redrawing around dialogs), `src/uiconfig.cpp` (new options) and `src/ui.cpp` (detection at startup and cleanup on exit).
- **New code:** a module `src/uiimage.cpp/.h` for terminal detection, decoding, scaling, the ID cache and emitting kitty sequences.
- **Dependencies:** `stb_image` and `stb_image_resize2` (header-only, public domain/MIT) vendored in `ext/`. System `libwebp` for WebP stickers. `CMakeLists.txt` must be updated.
- **Protocol and database:** no changes to `wmchat`, the SQLite schema or the WhatsApp session.
- **Performance:** each image is decoded and transmitted only once per session and file; redraws only reposition.
