## Why

On the phone there are 13 locked chats ("Locked chats", Chat Lock). WhatsApp hides them from the main list and does not show their notifications. In WhatsApp Web and Desktop they only appear after entering the secret code.

nchat shows them as normal chats, with preview and notifications. whatsmeow receives the `lock` action from the app state (`LockChatAction{locked}`), but does not process it, and nchat discards it in `events.AppState`.

Review without network (MACs of the session's local indexes): `regular_low` has 62 chats with a `lock` entry (locked now or at some point) and the `setting_chatLock` setting with the secret code. The session only stores the MACs, not the values, so the full state of `regular_low` must be downloaded once to find out which are the 13.

## What Changes

- **Locked state synchronization**:
  - nchat processes the `lock` (`LockChatAction.locked`) and `setting_chatLock` (`ChatLockSettings`: `hideLockedChats` and `secretCode`) actions from the app state, when they arrive and afterwards;
  - once per profile, on connect, the full state of `regular_low` is downloaded (`FetchAppState(regular_low, fullSync)`), as the app does when recovering sync. It is marked in the profile and is not repeated;
  - locked chats are stored in `<profile>/locked_chats`, and the secret code (only its PBKDF2 hash, salt and iterations, as WhatsApp sends it) in `<profile>/chat_lock`. This way they are known without a connection.
- **Locked chats hidden, as in WhatsApp Web**:
  - they do not appear in the chat list, in Tab/Shift-Tab, when jumping to unread, or in Ctrl-f (`goto_chat`);
  - they do not appear in Alt-n. A number or link of a locked chat shows "This chat is locked." instead of opening it;
  - their messages are received and stored as usual, but do not generate a desktop notification, a bell, or count as unread;
  - replying to a status of a contact with a locked chat sends the reply, but does not switch to the chat: it stays in Status with "Reply sent to X.".
- **Alt-l (`toggle_locked`) shows or hides locked chats**:
  - if they are hidden, it asks for the secret code in a dialog with masked input (`*`). The code is compared locally with WhatsApp's hash and is never sent or stored;
  - correct code: a `▾ LOCKED` section appears at the end of the list with the locked chats, which are used like any other chat;
  - incorrect code: "Wrong secret code.";
  - if no code is configured on the phone: "Set a secret code for locked chats in WhatsApp on your phone." (WhatsApp Web does not show them without a code either);
  - Alt-l with the section visible hides it. When nchat is closed they are hidden again;
  - if the open chat becomes hidden, the first chat in the list is selected.
- Locking or unlocking on the phone is applied in nchat immediately. Locking or unlocking cannot be done from nchat.

## Capabilities

### New Capabilities
- `locked-chats`: locked chats hidden and without notifications, visible with the WhatsApp secret code in Alt-l.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`, `cgowm.go`: process `lock` and `setting_chatLock` in `events.AppState`; full download of `regular_low` once; check the code (PBKDF2) in Go.
- `lib/common/src/protocol.h`, `lib/wmchat/src/wmchat.{h,cpp}`: `UpdateLockedNotify`, `CheckSecretCodeRequest`/`CheckSecretCodeNotify`.
- `src/uimodel.{h,cpp}`: persisted set of locked chats, filter in list, navigation, unread and notifications, Alt-l and the code dialog, Alt-n cases and reply to statuses.
- `src/uilistview.cpp`: `▾ LOCKED` section.
- `src/uikeyconfig.cpp`, `src/uihelpdialog.cpp`, `README.md`: `toggle_locked` (Alt-l).
