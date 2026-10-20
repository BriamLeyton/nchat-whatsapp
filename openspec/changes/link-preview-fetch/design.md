## Context

- **`MediaInfo`:**
  - `Get(profileId, msgId)` reads `.meta` and `.thumb.jpg` and caches them in memory;
  - a worker thread makes frames and probes, and its results are applied on the UI thread from `TakePosterUpdated()`.
- **View:** the link preview already exists. It uses `title` and `description` from `.meta`, and the thumbnail with a maximum height of 4 rows, for messages without an attachment.
- **Tests:**
  - `curl -A facebookexternalhit/1.1` on a `facebook.com/share/r/...` link returns `og:title`, `og:description` and `og:image`. Without that user agent, Facebook returns a login page;
  - the texts come with HTML entities (`&#xed;`, `&#xa0;`, `&amp;`).
- **Decoding:** `stb_image` does not decode WebP, which is common in `og:image`. For that reason the thumbnail is converted to JPEG with `ffmpeg`.

## Goals / Non-Goals

**Goals:**
- Query only what is drawn, once per link, without blocking the UI or the frame thread.
- Be disabled with a single option.

**Non-Goals:**
- Preview when sending.
- Pages that require JavaScript or login.

## Decisions

### 1. Trigger
- It is triggered in the final pass of `UiHistoryView::Draw`, for a message without an attachment whose `MediaMeta` has no `title`, no `description` and no `link_fetched`.
- The first `http://` or `https://` URL in the text is taken: it ends at a space or line break, and trailing `.,;:!?)]}>"'` characters are removed.
- `MediaInfo::RequestLinkPreview(profileId, msgId, url)` is called.
- Before that, it is filtered by origin according to `link_preview_fetch`:

  | Message origin | Minimum level |
  |---|---|
  | sent by yourself | 1 |
  | 1:1 chat or status from a saved contact | 1 |
  | group (`@g.us`), any sender | 2 |
  | unknown number, channel or other | 3 |

  With `0` nothing is queried. Statuses are evaluated by the sender.
- It is not traffic to WhatsApp, so the rule about expired statuses does not apply.

### 1b. Saved contacts
- `ContactInfo` does not distinguish an address-book contact from a public name (push name). A contact is saved if whatsmeow has `FullName` or `FirstName`.
- `GetContacts` in Go writes `<profile>/saved_contacts`, one id per line, with the phone id and its `@lid` alias. It is rewritten on every sync, via a temporary file and `rename`.
- `MediaInfo::IsSavedContact(profileId, userId)` reads the file and reloads it if its `mtime` changed. Without the file, nobody is saved: with level 1 only your own links are queried.

### 2. Link thread
- It is a separate thread and queue from the frames, one query at a time, because a query can take up to 8 s.
- Repeated requests are discarded by `profileId/msgId`.
- A `url → result` map is kept for the session. If a URL was already queried, the result is copied to the new message without network.

### 3. Query
- **HTML:** `link_preview_command`, with `%1` replaced by the URL in escaped single quotes. The output is read with `popen`, capped at 1 MB.
  - Default: `curl -s -L -m 8 --max-redirs 5 --compressed --proto =http,https --proto-redir =http,https -A 'facebookexternalhit/1.1' '%1' </dev/null 2>/dev/null`. No `| head`, so the `curl` exit code is preserved; the 1 MB cap is enforced by nchat.
  - The command runs in its own process group. It is killed on exit from nchat, so nchat does not wait up to 8 s.
- **Parsing** without dependencies:
  - `<meta ...>` tags are scanned and their `property` or `name` and `content` attributes are read, in any order and with single or double quotes;
  - **priority:** `og:title` > `twitter:title` > `<title>`; `og:description` > `twitter:description` > `description`; `og:image` > `twitter:image`;
  - the entities `&amp; &lt; &gt; &quot; &apos; &nbsp;`, the Latin-1 ones (`&oacute;`, `&iquest;`...), common typographic ones (`&hellip;`, `&mdash;`...) and numeric `&#NNN;` and `&#xHH;` are decoded to UTF-8;
  - whitespace is replaced with a space and trimmed;
  - the title is limited to 200 characters and the description to 500.
- **Thumbnail:** if there is an image, it is resolved against the URL when relative (`//host/...` or `/path`). `link_thumbnail_command` runs with `%1` = image and `%2` = `<msgId>.thumb.jpg`.
  - Default: `ffmpeg -nostdin -y -v error -protocol_whitelist http,https,tcp,tls -rw_timeout 8000000 -user_agent 'facebookexternalhit/1.1' -i '%1' -vf "scale='min(320,iw)':-2" -frames:v 1 '%2' >/dev/null 2>&1`.
  - Only `http` or `https` image URLs are accepted. An `og:image` such as `file:///...` is discarded.

### 4. Cache
- `<msgId>.meta` is written, adding the missing keys:
  - `title`, `description`, if they were obtained;
  - always `link_fetched=1`, also when the page has no metadata, so it is not retried.
- **Network error** (the command exits with a code other than 0, for example no connection or a nonexistent domain): it is not marked. It is retried in the next session, once per session. This way a startup without network does not leave links marked as failed.
- The result is queued and applied to the `MediaInfo` cache on the UI thread, with a redraw. It is the same pattern as the probes.
- `MediaMeta` gains `linkFetched` and `thumbPath`, the latter updated if the thumbnail was created.

### 5. Security and privacy
- **What is not done:** nothing from the page is executed. The URL is passed as a quoted argument, and single quotes are escaped.
- **Risk:** viewing a chat queries the visible links. The site sees the IP and learns that the link was viewed. For example, a tracking spam link would confirm that the number is active.
  - The origin filter limits it to saved contacts and groups by default.
  - The README warns about it. `link_preview_fetch=0` disables it and `3` opens it to everyone.
- **Default:** `link_preview_fetch=2`, at the user's request.

## Risks / Trade-offs

- **User agent:** `facebookexternalhit` makes Facebook and other sites return metadata. Some sites may respond differently. It is configurable.
- **Sites with login** (Instagram, some TikTok): no metadata. They are marked as queried and only the link is shown.
- **Disk:** a thumbnail of about 20 KB and a `.meta` per link seen.
