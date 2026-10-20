## Context

This is how the status of a message flows today (see proposal.md for the motivation):

- **Go (`gowm.go`):** `HandleReceipt` only processes `ReceiptTypeRead/ReadSelf` and `Played/PlayedSelf`, and calls `CWmNewMessageStatusNotify(connId, chatId, msgId, isRead)`. It ignores `ReceiptTypeDelivered` (`""`), and in groups any receipt from a participant marks the message as read.
- **New messages:** they arrive through `CWmNewMessagesNotify(..., isRead, isEdited)` from ~10 handlers (text, image, sticker, …). Each handler receives `isSyncRead`, which distinguishes history sync from live messages. Messages sent from nchat also pass through those handlers after `client.SendMessage` (with `isSyncRead=false`).
- **C++:** `ChatMessage.isRead` (bool) is stored in `messages.isRead`. `UiModel` applies `NewMessageStatusNotify` in memory, and `MessageCache` persists it with `UPDATE ... SET isRead`.
- **History:** `UiHistoryView::Draw` adds `read_indicator` to the header of **any** message with `isRead`.
- **Schema:** the cache has sequential migrations in `MessageCache` (today version 12). Message insertion reads the previous row to preserve reactions.
- **Colors:** `UiColorConfig` supports color names, `usercolor` and hex (`0xRRGGBB`, if the terminal allows redefining colors). In Ghostty hex works.

## Goals / Non-Goals

**Goals:**
- Receipt aggregation is the protocol's responsibility (Go). C++ only receives a final status per message.
- The status is monotonic in every layer: it never goes back, not with late receipts, not with re-inserts, not with history sync.
- Additive schema changes, without rewriting existing data.

**Non-Goals:**
- A "pending" clock, per-participant info, and statuses for Telegram/Signal (their `isRead` still maps to `read`).

## Decisions

### 1. `DeliveryStatus` enum with "unknown"
In `protocol.h`:

```cpp
enum DeliveryStatus { DeliveryStatusUnknown = -1, DeliveryStatusNone = 0, DeliveryStatusSent = 1, DeliveryStatusDelivered = 2, DeliveryStatusRead = 3 };
```

`ChatMessage.deliveryStatus` (default `Unknown`) and `NewMessageStatusNotify.deliveryStatus` are added. `isRead` is kept: received messages still use it for unread logic, and other protocols only send `isRead`.

In the UI, the effective status of an outgoing message is computed like this:
- `Unknown` → `isRead ? Read : Delivered` (inference for previous messages, decided with the user);
- any other value → that value.

*Rejected alternative:* encoding the status in `isRead` (0/1/2). It breaks the boolean semantics used in the unread calculation and in other protocols.

### 2. Receipt aggregation and persistence in Go (`receipts.db`)
New SQLite file `<profile>/receipts.db`, using the `mattn/go-sqlite3` driver already in use. whatsmeow's `session.db` is not touched.

Tables:
- `msgstatus(chatId, msgId, status, recipients, timeUpdated, PRIMARY KEY(chatId,msgId))`
- `participantreceipts(chatId, msgId, participant, status, PRIMARY KEY(chatId,msgId,participant))`

Flow in `HandleReceipt`:
1. Receipts of type `Sender`, `Retry`, `Inactive`, `PeerMsg`, `ServerError`, and receipts whose `Sender.User` is the user itself (other devices) are ignored.
2. Mapping: `Delivered("")` → 2; `Read`/`Played` → 3. `ReadSelf`/`PlayedSelf` are the user's own reads of received messages: they are emitted as today (`isRead=1`, status `Unknown`) and do not affect sent messages.
3. **Individual chat:** `status = max(current, new)`.
4. **Group:**
   - Upsert into `participantreceipts` with `max`.
   - `recipients` = number of group participants minus the user. It comes from `GetGroupInfo`, cached in memory per session, and is stored in `msgstatus` the first time so it does not change with later joins or leaves.
   - Aggregated status: `Read` if `count(status>=3) >= recipients`; otherwise `Delivered` if `count(status>=2) >= recipients`; otherwise `Sent`.
5. Only if the aggregated status goes up, `CWmNewMessageStatusNotify(connId, chatId, msgId, isRead, status)` is called.

Also, at startup, rows with `timeUpdated` older than 60 days are deleted to bound the size.

*Rejected alternative:* aggregating in C++ (`MessageCache`). It would mix WhatsApp-specific semantics (participants, own devices) into the generic layer and force passing participant IDs across the C bridge.

### 3. Initial status for new messages
`CWmNewMessagesNotify` adds a `deliveryStatus` parameter. In each handler:
- if `fromMe && !isSyncRead`: the status stored in `receipts.db` is used (covers the race where a receipt arrives before the message) or, if there is none, `Sent`;
- in any other case: `Unknown`.

So messages from history sync use the inference, and received messages have no status.

### 4. C++ monotonicity and schema migration
- **Migration 12 → 13:** `ALTER TABLE messages ADD COLUMN deliveryStatus INT DEFAULT -1;`. Existing messages stay `Unknown`; it is additive and does not break `nchat-status`.
- **Insertion:** when a message is re-inserted, for example when edited, the previous row is read (as is already done for reactions) and `max(previous, new)` is stored. `WmNewMessagesNotify`, on the edit path, already preserves `isRead`; `deliveryStatus` is preserved too.
- **Update:** `UPDATE messages SET deliveryStatus = MAX(COALESCE(deliveryStatus,-1), ?)`, plus `isRead = 1` when the status is `Read`, in the same deferred request that updates `isRead` today.
- **`UiModel`:** on receiving `NewMessageStatusNotify` it applies `max` in memory.

### 5. Representation in `UiHistoryView`
The indicator is drawn as a separate segment after `wheader`, with its own color (`history_receipt_*_color`) and the name attribute (normal or selected). It is only drawn if `msg.isOutgoing` and `timeSent != INT64_MAX` (not pending). The width calculation in `TrimPadWString` includes the indicator, so it does not overflow.

`read_indicator` is no longer used in the history. It is kept in `uiconfig` so existing configurations do not break, but it has no effect.

### 6. Configuration
- **`UiConfig`:** `receipt_sent_indicator=✓`, `receipt_delivered_indicator=✓✓`, `receipt_read_indicator=✓✓`.
- **`UiColorConfig`:** `history_receipt_{sent,delivered,read}_color_{fg,bg}`.
  - `fg` defaults are always hex: `0x8696a0` / `0x8696a0` / `0x53bdeb` (`blue` with 8 colors), and `bg` empty.
  - The default is not chosen based on the terminal, because it is saved in `color.conf` on first start and would stay fixed.
  - Instead, `GetColorId` approximates any hex to the nearest color in the xterm-256 palette when the terminal cannot redefine colors (e.g. tmux). Before, it was discarded with a warning.

## Risks / Trade-offs

- [`GetGroupInfo` is a network call per group] → Mitigation: it is done only for receipts of own messages in groups, with an in-memory per-session cache and `recipients` stored per message. If it fails, `recipients=1` is used (behavior similar to today) and a warning is logged.
- [Large groups or communities: many receipts and rows] → Mitigation: simple upserts with an index on the primary key and pruning at 60 days. In groups with more than 256 participants, aggregation is done without storing per participant (counts only) to limit rows.
- [Divergence from upstream because of schema 13] → Mitigation: the change is additive. If upstream uses version 13 for something else, it is renumbered on merge. Documented here.
- [Old unread messages that were never actually delivered will show as `✓✓`] → Trade-off accepted by the user (inference).
- [`ReceiptTypeDelivered` can arrive several times (recipient's multi-device)] → Mitigation: idempotent upsert per participant (user without device, `Sender.ToNonAD()`).

## Migration Plan

- On first start the 12 → 13 migration is applied automatically and `receipts.db` is created.
- Existing backup: `~/.config/nchat.bak-20261014`. It is advisable to make a new backup of `history/` before the first start with this version.
- Rollback: an earlier binary ignores the new column and `receipts.db`.
