## Purpose

Sincronizar de forma confiable los chats fijados y archivados con WhatsApp, y organizar la lista de chats en secciones colapsables: fijados, otros y archivados.

## ADDED Requirements

### Requirement: Recuperación del estado de WhatsApp
Si el envío de un cambio de fijado o archivado, o una sincronización entrante, falla porque el estado local de WhatsApp está corrupto (`mismatching LTHash`), el sistema SHALL hacer un resync completo de esa colección desde el servidor. Si el snapshot del servidor también falla la verificación, el sistema SHALL pedir una copia de recuperación al teléfono, una vez por sesión y colección. Tras un resync o una recuperación exitosos, los fijados, archivados y silenciados MUST reflejarse en la lista sin reiniciar nchat.

#### Scenario: Alt-p con estado corrupto
- **WHEN** el estado `regular_low` está corrupto, el usuario presiona `Alt-p` en un chat y el teléfono está en línea
- **THEN** nchat recupera el estado y el siguiente `Alt-p` fija el chat en nchat y en el teléfono

#### Scenario: Recuperación desde el teléfono
- **WHEN** llega la copia de recuperación del teléfono
- **THEN** los chats fijados y archivados en el teléfono aparecen así en nchat sin reiniciar

### Requirement: Secciones de la lista
La lista de chats SHALL agrupar los chats en tres secciones, en este orden: `Fijados` (fijados), `Otros` (ni fijados ni archivados) y `Archivados` (archivados). Dentro de cada sección se mantiene el orden actual. Cuando hay más de una sección con chats, cada sección con chats SHALL mostrar una fila de título. Las filas de título MUST NOT ser seleccionables. Cuando solo `Otros` tiene chats, la lista MUST NOT mostrar títulos.

#### Scenario: Lista con fijados y archivados
- **WHEN** hay 2 chats fijados, 10 sin fijar ni archivar y 74 archivados, y ninguna sección está colapsada
- **THEN** la lista muestra `Fijados`, los 2 fijados, `Otros`, los 10, `Archivados` y los 74

#### Scenario: Navegación entre secciones
- **WHEN** el chat seleccionado es el último fijado y el usuario pasa al siguiente chat
- **THEN** la selección salta el título y queda en el primer chat de `Otros`

#### Scenario: Solo otros
- **WHEN** no hay chats fijados ni archivados
- **THEN** la lista se ve como antes de este cambio

### Requirement: Colapsar secciones
La tecla `toggle_others` (por defecto `Alt-o`) SHALL colapsar o expandir `Otros`, y `toggle_archived` (por defecto `Alt-h`) `Archivados`. `Fijados` no se colapsa. Una sección colapsada:
- muestra solo su título, seguido del número de chats y de `unread_indicator` si alguno tiene no leídos;
- sus chats se omiten al pasar al chat siguiente o anterior;
- si el chat seleccionado queda oculto al colapsarla, la selección MUST pasar al chat visible más cercano, o a ninguno si no hay chats visibles.

Abrir un chat oculto por otra vía (ir a no leído, ir a chat, seleccionar contacto) MUST expandir su sección. El estado de cada sección SHALL guardarse en `list_others_collapsed` y `list_archived_collapsed`, y restaurarse al arrancar. `Archivados` empieza colapsada por defecto.

#### Scenario: Colapsar otros
- **WHEN** `Otros` está expandida y el usuario presiona `Alt-o`
- **THEN** la sección muestra solo `Otros (N)` y Tab recorre los chats de las secciones expandidas

#### Scenario: Mensaje nuevo en sección colapsada
- **WHEN** `Otros` está colapsada y llega un mensaje a uno de sus chats
- **THEN** el título muestra `Otros (N) *`

#### Scenario: Ir a no leído oculto
- **WHEN** el chat no leído al que lleva `unread_chat` está en `Otros` colapsada
- **THEN** esa sección se expande y se abre el chat

#### Scenario: No leídos archivados
- **WHEN** `Archivados` está colapsada y un chat archivado tiene no leídos
- **THEN** `unread_chat` lo omite

#### Scenario: Reinicio
- **WHEN** el usuario cierra nchat con `Otros` colapsada y `Archivados` expandida
- **THEN** al volver a abrir, las secciones conservan ese estado

### Requirement: Chats archivados visibles
Los chats archivados SHALL aparecer en la sección `Archivados`, ordenados por último mensaje. Se pueden abrir, leer y responder. Los mensajes nuevos de un chat archivado MUST mostrarse en su historial y marcarlo como no leído, pero MUST NOT generar notificación ni desarchivarlo. Archivar o desarchivar con `Alt-a` o desde el teléfono SHALL mover el chat de sección en vivo.

#### Scenario: Mensaje en chat archivado
- **WHEN** llega un mensaje a un chat archivado
- **THEN** el chat sigue en `Archivados`, muestra `*` y no suena notificación

#### Scenario: Archivar con Alt-a
- **WHEN** el usuario presiona `Alt-a` en un chat de `Otros` y `Archivados` está colapsada
- **THEN** el chat pasa a `Archivados` y la selección pasa al chat visible más cercano

#### Scenario: Desarchivar desde el teléfono
- **WHEN** el usuario desarchiva un chat en el teléfono
- **THEN** el chat pasa a `Otros` en nchat sin reiniciar

### Requirement: Configuración
El sistema SHALL leer:
- de `ui.conf`: `list_pinned_header` (por defecto `Fijados`), `list_others_header` (`Otros`), `list_archived_header` (`Archivados`), `list_others_collapsed` (`0`) y `list_archived_collapsed` (`1`);
- de `key.conf`: `toggle_others` (por defecto `Alt-o`) y `toggle_archived` (`Alt-h`);
- de `color.conf`: `list_color_header_fg/_bg`.

Las claves MUST agregarse con su valor por defecto cuando no existan, sin modificar las existentes.

#### Scenario: Primera ejecución tras actualizar
- **WHEN** el usuario arranca esta versión con sus archivos de configuración existentes
- **THEN** los archivos incluyen las claves nuevas con sus valores por defecto y el resto queda igual
