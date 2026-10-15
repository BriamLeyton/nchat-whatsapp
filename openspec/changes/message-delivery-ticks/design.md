## Context

Así fluye hoy el estado de un mensaje (ver proposal.md para la motivación):

- **Go (`gowm.go`):** `HandleReceipt` solo procesa `ReceiptTypeRead/ReadSelf` y `Played/PlayedSelf`, y llama `CWmNewMessageStatusNotify(connId, chatId, msgId, isRead)`. Ignora `ReceiptTypeDelivered` (`""`), y en grupos cualquier receipt de un participante marca el mensaje como leído.
- **Mensajes nuevos:** llegan por `CWmNewMessagesNotify(..., isRead, isEdited)` desde ~10 handlers (texto, imagen, sticker, …). Cada handler recibe `isSyncRead`, que distingue history sync de mensajes en vivo. Los mensajes enviados desde nchat también pasan por esos handlers después de `client.SendMessage` (con `isSyncRead=false`).
- **C++:** `ChatMessage.isRead` (bool) se guarda en `messages.isRead`. `UiModel` aplica `NewMessageStatusNotify` en memoria, y `MessageCache` lo persiste con `UPDATE ... SET isRead`.
- **Historial:** `UiHistoryView::Draw` agrega `read_indicator` al encabezado de **cualquier** mensaje con `isRead`.
- **Esquema:** la caché tiene migraciones secuenciales en `MessageCache` (hoy versión 12). La inserción de mensajes lee la fila previa para preservar reacciones.
- **Colores:** `UiColorConfig` soporta nombres de color, `usercolor` y hex (`0xRRGGBB`, si la terminal permite redefinir colores). En Ghostty los hex funcionan.

## Goals / Non-Goals

**Goals:**
- Que la agregación de receipts sea responsabilidad del protocolo (Go). C++ solo recibe un estado final por mensaje.
- Que el estado sea monótono en todas las capas: nunca retrocede, ni con receipts tardíos, ni con re-inserciones, ni con el history sync.
- Cambios aditivos en el esquema, sin reescribir datos existentes.

**Non-Goals:**
- Reloj de "pendiente", info por participante, y estados para Telegram/Signal (su `isRead` se sigue mapeando a `read`).

## Decisions

### 1. Enum `DeliveryStatus` con "desconocido"
En `protocol.h`:

```cpp
enum DeliveryStatus { DeliveryStatusUnknown = -1, DeliveryStatusNone = 0, DeliveryStatusSent = 1, DeliveryStatusDelivered = 2, DeliveryStatusRead = 3 };
```

`ChatMessage.deliveryStatus` (por defecto `Unknown`) y `NewMessageStatusNotify.deliveryStatus` se agregan. `isRead` se mantiene: los mensajes recibidos siguen usándolo para la lógica de no leídos, y otros protocolos solo envían `isRead`.

En la UI, el estado efectivo de un mensaje saliente se calcula así:
- `Unknown` → `isRead ? Read : Delivered` (inferencia para mensajes previos, decidida con el usuario);
- cualquier otro valor → ese valor.

*Alternativa descartada:* codificar el estado en `isRead` (0/1/2). Rompe la semántica booleana usada en el cálculo de no leídos y en otros protocolos.

### 2. Agregación y persistencia de receipts en Go (`receipts.db`)
Nuevo archivo SQLite `<profile>/receipts.db`, con el driver `mattn/go-sqlite3` que ya se usa. No se toca `session.db` de whatsmeow.

Tablas:
- `msgstatus(chatId, msgId, status, recipients, timeUpdated, PRIMARY KEY(chatId,msgId))`
- `participantreceipts(chatId, msgId, participant, status, PRIMARY KEY(chatId,msgId,participant))`

Flujo en `HandleReceipt`:
1. Se ignoran receipts de tipo `Sender`, `Retry`, `Inactive`, `PeerMsg`, `ServerError`, y receipts cuyo `Sender.User` es el propio usuario (otros dispositivos).
2. Mapeo: `Delivered("")` → 2; `Read`/`Played` → 3. `ReadSelf`/`PlayedSelf` son lecturas propias de mensajes recibidos: se emiten como hoy (`isRead=1`, estado `Unknown`) y no afectan a los enviados.
3. **Chat individual:** `status = max(actual, nuevo)`.
4. **Grupo:**
   - Se hace upsert en `participantreceipts` con `max`.
   - `recipients` = número de participantes del grupo menos el propio usuario. Sale de `GetGroupInfo`, con caché en memoria por sesión, y se guarda en `msgstatus` la primera vez para que no cambie con altas o bajas posteriores.
   - Estado agregado: `Read` si `count(status>=3) >= recipients`; si no, `Delivered` si `count(status>=2) >= recipients`; si no, `Sent`.
5. Solo si el estado agregado sube, se llama `CWmNewMessageStatusNotify(connId, chatId, msgId, isRead, status)`.

Además, al arrancar se borran las filas con `timeUpdated` de más de 60 días, para acotar el tamaño.

*Alternativa descartada:* agregar en C++ (`MessageCache`). Mezclaría semántica específica de WhatsApp (participantes, dispositivos propios) con la capa genérica y obligaría a pasar los IDs de participantes por el puente C.

### 3. Estado inicial en mensajes nuevos
`CWmNewMessagesNotify` agrega el parámetro `deliveryStatus`. En cada handler:
- si `fromMe && !isSyncRead`: se toma el estado guardado en `receipts.db` (cubre la carrera de un receipt que llega antes del mensaje) o, si no hay, `Sent`;
- en cualquier otro caso: `Unknown`.

Así los mensajes del history sync usan la inferencia, y los recibidos no tienen estado.

### 4. Monotonía en C++ y migración de esquema
- **Migración 12 → 13:** `ALTER TABLE messages ADD COLUMN deliveryStatus INT DEFAULT -1;`. Los mensajes existentes quedan en `Unknown`; es aditiva y no rompe `nchat-status`.
- **Inserción:** al reinsertar un mensaje, por ejemplo al editarlo, se lee la fila previa (como ya se hace con las reacciones) y se guarda `max(previo, nuevo)`. `WmNewMessagesNotify`, en el camino de edición, ya preserva `isRead`; se preserva también `deliveryStatus`.
- **Actualización:** `UPDATE messages SET deliveryStatus = MAX(COALESCE(deliveryStatus,-1), ?)`, más `isRead = 1` cuando el estado es `Read`, en el mismo request diferido que hoy actualiza `isRead`.
- **`UiModel`:** al recibir `NewMessageStatusNotify` aplica `max` en memoria.

### 5. Representación en `UiHistoryView`
El indicador se dibuja como un segmento aparte después de `wheader`, con su propio color (`history_receipt_*_color`) y el atributo de nombre (normal o seleccionado). Solo se dibuja si `msg.isOutgoing` y `timeSent != INT64_MAX` (no pendiente). El cálculo de ancho de `TrimPadWString` incluye el indicador, para no desbordar.

`read_indicator` deja de usarse en el historial. Se conserva en `uiconfig` para no romper configuraciones, pero queda sin efecto.

### 6. Configuración
- **`UiConfig`:** `receipt_sent_indicator=✓`, `receipt_delivered_indicator=✓✓`, `receipt_read_indicator=✓✓`.
- **`UiColorConfig`:** `history_receipt_{sent,delivered,read}_color_{fg,bg}`.
  - `fg` por defecto siempre en hex: `0x8696a0` / `0x8696a0` / `0x53bdeb` (`blue` con 8 colores), y `bg` vacío.
  - No se elige el default según la terminal, porque se guarda en `color.conf` en el primer arranque y quedaría fijo.
  - En cambio, `GetColorId` aproxima cualquier hex al color más cercano de la paleta xterm-256 cuando la terminal no puede redefinir colores (p. ej. tmux). Antes se descartaba con un warning.

## Risks / Trade-offs

- [`GetGroupInfo` es una llamada de red por grupo] → Mitigación: se hace solo para receipts de mensajes propios en grupos, con caché en memoria por sesión y `recipients` guardado por mensaje. Si falla, se usa `recipients=1` (comportamiento similar al actual) y se registra un warning.
- [Grupos grandes o comunidades: muchos receipts y filas] → Mitigación: upserts simples con índice por la clave primaria y poda a 60 días. En grupos con más de 256 participantes se agrega sin guardar por participante (solo conteos) para limitar filas.
- [Divergencia con upstream por el esquema 13] → Mitigación: el cambio es aditivo. Si upstream usa la versión 13 para otra cosa, se renumera al hacer merge. Queda documentado aquí.
- [Mensajes viejos no leídos que en realidad nunca se entregaron se verán como `✓✓`] → Trade-off aceptado por el usuario (inferencia).
- [`ReceiptTypeDelivered` puede llegar varias veces (multi-dispositivo del destinatario)] → Mitigación: upsert idempotente por participante (usuario sin dispositivo, `Sender.ToNonAD()`).

## Migration Plan

- Al primer arranque se aplica automáticamente la migración 12 → 13 y se crea `receipts.db`.
- Respaldo existente: `~/.config/nchat.bak-20261014`. Conviene hacer un respaldo nuevo de `history/` antes del primer arranque con esta versión.
- Rollback: un binario anterior ignora la columna nueva y `receipts.db`.
