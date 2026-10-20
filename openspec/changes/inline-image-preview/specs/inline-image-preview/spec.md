## Purpose

Show WhatsApp image attachments as thumbnails inside the chat history in terminals that support the kitty graphics protocol (e.g. Ghostty), keeping the attachment line as the fallback.

## ADDED Requirements

### Requirement: Inline rendering of image attachments
The system SHALL render as an inline thumbnail in the history every message attachment whose file is a supported image (JPEG, PNG, WebP, or the first frame of a GIF) when the feature is enabled and the terminal supports it. The thumbnail MUST appear immediately below the attachment line of the same message and above the message's text (caption).

#### Scenario: Received and downloaded photo
- **WHEN** the current chat contains a message with a downloaded JPEG photo and the message is within the visible area of the history
- **THEN** the history shows the line `📎 <file>.jpg`, below it the photo thumbnail, and below that the caption if one exists

#### Scenario: WebP sticker
- **WHEN** the current chat contains a downloaded and visible `.webp` sticker
- **THEN** the history shows the sticker as an inline thumbnail, keeping its transparency over the panel background

#### Scenario: Attachment that is not an image
- **WHEN** the message has a PDF, audio or video attachment
- **THEN** the history shows only the attachment line, as before this change

### Requirement: Thumbnail size
The thumbnail SHALL take at most `inline_image_max_height` terminal rows (default 10) and at most the width of the history panel. It MUST keep the aspect ratio of the original image and MUST NOT enlarge images smaller than that maximum.

#### Scenario: Large vertical photo
- **WHEN** a 3000×4000 pixel photo is shown with `inline_image_max_height=10`
- **THEN** the thumbnail takes exactly 10 rows and the proportional width in columns

#### Scenario: Panoramic photo in a narrow panel
- **WHEN** a 4000×1000 pixel photo is shown and the proportional width for 10 rows exceeds the panel width
- **THEN** the thumbnail is limited to the panel width and reduces its rows to keep the proportion

#### Scenario: Small image
- **WHEN** an image whose natural size fits in fewer rows than the maximum is shown
- **THEN** the thumbnail is drawn at its natural size without enlarging it

### Requirement: Fallback to the attachment line
When the image cannot be rendered, the system SHALL show only the attachment line (`📎 <file>` plus the status indicator), without reserving empty rows. This MUST apply when the terminal does not support the kitty graphics protocol, when `inline_images_enabled=0`, when the file cannot be read or decoded, and while the image is not downloaded.

#### Scenario: Terminal without graphics support
- **WHEN** nchat runs in a terminal that does not answer the kitty graphics protocol query
- **THEN** all image attachments are shown only as a `📎` line, as before this change

#### Scenario: Feature disabled
- **WHEN** `ui.conf` contains `inline_images_enabled=0`
- **THEN** no graphics sequence is emitted and attachments are shown only as a `📎` line

#### Scenario: Corrupt file
- **WHEN** the downloaded file of an image cannot be decoded
- **THEN** only the `📎` line is shown and the error is logged without interrupting the UI

#### Scenario: Image still downloading
- **WHEN** the image is in the downloading state
- **THEN** the line `📎 <file> ⇄` is shown and, when the download finishes, the history is redrawn with the thumbnail

### Requirement: Automatic download of visible images
The system SHALL automatically start downloading every downloadable image attachment that appears in the visible area of the history when the feature is enabled and the terminal supports it, regardless of the value of `attachment_prefetch`. Non-image attachments MUST keep respecting `attachment_prefetch`.

#### Scenario: Prefetch only selected
- **WHEN** `attachment_prefetch=1` and the user opens a chat with three visible photos not yet downloaded
- **THEN** the three photos start downloading and are shown as thumbnails when finished

#### Scenario: Unselected document
- **WHEN** `attachment_prefetch=1` and there is a visible PDF that is not selected
- **THEN** the PDF is not downloaded

### Requirement: Visual consistency with the text UI
Thumbnails SHALL be drawn only inside the history panel area and MUST NOT cover other panels or dialogs. When scrolling, changing chat, resizing the terminal or closing a dialog, thumbnails MUST be in the position of their corresponding message, with no leftovers from earlier drawings. A thumbnail that only partially fits at the top edge of the panel MUST be clipped to its visible part.

#### Scenario: Open a dialog
- **WHEN** thumbnails are visible and the user opens a dialog (e.g. the emoji or contact list) that overlaps the history
- **THEN** the thumbnails disappear while the dialog is open and reappear in their place when it closes

#### Scenario: Scroll in the history
- **WHEN** the user scrolls up one page in the history
- **THEN** the thumbnails of messages leaving the view disappear and those of messages entering it are drawn in their new position

#### Scenario: Chat change
- **WHEN** the user switches to another chat
- **THEN** no thumbnail from the previous chat remains on screen

#### Scenario: Image clipped at the top edge
- **WHEN** the top part of a thumbnail would be above the history panel
- **THEN** only the rows of the thumbnail that fall inside the panel are drawn

#### Scenario: Exiting nchat
- **WHEN** the user exits nchat
- **THEN** all images transmitted to the terminal are deleted and the terminal is left with no graphics leftovers

### Requirement: Configuration
The system SHALL read `inline_images_enabled` (boolean, default `1`) and `inline_image_max_height` (integer number of rows, default `10`) from `ui.conf`. The keys MUST be added to `ui.conf` with their default value when they do not exist, without modifying other existing keys.

#### Scenario: First run after updating
- **WHEN** the user starts the new version with an existing `ui.conf` that does not have the new keys
- **THEN** `ui.conf` includes `inline_images_enabled=1` and `inline_image_max_height=10`, and the rest of the configuration stays the same

#### Scenario: Custom height
- **WHEN** `inline_image_max_height=5`
- **THEN** no thumbnail takes more than 5 rows

### Requirement: Selection and opening of messages unchanged
Message selection, opening attachments, saving, replying and reactions SHALL behave as before. Each thumbnail MUST count as part of the message it belongs to for scrolling and selection.

#### Scenario: Open an image with a thumbnail
- **WHEN** the user selects a message with a thumbnail and uses the open-attachment key
- **THEN** the image opens with the configured external command, as before
