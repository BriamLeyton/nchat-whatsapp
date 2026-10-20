## 1. Implementation

- [x] 1.1 Configs `link_preview_fetch`, `link_preview_command` and `link_thumbnail_command`.
- [x] 1.2 `MediaInfo`: link thread and queue, per-session URL map, `RequestLinkPreview`, and `linkFetched` in `MediaMeta`.
- [x] 1.3 HTML parsing: meta tags, `<title>`, entities, whitespace and length limits; resolution of relative image URLs.
- [x] 1.4 Writing `.meta` and the thumbnail, results applied on the UI thread with a redraw.
- [x] 1.5 `UiHistoryView::Draw`: extract the first URL and request the query in the final pass.
- [x] 1.6 README with privacy notice and `link_preview_fetch` levels.
- [x] 1.7 Go: write `<profile>/saved_contacts` in `GetContacts`; `MediaInfo::IsSavedContact` with reload by `mtime`.
- [x] 1.8 Origin filter in `UiHistoryView::Draw` according to the level (own, saved contact, group, other).

## 2. Verification

- [x] 2.1 Unit test of the parsing with sample HTML (Open Graph, Twitter, `<title>`, entities, attributes in another order).
- [x] 2.2 Isolated test program, with network and without launching nchat (the WhatsApp session cannot be opened twice): the `MediaInfo` pipeline with a Facebook link and a YouTube link produces `.meta` and `.thumb.jpg`.
- [x] 2.3 Sandbox without network, with those `.meta` files already generated: the preview is shown, and on restart no queries are made (log).
- [x] 2.4 With `link_preview_fetch=0` there are no queries, and a page without metadata is not retried.
- [x] 2.6 Sandbox without network: with levels 1, 2 and 3, the log shows requests only for the allowed origins.
- [ ] 2.5 Live in Ghostty: old and sent links show the preview with an inline thumbnail.
