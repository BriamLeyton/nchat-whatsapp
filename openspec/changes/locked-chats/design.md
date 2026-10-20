## Context

- whatsmeow delivers each app state mutation as `events.AppState{Index, SyncActionValue}`. There is no dedicated event for `lock`. nchat only logs it in `gowm.go` (`case *events.AppState`).
- `lock`: index `["lock", <jid>]`, value `LockChatAction{locked}`. The jid may be a LID; `GetChatId` converts it to nchat's chat id, as in `HandleArchive`.
- `setting_chatLock`: index `["setting_chatLock"]`, value `ChatLockSettings{hideLockedChats, secretCode}`. `secretCode` is `UserPassword{encoding, transformer (PBKDF2_HMAC_SHA512 or SHA384), transformerArg (salt, iterations), transformedData}`.
- Both are in the `regular_low` collection (verified by MAC without network).
- `FetchAppState(ctx, name, fullSync=true, onlyIfNotSynced=false)` downloads the snapshot again and emits all mutations with `FromFullSync`.
- Archived and pinned chats are already received and persisted per chat. The set of seen statuses is persisted in a profile file (`status_seen`).

## Decisions

### 1. Go
- In `events.AppState`:
  - `lock`: `CWmUpdateLockedNotify(connId, GetChatId(jid), locked)`;
  - `setting_chatLock`: `<profile>/chat_lock` is written (serialized `ChatLockSettings` proto, atomic write) and `CWmUpdateLockedNotify(connId, "", hasCode)` is sent so the UI knows a code exists.
- Full download: after `events.Connected`, if `<profile>/locked_synced` does not exist, in a goroutine `FetchAppState(regular_low, true, false)` with `EmitAppStateEventsOnFullSync` and the `ResyncAppState` mutex; when it finishes without error, `locked_synced` is created.
  - If the server snapshot does not verify (`ErrMismatchingLTHash`), a recovery snapshot is requested from the phone (`RequestAppStateRecovery`, like WhatsApp Web). When it arrives (`AppStateSyncComplete{Recovery}` of `regular_low`) its events are emitted and `locked_synced` is created.
  - With another error, or with no response from the phone, it is retried on the next connection.
- `WmCheckSecretCode(connId, code)`: reads `chat_lock`, derives PBKDF2 with the hash, salt and iterations from `transformerArg`, and compares with `transformedData` in constant time. The code is encoded according to `encoding` (UTF-8). Result: 0 correct, 1 incorrect, 2 no code. It is answered with `CheckSecretCodeNotify`.
- The exact keys of `transformerArg` and the encoding are confirmed in the implementation with the real value (without printing the hash to the log).

### 2. UI persistence
- `<profile>/locked_chats`: one chat id per line, rewritten on change. It is loaded at startup, before drawing the list.
- `m_LockedChats[profileId]` (set) and `m_LockedVisible` (session, starts as `false`).

### 3. Hiding
- `IsChatHidden(profileId, chatId)` = locked and `!m_LockedVisible`.
- It is applied to: building the list and its sections (`UiListView`), `next_chat`/`prev_chat`, jump to unread, `goto_chat`, Alt-n (contacts with a locked chat) and open-by-link (notice "This chat is locked.").
- Notifications: when receiving messages, `DesktopNotify`, the bell and the unread count (terminal title and Status row) are skipped for locked chats, even if visible.
- Status reply: if the author's chat is hidden, there is no chat switch and the notice "Reply sent to X." is shown in Status.
- If the current chat becomes hidden (when a `lock` arrives or when hiding with Alt-l), the first visible chat is selected.

### 4. Alt-l
- Hidden: if there is no `chat_lock` with a code, a notice to set the code. If there is, a "Secret code" dialog with masked input; Enter sends `CheckSecretCodeRequest`. Nothing is shown while it is being verified; the result arrives through `CheckSecretCodeNotify`.
- Correct: `m_LockedVisible = true`, the list shows `▾ LOCKED` at the end (after Archived). Visible: Alt-l hides without asking anything.
- The code lives only on the dialog stack and in the request; it is not logged or stored.

## Risks / Trade-offs

- The full download of `regular_low` also re-emits archived, pinned and other settings in that collection. The current handlers are idempotent (they apply the current state).
- It is a request to WhatsApp, the same as the app's recovery request. It is done once per profile.
- `chat_lock` contains the hash of the secret code, which could be attacked by brute force (short codes). It is stored with 0600 permissions, the same as the session, which already contains the account keys.
- `hideLockedChats` in `ChatLockSettings` is stored but does not change behavior: on this linked device they are always hidden, as in WhatsApp Web.
