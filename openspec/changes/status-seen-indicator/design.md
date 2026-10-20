## Context

- `UpdateChatInfoIsUnread` decides a chat's `*` according to the `isRead` of the most recent message.
- `UiHistoryView` draws the sender's name in the header line of each message (`GetContactNameLocked`). In the same place it calls `MarkReadLocked` with `p_Final`, and knows whether the message is the selected one (`isSelectedMessage`).
- Since `status-read-on-open`, `MarkRead` does nothing in Status except when opening.
- Other per-profile files already live in `FileUtil::GetApplicationDir() + "/profiles/" + profileId`, such as `saved_contacts`.

## Decisions

### 1. New status
A status is new if it meets all of these:
- it is incoming and not opened (`!isOutgoing && !isRead`);
- its `timeSent` is later than the profile's baseline;
- its id is not in the set of reviewed ones.

### 2. `status_seen` file
- Text format: the first line is `baseline <ms>`; the following ones are `<msgId> <timeSent>`.
- It is read (or created) when the Status row's mark is first computed, when loading chats, before new statuses arrive.
- If it does not exist, the baseline is the current time and the file is written, so statuses that already existed do not show as new.
- When writing, entries with `timeSent` older than 30 days are discarded, and the baseline moves up to that cutoff. A status older than 30 days is never shown as new.
- It is written with a temporary file and `rename`, like `saved_contacts`, each time a status is added.

### 3. Marking as reviewed on selection
- In `UiHistoryView`, with `p_Final`, if the chat is Status and the message is the selected one, `MarkStatusSeenLocked` is called.
- `MarkStatusSeen` only adds the id to the set, writes the file and recalculates the `*`. It does not touch `isRead` and sends nothing.
- The status line is drawn without `●` on the next refresh.

### 4. Mark on the Status row
- In `UpdateChatInfoIsUnread`, for Status: unread if any status in the chat is new, walking the loaded messages.
- `UiListView` draws the Status row with `status_new_indicator` instead of `unread_indicator` when it is unread.
- The rest of the function (archived, muted, notification) stays the same. Status does not notify, because of `status-updates-entry`.

### 5. Mark
- `status_new_indicator` in `ui.conf`, default `●`: followed by a space and placed before the name on each new status, and at the end of the Status row in the list, in place of the `*`.

## Risks / Trade-offs

- **Single device:** reviewed statuses are not synced with the phone. The phone keeps using its own seen state.
- **Only what is loaded:** the `*` is computed from the statuses loaded in memory. Old statuses that were not loaded do not count.
