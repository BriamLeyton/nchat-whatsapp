## 1. Model

- [x] 1.1 Add `m_HistoryViewOffset` with `GetHistoryViewOffsetLocked`/`SetHistoryViewOffsetLocked`. Initialize it with `messageOffset` when the selection is activated. In `OnKeyPrevPage`, with the selection active, start from `viewOffset`.

## 2. View

- [x] 2.1 In `UiHistoryView::Draw`: anchor at `viewOffset` with the selection active (or at `messageOffset` without it), highlight by index, record whether the selected message was drawn in full, adjust `view` (`sel < view` → `view = sel`; not visible → `view+1` and redraw) and save the view. Place images only from the final draw.

## 3. Verification

- [x] 3.1 Offline sandbox:
  - [x] `↑` from the input does not move the history;
  - [x] repeated `↑` moves the selection up without moving anything until the top edge;
  - [x] past the edge, scrolls one message;
  - [x] `↓` goes back to the bottom edge and scrolls;
  - [x] `↓` from the last message exits and shows the end;
  - [x] `PgUp`/`PgDn` the same as before;
  - [x] message taller than the view truncated with `[...]`.
- [x] 3.2 Live in Ghostty: selection with inline images, jump to quoted, react and edit on the selected message.
