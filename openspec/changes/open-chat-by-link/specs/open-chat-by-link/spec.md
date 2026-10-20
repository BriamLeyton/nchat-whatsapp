## Purpose

Abrir o crear un chat desde el diálogo de contactos escribiendo o pegando un número o un link de WhatsApp, con el texto del link listo para enviar.

## ADDED Requirements

### Requirement: Número o link en el filtro de contactos
Cuando el filtro del diálogo de contactos contiene un link de WhatsApp (`wa.me`, `api.whatsapp.com/send`, `whatsapp://send`) o un número internacional de 8 a 15 dígitos, el diálogo SHALL mostrar como primera fila `Open chat with +<número>`. El número SHALL salir del path o del parámetro `phone`, sin caracteres que no sean dígitos.

#### Scenario: Link de api.whatsapp.com
- **WHEN** el usuario pega `https://api.whatsapp.com/send/?phone=15551234567&text=Hello+ACME.&type=phone_number&app_absent=0` en Alt-n
- **THEN** la primera fila es `Open chat with +15551234567`

#### Scenario: Número escrito con formato
- **WHEN** el usuario escribe `+1 555 123 4567`
- **THEN** la primera fila es `Open chat with +15551234567`

#### Scenario: Texto normal
- **WHEN** el usuario escribe un nombre
- **THEN** no aparece la fila y la lista filtra contactos como antes

### Requirement: Abrir o crear el chat
Al elegir la fila, si existe un chat con ese número SHALL abrirse (desarchivándolo si estaba archivado) sin consultar a WhatsApp. Si no existe, nchat SHALL consultar una sola vez si el número está registrado; si lo está, SHALL crear y abrir el chat con el número que devuelve WhatsApp (`<número>@s.whatsapp.net`), nunca con su LID, para que los mensajes enviados y recibidos queden en ese mismo chat. Si no está registrado SHALL mostrar "+<número> is not on WhatsApp.". Sin conexión SHALL mostrar "Not connected, please try again later." y MUST NOT consultar. La consulta MUST NOT hacerse mientras el usuario escribe en el filtro.

#### Scenario: Chat existente
- **WHEN** el usuario elige la fila de un número con chat
- **THEN** se abre ese chat y no se envía ninguna consulta

#### Scenario: Número nuevo registrado
- **WHEN** el usuario elige la fila de un número sin chat que está en WhatsApp
- **THEN** se crea el chat y queda abierto

#### Scenario: Primer mensaje a un número nuevo
- **WHEN** el usuario crea el chat de un número nuevo y envía un mensaje
- **THEN** el mensaje aparece en ese chat, que sube al inicio de su sección y sigue seleccionado, sin crear un segundo chat

#### Scenario: Número no registrado
- **WHEN** el número no está en WhatsApp
- **THEN** aparece "+<número> is not on WhatsApp." y no se crea ningún chat

### Requirement: Texto del link en la caja de mensaje
Si el link tiene `text`, al abrir el chat el texto decodificado SHALL quedar en la caja de mensaje con el cursor al final, sin enviarse. Si el chat ya tenía un borrador, el borrador SHALL mantenerse y SHALL aparecer "Draft kept, link text not inserted.".

#### Scenario: Texto con acentos
- **WHEN** el link tiene `text=Hello+ACME.+I+want+more+information`
- **THEN** la caja de mensaje muestra `Hola ACME. Quiero más información` y nada se envía

#### Scenario: Borrador existente
- **WHEN** el chat ya tiene texto escrito en la caja
- **THEN** el texto escrito no cambia y aparece el aviso
