## Why

Today nchat only distinguishes "read / unread" and shows it with a `✓` in the header of any message, even received ones. It is not possible to know whether a sent message reached the recipient's phone, or whether everyone in a group read it. WhatsApp uses three visual states (gray ✓, gray ✓✓, blue ✓✓) that the user already recognizes.

## What Changes

- **Sent** messages show their delivery status in the header, as WhatsApp does:
  - sent (accepted by the server): gray `✓`
  - delivered: gray `✓✓`
  - read (or played, for voice notes and "view once"): blue `✓✓`
- **Received** messages stop showing the read `✓`. **BREAKING** (visual): this changes the current behavior of `read_indicator` in the history.
- In groups, "delivered" and "read" are marked only when **all** other participants delivered or read it, as in WhatsApp.
- The "delivered" status is persisted in the message cache. Per-participant counts for groups are persisted to survive restarts.
- Sent messages from before this change (with no stored status): if they are read they show as read; otherwise they are assumed delivered (gray `✓✓`).
- New symbol options in `ui.conf` (`receipt_sent_indicator`, `receipt_delivered_indicator`, `receipt_read_indicator`) and color options in `color.conf` (`history_receipt_sent_color`, `history_receipt_delivered_color`, `history_receipt_read_color`), with WhatsApp's default colors. In terminals without hex colors, any hex color in `color.conf` is approximated to the 256-color palette.

Out of scope: a clock icon for messages pending send, a "message info" view with per-participant detail, and statuses for Telegram/Signal beyond their current read state.

## Capabilities

### New Capabilities
- `message-delivery-status`: delivery statuses of sent messages (sent, delivered, read), their aggregation in groups, their persistence and their visual representation in the history.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- **Go (`lib/wmchat/go/gowm.go`):** process `ReceiptTypeDelivered` in addition to read and played. Aggregate per participant in groups (group size via `GetGroupInfo`, cached). New file `receipts.db` in the profile directory for group counts.
- **C bridge (`lib/wmchat/src/wmchat.{h,cpp}`):** `WmNewMessageStatusNotify` adds a status parameter.
- **Common protocol (`lib/common/src/protocol.h`):** `ChatMessage` and `NewMessageStatusNotify` add the delivery status. `isRead` is kept for compatibility with Telegram and Signal.
- **Cache (`lib/ncutil/src/messagecache.cpp`):** schema migration 12 → 13 with the column `messages.deliveryStatus` (NULL for previous messages), and read/write of the field. The user's `nchat-status` script is not affected.
- **UI:** `src/uihistoryview.cpp` (indicator and color), `src/uimodel.cpp` (applying the received status), `src/uiconfig.cpp` and `src/uicolorconfig.cpp` (new options).
- **Reversibility:** the new column is ignored by earlier binaries. Rollback with the backup `~/.config/nchat.bak-20261014` or simply with the previous binary.
