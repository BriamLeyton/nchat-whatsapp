## Purpose

Marcar un estado como visto, y avisar a quien lo publicó, solo cuando el usuario lo abre, no al pasar por la lista de estados.

## ADDED Requirements

### Requirement: Estados no vistos al mostrarse
Mostrar un estado en el chat de estados MUST NOT marcarlo como visto ni enviar el aviso de visto. Los demás chats SHALL seguir marcándose al verse.

#### Scenario: Pasar por la lista
- **WHEN** el usuario entra a Status y ve 10 estados nuevos sin abrir ninguno
- **THEN** los 10 siguen como no vistos, no se envía ningún aviso y la fila de Status sigue con indicador de no leído

#### Scenario: Chat normal
- **WHEN** el usuario abre un chat de un contacto con mensajes nuevos
- **THEN** los mensajes se marcan como leídos como antes

### Requirement: Visto al abrir
Al abrir un estado recibido no leído con `open` o `open_msg`, el sistema SHALL marcarlo como visto. Si está vigente SHALL enviar el aviso de visto. Si está vencido SHALL marcarlo solo en local, sin aviso.

#### Scenario: Abrir imagen de estado vigente
- **WHEN** el usuario selecciona un estado de imagen vigente y presiona `open`
- **THEN** se abre la imagen, el estado queda visto y quien lo publicó recibe el aviso

#### Scenario: Abrir estado de texto
- **WHEN** el usuario selecciona un estado de texto y presiona `open_msg`
- **THEN** el estado queda visto

#### Scenario: Seleccionar sin abrir
- **WHEN** el usuario solo mueve el cursor sobre un estado
- **THEN** el estado sigue como no visto

#### Scenario: Estado vencido
- **WHEN** el usuario presiona `open` sobre un estado vencido no leído
- **THEN** el estado queda visto en local, y no se envía aviso ni se descarga nada
