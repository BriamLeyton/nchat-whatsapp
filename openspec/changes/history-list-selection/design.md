## Context

- `m_MessageOffset[profile][chat]` is the index (0 = most recent) of the selected message. When there is no selection, it is the message anchored at the bottom after `PgUp`.
- `UiHistoryView::Draw` draws from bottom to top starting at `m_MessageOffset`, and highlights only the first drawn message (`firstMessage`). That is why the selection always ends up at the bottom.
- `m_MessageOffset` is used by about 10 actions on the selected message (edit, delete, react, open, pin, …), in addition to jumps (quoted, find, pinned) and `PgUp`/`PgDn`, which also use `m_MessageOffsetStack`.
- The height of each message depends on the width, the reactions, the inline images and the `[...]` truncation. Only the view knows it.

## Goals / Non-Goals

**Goals:**
- Move the selection while the view stays fixed.
- Do not change the meaning of `m_MessageOffset`, which is always the selected message.

**Non-Goals:**
- Line-by-line scrolling.
- Mouse selection.
- Changing `PgUp`/`PgDn`.

## Decisions

### 1. Separate view scroll position
- **New state:** `m_HistoryViewOffset[profile][chat]`, the index of the message anchored at the bottom of the view.
- **`...Locked` getter and setter:** the view can adjust it during `Draw`.
- **Without an active selection:** the view uses `m_MessageOffset` as the anchor, as today, so `PgUp`/`PgDn` do not change.
- **When the selection is activated** (`↑` from the input): `viewOffset = messageOffset`. The initially selected message is the bottom one, and what is shown does not change.

*Rejected alternative:* making `m_MessageOffset` the view's anchor and adding a separate selection index. It would require changing all the actions on the selected message.

### 2. View adjustment in `Draw`
With the selection active, `Draw` does:
1. If `sel < view`, set `view = sel`, because the selected message is now below the view.
2. Draw from `view` and record whether the selected message was drawn in full (its header is at `y >= 0`) or was outside or cut at the top.
3. If it was not seen in full and `view < sel`, add 1 to `view` and redraw. Repeat until it is visible, or until `view == sel`, where today's `[...]` truncation applies.
4. Save `view` in the model.

The first condition covers scrolling down and the third covers scrolling up. That way the view moves the minimum amount. Each iteration only draws in memory (ncurses does not refresh until `wrefresh`), and in practice there are 1 or 2 iterations.

Today's `[...]` truncation is applied to the first drawn message. It stays the same, because it only affects the bottom message, which is the selected one only when `view == sel`.

Inline images are placed only from the final draw.

### 3. Highlight
`isSelectedMessage` changes from `firstMessage && selectActive` to `index == messageOffset && selectActive`.

### 4. Exit and jumps
- **When the selection is turned off:** `viewOffset` is ignored while there is no selection and is reset on the next activation, so it does not need to be cleared.
- **Jumps** (quoted, find, pinned) only change `m_MessageOffset`. The rule in point 2 shows them with minimum scrolling.
- **`PgUp` with an active selection:** before calculating, it sets `messageOffset = viewOffset`. That way it pages from what is visible, not from the selected message.

## Risks / Trade-offs

- **New messages while the selection is active:** indexes are counted from the most recent, so the view and the selection move down one message together. This already happens today with the selection, and the view stays consistent.
- **Redraw loop:** bounded by `sel - view` iterations. In the worst case, with a far jump, that is dozens of in-memory draws, which is acceptable.
