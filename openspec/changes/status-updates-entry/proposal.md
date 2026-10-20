## Why

WhatsApp statuses already reach nchat as the `status@broadcast` chat ("Status Updates"), but they are sorted like any other chat, muted, mixed into "Others". They are not distinguished from conversations, and if "Others" is collapsed they are hidden. In WhatsApp, statuses live apart, at the top and outside the chat list.

## What Changes

- Statuses are shown as a fixed row at the top of the list, before the "Pinned" section and outside all sections.
- The row has its own style (by default, green text `#6fcfa8` on a subtle dark green `#173029`, in bold), different from the section titles.
- The row is selectable like a chat: with `Tab`/`Shift-Tab`, go to chat, or with the arrow in the list. When selected, the history shows the statuses as today.
- The row shows `*` when there are unseen statuses, like an unread chat.
- The row is not counted in any section and is never hidden when collapsing "Others" or "Archived".
- Pinning or archiving (`Alt-p`, `Alt-a`) on the row does nothing, because WhatsApp does not allow it.
- `status_broadcast=0` still hides statuses completely. With `1` (default) they remain muted.
- New options: `list_status_name` in `ui.conf`, and `list_color_status_fg/_bg` and `list_attr_status` in `color.conf`.

Out of scope: filtering statuses older than 24 hours, grouping statuses by contact, viewing statuses in a separate viewer, and publishing statuses.

## Capabilities

### New Capabilities
- `status-updates-entry`: fixed statuses entry at the top of the chat list, with its own style and outside the sections.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: new section for statuses, sorted first. Not counted in section titles or collapsing. `Alt-p`/`Alt-a` are ignored on it.
- `src/uilistview.cpp`: draws the statuses row with its style and name.
- `src/uiconfig.cpp`, `src/uicolorconfig.cpp`: new options.
- `README.md`: documents the options.
- No protocol or cache changes.
