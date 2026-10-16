## Context

- `UiHistoryView` dibuja el historial de abajo hacia arriba en `m_PaddedWin`, y junta las imágenes inline en `imagePlacements` para ubicarlas después del refresco con `UiImage`.
- La barra inferior (`UiStatusView`) muestra nombre, presencia (`GetChatStatusLocked`) y, opcionalmente, número (`GetContactPhoneLocked`).
- Los miembros de grupo ya se piden y guardan en `m_GroupMembers` (`RequestGroupMembers`, `GetGroupMembers`).
- El filtro de Status por remitente (`SetStatusFilter`, `ClearStatusFilter`) carga desde la DB con `MessageCache::FetchSenderMessagesSync`.
- whatsmeow ofrece `GetProfilePictureInfo(jid, {Preview, ExistingID})`: devuelve la URL y el id de la foto, `nil` si el id no cambió, y errores conocidos si la foto está oculta o no existe.

## Decisions

### 1. Banner dentro del historial
- Las primeras filas de `m_PaddedWin` se reservan para el banner, así no cambia el layout de las demás vistas. El historial se dibuja en el alto restante.
- Alto: 3 filas en total, sin separador. La foto ocupa las 3 filas, con el ancho en columnas que corresponda, a la izquierda del texto.
- Filas de texto:
  - contacto: `nombre · número`; presencia; estados (vacía si no hay);
  - grupo: nombre; `N participants`; vacía.
- Las 3 filas se pintan con `history_banner_color` y `history_banner_attr`, para separarlas de los mensajes sin gastar una fila. Default: fondo como la barra de estado y nombre en negrita.
- La foto se agrega a `imagePlacements` como cualquier imagen inline. Sin foto, o sin soporte de imágenes, el texto empieza en la columna 0.
- Status no tiene banner. Con `chat_banner_enabled=0` no se reserva nada.

### 2. Foto de perfil
- Archivos: `<profile>/avatars/<chatId>.jpg` (foto), `<chatId>.id` (id de la foto o `none`).
- Al abrir un chat (`OnCurrentChatChanged`), y al conectarse para el chat abierto, si `.id` no existe o tiene más de 24 h (mtime), se envía `GetAvatarRequest{chatId, basePath}`. Go lee el id guardado de `.id`. Sin conexión no se envía. Se pide como máximo una vez por chat por sesión, aunque falle.
- En Go, `WmGetAvatar(connId, chatId, existingId, path)`:
  - `GetProfilePictureInfo` con `Preview: true` y `ExistingID`;
  - `nil`: no cambió, solo se renueva el mtime de `.id`;
  - error de foto oculta o inexistente: `.id = none` y se borra la foto;
  - URL: descarga con HTTP (timeout 10 s), escritura atómica de la foto y luego de `.id`;
  - luego `NewAvatarNotify{chatId}` para redibujar. Un error de red no escribe nada, y se reintenta en la próxima sesión.
- `UiImage` cachea por ruta, tamaño y celda. Para que una foto nueva reemplace la anterior, la clave de caché incluye el mtime del archivo (para todas las imágenes inline).

### 3. Presencia con color
- `GetChatPresence(profileId, chatId)` devuelve el tipo (`typing`, `online`, `away`, `seen`, ninguno) y el texto, con la misma lógica que hoy usa `GetChatStatus`. `GetChatStatus` pasa a usarla, así la barra inferior no cambia.
- Colores en `color.conf`: `history_banner_typing_color` (verde), `history_banner_online_color` (verde), `history_banner_away_color` (amarillo), `history_banner_seen_color` (gris). ` · muted` usa `history_banner_seen_color`.

### 4. Abrir la foto con Ctrl-v
- `OnKeyOpenAttachment` sin selección, en un chat con banner, abre la foto completa:
  - si existe `<chatId>.full.jpg`, se abre con `attachment_open_command`, como cualquier adjunto;
  - si no, y la preview existe, se envía `GetAvatarRequest{chatId, full: true}`. Al llegar `NewAvatarNotify{chatId, full: true}`, se abre si el chat sigue siendo el actual;
  - si `.id` es `none`, aparece "No profile photo.".
- En Go, `full` usa `Preview: false` y escribe `<chatId>.full.jpg`. Al guardar una preview con id distinto, se borra `<chatId>.full.jpg`.

### 5. Estados del contacto
- Al abrir el chat de un contacto, y cuando llega un estado nuevo de él, se cuentan sus estados de las últimas 24 h con `FetchSenderMessagesSync` (bloque de 50, desde ahora). Se guardan como `{total, nuevos}` por contacto, usando `IsStatusNew` para los nuevos.
- También se recalcula al volver del filtro de Status, para reflejar lo revisado.
- No se descarga nada: solo se cuentan.

### 6. Alt-u en el chat de un contacto
- `OnKeyStatusFilter` en un chat 1:1 (no grupo, no Status):
  - guarda `m_StatusReturnChat` = chat actual;
  - pasa a Status con `GotoChat` y aplica `SetStatusFilter(contactId)`.
- En Status con `m_StatusReturnChat` guardado, Alt-u limpia el filtro y vuelve con `GotoChat(m_StatusReturnChat)`.
- Salir de Status de otra forma (cambiar de chat) olvida `m_StatusReturnChat`, igual que hoy se limpia el filtro.
- Si el contacto no tiene ningún estado guardado, aparece "No status updates from <nombre>." y no se cambia de chat.
- En grupos, Alt-u no hace nada.

## Risks / Trade-offs

- **Peticiones a WhatsApp:** la foto se pide con la misma consulta que usa la app al abrir un chat, como máximo una vez cada 24 h por chat, y con `ExistingID` no se descarga si no cambió.
- **Alto del historial:** el banner ocupa 3 filas. Se puede desactivar con `chat_banner_enabled=0`.
- **Duplicado con la barra inferior:** nombre y presencia aparecen arriba y abajo. La barra inferior no se toca en este cambio, porque también muestra el perfil y el modo de desarrollador.
