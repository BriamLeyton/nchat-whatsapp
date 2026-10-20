## Purpose

Mostrar en el historial el estado de entrega de los mensajes enviados (enviado, entregado, leído) con los símbolos y colores de WhatsApp, agregando correctamente los receipts de grupos y conservando el estado entre reinicios.

## ADDED Requirements

### Requirement: Estados de entrega de mensajes enviados
El sistema SHALL asignar a cada mensaje enviado por el usuario uno de estos estados, en orden creciente: `sent` (aceptado por el servidor), `delivered` (entregado al destinatario) y `read` (leído o reproducido). El estado de un mensaje MUST NOT retroceder: un receipt de un estado menor que el actual MUST ser ignorado.

#### Scenario: Mensaje recién enviado
- **WHEN** el usuario envía un mensaje y el servidor lo acepta
- **THEN** el mensaje queda en estado `sent`

#### Scenario: Receipt de entrega en chat individual
- **WHEN** llega un receipt de entrega del destinatario para un mensaje en `sent`
- **THEN** el mensaje pasa a `delivered`

#### Scenario: Receipt de lectura en chat individual
- **WHEN** llega un receipt de lectura o de reproducción del destinatario
- **THEN** el mensaje pasa a `read`

#### Scenario: Receipt de entrega tardío
- **WHEN** un mensaje ya está en `read` y llega un receipt de entrega
- **THEN** el mensaje permanece en `read`

#### Scenario: Mensaje enviado desde otro dispositivo
- **WHEN** el usuario envía un mensaje desde su teléfono y nchat lo recibe sincronizado
- **THEN** el mensaje aparece en estado `sent` y avanza con los receipts que lleguen después

### Requirement: Agregación en grupos
En chats de grupo, el sistema SHALL marcar un mensaje enviado como `delivered` solo cuando todos los demás participantes del grupo lo entregaron (o leyeron), y como `read` solo cuando todos los demás participantes lo leyeron. Los receipts de los otros dispositivos del propio usuario MUST NOT contar como participantes.

#### Scenario: Grupo parcialmente leído
- **WHEN** en un grupo de 5 participantes (el usuario y 4 más) 3 participantes leyeron el mensaje y el cuarto solo lo recibió
- **THEN** el mensaje está en `delivered`

#### Scenario: Grupo leído por todos
- **WHEN** los 4 demás participantes leyeron el mensaje
- **THEN** el mensaje pasa a `read`

#### Scenario: Reinicio con lecturas parciales
- **WHEN** 3 de 4 participantes leyeron el mensaje, el usuario reinicia nchat y luego lee el cuarto
- **THEN** el mensaje pasa a `read`

### Requirement: Persistencia del estado
El sistema SHALL guardar el estado de entrega de cada mensaje enviado en la caché local, de modo que al reiniciar nchat se muestre el último estado conocido. La actualización de la caché MUST preservar todos los mensajes y datos existentes.

#### Scenario: Reinicio
- **WHEN** un mensaje quedó en `delivered` y el usuario reinicia nchat
- **THEN** el mensaje se muestra como `delivered`

#### Scenario: Actualización desde la versión anterior
- **WHEN** el usuario arranca esta versión con una caché existente
- **THEN** la caché se actualiza sin perder mensajes, y las consultas existentes sobre `messages` y `contacts2` siguen funcionando

### Requirement: Estado inferido para mensajes previos
Para mensajes enviados que no tienen estado de entrega guardado (anteriores a este cambio), el sistema SHALL mostrar `read` si el mensaje está marcado como leído y `delivered` en caso contrario. Un receipt posterior MUST poder avanzar ese estado.

#### Scenario: Mensaje viejo leído
- **WHEN** un mensaje enviado previo a la actualización está marcado como leído
- **THEN** se muestra como `read`

#### Scenario: Mensaje viejo no leído
- **WHEN** un mensaje enviado previo a la actualización no está marcado como leído
- **THEN** se muestra como `delivered`

### Requirement: Representación visual
El historial SHALL mostrar el indicador de estado al final del encabezado (nombre y hora) de cada mensaje enviado: `✓` para `sent`, `✓✓` para `delivered` y `✓✓` para `read`. Por defecto, `sent` y `delivered` MUST usar el gris de WhatsApp (`#8696a0`) y `read` el azul de WhatsApp (`#53bdeb`). En terminales que no pueden redefinir colores pero tienen 256 colores, MUST usarse el color más cercano de la paleta xterm-256; con 8 colores, `blue` para `read` y el color por defecto para el resto. Los mensajes recibidos MUST NOT mostrar indicador de estado. Los mensajes pendientes de envío MUST NOT mostrar indicador.

#### Scenario: Mensaje entregado
- **WHEN** un mensaje enviado está en `delivered`
- **THEN** su encabezado termina en `✓✓` en gris

#### Scenario: Mensaje leído
- **WHEN** un mensaje enviado está en `read`
- **THEN** su encabezado termina en `✓✓` en azul

#### Scenario: Mensaje recibido leído
- **WHEN** el usuario leyó un mensaje recibido
- **THEN** su encabezado no muestra `✓`

#### Scenario: Selección del mensaje
- **WHEN** un mensaje enviado está seleccionado
- **THEN** el indicador mantiene su símbolo y su color, con el atributo de selección

### Requirement: Configuración de símbolos y colores
El sistema SHALL leer los símbolos desde `ui.conf` (`receipt_sent_indicator`, `receipt_delivered_indicator`, `receipt_read_indicator`) y los colores desde `color.conf` (`history_receipt_sent_color_fg/_bg`, `history_receipt_delivered_color_fg/_bg`, `history_receipt_read_color_fg/_bg`). Las claves MUST agregarse con su valor por defecto cuando no existan, sin modificar las claves existentes.

#### Scenario: Primera ejecución tras actualizar
- **WHEN** el usuario arranca esta versión con `ui.conf` y `color.conf` existentes
- **THEN** ambos archivos incluyen las claves nuevas con sus valores por defecto y el resto queda igual

#### Scenario: Color personalizado
- **WHEN** `color.conf` contiene `history_receipt_read_color_fg=green`
- **THEN** los mensajes leídos muestran `✓✓` en verde
