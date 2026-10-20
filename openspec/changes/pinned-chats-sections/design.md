## Context

- **Pins and archived chats upstream:**
  - `Alt-p` and `Alt-a` send `PinChatRequest` and `ArchiveChatRequest`, which Go turns into app state patches (`regular_low`).
  - Incoming changes arrive as `events.Pin` and `events.Archive`.
- **Archived chats in the UI:**
  - `NewChatsNotify` and `UpdateArchivedNotify` remove them from `m_ChatVec`.
  - `NewMessagesNotify` discards their messages (early `return`).
  - `UpdateChatInfoIsUnread` does not notify.
  - `chats2.isArchived` persists the state.
- **Order:** `UiModel::SortChats` puts pinned chats first and then sorts by mute and time.
- **Drawing:** `UiListView::Draw` draws one row per chat in `m_ChatVec`, centering the selection.
- **Fixed failure (commit `3f567a41`):**
  - The local `regular_low` was at v334 and the server snapshot at v347; both failed with `mismatching LTHash`.
  - Recovery from the phone (`COMPANION_SYNCD_SNAPSHOT_FATAL_RECOVERY`) worked: it brought 1 pinned chat and 74 archived, and the following patches now apply (v348).

## Goals / Non-Goals

**Goals:** three collapsible sections without restructuring `m_ChatVec`, which about 40 functions use; archived chats visible without notifications.

**Non-Goals:** local-only pinning, manual ordering among pinned chats.

## Decisions

### 1. State recovery (implemented)
- **`ResyncAppState`:**
  - Runs `FetchAppState(name, fullSync=true)` with `EmitAppStateEventsOnFullSync` active, so the events update the UI, and then restores the previous value.
  - A mutex serializes resyncs, and it is skipped if the collection was resynced less than 30 s ago.
- **If the snapshot fails with LTHash**, `RequestAppStateRecovery` sends `BuildAppStateRecoveryRequest` to the phone, once per session and collection, and keeps the flag active until `AppStateSyncComplete{Recovery: true}`.
- **`SendAppStateWithResync`** retries the pin or archive patch once after a successful resync.

### 2. Archived chats inside `m_ChatVec`
- **Entry into the list:** the `continue`/`erase` that exclude them in `NewChatsNotify` and `UpdateArchivedNotify` are removed. When archiving or unarchiving, the list is re-sorted with `SortChats` and `m_CurrentChat` is kept, recalculating `m_CurrentChatIndex`.
- **New messages:** the `return` in `NewMessagesNotify` is removed for archived chats. The current logic in `UpdateChatInfoIsUnread` already prevents notifying archived chats.
- **Order in `SortChats`:** the primary key is the section, `Pinned=0`, `Others=1` and `Archived=2`. A chat that is both pinned and archived counts as pinned (WhatsApp does not allow it, but it is covered). Inside `Archived` it is sorted by time only, ignoring mute.

*Rejected alternative:* a separate vector for archived chats. It would require changing every function that indexes `m_ChatVec`.

### 3. Section and visibility in `UiModel`
- **`GetChatSection(profileId, chatId)`** returns `Fijados`, `Otros` or `Archivados`.
- **`IsChatHiddenLocked(index)`** returns true if the chat's section is collapsed.
- **Navigation:** `OnKeyNextChat` and `OnKeyPrevChat` skip hidden chats, with wrap-around as today. If all are hidden, they do nothing.
- **`OnKeyToggleOthers` and `OnKeyToggleArchived`:** invert the state, save it with `UiConfig::SetBool` and, if the current chat becomes hidden, select the nearest visible one.
- **Automatic expansion:** `OnKeyUnreadChat`, `goto_chat` and contact selection, when their target is hidden, expand its section.
- **Behavior when only `Otros` has chats:** collapsing has no effect, so the list is not left empty without titles.

### 4. Virtual rows in `UiListView`
`Draw` builds rows `{title(section, total, hasUnread) | chat(index)}` walking `m_ChatVec`, which is already grouped by section:
- a title is inserted when the section changes, except when the only section with chats is `Otros`;
- collapsed sections contribute no chat rows, and their title shows `(N)` and `*`.

Scrolling centers on the row of the selected chat. If there is no visible selection, the first row is used. Titles use `list_color_header` with `A_BOLD`, and their `*` uses `list_color_unread`.

### 5. Keys
`toggle_others` uses `\33\157` (Alt-o) and `toggle_archived` uses `\33\150` (Alt-h). Both combinations are free in the defaults of `uikeyconfig.cpp`.

## Risks / Trade-offs

- **74 more archived chats in the list.** Mitigation: `Archivados` starts collapsed.
- **Behavior change: archived chats' messages are now loaded in the UI.** The memory cost is the same as for any chat; the history was already stored in the cache.
- **Recovery depends on the phone being online.** If it does not respond, everything stays as before (warning in the log) and it is retried in the next session.
- **WhatsApp limits pinned chats to 3.** The server rejects the fourth; it is recorded in the log.
