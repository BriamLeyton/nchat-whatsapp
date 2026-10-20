## Purpose

Show videos, audios, links and text statuses better in all chats, using only the data the message carries and local processes, without new requests to WhatsApp.

## ADDED Requirements

### Requirement: Metadata without network
When receiving a message from any chat, the system SHALL store next to the attachment, in the profile's `tmp` directory, the message's embedded thumbnail (`<msgId>.thumb.jpg`) and a file `<msgId>.meta`:
- video and audio: duration and size;
- audio: whether it is a voice note;
- extended text: link title and description;
- statuses: background color and text color.

A message without that data MUST NOT generate files.

Storing them MUST NOT generate network requests. If they already exist, they MUST NOT be overwritten.

#### Scenario: Video status arrives
- **WHEN** a 15 s, 4.2 MB video status arrives with a thumbnail
- **THEN** `<msgId>.thumb.jpg` and `<msgId>.meta` exist, with `seconds=15` and the size in bytes, and no download was made

#### Scenario: Link in a group
- **WHEN** a message with a link carrying a title and thumbnail arrives in a group
- **THEN** `<msgId>.thumb.jpg` and `<msgId>.meta` exist with the title

#### Scenario: Plain text
- **WHEN** a text message without a link arrives
- **THEN** no file is written

### Requirement: Video extension
The system SHALL save `video/mp4` videos with the `.mp4` extension.

#### Scenario: New video
- **WHEN** a `video/mp4` video arrives
- **THEN** its file path ends in `.mp4`, not `.f4v`

### Requirement: Video and audio line
For a video or audio attachment with metadata, the attachment line SHALL show `▶` with the duration in `m:ss` and the size in MB with one decimal, when they exist. A voice note SHALL show `🎤` with the duration. Without metadata it SHALL look as it does today.

#### Scenario: Video with metadata
- **WHEN** a 75 s, 4 404 019 byte video is shown, in a chat or in statuses
- **THEN** the line is `📎 <file> ▶ 1:15 · 4.2 MB`

#### Scenario: Voice note
- **WHEN** a 42 s voice note is shown
- **THEN** the line is `📎 <file> 🎤 0:42`

#### Scenario: Old message
- **WHEN** a video received before this change is shown
- **THEN** the line is the same as today

### Requirement: Video frame
With inline images active, a downloaded video from any chat SHALL be shown inline with a frame generated locally by `video_thumbnail_command`:
- generation MUST happen outside the UI thread, and the history SHALL be redrawn when it finishes;
- if the command is empty or fails, the embedded thumbnail SHALL be used if it exists;
- a video that failed MUST NOT be retried in the same session.

#### Scenario: Downloaded video
- **WHEN** a downloaded video without a previous frame is shown
- **THEN** the UI keeps responding, `<file>.poster.jpg` is created and then shown inline

#### Scenario: Without ffmpegthumbnailer
- **WHEN** the command fails
- **THEN** the embedded thumbnail is shown, or only the line if there is no thumbnail, and it is not retried

### Requirement: Fallback thumbnail
An image or video not downloaded, from any chat, SHALL show its embedded thumbnail inline, if one exists. This MUST NOT trigger downloads, even for expired statuses.

#### Scenario: Expired photo not downloaded
- **WHEN** an expired status with a photo not downloaded and a stored thumbnail is shown
- **THEN** the thumbnail is shown inline and there is no download request

### Requirement: Link preview
A message from any chat with a link title or description SHALL show, below the text:
- a `▏` line with the title in bold;
- up to 2 `▏` lines with the description;
- the inline thumbnail, with a maximum height of 4 rows.

#### Scenario: Facebook link
- **WHEN** a message or status with a link carrying a title, description and thumbnail is shown
- **THEN** the URL, the title, the description and the thumbnail are shown

### Requirement: Text status color
A status without an attachment that has a background color SHALL draw its text lines with that background:
- the text color is the status's; if absent, white or black depending on the background's luminance;
- if no colors are available, or the color pair limit has been exceeded, it SHALL be drawn without color.

#### Scenario: Purple text status
- **WHEN** a text status with background `0xff8a2be2` is shown
- **THEN** its text lines have a purple background and the header looks normal
