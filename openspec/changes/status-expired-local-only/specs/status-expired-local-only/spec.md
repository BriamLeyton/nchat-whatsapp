## Purpose

Evitar que nchat genere tráfico hacia WhatsApp por estados vencidos, y evitar publicar estados por accidente desde el chat de estados.

## ADDED Requirements

### Requirement: Estado vencido
El sistema SHALL considerar vencido a todo mensaje del chat de estados (`status@broadcast`) cuya hora de publicación tenga más de 24 horas respecto de la hora actual.

#### Scenario: Estado de ayer
- **WHEN** un estado se publicó hace 25 horas
- **THEN** se considera vencido

#### Scenario: Estado de hoy
- **WHEN** un estado se publicó hace 3 horas
- **THEN** no se considera vencido y funciona como hoy

### Requirement: Visto solo local
Al mostrar un estado vencido no leído, el sistema SHALL marcarlo como leído en memoria y en la caché local, y MUST NOT enviar la confirmación de lectura a WhatsApp.

#### Scenario: Recorrer estados viejos no leídos
- **WHEN** el usuario recorre estados de hace 5 días que estaban sin leer
- **THEN** quedan leídos en nchat, también después de reiniciar, y no se envía ninguna confirmación de lectura

#### Scenario: Estado vigente
- **WHEN** el usuario ve un estado de hace 2 horas sin leer
- **THEN** se envía la confirmación de lectura como hoy

### Requirement: Sin descargas de estados vencidos
El sistema MUST NOT pedir a WhatsApp la descarga del adjunto de un estado vencido por ningún motivo: descarga automática, imagen inline, selección, abrir, guardar o ver en visor externo. Si el adjunto ya está descargado, SHALL usarse el archivo local. Si no, el historial MUST mostrar la línea `📎` sin indicador de descarga en curso, y abrir o guardar MUST mostrar el aviso "Expired status, attachment not available."

#### Scenario: Foto inline vencida sin descargar
- **WHEN** se muestra un estado vencido con una foto que nunca se descargó
- **THEN** se ve la línea `📎` sin imagen y no se hace ninguna petición de descarga

#### Scenario: Foto vencida ya descargada
- **WHEN** se muestra un estado vencido cuya foto ya está en disco
- **THEN** la foto se ve inline como hoy

#### Scenario: Abrir adjunto vencido
- **WHEN** el usuario selecciona un estado vencido sin archivo descargado y lo abre
- **THEN** aparece el aviso y no se hace ninguna petición de descarga

### Requirement: Sin reacciones a estados vencidos
Reaccionar a un estado vencido SHALL NOT enviar nada a WhatsApp, y MUST mostrar el aviso "Expired status, cannot react."

#### Scenario: Reaccionar a estado viejo
- **WHEN** el usuario selecciona un estado de hace 3 días y presiona `Alt-s`
- **THEN** aparece el aviso y no se envía la reacción

### Requirement: Chat de estados de solo lectura
En el chat de estados, enviar texto, enviar archivos y editar mensajes SHALL NOT enviar nada a WhatsApp, y MUST mostrar el aviso "Sending to Status Updates is not supported." El texto escrito en la caja de entrada MUST conservarse.

#### Scenario: Enter en Status
- **WHEN** el usuario escribe "hola" en el chat de estados y presiona `Enter`
- **THEN** aparece el aviso, no se publica ningún estado y "hola" sigue en la caja de entrada

#### Scenario: Enviar archivo en Status
- **WHEN** el usuario intenta enviar un archivo en el chat de estados
- **THEN** aparece el aviso y no se envía nada
