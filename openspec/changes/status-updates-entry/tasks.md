## 1. Model

- [x] 1.1 Add `ChatSectionStatus=0` to the enum and renumber (Count=4). `GetChatSection` returns Status for `status@broadcast`. Adjust per-section arrays and `SetSectionCollapsed`.
- [x] 1.2 `IsChatHidden`: never hides Status, and excludes Status from the count of other sections.
- [x] 1.3 `OnKeyPinChat`/`OnKeyArchiveChat`: ignore on `status@broadcast`.

## 2. View and configuration

- [x] 2.1 Options `list_status_name=◉ Status`, `list_color_status_fg=0x6fcfa8`, `list_color_status_bg=0x173029`, `list_attr_status=bold`.
- [x] 2.2 `UiListView::Draw`: titles only if Pinned or Archived has chats. Status row without a title, with its name, style, selection and `*`.
- [x] 2.3 README: document the new options.

## 3. Verification

- [x] 3.1 Offline sandbox:
  - [x] the statuses row appears first, before `PINNED`;
  - [x] `Alt-o` collapses "Others" without hiding statuses, and the count does not include them;
  - [x] `Tab`/`Shift-Tab` go through the row, and the history shows the statuses;
  - [x] selected is distinguishable from unselected;
  - [x] `Alt-p`/`Alt-a` on the row do nothing;
  - [x] with `status_broadcast=0` it does not appear.
- [ ] 3.2 Live in Ghostty: a new status puts `*` without a notification, and viewing it removes it.
