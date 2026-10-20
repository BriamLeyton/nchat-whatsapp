## Purpose

Mostrar arriba del historial una cabecera del chat con foto, datos y estados del contacto, como en WhatsApp, y abrir sus estados con Alt-u desde el chat.

## ADDED Requirements

### Requirement: Banner del chat
Con `chat_banner_enabled=1`, el historial de un chat de contacto o de grupo SHALL mostrar en sus primeras 3 filas un banner con la foto de perfil (si hay soporte de imágenes y foto) y el texto a su derecha, con su propio color y sin línea separadora. Para un contacto SHALL mostrar `nombre · número`, presencia y estados, una fila cada uno; para un grupo, nombre y número de participantes. El banner MUST NOT ocupar más de 3 filas. El chat de Status MUST NOT mostrar banner.

#### Scenario: Chat de contacto
- **WHEN** el usuario abre el chat de un contacto en Ghostty
- **THEN** las 3 primeras filas del historial muestran su foto, `nombre · número`, su presencia y sus estados

#### Scenario: Grupo
- **WHEN** el usuario abre un grupo
- **THEN** el banner muestra la foto del grupo, su nombre y `N participants`

#### Scenario: Sin imágenes
- **WHEN** la terminal no soporta imágenes inline
- **THEN** el banner muestra solo el texto

### Requirement: Colores de presencia
La presencia del banner SHALL usar un color por tipo, configurable: escribiendo y en línea en verde, ausente en amarillo, última vez en gris. Un chat silenciado SHALL agregar ` · muted` en gris.

#### Scenario: Contacto escribiendo
- **WHEN** el contacto empieza a escribir
- **THEN** la fila de presencia muestra `typing` en verde

#### Scenario: Última vez
- **WHEN** el contacto se desconecta
- **THEN** la fila muestra `seen <hora>` en gris

### Requirement: Abrir la foto de perfil
En un chat con banner y sin mensaje seleccionado, `open` (Ctrl-v) SHALL abrir la foto de perfil en tamaño completo con `attachment_open_command`, pidiéndola a WhatsApp solo la primera vez y guardándola en caché. Sin foto SHALL aparecer "No profile photo.". Con un mensaje seleccionado, `open` MUST seguir abriendo su adjunto.

#### Scenario: Abrir la foto
- **WHEN** el usuario pulsa Ctrl-v en el chat de un contacto sin mensaje seleccionado
- **THEN** la foto de perfil completa se abre en el visor externo

#### Scenario: Foto cambiada
- **WHEN** el contacto cambia su foto y el banner recibe la nueva preview
- **THEN** la próxima apertura pide y muestra la foto nueva

### Requirement: Estados en el banner
En el chat de un contacto, el banner SHALL mostrar sus estados de las últimas 24 h: `● N new status updates · Alt-u` si hay sin revisar, o `◯ N status updates · Alt-u` si todos están revisados. Sin estados de las últimas 24 h la fila SHALL quedar vacía. Contar MUST NOT descargar adjuntos.

#### Scenario: Estados sin revisar
- **WHEN** el contacto publicó 3 estados hoy y ninguno se revisó
- **THEN** el banner muestra `● 3 new status updates · Alt-u`

#### Scenario: Estados revisados
- **WHEN** el usuario revisa todos los estados del contacto y vuelve a su chat
- **THEN** el banner muestra `◯ 3 status updates · Alt-u`

### Requirement: Alt-u desde el chat
En el chat de un contacto, Alt-u SHALL pasar a Status filtrado por ese contacto, y Alt-u en ese filtro SHALL volver al chat del contacto. Si el contacto no tiene estados guardados, SHALL aparecer "No status updates from <nombre>." sin cambiar de chat.

#### Scenario: Ver y cerrar
- **WHEN** el usuario pulsa Alt-u en el chat de un contacto y luego Alt-u otra vez
- **THEN** primero ve solo los estados de ese contacto, y después vuelve a su chat

### Requirement: Foto de perfil en caché
La foto SHALL pedirse a WhatsApp en tamaño reducido al abrir el chat, guardarse en `<profile>/avatars/`, y volver a consultarse como máximo una vez cada 24 h por chat, enviando el id guardado para no descargarla si no cambió. Una foto oculta o inexistente SHALL recordarse por 24 h.

#### Scenario: Reabrir el chat
- **WHEN** el usuario abre el mismo chat dos veces en menos de 24 h
- **THEN** solo la primera vez se consulta a WhatsApp

#### Scenario: Foto oculta
- **WHEN** el contacto oculta su foto
- **THEN** el banner se muestra sin foto y no se vuelve a consultar en 24 h
