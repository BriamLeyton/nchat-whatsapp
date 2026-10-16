## Context

- **Protocolo:** `GetChatId` en `gowm.go` ya agrupa los estados en el chat `status@broadcast`, y al sincronizar contactos se le da el nombre "Status Updates". Hoy hay 1563 estados en la caché del usuario.
- **Visibilidad:** `status_broadcast` (0 oculto, 1 silenciado, 2 visible) se aplica con `IsChatForceHidden`/`IsChatForceMuted`.
- **Secciones:** `GetChatSection` devuelve Pinned/Others/Archived. `SortChats` ordena por sección y calcula `m_SectionChatCount`. `IsChatHidden` oculta chats de secciones colapsadas cuando hay chats en otras secciones. `UiListView::Draw` muestra títulos solo si algún chat no está en "Others".

## Goals / Non-Goals

**Goals:**
- Estados primero, fuera de las secciones, con estilo propio.
- Reusar el chat existente: selección, historial, no leídos e imágenes inline siguen funcionando igual.

**Non-Goals:**
- Filtrar por 24 horas, agrupar por contacto, visor de estados y publicar.

## Decisions

### 1. Sección propia ordenada primero
Se agrega `ChatSectionStatus` al enum como valor 0, y las demás suben uno: Status=0, Pinned=1, Others=2, Archived=3, Count=4.
- `GetChatSection` devuelve `ChatSectionStatus` para `status@broadcast`, antes de mirar fijado o archivado.
- `SortChats` lo deja primero sin cambios, porque ya ordena por sección.
- `m_SectionCollapsed[Status]` es siempre `false`, y `SetSectionCollapsed` lo ignora, igual que con Pinned.
- Los arrays por sección (`m_SectionCollapsed`, `m_SectionChatCount`, contadores y títulos de la lista) pasan a tamaño 4. La persistencia usa nombres de opción, así que no cambia.

*Alternativa descartada:* una fila sintética fuera de `m_ChatVec`. Obligaría a duplicar la selección, el historial y los no leídos.

### 2. Fuera de títulos y colapso
- **`IsChatHidden`:** devuelve `false` para Status. Al calcular "chats en otras secciones" se excluye Status, para que con solo "Others" + estados no se oculte "Others" sin título visible.
- **`UiListView::Draw`:** los títulos se muestran si hay chats en Pinned o Archived (antes: "si algún chat no está en Others"). La fila de Status nunca lleva título.

### 3. Dibujo
- **Nombre:** `list_status_name` (por defecto `◉ Status`), en lugar del nombre del contacto.
- **Estilo:** `list_attr_status | list_color_status` en toda la fila. Si está seleccionada se suma `list_attr_selected` (por defecto `reverse`).
- **No leídos:** el indicador `*` usa el mismo color de estilo de la fila, para no romper el fondo.
- **Colores por defecto:** `list_color_status_fg=0x111b21`, `list_color_status_bg=0x25d366`, `list_attr_status=bold`. En terminales sin colores hex se aproximan a la paleta 256, como en los ticks.

### 4. Fijar y archivar
`OnKeyPinChat` y `OnKeyArchiveChat` salen sin hacer nada si el chat actual es `status@broadcast`.

## Risks / Trade-offs

- **Renumerar el enum:** cualquier código que use los valores de sección como números fijos se rompe. Mitigación: solo se usan por nombre (revisado con grep), y la persistencia usa claves de opción.
- **Muchos estados viejos en el historial (1563):** se ven todos, como hoy. El filtro por 24 horas queda fuera de alcance.
- **Contraste del `*` sobre el verde:** se dibuja con el color de la fila. Si se prefiere otro color, se puede configurar con `list_color_status`.
