## Why

En el teléfono hay 13 chats restringidos ("Chats restringidos", Chat Lock). WhatsApp los oculta de la lista principal y no muestra sus notificaciones. En WhatsApp Web y Desktop solo aparecen después de escribir el código secreto.

nchat los muestra como chats normales, con vista previa y notificaciones. whatsmeow recibe la acción `lock` del app state (`LockChatAction{locked}`), pero no la procesa, y nchat la descarta en `events.AppState`.

Revisión sin red (MAC de índices de la sesión local): en `regular_low` hay 62 chats con una entrada `lock` (restringidos ahora o alguna vez) y el ajuste `setting_chatLock` con el código secreto. La sesión guarda solo los MAC, no los valores, así que hace falta descargar una vez el estado completo de `regular_low` para saber cuáles son los 13.

## What Changes

- **Sincronización del estado restringido**:
  - nchat procesa las acciones `lock` (`LockChatAction.locked`) y `setting_chatLock` (`ChatLockSettings`: `hideLockedChats` y `secretCode`) del app state, al llegar y en adelante;
  - una sola vez por perfil, al conectarse, descarga el estado completo de `regular_low` (`FetchAppState(regular_low, fullSync)`), como hace la app al recuperar la sincronización. Queda marcado en el perfil y no se repite;
  - los chats restringidos se guardan en `<profile>/locked_chats`, y el código secreto (solo su hash PBKDF2, sal e iteraciones, como lo envía WhatsApp) en `<profile>/chat_lock`. Así se conocen sin conexión.
- **Chats restringidos ocultos, como en WhatsApp Web**:
  - no aparecen en la lista de chats, ni en Tab/Shift-Tab, ni al saltar a no leídos, ni en Ctrl-f (`goto_chat`);
  - no aparecen en Alt-n. Un número o link de un chat restringido muestra "This chat is locked." en vez de abrirlo;
  - sus mensajes se reciben y se guardan como siempre, pero no generan notificación de escritorio, ni campana, ni cuentan como no leídos;
  - responder a un estado de un contacto con chat restringido envía la respuesta, pero no cambia al chat: se queda en Status con "Reply sent to X.".
- **Alt-l (`toggle_locked`) muestra u oculta los restringidos**:
  - si están ocultos, pide el código secreto en un diálogo con la entrada enmascarada (`*`). El código se compara localmente con el hash de WhatsApp y nunca se envía ni se guarda;
  - código correcto: aparece al final de la lista una sección `▾ LOCKED` con los chats restringidos, que se usan como cualquier chat;
  - código incorrecto: "Wrong secret code.";
  - si no hay código configurado en el teléfono: "Set a secret code for locked chats in WhatsApp on your phone." (WhatsApp Web tampoco los muestra sin código);
  - Alt-l con la sección visible la oculta. Al cerrar nchat vuelven a quedar ocultos;
  - si el chat abierto queda oculto, se cambia al primer chat de la lista.
- Restringir o quitar la restricción en el teléfono se aplica en nchat al momento. Desde nchat no se puede restringir ni quitar la restricción.

## Capabilities

### New Capabilities
- `locked-chats`: chats restringidos ocultos y sin notificaciones, visibles con el código secreto de WhatsApp en Alt-l.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`, `cgowm.go`: procesar `lock` y `setting_chatLock` en `events.AppState`; descarga completa de `regular_low` una vez; comprobar el código (PBKDF2) en Go.
- `lib/common/src/protocol.h`, `lib/wmchat/src/wmchat.{h,cpp}`: `UpdateLockedNotify`, `CheckSecretCodeRequest`/`CheckSecretCodeNotify`.
- `src/uimodel.{h,cpp}`: conjunto de chats restringidos persistido, filtro en lista, navegación, no leídos y notificaciones, Alt-l y diálogo de código, casos de Alt-n y respuesta a estados.
- `src/uilistview.cpp`: sección `▾ LOCKED`.
- `src/uikeyconfig.cpp`, `src/uihelpdialog.cpp`, `README.md`: `toggle_locked` (Alt-l).
