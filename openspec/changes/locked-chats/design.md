## Context

- whatsmeow entrega cada mutación del app state como `events.AppState{Index, SyncActionValue}`. Para `lock` no hay evento propio. nchat solo la registra en `gowm.go` (`case *events.AppState`).
- `lock`: índice `["lock", <jid>]`, valor `LockChatAction{locked}`. El jid puede ser LID; `GetChatId` lo convierte al id de chat de nchat, como en `HandleArchive`.
- `setting_chatLock`: índice `["setting_chatLock"]`, valor `ChatLockSettings{hideLockedChats, secretCode}`. `secretCode` es `UserPassword{encoding, transformer (PBKDF2_HMAC_SHA512 o SHA384), transformerArg (sal, iteraciones), transformedData}`.
- Ambos están en la colección `regular_low` (verificado por MAC sin red).
- `FetchAppState(ctx, name, fullSync=true, onlyIfNotSynced=false)` vuelve a descargar el snapshot y emite todas las mutaciones con `FromFullSync`.
- Archivados y fijados ya se reciben y persisten por chat. El conjunto de vistos de estados se persiste en un archivo del perfil (`status_seen`).

## Decisions

### 1. Go
- En `events.AppState`:
  - `lock`: `CWmUpdateLockedNotify(connId, GetChatId(jid), locked)`;
  - `setting_chatLock`: se escribe `<profile>/chat_lock` (proto `ChatLockSettings` serializado, escritura atómica) y se avisa con `CWmUpdateLockedNotify(connId, "", hasCode)` para que la UI sepa que hay código.
- Descarga completa: tras `events.Connected`, si no existe `<profile>/locked_synced`, en una goroutine `FetchAppState(regular_low, true, false)` con `EmitAppStateEventsOnFullSync` y el mutex de `ResyncAppState`; al terminar sin error se crea `locked_synced`.
  - Si el snapshot del servidor no verifica (`ErrMismatchingLTHash`), se pide al teléfono un snapshot de recuperación (`RequestAppStateRecovery`, como WhatsApp Web). Al llegar (`AppStateSyncComplete{Recovery}` de `regular_low`) se emiten sus eventos y se crea `locked_synced`.
  - Con otro error, o sin respuesta del teléfono, se reintenta en la próxima conexión.
- `WmCheckSecretCode(connId, code)`: lee `chat_lock`, deriva PBKDF2 con el hash, sal e iteraciones de `transformerArg`, y compara con `transformedData` en tiempo constante. Codificación del código según `encoding` (UTF-8). Resultado: 0 correcto, 1 incorrecto, 2 sin código. Se responde con `CheckSecretCodeNotify`.
- Las claves exactas de `transformerArg` y la codificación se confirman en la implementación con el valor real (sin mostrar el hash en el log).

### 2. Persistencia en la UI
- `<profile>/locked_chats`: un chat id por línea, reescrito al cambiar. Se carga al iniciar, antes de dibujar la lista.
- `m_LockedChats[profileId]` (set) y `m_LockedVisible` (sesión, inicia en `false`).

### 3. Ocultar
- `IsChatHidden(profileId, chatId)` = restringido y `!m_LockedVisible`.
- Se aplica en: armado de la lista y secciones (`UiListView`), `next_chat`/`prev_chat`, salto a no leídos, `goto_chat`, Alt-n (contactos con chat restringido) y open-by-link (aviso "This chat is locked.").
- Notificaciones: en la recepción de mensajes se omiten `DesktopNotify`, campana y el conteo de no leídos (título de la terminal y fila Status) para chats restringidos, aunque estén visibles.
- Respuesta a estado: si el chat del autor está oculto, no hay cambio de chat y el aviso "Reply sent to X." se muestra en Status.
- Si el chat actual pasa a oculto (al llegar `lock` u ocultar con Alt-l), se cambia al primer chat visible.

### 4. Alt-l
- Oculto: si no hay `chat_lock` con código, aviso de configurar el código. Si hay, diálogo "Secret code" con entrada enmascarada; Enter envía `CheckSecretCodeRequest`. Mientras se verifica no se muestra nada; el resultado llega por `CheckSecretCodeNotify`.
- Correcto: `m_LockedVisible = true`, la lista muestra `▾ LOCKED` al final (después de Archived). Visible: Alt-l oculta sin pedir nada.
- El código vive solo en la pila del diálogo y en la petición; no se registra en el log ni se guarda.

## Risks / Trade-offs

- La descarga completa de `regular_low` re-emite también archivados, fijados y otros ajustes de esa colección. Los manejadores actuales son idempotentes (aplican el estado actual).
- Es una petición a WhatsApp, igual a la de recuperación de la app. Se hace una sola vez por perfil.
- `chat_lock` contiene el hash del código secreto, que se podría atacar por fuerza bruta (códigos cortos). Se guarda con permisos 0600, igual que la sesión, que ya contiene las llaves de la cuenta.
- `hideLockedChats` en `ChatLockSettings` se guarda pero no cambia el comportamiento: en este dispositivo vinculado siempre se ocultan, como en WhatsApp Web.
