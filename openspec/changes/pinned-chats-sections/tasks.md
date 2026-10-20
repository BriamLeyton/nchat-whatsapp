## 1. WhatsApp state recovery

- [x] 1.1 Add `ResyncAppState`, `RequestAppStateRecovery` and `SendAppStateWithResync` in `gowm.go`; use them in `WmPinChat` and `WmArchiveChat`; handle `events.AppStateSyncError` and `AppStateSyncComplete{Recovery}`. Verify it compiles and that `gofmt -l` is clean.
- [x] 1.2 Live: the log shows `recovered from phone`; the phone's pins appear in nchat; `Alt-p` pins in nchat and on the phone.

## 2. Configuration

- [x] 2.1 Add the keys to `ui.conf` (`list_pinned_header`, `list_others_header`, `list_archived_header`, `list_others_collapsed`, `list_archived_collapsed`), to `key.conf` (`toggle_others=\33\157`, `toggle_archived=\33\150`) and to `color.conf` (`list_color_header_fg/_bg`). Verify with `diff` on copies that only keys are added.

## 3. Model

- [x] 3.1 Include archived chats in `m_ChatVec` (`NewChatsNotify`, `UpdateArchivedNotify`, contact selection) and stop discarding their messages in `NewMessagesNotify`, without notifying them. Keep the current chat when archiving or unarchiving.
- [x] 3.2 Sort by section in `SortChats` (pinned, others, archived by time).
- [x] 3.3 Add `GetChatSection`, `IsChatHiddenLocked`, the persisted collapsed state, `OnKeyToggleOthers` and `OnKeyToggleArchived`, the navigation that skips hidden chats, and automatic expansion on unread, go-to-chat and contact selection.

## 4. View

- [x] 4.1 In `UiListView::Draw`: virtual rows with titles, `(N)` and `*` on collapsed sections, scrolling based on the selected row, no titles when only `Otros` has chats.
- [x] 4.2 Document `Alt-o`, `Alt-h` and the new keys in `README.md`.

## 5. Verification

- [x] 5.1 In the offline sandbox, with pinned and archived chats in a copy of the DB:
  - [x] the order and titles are correct;
  - [x] Tab skips titles and hidden chats;
  - [x] `Alt-o` and `Alt-h` collapse and expand, and the title shows `(N)` and `*`;
  - [x] `Ctrl-f` toward a hidden chat expands its section;
  - [x] the collapsed state survives a restart;
  - [x] without pinned or archived chats there are no titles.
- [ ] 5.2 Live in Ghostty:
  - [ ] `Alt-a` moves the chat between sections;
  - [ ] a message to an archived chat appears with `*` and without notification;
  - [ ] archiving or unarchiving from the phone is reflected live.
