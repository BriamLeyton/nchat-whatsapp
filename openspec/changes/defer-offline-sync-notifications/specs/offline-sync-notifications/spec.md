## Purpose

Evitar que, al reconectar tras una suspensión o un corte de red, suenen de golpe las notificaciones de mensajes que ya se leyeron en el teléfono. Solo se avisa de los chats que siguen sin leer cuando termina la sincronización offline.

## ADDED Requirements

### Requirement: Retención durante la sincronización offline
Cuando WhatsApp anuncia que va a entregar eventos pendientes de un perfil, nchat SHALL entrar en retención para ese perfil. Durante la retención, los mensajes entrantes MUST NOT generar campana ni notificación de escritorio al llegar. Los mensajes SHALL guardarse y mostrarse en la lista y el historial como siempre, incluido el marcador de no leído.

#### Scenario: Reconexión tras suspender
- **WHEN** el equipo vuelve de suspensión, nchat reconecta y WhatsApp entrega 20 mensajes pendientes en 5 chats
- **THEN** ningún mensaje suena ni genera notificación de escritorio mientras se reciben

#### Scenario: Reconexión sin eventos pendientes
- **WHEN** nchat reconecta y WhatsApp no anuncia eventos pendientes
- **THEN** no hay retención y los mensajes nuevos notifican como siempre

### Requirement: Notificación al terminar la sincronización
Al terminar la sincronización offline, tras un margen de 2 segundos, nchat SHALL revisar los chats que habrían notificado durante la retención. Para cada uno, si su último mensaje entrante sigue sin leer, SHALL enviar una sola notificación de escritorio con ese mensaje. Si se leyó en otro dispositivo durante la sincronización, MUST NOT notificar. La campana SHALL sonar como mucho una vez por fin de retención. Esto aplica aunque `notify_every_unread` esté activo.

#### Scenario: Mensajes ya leídos en el teléfono
- **WHEN** la sincronización entrega 3 mensajes de Ana y después el recibo de leído de esos mensajes desde el teléfono
- **THEN** al terminar no hay campana ni notificación de Ana

#### Scenario: Chat que sigue sin leer
- **WHEN** la sincronización entrega 4 mensajes de Juan sin recibo de leído
- **THEN** al terminar aparece una sola notificación de Juan con su último mensaje, y la campana suena una vez

#### Scenario: Varios chats sin leer
- **WHEN** al terminar siguen sin leer los chats de Juan y de Marta
- **THEN** aparece una notificación por cada chat y la campana suena una sola vez

### Requirement: Reglas de notificación existentes
La notificación diferida MUST respetar las mismas reglas que la notificación inmediata: chats silenciados (salvo menciones o `muted_notify_unread`), archivados y restringidos no notifican. `terminal_bell_active`/`terminal_bell_inactive` y `desktop_notify_*` SHALL evaluarse en el momento de notificar. Los mensajes enviados antes de la conexión inicial de nchat MUST NOT notificar, igual que hoy.

#### Scenario: Chat silenciado
- **WHEN** un chat silenciado sin mención sigue sin leer al terminar la sincronización
- **THEN** no notifica

#### Scenario: Arranque de nchat
- **WHEN** nchat arranca y WhatsApp entrega mensajes antiguos pendientes
- **THEN** no notifican, igual que antes de este cambio

### Requirement: Tiempo máximo de retención
Si la sincronización no termina en 60 segundos desde que empezó, nchat SHALL terminar la retención y aplicar la misma revisión que al terminar normalmente. Una desconexión durante la retención MUST NOT dejar la retención activa para siempre.

#### Scenario: Fin de sincronización perdido
- **WHEN** empieza la sincronización offline y no llega su fin en 60 segundos
- **THEN** nchat termina la retención y notifica los chats que siguen sin leer

### Requirement: Notificación inmediata sin cambios
Fuera de la retención, los mensajes nuevos SHALL notificar al llegar, como antes de este cambio.

#### Scenario: Mensaje con nchat conectado
- **WHEN** nchat está conectado sin sincronización offline en curso y llega un mensaje
- **THEN** notifica en el momento según la configuración actual
