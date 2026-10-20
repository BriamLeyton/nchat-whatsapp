## Context

- `UiHistoryView::Draw` llama `MarkReadLocked` por cada mensaje recibido no leído que dibuja.
- `UiModel::Impl::MarkRead` aplica las condiciones de `mark_read_*`. Si el estado está vencido, lo marca en local (memoria y caché, sin petición). Si no, envía `MarkMessageReadRequest`.
- `open` (`OnKeyOpenAttachment`) abre el adjunto del mensaje seleccionado. Si no está descargado, primero lo descarga y lo abre cuando llega. Para estados vencidos la descarga está bloqueada.
- `open_msg` (`OnKeyOpenMsg`) muestra el texto del mensaje seleccionado en el visor.

## Decisions

### 1. Bloqueo por dibujo
- `MarkRead` gana un parámetro `p_Opened = false`.
- Si el chat es de estados y `p_Opened` es falso, sale sin hacer nada. El dibujo nunca pasa `true`.

### 2. Marcado al abrir
- Nueva función `MarkStatusOpened()`: toma el mensaje seleccionado del chat actual. Si es un estado recibido y no leído, llama `MarkRead(profile, chat, msgId, true, true)`.
- Se llama desde `OnKeyOpenAttachment` (solo cuando la acción viene del usuario, no del aviso de descarga) y desde `OnKeyOpenMsg`.
- **Momento:** se marca al presionar la tecla, aunque el adjunto todavía deba descargarse, porque la intención de verlo ya existe. Si es un estado vencido sin archivo, no se puede abrir; aun así se marca en local, sin aviso.
- Las demás condiciones de `MarkRead` se mantienen: terminal activa, etc.

## Risks / Trade-offs

- **Imágenes inline:** con `inline_images_enabled` la imagen del estado se ve en la lista sin abrirla, y no se marca. Es lo pedido: visto = abierto.
- **Fila de Status:** queda con indicador de no leído mientras haya estados sin abrir. Ya pasa hoy con estados fuera de pantalla.
