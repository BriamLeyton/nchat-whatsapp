## 1. Classification by file type

- [x] 1.1 Add `enum FileCategory` (`Other`, `Image`, `Video`, `Audio`, `Pdf`, `Document`, `Spreadsheet`, `Archive`) and `FileUtil::GetFileCategory(const std::string& p_Path)` in `lib/ncutil/src/fileutil.h/.cpp`, with the extension table from the spec, using the lowercased extension of `BaseName`. Verify that `./make.sh` compiles without new warnings.
- [x] 1.2 Add `tests/test003.cpp` (registered in `tests/CMakeLists.txt`) covering: each category, uppercase extension (`Factura.PDF`), unknown extension (`firmware.bin`), no extension (`LEEME`) and a dot only in the directory (`/tmp/a.zip/LEEME`). Verify with `./make.sh tests` that it passes.

## 2. Configuration

- [x] 2.1 Add to the defaults in `src/uiconfig.cpp` `attachment_type_icons_enabled=1` and `attachment_indicator_image/video/audio/voice/pdf/document/spreadsheet/archive` with the emoji from the spec, as UTF-8 escapes like `attachment_indicator`. Verify that after starting and exiting, `~/.config/nchat/ui.conf` contains the new keys and the rest stays the same (`diff` against a backup).
- [x] 2.2 Document the new options in `README.md`, in the `ui.conf` list and with a section `### attachment_type_icons_enabled` next to `### attachment_indicator`. Verify by reading the diff.

## 3. History

- [x] 3.1 In `src/uihistoryview.cpp`, add the `static` map category → icon and choose the attachment-line icon using `GetFileCategory` and `meta->isPtt`, with fallback to `attachment_indicator` if the icon is empty or the feature is disabled. Verify in a real chat that a photo, a video, a PDF and a `.zip` show `📷`, `🎬`, `📕` and `📦`.
- [x] 3.2 Omit `🎤 ` from the duration suffix of voice notes when the feature is enabled. Verify that a voice note looks like `🎤 <file>.ogg 0:42` and, with `attachment_type_icons_enabled=0`, `📎 <file>.ogg 🎤 0:42`.
- [x] 3.3 Replace the prefix-based detection of the attachment line (`rfind(attachmentIndicator, 0)`) with a `hasAttachmentLine` flag and `lineIndex == 0`, with a comment about the insertion order. Verify that all attachment lines use `history_text_attachment_color` and that a caption starting with `📷` uses the normal color.

- [x] 3.4 Add `link_preview_indicator` (default `🌐`) to `src/uiconfig.cpp` and `README.md`, and prepend it to the first line of the link preview in `src/uihistoryview.cpp`, shrinking that line's width by the icon width. Verify in a real chat a preview with a title (`▏ 🌐 Title`), one without a title and, with `link_preview_indicator=`, that the preview looks as before.

## 4. Full verification

- [x] 4.1 Test with the real session: a non-downloaded attachment (`+`), downloading (`⇄`) and failed (`✗`) keep the status suffix; the inline thumbnail stays below the attachment line; opening and saving attachments still works. Test a custom icon (`attachment_indicator_pdf=[pdf]`) and an empty one (`attachment_indicator_image=`).
