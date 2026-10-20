## Context

The attachment line is built in `UiHistoryView::Draw` (`src/uihistoryview.cpp`): `attachmentIndicator + name + status + mediaStr`. `attachmentIndicator` is a `static std::wstring` loaded from `attachment_indicator` in `ui.conf`. Further down, while drawing, the line gets the attachment color if it starts with that same prefix (`wline->rfind(attachmentIndicator, 0) == 0`). With one icon per category, that comparison no longer works.

File classification by extension already exists in `UiImage::IsImageFile`, `IsVideoFile` and `IsAudioFile` (`src/uiimage.cpp`). They decide the inline thumbnail and the duration probe. `MediaMeta::isPtt` (metadata sidecar) marks voice notes, and today the duration suffix adds `🎤` for them. Received messages only carry the file path; `FileInfo::fileType` (MIME) is only filled when sending, so it cannot be used for classification.

Unit tests (`tests/`) link only `libncutil`, so the new logic should live there.

## Goals / Non-Goals

**Goals:**
- Pure extension → category classification, with a unit test.
- Change only the attachment line of the history view; same layout, same color.

**Non-Goals:**
- Replacing `UiImage::IsImageFile` and friends: they keep controlling the inline thumbnail and the probe. Unifying them is left for another change.
- Passing the icons through `StrUtil::Textize` when `emoji_enabled=0`: the current `📎` is not converted either. Users can set ASCII icons in `ui.conf`.

## Decisions

### Emoji per category, not Nerd Font per extension
Standard emoji with default emoji presentation are used (`📷 🎬 🎵 🎤 📕 📄 📊 📦`). They take two columns in any modern terminal and do not need a special font. Emoji with default text presentation (`🖼 U+1F5BC`, `🗜 U+1F5DC`) are avoided, because without `U+FE0F` their width varies between terminals and breaks the wrap.
Rejected alternative: Nerd Font glyphs per extension (as eza or lf do). They give more detail, but look broken without the font. They can still be set by hand in the per-category options.
The user confirmed emoji per category and the default icon set. They also tried the Nerd Font `fa` and `md` sets and rejected them as looking flat.

### Classification in `FileUtil`
New `enum FileCategory` and `FileUtil::GetFileCategory(const std::string& p_Path)` in `lib/ncutil/src/fileutil.h/.cpp`. It takes the extension of `BaseName(p_Path)` (so a dot in a directory name does not count), lowercases it, and looks it up in a `static const std::map<std::string, FileCategory>` table. It returns `FileCategoryOther` if not found.
Rejected alternative: a new function in `UiImage`. It cannot be tested without linking the UI.

### Icon resolved in the view
In `uihistoryview.cpp`, a local function `GetAttachmentIndicator(FileCategory, bool isPtt)` returns the `std::wstring` for the icon plus a space. The icons are cached in a `static std::map<FileCategory, std::wstring>` built once from `UiConfig`, like the other `static` indicators in the file. If `isPtt` and the feature is enabled, the voice category wins. If the category icon is empty or `attachment_type_icons_enabled=0`, `attachment_indicator` is used.

### Voice note suffix
In the `meta->isPtt` branch of `mediaStr`, `🎤 ` is omitted when the feature is enabled, leaving `" " + duration`. With the feature disabled nothing changes.

### Link preview icon
The icon goes on the first line of the preview (the title, or the first line of the description), after `▏ `, not on the line with the URL text. The preview is the only thing that identifies the message as a link without parsing URLs, and a message can have text before the URL. It is a separate option, `link_preview_indicator` (default `🌐`, the emoji equivalent of `nf-md-web` that the user chose), because a link is not an attachment and `attachment_type_icons_enabled` does not control it. If there is no title, the description is wrapped with the preview width minus the icon width, so the first line does not overflow the panel.
Rejected alternative: icon at the start of the text line that contains the URL. It would require detecting URLs in the wrapped text, and messages without a preview would still look the same as today.

### Color by line index, not by prefix
The attachment line is always inserted at `wlines.begin()`, and nothing is inserted before it afterwards (the quote is inserted before it; the thumbnail, the preview and the reactions go after it). The prefix comparison is replaced with `isAttachment = hasAttachmentLine && (lineIndex == 0)`, where `hasAttachmentLine` is set when `fileStr` is inserted. This way any icon gets the color, and a caption that starts with `📷` no longer gets it by mistake.
Rejected alternative: compare against the set of all icons. It keeps the false positive with the caption.

## Risks / Trade-offs

- [A future change inserts a line before the attachment] → Index 0 stops being correct. Mitigation: a comment at the insertion point and in the color verification.
- [Emoji width differs between terminals] → Only emoji with default emoji presentation are used. Icons can be changed in `ui.conf`.
- [A sticker looks like an image `📷`] → Accepted. Stickers are `.webp` and there is no metadata that tells them apart without touching the protocol.

## Migration Plan

No migration: the new options have default values, and existing `ui.conf` files receive them when saved. To go back to the previous behavior: `attachment_type_icons_enabled=0`.
