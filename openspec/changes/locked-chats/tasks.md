## 1. Go and protocol

- [x] 1.1 `protocol.h`, `wmchat.{h,cpp}`, `cgowm.go`: `UpdateLockedNotify{chatId, isLocked}`, `CheckSecretCodeRequest{code}` and `CheckSecretCodeNotify{result}`.
- [x] 1.2 `gowm.go`: `lock` and `setting_chatLock` in `events.AppState` (LID with `GetChatId`); atomic `chat_lock` with 0600.
- [x] 1.3 Full download of `regular_low` once (marks `locked_synced`), in a goroutine after connecting.
- [x] 1.4 `WmCheckSecretCode`: PBKDF2 according to `transformer`/`transformerArg`, constant-time comparison; no code in the log.

## 2. UI

- [x] 2.1 `m_LockedChats` loaded and saved in `<profile>/locked_chats`; `m_LockedVisible`; `IsChatHidden`.
- [x] 2.2 Hiding in the list, Tab, unread, `goto_chat`, Alt-n and open-by-link ("This chat is locked."); switch chat if the current one becomes hidden.
- [x] 2.3 No notifications or unread for locked chats; status reply without chat switch if the author is hidden.
- [x] 2.4 Alt-l (`toggle_locked`): masked code dialog, result by notify, notices; `▾ LOCKED` section in `UiListView`.

## 3. Documentation

- [x] 3.1 `uikeyconfig.cpp`, help and README: `toggle_locked`, `locked_chats`, `chat_lock`.

## 4. Verification

- [x] 4.1 Sandbox without network: test `locked_chats` hides chats in the list, Tab, `goto_chat` and Alt-n; incoming message without notification; Alt-l with a test `chat_lock` (known code) shows and hides; incorrect code and no code.
- [ ] 4.2 Live in Ghostty: the full download finds the 13 locked chats (log with the count); Alt-l with the real code; locking and unlocking on the phone is reflected in nchat.
