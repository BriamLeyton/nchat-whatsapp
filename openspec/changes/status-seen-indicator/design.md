## Context

- `UpdateChatInfoIsUnread` decide el `*` de un chat según `isRead` del mensaje más reciente.
- `UiHistoryView` dibuja el nombre del remitente en la línea de cabecera de cada mensaje (`GetContactNameLocked`). En el mismo lugar llama a `MarkReadLocked` con `p_Final`, y sabe si el mensaje es el seleccionado (`isSelectedMessage`).
- Desde `status-read-on-open`, `MarkRead` no hace nada en Status salvo al abrir.
- Otros archivos por perfil ya viven en `FileUtil::GetApplicationDir() + "/profiles/" + profileId`, como `saved_contacts`.

## Decisions

### 1. Estado nuevo
Un estado es nuevo si cumple todo:
- es entrante y no está abierto (`!isOutgoing && !isRead`);
- su `timeSent` es posterior a la línea base del perfil;
- su id no está en el conjunto de revisados.

### 2. Archivo `status_seen`
- Formato de texto: la primera línea es `baseline <ms>`; las siguientes, `<msgId> <timeSent>`.
- Se lee (o se crea) al calcular por primera vez la marca de la fila Status, al cargar los chats, antes de que lleguen estados nuevos.
- Si no existe, la línea base es la hora actual y se escribe el archivo, así los estados que ya existían no salen como nuevos.
- Al escribir se descartan entradas con `timeSent` de más de 30 días, y la línea base sube a ese corte. Un estado de más de 30 días nunca sale como nuevo.
- Se escribe con archivo temporal y `rename`, como `saved_contacts`, cada vez que se agrega un estado.

### 3. Pasar a revisado al seleccionar
- En `UiHistoryView`, con `p_Final`, si el chat es Status y el mensaje es el seleccionado, se llama a `MarkStatusSeenLocked`.
- `MarkStatusSeen` solo agrega el id al conjunto, escribe el archivo y recalcula el `*`. No toca `isRead` ni envía nada.
- La línea del estado se dibuja ya sin `●` en el siguiente refresco.

### 4. Marca en la fila Status
- En `UpdateChatInfoIsUnread`, para Status: no leído si algún estado del chat es nuevo, recorriendo los mensajes cargados.
- `UiListView` dibuja la fila Status con `status_new_indicator` en lugar de `unread_indicator` cuando está no leída.
- El resto de la función (archivado, silenciado, notificación) sigue igual. Status no notifica, por `status-updates-entry`.

### 5. Marca
- `status_new_indicator` en `ui.conf`, default `●`: seguido de un espacio y antes del nombre en cada estado nuevo, y al final de la fila Status de la lista, en el lugar del `*`.

## Risks / Trade-offs

- **Un solo dispositivo:** los revisados no se sincronizan con el teléfono. El teléfono sigue usando sus propios vistos.
- **Solo lo cargado:** el `*` se calcula con los estados cargados en memoria. Los estados viejos que no se cargaron no cuentan.
