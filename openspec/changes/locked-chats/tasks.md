## 1. Go y protocolo

- [x] 1.1 `protocol.h`, `wmchat.{h,cpp}`, `cgowm.go`: `UpdateLockedNotify{chatId, isLocked}`, `CheckSecretCodeRequest{code}` y `CheckSecretCodeNotify{result}`.
- [x] 1.2 `gowm.go`: `lock` y `setting_chatLock` en `events.AppState` (LID con `GetChatId`); `chat_lock` atómico con 0600.
- [x] 1.3 Descarga completa de `regular_low` una vez (marca `locked_synced`), en goroutine tras conectarse.
- [x] 1.4 `WmCheckSecretCode`: PBKDF2 según `transformer`/`transformerArg`, comparación en tiempo constante; sin código en el log.

## 2. UI

- [x] 2.1 `m_LockedChats` cargado y guardado en `<profile>/locked_chats`; `m_LockedVisible`; `IsChatHidden`.
- [x] 2.2 Ocultar en lista, Tab, no leídos, `goto_chat`, Alt-n y open-by-link ("This chat is locked."); cambiar de chat si el actual queda oculto.
- [x] 2.3 Sin notificaciones ni no leídos para restringidos; respuesta a estado sin cambio de chat si el autor está oculto.
- [x] 2.4 Alt-l (`toggle_locked`): diálogo de código enmascarado, resultado por notify, avisos; sección `▾ LOCKED` en `UiListView`.

## 3. Documentación

- [x] 3.1 `uikeyconfig.cpp`, ayuda y README: `toggle_locked`, `locked_chats`, `chat_lock`.

## 4. Verificación

- [x] 4.1 Sandbox sin red: `locked_chats` de prueba oculta los chats en lista, Tab, `goto_chat` y Alt-n; mensaje entrante sin notificación; Alt-l con `chat_lock` de prueba (código conocido) muestra y oculta; código incorrecto y sin código.
- [ ] 4.2 En vivo en Ghostty: la descarga completa encuentra los 13 restringidos (log con el conteo); Alt-l con el código real; restringir y quitar en el teléfono se refleja en nchat.
