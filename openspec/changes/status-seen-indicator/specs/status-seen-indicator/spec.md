## Purpose

Mostrar qué estados son nuevos y quitar esa marca al recorrerlos con el cursor, sin marcarlos como vistos, y mostrar la misma marca en la fila Status.

## ADDED Requirements

### Requirement: Marca de estado nuevo
Un estado entrante sin abrir, recibido después de la línea base y no revisado SHALL mostrarse con `status_new_indicator` (default `●`) antes del nombre del contacto. Los estados abiertos, revisados o propios MUST NOT llevar la marca.

#### Scenario: Estado nuevo
- **WHEN** llega un estado de un contacto y el usuario entra a Status
- **THEN** la línea del estado muestra `●` antes del nombre

#### Scenario: Estado abierto
- **WHEN** el usuario abre un estado con `open` u `open_msg`
- **THEN** la línea del estado no muestra `●`

### Requirement: Revisar al seleccionar
Seleccionar un estado con el cursor SHALL quitarle la marca. Seleccionar MUST NOT marcarlo como visto ni enviar aviso de visto.

#### Scenario: Recorrer con el cursor
- **WHEN** el cursor de selección pasa por un estado nuevo
- **THEN** el estado pierde la marca
- **AND** sigue sin ver hasta abrirlo con `open` u `open_msg`

#### Scenario: Estado no recorrido
- **WHEN** un estado nuevo se muestra en pantalla pero no se selecciona
- **THEN** conserva la marca

### Requirement: Marca en la fila Status
La fila Status de la lista SHALL mostrar `status_new_indicator` en lugar de `unread_indicator`, si y solo si hay al menos un estado nuevo entre los cargados.

#### Scenario: Todos revisados
- **WHEN** el usuario selecciona todos los estados nuevos
- **THEN** la marca de la fila Status desaparece

#### Scenario: Llega uno nuevo
- **WHEN** llega un estado nuevo con Status fuera de pantalla
- **THEN** la fila Status muestra `●`, sin notificación

### Requirement: Revisados persistentes
Los estados revisados SHALL guardarse en `<profile>/status_seen`. Sin ese archivo, los estados existentes al primer arranque MUST contar como revisados.

#### Scenario: Reinicio
- **WHEN** el usuario revisó estados, cierra nchat y lo vuelve a abrir
- **THEN** esos estados no muestran la marca

#### Scenario: Primer arranque
- **WHEN** nchat arranca sin `status_seen`
- **THEN** ningún estado existente muestra la marca
