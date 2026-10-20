## Context

- `UiContactListDialog` arma la lista desde `GetContactInfos`, filtrando por nombre con `m_FilterStr`. Al elegir un contacto, `OnKeySelectContact` llama `OpenCreateChat({profileId, contactId})`.
- `OpenCreateChat` abre el chat si existe en `m_ChatInfos` (desarchivándolo); si no, envía `CreateChatRequest{userId}`. En wmchat, `CreateChatRequest` responde `CreateChatNotify{success=true, id=userId}` sin consultar a WhatsApp.
- El borrador de cada chat vive en `m_EntryStr[profileId][chatId]` y `m_EntryPos`.
- whatsmeow ofrece `IsOnWhatsApp(ctx, ["+57..."])`, que devuelve `IsIn` y el `JID` del número.

## Decisions

### 1. Detección en el filtro
- `LinkUtil::ParseWhatsAppLink(filter, phone, text)` (función libre en `uicontactlistdialog.cpp` o `strutil`):
  - link: host `wa.me`, `api.whatsapp.com` o esquema `whatsapp://`; `phone` sale del path (`wa.me/<n>`) o del parámetro `phone=`; `text` del parámetro `text=`, decodificado (`+` y `%XX`, UTF-8);
  - número: el filtro solo tiene dígitos, espacios, `+`, `-`, `(`, `)` y `.`, y quedan entre 8 y 15 dígitos;
  - se quitan todos los caracteres que no son dígitos.
- Con un resultado válido, la primera fila es `Open chat with +<phone>`. Los contactos se siguen filtrando por el texto, pero un link no coincide con ningún nombre, así que normalmente solo queda esa fila.
- `UiContactListItem` gana `phone` y `text`. La fila nueva tiene `contactId` vacío y `phone` lleno.

### 2. Abrir o crear
- `OnKeySelectContact` con `phone` llama `OpenChatByPhone(profileId, phone, text)`:
  - `chatId = phone + "@s.whatsapp.net"`. Si existe en `m_ChatInfos`, `OpenCreateChat` (abre y desarchiva) y luego se aplica el texto. Sin red.
  - Si no existe y no hay conexión: aviso "Not connected, please try again later.".
  - Si no existe: se guarda `m_PendingLinkText[profileId][phone] = text` y se envía `CreateChatRequest{userId="", phone}`.
- Se prefiere el JID que devuelve WhatsApp, no el armado a mano: `IsOnWhatsApp` puede corregir el número (por ejemplo, el 9 de México).

### 3. wmchat y Go
- `CreateChatRequest` gana `phone`. Con `phone` vacío se mantiene el comportamiento actual (contacto conocido).
- Con `phone`: `CWmCreateChat(connId, phone)`. En Go, en una goroutine:
  - `IsOnWhatsApp(ctx, ["+" + phone])` con timeout de 10 s;
  - `IsIn`: `WmCreateChatNotify(connId, chatId, phone, 0)`. `chatId` es `PhoneNumber` de la respuesta (`<número>@s.whatsapp.net`), o el JID pasado por `GetChatId` si no viene. `IsOnWhatsApp` suele devolver como `JID` el LID (`@lid`); si el chat se creara con él, los mensajes (que `GetChatId` convierte a número) caerían en otro chat, y la selección se quedaría en el chat vacío;
  - no registrado: `WmCreateChatNotify(connId, "", phone, 1)`; error: `-1`.
- `CreateChatNotify` gana `phone` y `result` (0 ok, 1 no registrado, -1 error). `success` sigue siendo `result == 0`.

### 4. UI al recibir `CreateChatNotify`
- Éxito: si el chat ya estaba en `m_ChatInfos` (el JID corregido coincidía con uno existente), se abre como en `OpenCreateChat`, sin pisar su `ChatInfo`. Si no, se agrega como hoy.
- Texto pendiente (`m_PendingLinkText[profileId][phone]`), si hay: se aplica y se borra.
- `result == 1`: "+<phone> is not on WhatsApp.". `result == -1`: "Could not check +<phone>, please try again.". Se borra el texto pendiente.

### 5. Aplicar el texto
- Si `m_EntryStr[profileId][chatId]` está vacío: se pone el texto y el cursor al final.
- Si no: se deja el borrador y aparece "Draft kept, link text not inserted.".
- El texto no se envía: el usuario lo envía con Enter.

## Risks / Trade-offs

- `IsOnWhatsApp` es una consulta usync, la misma que hace la app al abrir un link. Se hace solo por Enter explícito sobre la fila, una vez, nunca por escribir en el filtro.
- Un chat creado y sin mensajes queda en la lista solo en esta sesión, como hoy al elegir un contacto sin chat.
- Un número sin código de país se interpreta mal. La fila muestra `+<phone>` antes de confirmar.
