## Why

Attachments and links look poor in all chats (people, groups and statuses):
- **Videos:**
  - they show as a single `📎 XXXX.f4v` line with no preview, duration or size;
  - the `.f4v` extension comes from the system's MIME library, which puts it before `.mp4`.
- **Voice notes and audios:** they do not show their duration.
- **Images not downloaded** (download in progress, failed, or expired status): they only show `📎`, even though the message carries a thumbnail that requires no network.
- **Links:** they look like a bare URL, even though the message carries a title, description and thumbnail.
- **Text-only statuses:** they lose their background color.

All of that data comes inside the received message. Showing it does not generate any request to WhatsApp.

In the user's cache there are 1568 statuses: 939 images, 546 videos and 83 text or links.

## What Changes

- **Metadata without network:** when receiving a message from any chat, the protocol stores next to the attachment:
  - the embedded thumbnail (`<msgId>.thumb.jpg`);
  - a `<msgId>.meta` file with duration and size (video and audio), whether it is a voice note, link title and description, and background and text colors (statuses).
- **Videos:**
  - they are saved as `.mp4`;
  - the attachment line shows `▶ 1:15 · 4.2 MB` when metadata is available;
  - inline, a frame of the downloaded video is shown, generated locally with a configurable command (default `ffmpegthumbnailer`). If there is no frame, the embedded thumbnail is used.
- **Audio:** a voice note shows `🎤 0:42`, and another audio shows `▶ 3:10 · 4.5 MB`.
- **Fallback thumbnail:** an image or video not downloaded shows the embedded low-resolution thumbnail inline, instead of only `📎`. This does not trigger downloads, and it respects expired statuses.
- **Links:** below the text the title and description are shown, and the thumbnail inline at reduced size.
- **Text-only statuses:** they are drawn with their background and text colors.
- Messages received before this change have no metadata and look as they do today, except for the video frame, which only needs the downloaded file.

Out of scope:
- status prefetching and disk cleanup (change `status-prefetch-cleanup`);
- playing video or audio inside the terminal (opening still uses the external viewer, for example mpv);
- getting link previews by querying the web.

## Capabilities

### New Capabilities
- `media-preview`: preview of videos, audios, thumbnails, links and text statuses in all chats, without network traffic.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`:
  - `ExtensionByType` prefers `.mp4`;
  - the image, video, audio and extended text handlers write the thumbnail and the metadata.
- `src/uihistoryview.cpp`: video and audio line, inline image from frame or thumbnail, link preview, and text status color.
- New `src/mediainfo.{h,cpp}`: metadata reading and caching, and background frame generation.
- `src/uicolorconfig.{h,cpp}`: on-demand color pairs for ARGB colors.
- `src/uiconfig.cpp`: `video_thumbnail_command`.
- `README.md`.
- No changes to the protocol's C interface or to the cache schema.
