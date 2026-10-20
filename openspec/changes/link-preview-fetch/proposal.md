## Why

`media-preview` shows a link preview only if the message carries it. The following have no preview:
- **old links:** they were received before that change;
- **links sent from nchat:** nchat does not generate it; the official app builds it when sending;
- **links that arrive without a preview:** this happens when the sender did not generate one.

The only way to obtain it is to query the link's page and read its Open Graph tags (`og:title`, `og:description`, `og:image`). This is not traffic to WhatsApp, but it is traffic to the link's site.

## What Changes

- **Query only what is visible:** when showing a message with a link, without an attachment and without a known title or description, the page is queried in the background:
  - one query at a time, on a dedicated thread, so frames and probes are not slowed down;
  - title, description and image are taken from Open Graph. If they do not exist, `<title>` and `meta description` are used;
  - the image is downloaded and reduced to a JPEG thumbnail.
- **Cache:**
  - the result is stored in the same files as `media-preview` (`<msgId>.meta` and `<msgId>.thumb.jpg`), so each link is queried only once, also across restarts;
  - a link that fails is marked as queried and is not retried;
  - the same URL in several messages is queried only once per session.
- **Configuration:**
  - `link_preview_fetch` defines whose links are queried, because a link from an unknown sender may be for tracking:
    - `0`: never;
    - `1`: only contacts saved in the address book;
    - `2`: saved contacts and groups (default);
    - `3`: everyone, including unknown numbers;
  - links sent by yourself are queried with any value other than 0;
  - `link_preview_command` and `link_thumbnail_command` are configurable. By default they use `curl` and `ffmpeg`, with time and size limits.

Out of scope:
- generating the preview when sending, so the recipient sees it;
- executing JavaScript, or sites that require login to provide metadata (for example Instagram).

## Capabilities

### New Capabilities
- `link-preview-fetch`: link previews by querying the page, only for visible messages, with an on-disk cache.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`: write `<profile>/saved_contacts` with the ids of contacts saved in the address book (without changing the C interface).
- `src/mediainfo.{h,cpp}`:
  - link thread;
  - HTML parsing (meta tags and entities);
  - writing `.meta` and the thumbnail;
  - applying results on the UI thread.
- `src/uihistoryview.cpp`: request the query for the first link of a visible message.
- `src/uiconfig.cpp`: `link_preview_fetch`, `link_preview_command`, `link_thumbnail_command`.
- `README.md`: behavior and privacy notice.
