## Context

- **Messages in memory:** `m_MessageVec[profile][chat]` stores the ids from most recent to oldest, and `m_Messages` stores the messages. `m_MessageOffset` indexes `m_MessageVec`, and about 15 functions (history, selection, open, save, react, quote, …) read it together with the offset.
- **Loading:** `RequestMessages` asks the cache for blocks starting from the oldest message, according to the offset and the height of the history.
- **Statuses:** the `status@broadcast` chat has its own row in the list (`status-updates-entry`). Each message has a `senderId` (the status author).
- **Dialogs:** `UiListDialog` provides the list with search as you type. `UiGroupMemberListDialog` is an example of a contact list.

## Goals / Non-Goals

**Goals:**
- Filter without touching `m_MessageVec`, which is updated by the network and the cache.
- Make existing actions work without changing their logic, only the list they read.

**Non-Goals:**
- Filters in other chats, by date or by message type.

## Decisions

### 1. Shown list
- **State:** `m_StatusFilterSenderId` (empty = no filter) and `m_StatusFilterVec`, with the ids from `m_MessageVec` of statuses whose `senderId` matches.
- **`GetShownMessageVec(profile, chat)`:** returns `m_StatusFilterVec` if the chat is the statuses chat and a filter is set, and `m_MessageVec` in any other case.
- **Readers:** the functions that read `m_MessageVec` together with `m_MessageOffset`, and `GetMessageVecLocked` (history), switch to the shown list. Those that modify `m_MessageVec` (new messages, deletions, chat change) do not change, and afterwards rebuild the filter if applicable.
- **Rebuild:** walks `m_MessageVec` of the statuses chat. With 1500 statuses it is cheap, and it is only done with the filter active and when the statuses chat's messages change.

*Rejected alternative:* filtering in `UiHistoryView`. Actions on the selected message would still index the full list and would point to the wrong message.

### 2. Apply and remove
- **From the selection:** the `senderId` of the selected message is taken, the list is rebuilt and `m_MessageOffset` becomes its index in the filtered list. The view (`m_HistoryViewOffset`) is set to the offset.
- **From the dialog:** offset 0 and selection inactive, to show the end.
- **On removal:** offset 0, selection inactive, and the `PgUp` stack is emptied.
- **On chat change:** `OnCurrentChatChanged` removes the filter if it was active.

### 3. Loading only the contact's statuses
Measured in the sandbox: the 1554 cached statuses (since Sep 30) take about 3.4 MB of RAM (~2.2 KB each), and about 100 arrive per day. Loading blocks of all statuses to fill a filter would grow without limit over time.

So, with the filter active:
- **Query by author:** `MessageCache::FetchSenderMessagesSync(profile, chat, senderId, beforeTime, limit)` fetches only the contact's statuses, older than the last filtered one, in blocks of 50.
- **Out of sequence:** they are inserted into `m_Messages`/`m_MessageVec` without moving `m_OldestMessageId`, the same way quoted messages are handled today. Normal chat loading does not change.
- **Filling:** `RequestMessages` of the statuses chat with a filter requests blocks until the filtered list reaches `offset + height`, or until a block returns fewer than 50.
- **Release:** when the filter is removed, the statuses loaded only for the filter that are outside the sequentially loaded range are deleted from memory. RAM returns to what it was before filtering.

### 4. Contacts dialog
- **Data source:** the cache, not memory, so that statuses not yet loaded are included. New synchronous query `MessageCache::GetSenderLastTimesSync(profile, chat)`: `SELECT senderId, MAX(timeSent) FROM messages WHERE chatId=? GROUP BY senderId ORDER BY 2 DESC`.
- **Class:** `UiStatusContactDialog : UiListDialog`. It shows contact names (`GetContactNameLocked`) and filters as you type, like `UiGroupMemberListDialog`.
- **Without the mutex:** it is opened without the model's mutex, like the other dialogs.

### 5. Indicator and key
- **Statuses row:** with a filter, `UiListView` appends ` · <name>` after `list_status_name`, cut to the width.
- **Key:** `status_filter=\33\165` (`Alt-u`, free). It is added to the help modal under "Selected message", as "Filter status by contact", and to the README.

## Risks / Trade-offs

- **Readers not migrated:** a function that still reads `m_MessageVec` with the filtered offset would point to the wrong message. Mitigation: all uses of `m_MessageVec` together with `messageOffset` are reviewed with grep, and the verification includes opening, reacting and jumping to the quoted message with the filter active.
- **Synchronous queries:** the dialog and the per-author loading query the cache with the model's mutex held. With 1500 rows and blocks of 50 they take milliseconds.
- **Cache growth:** the statuses cache grows by ~100 per day and is not pruned. This does not affect RAM except when scrolling through the whole statuses history without a filter, as in any chat. Pruning is left for another change.
- **New messages with a filter:** a new status from the filtered contact shifts the selection one place, the same as today without a filter.
