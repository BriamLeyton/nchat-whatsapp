## Context

The history (`src/uihistoryview.cpp`) is drawn from bottom to top in an ncurses window (`m_PaddedWin`). It first builds a vector of lines `wlines` per message (attachment, quote, text, reactions) and ends with `wrefresh`. ncurses knows nothing about images. Any graphics have to be written directly to the terminal after `wrefresh`, in cells that ncurses leaves blank.

The code already writes raw sequences outside ncurses: `printf("\033[?1004h")` + `fflush` in `UiModel::Impl::TerminalControlResume`. That is a precedent.

Data available per message: `FileInfo { fileStatus, fileId, filePath, fileType }`. In WhatsApp, `filePath` already has the correct extension (`.jpg`, `.webp`, …) before download (`gowm.go`, `HandleImageMessage`/`HandleStickerMessage`). `fileType` is not filled for received messages.

The dialogs (`UiListDialog::Run`, etc.) are drawn on top of the history with their own ncurses windows and, when closed, call `ReinitView()` + `Draw()`.

System libraries present: `libwebp`, `libjpeg-turbo`, `libpng`, `libmagic`.

## Goals / Non-Goals

**Goals:**
- Draw thumbnails with the kitty graphics protocol without changing the existing ncurses drawing model.
- Isolate all graphics in a new module so `uihistoryview.cpp` only reserves rows and records positions.
- Keep each redraw cheap: transmit the pixels once, then only reposition.

**Non-Goals:**
- kitty Unicode placeholders (U+10EEEE). They require combining diacritics and 24-bit colors encoded in cells, which ncurses does not handle reliably.
- Asynchronous decoding on a separate thread. It stays as an improvement if lag is noticeable.
- Support for Sixel or iTerm2, and tmux passthrough.

## Decisions

### 1. Direct placement with IDs (`a=T`/`a=p`) instead of Unicode placeholders
Each image is transmitted once with a numeric ID (`a=t,i=<id>,f=32,s=<w>,v=<h>`). Each `Draw()` does two things:
1. Deletes all visible placements without freeing the data (`a=d,d=a`).
2. For each registered thumbnail, moves the cursor (`ESC[<row>;<col>H`) and places the image (`a=p,i=<id>,c=<cols>,r=<rows>,C=1,q=2`).

`C=1` prevents the cursor from moving. `q=2` suppresses the terminal's responses, which would otherwise arrive as garbage input to `wgetch`. The sequence is wrapped in `ESC 7` / `ESC 8` (save/restore cursor) so it does not desynchronize ncurses.

*Rejected alternative:* Unicode placeholders, see Non-Goals. Their advantage (free scrolling and clipping) does not outweigh the fragility with ncurses.

### 2. Decode on the client and send RGBA (`f=32`)
The kitty protocol only accepts PNG (`f=100`) or raw pixels. WhatsApp photos are JPEG and stickers are WebP. They are decoded locally:
- JPEG, PNG and GIF (first frame) with `stb_image.h`.
- WebP with `libwebp` (`WebPDecodeRGBA`). Animated WebP stickers (~20% of the stickers in the real cache) use the first frame via `libwebpdemux` (`WebPAnimDecoder`), the same as with GIF.

It is then scaled with `stb_image_resize2.h` to the exact pixel size of the thumbnail and sent base64-encoded in 4096-byte chunks (`m=1`/`m=0`), with direct transmission (`t=d`).

*Rejected alternatives:* `t=f` (the terminal reads the file) does not work for JPEG or WebP and fails if the terminal runs on another host. Using `libjpeg`/`libpng` directly means more code. ImageMagick is a heavy dependency.

### 3. Size in cells from the cell size in pixels
The cell size is obtained with `ioctl(TIOCGWINSZ)` (`ws_xpixel/ws_col`, `ws_ypixel/ws_row`). If it is 0, `ESC[16t` is queried. If that does not answer either, 8×16 is assumed.

With that: `rows = min(max_height, ceil(h_px / cell_h))`, `cols = ceil(w_px * scale / cell_w)`, limited to the panel width. Pixels are scaled to `cols*cell_w × rows*cell_h` keeping the aspect ratio, never above their natural size.

### 4. Support detection at startup, before `initscr()`
In `src/ui.cpp`, before `initscr()`, the terminal is put into raw mode and a graphics query is sent (`ESC_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA ESC\`) followed by DA1 (`ESC[c`). The reply is read with a timeout of ~200 ms:
- If `ESC_Gi=31;OK` arrives before the DA1 reply, support is present.
- If only DA1 arrives, or the timeout expires, there is no support and the fallback is used.

If the `TMUX` variable is set, it is not enabled. This is robust against `TERM`, which in Ghostty is sometimes `xterm-256color`.

*Rejected alternative:* checking `TERM_PROGRAM=ghostty`/`TERM=xterm-kitty`. It fails with multiplexers and with `ssh`.

### 5. Thumbnail registration in `UiHistoryView::Draw`
When building `wlines` for a message with a renderable image, `rows` empty marked lines are inserted, right after the `📎` line. When walking the lines from bottom to top, the screen row of the first visible line of the image is recorded, along with how many rows were clipped at the top.

After `wrefresh(m_PaddedWin)`, `UiImage::PlaceAll(placements, origin)` is called. For top clipping, the protocol's pixel crop parameters are used (`y=<skip_rows*cell_h>`, `h=<visible_rows*cell_h>`) and `r=<visible_rows>`.

Truncation of the first message (`maxMessageLines`) counts the image rows as normal lines. If the image does not fit, it is skipped and the `📎` line remains.

### 6. Module `src/uiimage.{h,cpp}` (static class `UiImage`)
API:
- `Init()`: detection, called from `ui.cpp`.
- `IsEnabled()`
- `IsImageFile(path)`: by extension, `.jpg/.jpeg/.png/.webp/.gif`.
- `Prepare(path, maxCols, maxRows)`: returns `{id, cols, rows}` or a failure. It decodes, scales and transmits only the first time, and caches the result by `(path, maxCols, maxRows)`. Failures are also cached so they are not retried on every frame.
- `ClearPlacements()`, `PlaceAll(...)` and `Cleanup()`: `a=d,d=A` on exit, which frees the data.

Writing is done with `write(STDOUT_FILENO)` after `fflush(stdout)`, so it does not mix with the ncurses buffer.

### 7. Hiding around dialogs and terminal pauses
`UiImage::ClearPlacements()` is called:
- in the `UiDialog` constructor;
- in `UiModel::Impl::TerminalControlPause()`, for the external editor and `Ctrl-z`;
- at the start of `UiHistoryView::Draw` (always, even if there are no images).

When a dialog closes, the existing `ReinitView()` + `Draw()` places the thumbnails again. It must be verified that the history's `m_Dirty` is `true` in that flow.

### 8. Automatic download of images
In the `// File attachment` block of `UiHistoryView::Draw`, the prefetch condition is extended with `|| (UiImage::IsEnabled() && UiImage::IsImageFile(fileInfo.filePath))`. `DownloadAttachmentLocked(..., DownloadFileActionNone)` is reused. The redraw when the download finishes already exists.

### 9. Configuration
Added to the `UiConfig` defaults (`src/uiconfig.cpp`): `inline_images_enabled=1` and `inline_image_max_height=10`. The existing `UiConfig` mechanism already writes missing keys when saving.

## Risks / Trade-offs

- [Decoding 12 MP photos on the UI thread blocks for ~50–150 ms the first time] → Mitigation: per-session cache; each file is decoded once. If it is noticeable, move it to a worker thread with a `⇄` placeholder (follow-up).
- [Terminal responses leak to `wgetch` as garbage keys] → Mitigation: `q=2` on all execution sequences. The detection query is done before `initscr()` and the input is drained.
- [ncurses cursor desynchronization] → Mitigation: `ESC 7`/`ESC 8` around each batch and `C=1` on placements.
- [Terminal memory from many transmitted images] → Mitigation: LRU limit, for example 64 images. When one is evicted, `a=d,d=I,i=<id>` is sent.
- [Graphics leftovers if nchat crashes] → Accepted risk: the terminal clears them with `clear`/reset. `Cleanup()` is also called from the existing exit signal handlers.
- [Ghostty with `TERM=xterm-256color` and emoji width] → No impact: the image cells are spaces.
- [Under `herdr` or tmux there is no graphics support] → The automatic fallback is used; it is not a regression.

- [Ghostty frees image data on `ESC[2J` (clear screen) and when re-entering the alternate screen; this was verified empirically with `a=p` queries, which return `ENOENT`. ncurses emits `ESC[H ESC[2J` on every `ReinitView()` (`UiScreen` does `wclear(stdscr)`), and `TerminalControlResume` re-enters the alternate screen. If the cache keeps using dead IDs, the images disappear and the reserved space remains] → Mitigation: `UiImage::Invalidate()` after every clear or re-entry marks the images as not transmitted. The cache keeps the scaled RGBA (max 64 images, ~20–40 MB) and retransmits it when placing, without decoding again.

## Migration Plan

- No data migration. Build, and the symlink `~/.local/bin/nchat` picks up the new binary.
- To disable on the fly: `inline_images_enabled=0` in `~/.config/nchat/ui.conf`.
- Rollback: go back to the previous commit and rebuild, or restore `~/.local/bin/nchat.5.19.18.bak`.
