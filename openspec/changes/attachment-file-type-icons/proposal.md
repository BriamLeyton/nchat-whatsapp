## Why

Today every attachment in the history view starts with the same generic icon (`📎 factura.pdf`, `📎 IMG-2026.jpg`, `📎 backup.zip`). To tell what kind of attachment it is, you have to read the file name's extension. An icon per file type lets you recognize at a glance whether a message carries a photo, a video, audio, a voice note or a document, as WhatsApp does.

## What Changes

- The attachment line in the history view shows an icon based on the file's category, instead of `attachment_indicator` for all of them:
  - image `📷`, video `🎬`, audio `🎵`, voice note `🎤`, PDF `📕`, document `📄`, spreadsheet `📊`, archive `📦`.
  - Any other type (or no extension) keeps using `attachment_indicator` (`📎`).
- The category is decided by the file extension (case-insensitive). The voice note is recognized by the message's `isPtt` metadata.
- For voice notes the duration suffix stops repeating the microphone: `🎤 PTT-01.ogg 0:42` instead of `📎 PTT-01.ogg 🎤 0:42`.
- The first line of a link preview (the title, or the description if there is no title) starts with `🌐`: `▏ 🌐 Page title`. The icon is configured with `link_preview_indicator`; an empty value shows no icon.
- The color of the attachment line (`history_text_attachment_color`) is still applied with any icon.
- New options in `ui.conf`: `attachment_type_icons_enabled` (default `1`) and a configurable icon per category (`attachment_indicator_image`, `attachment_indicator_video`, `attachment_indicator_audio`, `attachment_indicator_voice`, `attachment_indicator_pdf`, `attachment_indicator_document`, `attachment_indicator_spreadsheet`, `attachment_indicator_archive`). With `attachment_type_icons_enabled=0` all attachments go back to using `attachment_indicator`.

Out of scope: Nerd Font icons per extension, detection by content (magic bytes) or by MIME, icons in other views (chat list, file dialogs), and distinguishing stickers from images.

## Capabilities

### New Capabilities
- `attachment-file-type-icons`: attachment-line icon based on the file type category, link preview icon, per-category configuration and fallback to the generic indicator.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- **UI code:** `src/uihistoryview.cpp` (choosing the attachment-line icon, the voice note suffix, the link preview icon and detection of the attachment line for the color) and `src/uiconfig.cpp` (new options).
- **New code:** extension → category classification in `lib/ncutil/src/fileutil.cpp/.h`, so it can be tested with the existing unit tests.
- **Tests:** new unit test in `tests/` for the classification.
- **Docs:** `README.md` (new `ui.conf` options).
- **Protocol and database:** no changes in `wmchat`, the SQLite schema or the cache.
