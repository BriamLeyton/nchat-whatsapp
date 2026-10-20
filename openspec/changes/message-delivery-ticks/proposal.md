## Why

Hoy nchat solo distingue "leído / no leído" y lo muestra con un `✓` en el encabezado de cualquier mensaje, incluso los recibidos. No se puede saber si un mensaje enviado llegó al teléfono del destinatario, ni si en un grupo lo leyeron todos. WhatsApp usa tres estados visuales (✓ gris, ✓✓ gris, ✓✓ azul) que el usuario ya reconoce.

## What Changes

- Los mensajes **enviados** muestran su estado de entrega en el encabezado, igual que WhatsApp:
  - enviado (aceptado por el servidor): `✓` gris
  - entregado: `✓✓` gris
  - leído (o reproducido, para notas de voz y "ver una vez"): `✓✓` azul
- Los mensajes **recibidos** dejan de mostrar el `✓` de leído. **BREAKING** (visual): cambia el comportamiento actual de `read_indicator` en el historial.
- En grupos, "entregado" y "leído" se marcan solo cuando **todos** los demás participantes entregaron o leyeron, como en WhatsApp.
- El estado "entregado" se persiste en la caché de mensajes. Los conteos por participante de grupo se persisten para sobrevivir reinicios.
- Mensajes enviados anteriores a este cambio (sin estado guardado): si están leídos se ven como leídos; si no, se asumen entregados (`✓✓` gris).
- Nuevas opciones de símbolos en `ui.conf` (`receipt_sent_indicator`, `receipt_delivered_indicator`, `receipt_read_indicator`) y de colores en `color.conf` (`history_receipt_sent_color`, `history_receipt_delivered_color`, `history_receipt_read_color`), con los colores de WhatsApp por defecto. En terminales sin colores hex, cualquier color hex de `color.conf` se aproxima a la paleta de 256 colores.

Fuera de alcance: ícono de reloj para mensajes pendientes de envío, vista "info del mensaje" con el detalle por participante, y estados para Telegram/Signal más allá de su leído actual.

## Capabilities

### New Capabilities
- `message-delivery-status`: estados de entrega de mensajes enviados (enviado, entregado, leído), su agregación en grupos, su persistencia y su representación visual en el historial.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- **Go (`lib/wmchat/go/gowm.go`):** procesar `ReceiptTypeDelivered` además de leído y reproducido. Agregar por participante en grupos (tamaño del grupo vía `GetGroupInfo`, en caché). Nuevo archivo `receipts.db` en el directorio del perfil para los conteos de grupo.
- **Puente C (`lib/wmchat/src/wmchat.{h,cpp}`):** `WmNewMessageStatusNotify` agrega un parámetro de estado.
- **Protocolo común (`lib/common/src/protocol.h`):** `ChatMessage` y `NewMessageStatusNotify` agregan el estado de entrega. `isRead` se mantiene por compatibilidad con Telegram y Signal.
- **Caché (`lib/ncutil/src/messagecache.cpp`):** migración de esquema 12 → 13 con la columna `messages.deliveryStatus` (NULL para mensajes previos), y lectura/escritura del campo. El script `nchat-status` del usuario no se ve afectado.
- **UI:** `src/uihistoryview.cpp` (indicador y color), `src/uimodel.cpp` (aplicar el estado recibido), `src/uiconfig.cpp` y `src/uicolorconfig.cpp` (opciones nuevas).
- **Reversibilidad:** la columna nueva es ignorada por binarios anteriores. Rollback con el respaldo `~/.config/nchat.bak-20261014` o simplemente con el binario previo.
