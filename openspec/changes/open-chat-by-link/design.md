## Context

- `UiContactListDialog` builds the list from `GetContactInfos`, filtering by name with `m_FilterStr`. When a contact is chosen, `OnKeySelectContact` calls `OpenCreateChat({profileId, contactId})`.
- `OpenCreateChat` opens the chat if it exists in `m_ChatInfos` (unarchiving it); otherwise it sends `CreateChatRequest{userId}`. In wmchat, `CreateChatRequest` answers `CreateChatNotify{success=true, id=userId}` without querying WhatsApp.
- Each chat's draft lives in `m_EntryStr[profileId][chatId]` and `m_EntryPos`.
- whatsmeow offers `IsOnWhatsApp(ctx, ["+15551234567"])`, which returns `IsIn` and the `JID` of the number.

## Decisions

### 1. Detection in the filter
- `LinkUtil::ParseWhatsAppLink(filter, phone, text)` (free function in `uicontactlistdialog.cpp` or `strutil`):
  - link: host `wa.me`, `api.whatsapp.com` or scheme `whatsapp://`; `phone` comes from the path (`wa.me/<n>`) or from the `phone=` parameter; `text` from the `text=` parameter, decoded (`+` and `%XX`, UTF-8);
  - number: the filter only has digits, spaces, `+`, `-`, `(`, `)` and `.`, and between 8 and 15 digits remain;
  - all characters that are not digits are removed.
- With a valid result, the first row is `Open chat with +<phone>`. Contacts are still filtered by the text, but a link matches no name, so normally only that row remains.
- `UiContactListItem` gains `phone` and `text`. The new row has an empty `contactId` and a filled `phone`.

### 2. Open or create
- `OnKeySelectContact` with `phone` calls `OpenChatByPhone(profileId, phone, text)`:
  - `chatId = phone + "@s.whatsapp.net"`. If it exists in `m_ChatInfos`, `OpenCreateChat` is called (it opens and unarchives) and then the text is applied. No network.
  - If it does not exist and there is no connection: notice "Not connected, please try again later.".
  - If it does not exist: `m_PendingLinkText[profileId][phone] = text` is stored and `CreateChatRequest{userId="", phone}` is sent.
- The JID returned by WhatsApp is preferred over one built by hand: `IsOnWhatsApp` may correct the number (for example, the 9 in Mexico).

### 3. wmchat and Go
- `CreateChatRequest` gains `phone`. With an empty `phone` the current behavior is kept (known contact).
- With `phone`: `CWmCreateChat(connId, phone)`. In Go, in a goroutine:
  - `IsOnWhatsApp(ctx, ["+" + phone])` with a 10 s timeout;
  - `IsIn`: `WmCreateChatNotify(connId, chatId, phone, 0)`. `chatId` is the `PhoneNumber` of the response (`<number>@s.whatsapp.net`), or the JID passed through `GetChatId` if it is not present. `IsOnWhatsApp` often returns the LID (`@lid`) as the `JID`; if the chat were created with it, the messages (which `GetChatId` converts to the number) would fall into another chat, and the selection would stay on the empty chat;
  - not registered: `WmCreateChatNotify(connId, "", phone, 1)`; error: `-1`.
- `CreateChatNotify` gains `phone` and `result` (0 ok, 1 not registered, -1 error). `success` is still `result == 0`.

### 4. UI on receiving `CreateChatNotify`
- Success: if the chat was already in `m_ChatInfos` (the corrected JID matched an existing one), it is opened as in `OpenCreateChat`, without overwriting its `ChatInfo`. If not, it is added as today.
- Pending text (`m_PendingLinkText[profileId][phone]`), if any: it is applied and deleted.
- `result == 1`: "+<phone> is not on WhatsApp.". `result == -1`: "Could not check +<phone>, please try again.". The pending text is deleted.

### 5. Applying the text
- If `m_EntryStr[profileId][chatId]` is empty: the text is set and the cursor goes to the end.
- Otherwise: the draft is kept and "Draft kept, link text not inserted." appears.
- The text is not sent: the user sends it with Enter.

## Risks / Trade-offs

- `IsOnWhatsApp` is a usync query, the same one the app makes when opening a link. It is done only with an explicit Enter on the row, once, never by typing in the filter.
- A chat created with no messages stays in the list only in this session, as today when choosing a contact with no chat.
- A number without a country code is misread. The row shows `+<phone>` before confirming.
