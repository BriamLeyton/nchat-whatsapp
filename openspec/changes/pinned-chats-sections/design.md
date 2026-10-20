## Context

- **Pins y archivados en upstream:**
  - `Alt-p` y `Alt-a` envían `PinChatRequest` y `ArchiveChatRequest`, que Go convierte en parches de app state (`regular_low`).
  - Los cambios entrantes llegan como `events.Pin` y `events.Archive`.
- **Archivados en la UI:**
  - `NewChatsNotify` y `UpdateArchivedNotify` los sacan de `m_ChatVec`.
  - `NewMessagesNotify` descarta sus mensajes (`return` temprano).
  - `UpdateChatInfoIsUnread` no notifica.
  - `chats2.isArchived` persiste el estado.
- **Orden:** `UiModel::SortChats` pone primero los fijados y luego ordena por silenciado y tiempo.
- **Dibujo:** `UiListView::Draw` dibuja una fila por chat de `m_ChatVec`, centrando la selección.
- **Falla resuelta (commit `3f567a41`):**
  - `regular_low` local estaba en v334 y el snapshot del servidor en v347; ambos fallaban con `mismatching LTHash`.
  - La recuperación desde el teléfono (`COMPANION_SYNCD_SNAPSHOT_FATAL_RECOVERY`) funcionó: trajo 1 chat fijado y 74 archivados, y los parches siguientes ya se aplican (v348).

## Goals / Non-Goals

**Goals:** tres secciones colapsables sin reestructurar `m_ChatVec`, que usan unas 40 funciones; archivados visibles sin notificaciones.

**Non-Goals:** fijado local, orden manual entre fijados.

## Decisions

### 1. Recuperación del estado (implementado)
- **`ResyncAppState`:**
  - Hace `FetchAppState(name, fullSync=true)` con `EmitAppStateEventsOnFullSync` activo, para que los eventos actualicen la UI, y después restaura el valor previo.
  - Un mutex serializa los resyncs, y se omite si la colección se resincronizó hace menos de 30 s.
- **Si el snapshot falla con LTHash**, `RequestAppStateRecovery` envía `BuildAppStateRecoveryRequest` al teléfono, una vez por sesión y colección, y deja el flag activo hasta `AppStateSyncComplete{Recovery: true}`.
- **`SendAppStateWithResync`** reintenta una vez el parche de pin o archive tras un resync exitoso.

### 2. Archivados dentro de `m_ChatVec`
- **Entrada a la lista:** se quitan los `continue`/`erase` que los excluyen en `NewChatsNotify` y `UpdateArchivedNotify`. Al archivar o desarchivar se reordena con `SortChats` y se mantiene `m_CurrentChat`, recalculando `m_CurrentChatIndex`.
- **Mensajes nuevos:** se quita el `return` de `NewMessagesNotify` para los archivados. La lógica actual de `UpdateChatInfoIsUnread` ya evita notificar a los archivados.
- **Orden en `SortChats`:** la clave primaria es la sección, `Fijados=0`, `Otros=1` y `Archivados=2`. Un chat fijado y archivado cuenta como fijado (WhatsApp no lo permite, pero se cubre). Dentro de `Archivados` se ordena solo por tiempo, ignorando silenciado.

*Alternativa descartada:* un vector aparte para archivados. Obligaría a tocar todas las funciones que indexan `m_ChatVec`.

### 3. Sección y visibilidad en `UiModel`
- **`GetChatSection(profileId, chatId)`** devuelve `Fijados`, `Otros` o `Archivados`.
- **`IsChatHiddenLocked(index)`** devuelve verdadero si la sección del chat está colapsada.
- **Navegación:** `OnKeyNextChat` y `OnKeyPrevChat` saltan los ocultos, con wrap-around igual que hoy. Si todos están ocultos, no hacen nada.
- **`OnKeyToggleOthers` y `OnKeyToggleArchived`:** invierten el estado, lo guardan con `UiConfig::SetBool` y, si el chat actual queda oculto, seleccionan el visible más cercano.
- **Expansión automática:** `OnKeyUnreadChat`, `goto_chat` y la selección de contacto, cuando su destino está oculto, expanden su sección.
- **Comportamiento cuando solo hay `Otros` con chats:** colapsar no tiene efecto, para no dejar la lista vacía sin títulos.

### 4. Filas virtuales en `UiListView`
`Draw` arma filas `{título(sección, total, hayNoLeídos) | chat(índice)}` recorriendo `m_ChatVec`, que ya viene agrupado por sección:
- se inserta un título al cambiar de sección, salvo cuando la única sección con chats es `Otros`;
- las secciones colapsadas no aportan filas de chat, y su título muestra `(N)` y `*`.

El desplazamiento se centra en la fila del chat seleccionado. Si no hay selección visible, se usa la primera fila. Los títulos usan `list_color_header` con `A_BOLD`, y su `*` usa `list_color_unread`.

### 5. Teclas
`toggle_others` usa `\33\157` (Alt-o) y `toggle_archived` usa `\33\150` (Alt-h). Las dos combinaciones están libres en los defaults de `uikeyconfig.cpp`.

## Risks / Trade-offs

- **74 archivados más en la lista.** Mitigación: `Archivados` empieza colapsada.
- **Cambio de comportamiento: los mensajes de archivados ahora se cargan en la UI.** El costo de memoria es igual al de cualquier chat; el historial ya se guardaba en la caché.
- **La recuperación depende del teléfono en línea.** Si no responde, todo sigue como antes (log de warning) y se reintenta en la próxima sesión.
- **WhatsApp limita a 3 los chats fijados.** El servidor rechaza el cuarto; queda en el log.
