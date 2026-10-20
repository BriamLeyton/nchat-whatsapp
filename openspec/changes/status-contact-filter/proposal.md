## Why

The statuses row shows all statuses mixed together, and there are more than 1500 in the cache. To see one person's statuses you have to scroll through the whole history. WhatsApp groups statuses by contact.

## What Changes

- New command `status_filter` (default `Alt-u`), which only acts in the statuses chat:
  - **With a status selected:** filters the history to show only the statuses of whoever posted it.
  - **Without a selection:** opens a dialog with the contacts that have statuses, sorted by their most recent status, with search as you type. Choosing one applies the filter.
  - **With a filter active:** the same command removes the filter and all statuses are shown again.
- While the filter is active, the statuses row shows the contact: `◉ Status · Jane Doe`.
- The filter is removed when leaving the statuses chat.
- With the filter active, the selection, `PgUp`/`PgDn` and actions on the selected status (open, save, react, …) work on the filtered list.
- Outside the statuses chat, `Alt-u` does nothing.

Out of scope: filtering from a contact's chat, filtering other chats or groups by sender, and the 24-hour filter.

## Capabilities

### New Capabilities
- `status-contact-filter`: filter of the statuses history by contact, from the selected status or from a dialog.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: filter state, shown message list (filtered or full) for the history and the actions, loading more statuses when the filtered list is not enough, and the command.
- `src/uistatuscontactdialog.{h,cpp}` (new): contacts-with-statuses dialog, based on `UiListDialog`.
- `src/uihistoryview.cpp`, `src/uilistview.cpp`: use the shown list and the filter name.
- `src/uikeyconfig.cpp`, `src/uihelpdialog.cpp`, `README.md`: new key and its help.
- No protocol or cache changes.
