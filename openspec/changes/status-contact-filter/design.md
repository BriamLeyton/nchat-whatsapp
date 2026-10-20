## Context

- **Mensajes en memoria:** `m_MessageVec[profile][chat]` guarda los ids del más reciente al más antiguo, y `m_Messages` guarda los mensajes. `m_MessageOffset` indexa `m_MessageVec`, y unas 15 funciones (historial, selección, abrir, guardar, reaccionar, citar, …) lo leen junto con el offset.
- **Carga:** `RequestMessages` pide a la caché bloques desde el mensaje más antiguo, según el offset y la altura del historial.
- **Estados:** el chat `status@broadcast` tiene una fila propia en la lista (`status-updates-entry`). Cada mensaje tiene `senderId` (autor del estado).
- **Diálogos:** `UiListDialog` da la lista con búsqueda al escribir. `UiGroupMemberListDialog` es un ejemplo de lista de contactos.

## Goals / Non-Goals

**Goals:**
- Filtrar sin tocar `m_MessageVec`, que se actualiza con la red y la caché.
- Que las acciones existentes funcionen sin cambiar su lógica, solo la lista que leen.

**Non-Goals:**
- Filtros en otros chats, por fecha o por tipo de mensaje.

## Decisions

### 1. Lista mostrada
- **Estado:** `m_StatusFilterSenderId` (vacío = sin filtro) y `m_StatusFilterVec`, con los ids de `m_MessageVec` de estados cuyo `senderId` coincide.
- **`GetShownMessageVec(profile, chat)`:** devuelve `m_StatusFilterVec` si el chat es el de estados y hay filtro, y `m_MessageVec` en cualquier otro caso.
- **Lectores:** las funciones que leen `m_MessageVec` junto con `m_MessageOffset`, y `GetMessageVecLocked` (historial), pasan a usar la lista mostrada. Los que modifican `m_MessageVec` (mensajes nuevos, borrados, cambio de chat) no cambian, y después reconstruyen el filtro si aplica.
- **Reconstrucción:** recorre `m_MessageVec` del chat de estados. Con 1500 estados es barato, y solo se hace con el filtro activo y al cambiar los mensajes del chat de estados.

*Alternativa descartada:* filtrar en `UiHistoryView`. Las acciones sobre el mensaje seleccionado seguirían indexando la lista completa y apuntarían al mensaje equivocado.

### 2. Aplicar y quitar
- **Desde la selección:** se toma el `senderId` del mensaje seleccionado, se reconstruye la lista y `m_MessageOffset` pasa a ser su índice en la lista filtrada. La vista (`m_HistoryViewOffset`) se iguala al offset.
- **Desde el diálogo:** offset 0 y selección inactiva, para ver el final.
- **Al quitar:** offset 0, selección inactiva y se vacía la pila de `PgUp`.
- **Al cambiar de chat:** `OnCurrentChatChanged` quita el filtro si estaba activo.

### 3. Carga solo de los estados del contacto
Medido en el sandbox: los 1554 estados en caché (desde el 30-sep) ocupan unos 3.4 MB de RAM (~2.2 KB cada uno), y llegan ~100 por día. Cargar bloques de todos los estados para llenar un filtro crecería sin límite con el tiempo.

Por eso, con el filtro activo:
- **Consulta por autor:** `MessageCache::FetchSenderMessagesSync(profile, chat, senderId, beforeTime, limit)` trae solo los estados del contacto, más antiguos que el último filtrado, en bloques de 50.
- **Fuera de secuencia:** se insertan en `m_Messages`/`m_MessageVec` sin mover `m_OldestMessageId`, igual que hoy los mensajes citados. La carga normal del chat no cambia.
- **Llenado:** `RequestMessages` del chat de estados con filtro pide bloques hasta que la lista filtrada alcanza `offset + altura`, o hasta que un bloque devuelve menos de 50.
- **Liberación:** al quitar el filtro se borran de memoria los estados cargados solo para el filtro que siguen fuera del rango cargado en secuencia. La RAM vuelve a lo de antes de filtrar.

### 4. Diálogo de contactos
- **Origen de los datos:** la caché, no la memoria, para incluir estados aún no cargados. Nueva consulta síncrona `MessageCache::GetSenderLastTimesSync(profile, chat)`: `SELECT senderId, MAX(timeSent) FROM messages WHERE chatId=? GROUP BY senderId ORDER BY 2 DESC`.
- **Clase:** `UiStatusContactDialog : UiListDialog`. Muestra los nombres de contacto (`GetContactNameLocked`) y filtra al escribir como `UiGroupMemberListDialog`.
- **Sin el mutex:** se abre sin el mutex del modelo, como los demás diálogos.

### 5. Indicador y tecla
- **Fila de estados:** con filtro, `UiListView` agrega ` · <nombre>` después de `list_status_name`, recortado al ancho.
- **Tecla:** `status_filter=\33\165` (`Alt-u`, libre). Se agrega al modal de ayuda en "Selected message", como "Filter status by contact", y al README.

## Risks / Trade-offs

- **Lectores no migrados:** una función que siga leyendo `m_MessageVec` con el offset filtrado apuntaría al mensaje equivocado. Mitigación: se revisan con grep todos los usos de `m_MessageVec` junto con `messageOffset`, y la verificación incluye abrir, reaccionar y saltar al citado con el filtro activo.
- **Consultas síncronas:** el diálogo y la carga por autor consultan la caché con el mutex del modelo tomado. Con 1500 filas y bloques de 50 son de milisegundos.
- **Crecimiento de la caché:** la caché de estados crece ~100 por día y no se poda. Esto no afecta la RAM salvo al recorrer todo el historial de estados sin filtro, como en cualquier chat. Podarla queda para otro change.
- **Mensajes nuevos con filtro:** un estado nuevo del contacto filtrado desplaza la selección un lugar, igual que hoy sin filtro.
