## 1. Model

- [x] 1.1 `IsExpiredStatus` and local read marking in `MarkRead` (memory + `MessageCache::UpdateMessageIsRead`, no request).
- [x] 1.2 `DownloadAttachment` sends nothing for expired statuses. `GetMessageAttachmentPath` leaves a pending notice if there is no file, and open/save/view show it.
- [x] 1.3 `PreReact` rejects expired statuses with a notice.
- [x] 1.4 Send text, send file and edit in Status: notice, nothing sent, input kept.

## 2. View and documentation

- [x] 2.1 `UiHistoryView::Draw`: no download and no downloadable indicator for expired statuses.
- [x] 2.2 README.

## 3. Verification

- [x] 3.1 Offline sandbox, checking in the log that no `download file`, `mark message read` or `send reaction` appears for expired statuses:
  - [x] browsing old unread statuses marks them read and survives a restart;
  - [x] expired photos and videos without a file show `📎` without downloading;
  - [x] opening and saving an expired attachment show the notice;
  - [x] `Alt-s` on an expired status shows the notice;
  - [x] `Enter` and sending a file in Status show the notice and keep the text.
- [ ] 3.2 Live in Ghostty: a status from today sends read confirmation and downloads as before, and an already downloaded expired photo is shown inline (in tmux inline images are disabled).
