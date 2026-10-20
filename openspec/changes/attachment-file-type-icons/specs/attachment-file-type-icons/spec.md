## Purpose

Show in the history view's attachment line an icon based on the file type (image, video, audio, voice note, PDF, document, spreadsheet, archive), and an icon on the link preview, so the content of a message can be recognized without reading its extension or URL.

## ADDED Requirements

### Requirement: Attachment icon per file category
The system SHALL prefix the history view's attachment line with the icon of the file's category, followed by a space, instead of the generic indicator. The category MUST be decided by the file name's extension, case-insensitively. The categories and their default icons SHALL be:

| Category | Extensions | Default icon |
|---|---|---|
| image | jpg, jpeg, png, webp, gif, heic, heif, bmp | `📷` |
| video | mp4, f4v, m4v, mov, webm, 3gp, mkv, avi | `🎬` |
| audio | oga, ogg, opus, mp3, m4a, aac, wav, amr, flac | `🎵` |
| PDF | pdf | `📕` |
| document | doc, docx, odt, rtf, txt, md, ppt, pptx, odp | `📄` |
| spreadsheet | xls, xlsx, ods, csv | `📊` |
| archive | zip, rar, 7z, tar, gz, tgz, bz2, xz | `📦` |

#### Scenario: Photo
- **WHEN** the history view shows a message with the attachment `IMG-2026.jpg`
- **THEN** the attachment line is `📷 IMG-2026.jpg`

#### Scenario: Uppercase extension
- **WHEN** the history view shows a message with the attachment `Factura.PDF`
- **THEN** the attachment line is `📕 Factura.PDF`

#### Scenario: Video with duration and size
- **WHEN** the history view shows a video `VID-01.mp4` with duration 1:15 and size 4.2 MB
- **THEN** the attachment line is `🎬 VID-01.mp4 ▶ 1:15 · 4.2 MB`

#### Scenario: Download status
- **WHEN** the attachment `backup.zip` is not downloaded yet
- **THEN** the attachment line is `📦 backup.zip` followed by the `downloadable_indicator`, the same as before this change except for the icon

### Requirement: Voice note icon
The system SHALL show the voice note icon (default `🎤`) on the attachment line of a message marked as a voice note (push-to-talk), regardless of the file extension. In that case the duration suffix MUST be shown without the microphone, so the icon is not repeated.

#### Scenario: Voice note with duration
- **WHEN** the history view shows a voice note `PTT-01.ogg` of 0:42
- **THEN** the attachment line is `🎤 PTT-01.ogg 0:42`

#### Scenario: Audio that is not a voice note
- **WHEN** the history view shows an audio file `cancion.mp3` that is not a voice note and has duration 3:10
- **THEN** the attachment line is `🎵 cancion.mp3 ▶ 3:10` (with the size if it is known)

### Requirement: Fallback to the generic indicator
The system SHALL use `attachment_indicator` (default `📎`) as the icon when the file has no extension or its extension does not belong to any category.

#### Scenario: Unknown extension
- **WHEN** the history view shows a message with the attachment `firmware.bin`
- **THEN** the attachment line is `📎 firmware.bin`

#### Scenario: No extension
- **WHEN** the history view shows a message with the attachment `LEEME`
- **THEN** the attachment line is `📎 LEEME`

### Requirement: Icon configuration
The system SHALL read from `ui.conf` the option `attachment_type_icons_enabled` (default `1`) and one icon per category: `attachment_indicator_image`, `attachment_indicator_video`, `attachment_indicator_audio`, `attachment_indicator_voice`, `attachment_indicator_pdf`, `attachment_indicator_document`, `attachment_indicator_spreadsheet` and `attachment_indicator_archive`, with the default values from the previous requirements. With `attachment_type_icons_enabled=0` all attachments MUST use `attachment_indicator`, and the voice note suffix MUST keep the microphone as it was before this change. If a category's icon is empty, that category MUST use `attachment_indicator`.

#### Scenario: Custom icon
- **WHEN** `ui.conf` has `attachment_indicator_pdf=[pdf]` and the history view shows `factura.pdf`
- **THEN** the attachment line is `[pdf] factura.pdf`

#### Scenario: Feature disabled
- **WHEN** `ui.conf` has `attachment_type_icons_enabled=0` and the history view shows `IMG-2026.jpg` and a voice note `PTT-01.ogg` of 0:42
- **THEN** the attachment lines are `📎 IMG-2026.jpg` and `📎 PTT-01.ogg 🎤 0:42`

#### Scenario: Empty category icon
- **WHEN** `ui.conf` has `attachment_indicator_image=` and the history view shows `IMG-2026.jpg`
- **THEN** the attachment line is `📎 IMG-2026.jpg`

### Requirement: Link preview icon
The system SHALL prefix the first line of a link's preview with the `link_preview_indicator` icon from `ui.conf` (default `🌐`), followed by a space. That first line is the title, or the first line of the description if there is no title. The remaining lines of the preview MUST NOT have an icon. With an empty `link_preview_indicator` the preview MUST look as it did before this change. Preview lines with the icon MUST NOT exceed the panel width.

#### Scenario: Preview with title
- **WHEN** the history view shows a message with a link whose preview has the title `Ejemplo` and a one-line description
- **THEN** the preview looks like `▏ 🌐 Ejemplo` and below it `▏ <description>`

#### Scenario: Preview without title
- **WHEN** a link's preview has only a description
- **THEN** the first line of the description starts with `▏ 🌐 ` and the other lines with `▏ `

#### Scenario: Link icon disabled
- **WHEN** `ui.conf` has `link_preview_indicator=` and the history view shows a preview with the title `Ejemplo`
- **THEN** the preview looks like `▏ Ejemplo`

#### Scenario: Message with a link and no preview
- **WHEN** the history view shows a message with a link that has no preview title or description
- **THEN** the message looks the same as before this change, without an icon

### Requirement: Attachment line style
The attachment line SHALL still be shown with the color `history_text_attachment_color` with any icon. A message text line that starts with an icon emoji MUST NOT receive the attachment color.

#### Scenario: Color with category icon
- **WHEN** the history view shows `📕 factura.pdf`
- **THEN** the line uses the color `history_text_attachment_color`

#### Scenario: Text starting with the same emoji
- **WHEN** a photo's caption starts with `📷 en la playa`
- **THEN** the caption line uses the normal text color, not the attachment color
