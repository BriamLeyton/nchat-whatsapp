## Why

Since `status-read-on-open`, browsing statuses no longer marks them as seen. That is why the `*` on the Status row does not go away even when the user has reviewed the whole list.

Today the Status `*` is computed like in any chat, looking only at the most recent status:
- if the most recent one is never opened, the `*` stays forever, even after it expires;
- if only the most recent one is opened, the `*` goes away even though other statuses remain unopened.

In addition, inside Status there is no way to tell which statuses have already been reviewed in the list.

The user wants to see which statuses are new, and for them to stop being new when browsed with the cursor, without opening them.

## What Changes

- Each incoming status goes through three situations:
  - **new:** shown with `●` before the contact's name;
  - **reviewed:** no `●`. It becomes reviewed when the selection cursor selects it. It is local only: no read receipt is sent;
  - **opened:** actually seen with `open` or `open_msg`, with the read receipt if it is current, as today. An opened status does not carry `●` either.
- The Status row in the list shows the same `●` mark (instead of `*`) when there is at least one new status.
- Reviewed statuses are stored in `<profile>/status_seen`, so they do not become new again after a restart.
- The first time it runs, without `status_seen`, statuses that already existed count as reviewed, so the whole history is not marked.
- The mark is configurable with `status_new_indicator` in `ui.conf` (default `●`).
- Other chats do not change.

## Capabilities

### New Capabilities
- `status-seen-indicator`: new-status mark that is removed when the status is selected, also on the Status row.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: per-profile set of reviewed statuses with its file; `IsStatusNew`, `MarkStatusSeen`; `UpdateChatInfoIsUnread` uses the mark for Status.
- `src/uihistoryview.cpp`: `●` before the name on new statuses; mark the selected status as reviewed.
- `src/uilistview.cpp`: the Status row uses `status_new_indicator` instead of `unread_indicator`.
- `src/uiconfig.cpp`: `status_new_indicator`.
- `README.md`: `status_new_indicator` and the meaning of the mark on statuses and on the Status row.
