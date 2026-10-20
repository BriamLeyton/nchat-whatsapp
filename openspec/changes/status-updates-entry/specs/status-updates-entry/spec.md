## Purpose

Mostrar los estados de WhatsApp como una entrada fija, llamativa y seleccionable al inicio de la lista de chats, separada de las conversaciones.

## ADDED Requirements

### Requirement: Entrada fija de estados
Cuando existe el chat de estados (`status@broadcast`) y `status_broadcast` es distinto de `0`, la lista SHALL mostrarlo como la primera fila, antes de cualquier título de sección y de cualquier chat. La fila MUST NOT pertenecer a ninguna sección: no se cuenta en los títulos, no se oculta al colapsar secciones y no cambia cuándo se muestran los títulos.

#### Scenario: Lista con fijados
- **WHEN** hay chats fijados y existe el chat de estados
- **THEN** la primera fila es la de estados y debajo aparece el título `PINNED`

#### Scenario: Others colapsado
- **WHEN** la sección "Others" está colapsada
- **THEN** la fila de estados sigue visible y "Others" no cuenta los estados

#### Scenario: Sin fijados ni archivados
- **WHEN** solo hay chats en "Others" y existe el chat de estados
- **THEN** no se muestran títulos de sección y la fila de estados queda primera

#### Scenario: Estados ocultos
- **WHEN** `status_broadcast=0`
- **THEN** no se muestra la fila de estados

### Requirement: Estilo propio
La fila de estados SHALL dibujarse con el nombre de `list_status_name` y con el color y el atributo de `list_color_status` y `list_attr_status`, distintos de los títulos de sección. Por defecto MUST usar texto `#6fcfa8` sobre fondo `#173029` en negrita. Cuando está seleccionada MUST usar además el atributo de selección de la lista.

#### Scenario: Fila sin seleccionar
- **WHEN** la fila de estados no está seleccionada
- **THEN** se ve con fondo verde oscuro y texto verde en negrita, ocupando todo el ancho de la lista

#### Scenario: Fila seleccionada
- **WHEN** la fila de estados está seleccionada
- **THEN** se ve con el atributo de selección y se distingue de cuando no está seleccionada

#### Scenario: Color personalizado
- **WHEN** `color.conf` contiene `list_color_status_bg=yellow`
- **THEN** la fila de estados se ve con fondo amarillo

### Requirement: Selección
La fila de estados SHALL poder seleccionarse como cualquier chat: con chat siguiente y anterior, con ir a chat y al arrancar si era el chat actual. Al seleccionarla, el historial MUST mostrar los estados.

#### Scenario: Chat anterior desde el primer chat
- **WHEN** el primer chat debajo de la fila de estados está seleccionado y el usuario va al chat anterior
- **THEN** queda seleccionada la fila de estados

#### Scenario: Historial de estados
- **WHEN** la fila de estados está seleccionada
- **THEN** el historial muestra los estados con el nombre de quien lo publicó

### Requirement: Estados no vistos
La fila de estados SHALL mostrar el indicador de no leído cuando hay estados no vistos, igual que un chat. Ver los estados MUST quitar el indicador.

#### Scenario: Estado nuevo
- **WHEN** un contacto publica un estado
- **THEN** la fila de estados muestra `*` y, con `status_broadcast=1`, no hay notificación

### Requirement: Sin fijar ni archivar
Fijar o archivar con la fila de estados seleccionada SHALL NOT tener efecto, y MUST NOT enviar cambios a WhatsApp.

#### Scenario: Alt-p sobre estados
- **WHEN** la fila de estados está seleccionada y el usuario presiona `Alt-p`
- **THEN** no cambia nada y no se envía nada a WhatsApp
