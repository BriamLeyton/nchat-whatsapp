## Purpose

Responder un estado enviando un mensaje privado al autor que cita el estado, como en WhatsApp, sin publicar nada en Status.

## ADDED Requirements

### Requirement: Respuesta privada al autor
En Status, con un estado vigente de otra persona seleccionado, enviar un texto SHALL mandarlo al chat privado del autor citando el estado con `RemoteJID` `status@broadcast`. Enviar en Status MUST NOT publicar un estado.

#### Scenario: Responder un estado
- **WHEN** el usuario selecciona un estado vigente de un contacto, escribe un texto y da Enter
- **THEN** el texto llega al chat privado del contacto citando el estado
- **AND** nchat sigue en Status y muestra "Reply sent to <nombre>."

#### Scenario: Sin estado seleccionado
- **WHEN** el usuario escribe en Status sin estado seleccionado y da Enter
- **THEN** no se envía nada, el texto queda en la entrada y aparece el aviso de que no se puede enviar a Status

### Requirement: Respuestas no permitidas
Responder un estado propio o vencido MUST NOT enviar nada y SHALL mostrar un aviso. El texto SHALL quedar en la entrada.

#### Scenario: Estado vencido
- **WHEN** el usuario intenta responder un estado de más de 24 h
- **THEN** aparece "Replying to expired status updates is not supported." y no se envía nada

#### Scenario: Estado propio
- **WHEN** el usuario intenta responder un estado propio
- **THEN** aparece "Replying to own status updates is not supported." y no se envía nada

### Requirement: Cita con miniatura
Si el estado es foto o video con miniatura incluida, la cita SHALL llevar esa miniatura y su texto. Responder MUST NOT descargar el adjunto del estado.

#### Scenario: Responder un video
- **WHEN** el usuario responde un estado de video no descargado
- **THEN** la cita lleva la miniatura incluida y no se descarga el video

### Requirement: Cita visible en el chat del autor
Cuando el mensaje citado no está en el chat, la cita SHALL buscarse en los estados cargados y mostrar su texto o archivo.

#### Scenario: Respuesta a un estado propio recibida
- **WHEN** un contacto responde a un estado propio
- **THEN** en su chat la cita muestra el texto o archivo del estado
