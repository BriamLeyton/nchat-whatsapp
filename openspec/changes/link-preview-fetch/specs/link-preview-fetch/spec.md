## Purpose

Mostrar la vista previa de links que no la traen en el mensaje (viejos, enviados desde nchat, o sin vista previa), consultando la página solo de los mensajes visibles, y guardando el resultado para no repetir la consulta.

## ADDED Requirements

### Requirement: Consulta de links visibles
Al mostrar un mensaje de un origen permitido (ver *Filtro por origen*), sin adjunto, con un link y sin vista previa conocida ni consulta previa, el sistema SHALL consultar en segundo plano la página del primer link y obtener título, descripción e imagen:
- de Open Graph;
- si no hay, de Twitter;
- si no hay, de `<title>` y `description`.

La consulta MUST NOT bloquear la UI, y SHALL hacerse una a la vez. Los mensajes no mostrados MUST NOT consultarse.

#### Scenario: Link viejo de Facebook
- **WHEN** se muestra un mensaje viejo con un link `facebook.com/share/r/...`
- **THEN** poco después se ven debajo del link el título, la descripción y la miniatura

#### Scenario: Chat no abierto
- **WHEN** un chat con links nunca se abre
- **THEN** no se consulta ninguno de sus links

#### Scenario: Desactivado
- **WHEN** `link_preview_fetch=0`
- **THEN** no se hace ninguna consulta

### Requirement: Filtro por origen
El sistema SHALL consultar links solo de los orígenes que permite `link_preview_fetch`:
- `0`: ninguno;
- `1`: mensajes propios, y chats 1:1 o estados de contactos guardados en la agenda;
- `2` (default): lo anterior, más cualquier mensaje de un grupo;
- `3`: todos, también números desconocidos y canales.

Un contacto que solo tiene nombre público, sin estar en la agenda, MUST NOT considerarse guardado.

#### Scenario: Desconocido con nivel 2
- **WHEN** `link_preview_fetch=2` y un número que no está en la agenda envía un link en un chat 1:1
- **THEN** no se consulta el link, y se ve solo el link

#### Scenario: Grupo con nivel 1
- **WHEN** `link_preview_fetch=1` y llega un link en un grupo
- **THEN** no se consulta el link

#### Scenario: Grupo con nivel 2
- **WHEN** `link_preview_fetch=2` y llega un link en un grupo de cualquier remitente
- **THEN** se consulta el link

#### Scenario: Link propio
- **WHEN** `link_preview_fetch=1` y uno mismo envía un link a un número desconocido
- **THEN** se consulta el link

### Requirement: Caché de vista previa
El resultado SHALL guardarse en `<msgId>.meta` y `<msgId>.thumb.jpg`, con la marca `link_fetched=1` aunque la consulta falle. Un mensaje marcado MUST NOT volver a consultarse, tampoco después de reiniciar. El mismo URL SHALL consultarse una sola vez por sesión, aunque aparezca en varios mensajes.

#### Scenario: Reinicio
- **WHEN** se reinicia nchat y se muestra un link ya consultado
- **THEN** se ve su vista previa sin nueva consulta

#### Scenario: Página sin metadatos
- **WHEN** la página no devuelve título, descripción ni imagen
- **THEN** se ve solo el link, y no se vuelve a consultar

### Requirement: Texto y miniatura seguros
Los textos SHALL decodificar las entidades HTML y limitarse a 200 caracteres el título y 500 la descripción. La miniatura SHALL convertirse a JPEG de 320 px de ancho como máximo. El URL MUST pasarse al comando entre comillas simples escapadas.

#### Scenario: Entidades
- **WHEN** el título es `D&#xed;a &amp; noche`
- **THEN** se muestra `Día & noche`

#### Scenario: URL con comilla
- **WHEN** el link contiene una comilla simple
- **THEN** el comando recibe el URL completo como un solo argumento, sin ejecutar nada extra
