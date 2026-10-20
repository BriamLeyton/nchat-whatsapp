## Why

Today, when entering message selection with `↑` from the text box, the selected message always ends up at the very bottom. Each `↑` shifts the whole history down by one message. The user loses sight of recent messages, and the chat "jumps" with every key press instead of behaving like a normal list.

## What Changes

- When selecting with `↑`, the history stays still and the selection moves up through the visible messages.
- The history only scrolls when the selection goes past an edge:
  - **Up:** the selected message enters fully from the top.
  - **Down, with `↓`:** the history scrolls down as needed to show it.
- When leaving the selection (`↓` on the last message), the history returns to the bottom as it does today.
- Actions that jump to a message outside the view (go to quoted, find, go to pinned) scroll the history to show it. If the message is already visible, nothing scrolls.
- `PgUp`/`PgDn` stay the same: they scroll the history and leave the selection.

## Capabilities

### New Capabilities
- `history-list-selection`: message selection in the history with list behavior (fixed view, moving selection).

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: new per-chat view scroll position, separate from the selected message index (`m_MessageOffset`, which is kept), and its reset on entering and leaving the selection.
- `src/uihistoryview.cpp`: draws from the view scroll position, highlights the selected message wherever it is, and adjusts the view if the selected message is not fully visible.
- No changes to protocol, cache or configuration.
