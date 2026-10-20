## 1. Common protocol and cache

- [x] 1.1 Add the `DeliveryStatus` enum and the `deliveryStatus` field to `ChatMessage` and `NewMessageStatusNotify` in `lib/common/src/protocol.h`. Verify everything compiles (`make -j$(nproc)`).
- [x] 1.2 Add the 12 → 13 schema migration in `MessageCache` (`deliveryStatus INT DEFAULT -1`). Verify on a **copy** of `db.sqlite` that `pragma table_info(messages)` shows the column, that `version.schema=13`, that the row count did not change, and that `nchat-status` works against the copy.
- [x] 1.3 Read and write `deliveryStatus` in the message INSERT and SELECT statements, preserving `max(previous, new)` on re-insert. Add the monotonic update (`MAX(COALESCE(...))`) in the deferred status request. Verify with a test in `tests/` or a harness on the copy: insert as `Read` and re-insert as `Sent` leaves `Read`.

## 2. Go: receipts and aggregation

- [x] 2.1 Create `receipts.db` (tables `msgstatus` and `participantreceipts`, pruning at 60 days) when loading the profile in `gowm.go`. Verify the file exists in the profile after startup and the log has no errors.
- [x] 2.2 Rewrite `HandleReceipt`: map `Delivered/Read/Played`, ignore irrelevant types and own devices, emit `ReadSelf/PlayedSelf` as today, aggregate in groups with `recipients` (with `GetGroupInfo` cache), and emit only when the status goes up. Verify with `-ee` logs that sending a message to an individual chat produces `sent → delivered → read`.
- [x] 2.3 Add `deliveryStatus` to `CWmNewMessagesNotify`, `CWmNewMessageStatusNotify` and the C bridge (`wmchat.h/.cpp`), and update the ~10 call sites (`fromMe && !isSyncRead` → stored status or `Sent`; otherwise `Unknown`). Verify it compiles and that a message sent from nchat is stored with `deliveryStatus>=1` (query on `db.sqlite`).

## 3. UI

- [x] 3.1 In `UiModel`, on receiving `NewMessageStatusNotify`, apply `max` to `deliveryStatus` in memory (and `isRead` only if the status is `Read` or the notify carries `isRead`). Verify the header updates live without reopening the chat.
- [x] 3.2 Add the defaults of `receipt_*_indicator` in `uiconfig.cpp` and of `history_receipt_*_color` in `uicolorconfig.cpp` (hex if `can_change_color()`, otherwise `gray/blue`). Verify with `diff` against the backup that `ui.conf` and `color.conf` only gain the new keys.
- [x] 3.3 In `UiHistoryView::Draw`: remove `read_indicator` from the header; draw the status segment only on outgoing non-pending messages, with the color of their status and the inference for `Unknown`, without overflowing the width. Verify in Ghostty: received messages without `✓`, old sent messages `✓✓` gray or blue, and that a new message goes `✓` → gray `✓✓` → blue `✓✓`.

## 4. Full verification

- [x] 4.1 Group: send to a small group and verify it stays gray `✓✓` until **all** read, then turns blue, and that if nchat is restarted in between the aggregated status is preserved.
- [x] 4.2 Regression: the unread count in the chat list does not change, editing an own message does not reset its checks, and `nchat-status` still works.
