## 1. Model

- [x] 1.1 Add `m_StatusFilterSenderId`, `m_StatusFilterVec`, `GetShownMessageVec` and the filter rebuild. Migrate to the shown list the readers of `m_MessageVec` that use `m_MessageOffset`, and `GetMessageVecLocked`.
- [x] 1.2 Rebuild the filter after changes to the statuses chat's messages, and remove it in `OnCurrentChatChanged`.
- [x] 1.3 `RequestMessages` with a filter: request blocks of 50 from the contact with `FetchSenderMessagesSync` until the filtered history is filled or the cache is exhausted. When the filter is removed, release the statuses loaded only for the filter.

## 2. Command and dialog

- [x] 2.1 `MessageCache::GetSenderLastTimesSync` (query grouped by `senderId`) and `MessageCache::FetchSenderMessagesSync`.
- [x] 2.2 `UiStatusContactDialog` (new) based on `UiListDialog`, with search.
- [x] 2.3 Key `status_filter=\33\165` and `UiModel::OnKeyStatusFilter`: remove the filter if active; otherwise filter by the selected author, or open the dialog without a selection. Do nothing outside statuses.

## 3. View and documentation

- [x] 3.1 `UiListView`: ` · <name>` on the statuses row when filtered.
- [x] 3.2 Help modal and README.

## 4. Verification

- [x] 4.1 Offline sandbox:
  - [x] `Alt-u` on a selected status filters and keeps the selection;
  - [x] `Alt-u` without a selection opens the dialog sorted by date, with search, and `Esc` does not filter;
  - [x] choosing a contact filters and loads their old statuses;
  - [x] `↑`/`↓`, `PgUp`/`PgDn` and open attachment on the filtered list;
  - [x] `Alt-u` again and changing chat remove the filter;
  - [x] RAM: filtering a contact with old statuses loads only theirs, and removing the filter releases them;
  - [x] the row shows `◉ Status · <name>`;
  - [x] `Alt-u` in another chat does nothing.
- [ ] 4.2 Live in Ghostty: a new status from another contact does not appear with the filter active, and one from the filtered contact does.
