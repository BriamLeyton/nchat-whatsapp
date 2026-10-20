## 1. Dependencies and build

- [x] 1.1 Vendor `stb_image.h` and `stb_image_resize2.h` (pinned versions, with license) in `ext/stb/`. Verify they exist and that the license header is intact.
- [x] 1.2 In `CMakeLists.txt`: add `ext/stb` to includes, `find_package`/`pkg_check_modules` for `libwebp` and link it to `nchat`. Verify that `cmake .. -DHAS_TELEGRAM=OFF && make -j$(nproc)` compiles without new warnings.

## 2. UiImage module

- [x] 2.1 Create `src/uiimage.h/.cpp` with the API from the design (`Init`, `IsEnabled`, `IsImageFile`, `Prepare`, `ClearPlacements`, `PlaceAll`, `Cleanup`) and add it to `CMakeLists.txt`. Verify it compiles.
- [x] 2.2 Implement detection (`a=q` query + DA1, timeout ~200 ms, disabled if `TMUX` is set) and cell size (`TIOCGWINSZ`, `ESC[16t`, fallback 8×16), with a log of the result. Verify in the log: `supported=1` in Ghostty and `supported=0` inside tmux.
- [x] 2.3 Implement decoding: JPEG/PNG/GIF with stb and WebP with libwebp, to RGBA. Scale to the computed cell size without upscaling. Verify with a test program in `build/` on a `.jpg` and a `.webp` from `~/.config/nchat/profiles/*/tmp`: expected dimensions and no errors.
- [ ] 2.4 Implement transmission (`a=t,f=32,t=d,q=2`, base64 in 4096-byte chunks, `m=1/0`), the cache by `(path,maxCols,maxRows)` with an LRU of 64 (`a=d,d=I` on eviction) and the failure cache. Verify with the log that the second `Prepare` call for the same file does not retransmit.
- [x] 2.5 Implement `ClearPlacements` (`a=d,d=a,q=2`), `PlaceAll` (cursor + `a=p,c,r,C=1,q=2`, crop `y/h`, wrapped in `ESC7/ESC8`) and `Cleanup` (`a=d,d=A`). Use `fflush(stdout)` + `write(STDOUT_FILENO)`. Verify the image appears in Ghostty and the input cursor stays in place.

## 3. UI integration

- [x] 3.1 Add `inline_images_enabled=1` and `inline_image_max_height=10` to the defaults in `src/uiconfig.cpp`. Verify that after starting and exiting, `~/.config/nchat/ui.conf` contains both keys and the rest stays the same (compare with `diff` against the backup).
- [x] 3.2 Call `UiImage::Init()` before `initscr()` and `UiImage::Cleanup()` before `endwin()` in `src/ui.cpp`. Verify that exiting with `Ctrl-q` leaves no images on screen.
- [x] 3.3 In `UiHistoryView::Draw`: `ClearPlacements()` at the start; for each downloaded image attachment call `Prepare` with the panel width and insert `rows` reserved lines after the `📎` line; record the visible row and clipped rows; after `wrefresh` call `PlaceAll`. Verify in a chat with photos that the thumbnail sits between the `📎` line and the caption.
- [ ] 3.4 Handle truncation of the first message (`maxMessageLines`): if the image does not fit, it is skipped and the `📎` line remains. Verify by selecting a message with a photo in a short terminal.
- [ ] 3.5 Extend the prefetch condition to auto-download image attachments when `UiImage::IsEnabled()`. Verify with `attachment_prefetch=1` that visible, unselected photos are downloaded and a PDF is not.
- [x] 3.6 Call `ClearPlacements()` in the `UiDialog` constructor and in `TerminalControlPause()`. Confirm that when a dialog closes the history is dirty and redrawn. Verify by opening and closing `Ctrl-s` (emoji) and the external editor: no thumbnails over the dialog, and they are redrawn on close.

- [x] 3.7 Retransmit images after a screen clear (`UiScreen`, `ReinitView`) and after re-entering the alternate screen (`TerminalControlResume`, protocol UI control), using the in-memory RGBA. Verify in Ghostty: after resizing only the height, closing a dialog with Esc or suspending/resuming, the images stay visible without restarting nchat.

## 4. Full verification

- [x] 4.1 Test in Ghostty with the real session: scroll with PageUp/PageDn, chat change with Tab, resize the window, WebP sticker with transparency and a panoramic photo in a narrow panel. No graphics leftovers or garbage keys in the input.
- [ ] 4.2 Test the fallback: with `inline_images_enabled=0`, inside tmux and with a corrupt `.jpg`, only the `📎` line is shown, without empty rows, and the error appears in `log.txt`.
- [x] 4.3 Verify that `nchat-status` (`~/.local/bin/nchat-status`) still works and that there were no schema changes in `db.sqlite`.
