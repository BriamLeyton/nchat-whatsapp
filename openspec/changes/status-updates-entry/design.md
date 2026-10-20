## Context

- **Protocol:** `GetChatId` in `gowm.go` already groups statuses into the `status@broadcast` chat, and when syncing contacts it is given the name "Status Updates". Today there are 1563 statuses in the user's cache.
- **Visibility:** `status_broadcast` (0 hidden, 1 muted, 2 visible) is applied with `IsChatForceHidden`/`IsChatForceMuted`.
- **Sections:** `GetChatSection` returns Pinned/Others/Archived. `SortChats` sorts by section and computes `m_SectionChatCount`. `IsChatHidden` hides chats in collapsed sections when there are chats in other sections. `UiListView::Draw` shows titles only if some chat is not in "Others".

## Goals / Non-Goals

**Goals:**
- Statuses first, outside the sections, with their own style.
- Reuse the existing chat: selection, history, unread and inline images keep working the same way.

**Non-Goals:**
- Filtering by 24 hours, grouping by contact, a separate statuses viewer, and publishing.

## Decisions

### 1. Own section sorted first
`ChatSectionStatus` is added to the enum as value 0, and the others move up by one: Status=0, Pinned=1, Others=2, Archived=3, Count=4.
- `GetChatSection` returns `ChatSectionStatus` for `status@broadcast`, before checking pinned or archived.
- `SortChats` leaves it first without changes, because it already sorts by section.
- `m_SectionCollapsed[Status]` is always `false`, and `SetSectionCollapsed` ignores it, as with Pinned.
- The per-section arrays (`m_SectionCollapsed`, `m_SectionChatCount`, counters and list titles) become size 4. Persistence uses option names, so it does not change.

*Rejected alternative:* a synthetic row outside `m_ChatVec`. It would require duplicating selection, history and unread handling.

### 2. Outside titles and collapsing
- **`IsChatHidden`:** returns `false` for Status. When computing "chats in other sections", Status is excluded, so that with only "Others" + statuses, "Others" is not hidden without a visible title.
- **`UiListView::Draw`:** titles are shown if there are chats in Pinned or Archived (before: "if some chat is not in Others"). The Status row never has a title.

### 3. Drawing
- **Name:** `list_status_name` (default `◉ Status`), instead of the contact's name.
- **Style:** `list_attr_status | list_color_status` across the whole row. If selected, `list_attr_selected` (default `reverse`) is added.
- **Unread:** the `*` indicator uses the same style color as the row, so as not to break the background.
- **Default colors:** `list_color_status_fg=0x6fcfa8`, `list_color_status_bg=0x173029` (a subtle dark green on dark backgrounds; WhatsApp's green `#25d366` turned out too loud), `list_attr_status=bold`. On terminals without hex colors they are approximated to the 256 palette, as with the ticks.

### 4. Pin and archive
`OnKeyPinChat` and `OnKeyArchiveChat` return without doing anything if the current chat is `status@broadcast`.

## Risks / Trade-offs

- **Renumbering the enum:** any code that uses section values as fixed numbers would break. Mitigation: they are only used by name (checked with grep), and persistence uses option keys.
- **Many old statuses in the history (1563):** all are shown, as today. The 24-hour filter is out of scope.
- **Contrast of `*` on green:** it is drawn with the row's color. If another color is preferred, it can be configured with `list_color_status`.
