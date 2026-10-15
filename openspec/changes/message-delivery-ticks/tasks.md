## 1. Protocolo común y caché

- [x] 1.1 Agregar el enum `DeliveryStatus` y el campo `deliveryStatus` a `ChatMessage` y `NewMessageStatusNotify` en `lib/common/src/protocol.h`. Verificar que todo compila (`make -j$(nproc)`).
- [x] 1.2 Agregar la migración de esquema 12 → 13 en `MessageCache` (`deliveryStatus INT DEFAULT -1`). Verificar sobre una **copia** de `db.sqlite` que `pragma table_info(messages)` muestra la columna, que `version.schema=13`, que el conteo de filas no cambió y que `nchat-status` funciona contra la copia.
- [x] 1.3 Leer y escribir `deliveryStatus` en los INSERT y SELECT de mensajes, preservando `max(previo, nuevo)` al reinsertar. Agregar la actualización monótona (`MAX(COALESCE(...))`) en el request diferido de estado. Verificar con un test en `tests/` o un harness sobre la copia: insertar en `Read` y reinsertar en `Sent` deja `Read`.

## 2. Go: receipts y agregación

- [x] 2.1 Crear `receipts.db` (tablas `msgstatus` y `participantreceipts`, poda a 60 días) al cargar el perfil en `gowm.go`. Verificar que el archivo existe en el perfil tras arrancar y que el log no tiene errores.
- [x] 2.2 Reescribir `HandleReceipt`: mapear `Delivered/Read/Played`, ignorar tipos no relevantes y dispositivos propios, emitir `ReadSelf/PlayedSelf` como hoy, agregar en grupos con `recipients` (con caché de `GetGroupInfo`) y emitir solo cuando el estado sube. Verificar con logs `-ee` que enviar un mensaje a un chat individual produce `sent → delivered → read`.
- [x] 2.3 Agregar `deliveryStatus` a `CWmNewMessagesNotify`, `CWmNewMessageStatusNotify` y al puente C (`wmchat.h/.cpp`), y actualizar los ~10 call sites (`fromMe && !isSyncRead` → estado guardado o `Sent`; si no, `Unknown`). Verificar que compila y que un mensaje enviado desde nchat se guarda con `deliveryStatus>=1` (consulta a `db.sqlite`).

## 3. UI

- [x] 3.1 En `UiModel`, al recibir `NewMessageStatusNotify`, aplicar `max` al `deliveryStatus` en memoria (y `isRead` solo si el estado es `Read` o el notify trae `isRead`). Verificar que el encabezado se actualiza en vivo sin reabrir el chat.
- [x] 3.2 Agregar los defaults de `receipt_*_indicator` en `uiconfig.cpp` y de `history_receipt_*_color` en `uicolorconfig.cpp` (hex si `can_change_color()`, si no `gray/blue`). Verificar con `diff` contra el respaldo que `ui.conf` y `color.conf` solo ganan las claves nuevas.
- [x] 3.3 En `UiHistoryView::Draw`: quitar `read_indicator` del encabezado; dibujar el segmento de estado solo en mensajes salientes no pendientes, con el color de su estado y la inferencia para `Unknown`, sin desbordar el ancho. Verificar en Ghostty: recibidos sin `✓`, enviados viejos `✓✓` gris o azul, y que un mensaje nuevo pasa por `✓` → `✓✓` gris → `✓✓` azul.

## 4. Verificación integral

- [x] 4.1 Grupo: enviar a un grupo pequeño y verificar que queda en `✓✓` gris hasta que **todos** leen, que luego pasa a azul, y que si se reinicia nchat entre medio el estado agregado se conserva.
- [x] 4.2 Regresión: el conteo de no leídos de la lista de chats no cambia, editar un mensaje propio no resetea sus checks, y `nchat-status` sigue funcionando.
