## Purpose

Permitir ver solo los estados de un contacto dentro del chat de estados, eligiéndolo desde un estado seleccionado o desde un diálogo.

## ADDED Requirements

### Requirement: Filtrar por el autor del estado seleccionado
En el chat de estados, con un estado seleccionado y sin filtro activo, el comando `status_filter` SHALL filtrar el historial para mostrar solo los estados del autor del estado seleccionado. El estado seleccionado MUST seguir seleccionado después de filtrar.

#### Scenario: Filtrar desde la selección
- **WHEN** el usuario selecciona un estado de Jane Doe y presiona `Alt-u`
- **THEN** el historial muestra solo los estados de Jane Doe y el mismo estado queda seleccionado

### Requirement: Diálogo de contactos con estados
En el chat de estados, sin selección y sin filtro activo, el comando `status_filter` SHALL abrir un diálogo con los contactos que tienen estados, ordenados por la fecha de su estado más reciente. El diálogo MUST permitir buscar escribiendo, como los demás diálogos de lista. Elegir un contacto MUST aplicar el filtro, y cancelar MUST dejar todo igual.

#### Scenario: Elegir contacto
- **WHEN** el usuario abre el diálogo, escribe "jane" y elige Jane Doe
- **THEN** el historial muestra solo los estados de Jane Doe, desde el más reciente

#### Scenario: Cancelar
- **WHEN** el usuario abre el diálogo y presiona `Esc`
- **THEN** no se aplica ningún filtro

#### Scenario: Contactos con estados antiguos
- **WHEN** un contacto solo tiene estados que todavía no se cargaron del historial
- **THEN** igual aparece en el diálogo

### Requirement: Quitar el filtro
Con un filtro activo, el comando `status_filter` SHALL quitar el filtro y mostrar todos los estados. Salir del chat de estados MUST quitar el filtro.

#### Scenario: Mismo comando
- **WHEN** hay un filtro activo y el usuario presiona `Alt-u`
- **THEN** el historial vuelve a mostrar todos los estados, desde el más reciente

#### Scenario: Cambiar de chat
- **WHEN** hay un filtro activo, el usuario va a otro chat y vuelve a estados
- **THEN** se ven todos los estados

### Requirement: Indicador del filtro
Mientras el filtro está activo, la fila de estados de la lista SHALL mostrar el nombre del contacto filtrado después del nombre de la fila.

#### Scenario: Fila con filtro
- **WHEN** el filtro de Jane Doe está activo
- **THEN** la fila muestra `◉ Status · Jane Doe`

### Requirement: Navegación y acciones sobre la lista filtrada
Con el filtro activo, la selección de mensajes, `PgUp`/`PgDn` y las acciones sobre el mensaje seleccionado SHALL operar sobre los estados filtrados. Si la lista filtrada no llena el historial y hay más estados en la caché, el sistema MUST cargar más estados hasta llenarlo o hasta que no queden.

#### Scenario: Subir por la lista filtrada
- **WHEN** el filtro de Jane Doe está activo y el usuario sube con `↑`
- **THEN** la selección pasa solo por estados de Jane Doe

#### Scenario: Abrir un estado filtrado
- **WHEN** el usuario abre el adjunto del estado seleccionado con el filtro activo
- **THEN** se abre el adjunto de ese estado, no el de otro

#### Scenario: Pocos estados cargados
- **WHEN** el contacto filtrado tiene estados viejos que aún no están en memoria
- **THEN** se cargan de la caché y aparecen en el historial

#### Scenario: Estado nuevo de otro contacto
- **WHEN** el filtro de Jane Doe está activo y llega un estado de otro contacto
- **THEN** no aparece en el historial filtrado

### Requirement: Solo en el chat de estados
Fuera del chat de estados, el comando `status_filter` SHALL NOT tener efecto.

#### Scenario: Alt-u en un chat normal
- **WHEN** el usuario presiona `Alt-u` en un chat que no es el de estados
- **THEN** no pasa nada
