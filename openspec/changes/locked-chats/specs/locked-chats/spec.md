## Purpose

Ocultar los chats restringidos de WhatsApp (Chat Lock) en nchat, sin notificaciones, y mostrarlos solo con el código secreto, como en WhatsApp Web.

## ADDED Requirements

### Requirement: Sincronización de chats restringidos
nchat SHALL procesar las acciones `lock` y `setting_chatLock` del app state y persistir los chats restringidos y el hash del código secreto en el perfil. Una sola vez por perfil SHALL descargar el estado completo de `regular_low` para conocer el estado actual. MUST NOT repetir esa descarga después de completarla.

#### Scenario: Primera conexión con la función
- **WHEN** nchat se conecta y el perfil no tiene la marca de descarga
- **THEN** descarga el estado completo de `regular_low`, guarda los chats restringidos y crea la marca

#### Scenario: Cambio en el teléfono
- **WHEN** el usuario restringe o quita la restricción de un chat en el teléfono
- **THEN** nchat lo oculta o lo muestra sin reiniciar

### Requirement: Chats restringidos ocultos
Mientras los restringidos están ocultos, nchat MUST NOT mostrarlos en la lista, en la navegación con Tab, en el salto a no leídos, en `goto_chat` ni en Alt-n. Sus mensajes MUST NOT generar notificación de escritorio, campana ni conteo de no leídos, estén ocultos o visibles. Un número o link de un chat restringido oculto SHALL mostrar "This chat is locked.".

#### Scenario: Mensaje en chat restringido
- **WHEN** llega un mensaje a un chat restringido
- **THEN** se guarda, pero no aparece en la lista ni notifica

#### Scenario: Link a chat restringido
- **WHEN** el usuario pega en Alt-n el link de un chat restringido oculto
- **THEN** aparece "This chat is locked." y no se abre el chat

### Requirement: Mostrar con el código secreto
Alt-l (`toggle_locked`) con los restringidos ocultos SHALL pedir el código secreto con entrada enmascarada y compararlo localmente con el hash de WhatsApp. El código MUST NOT enviarse, guardarse ni registrarse en el log. Con código correcto SHALL mostrar la sección `▾ LOCKED` al final de la lista hasta que el usuario pulse Alt-l de nuevo o cierre nchat. Con código incorrecto SHALL mostrar "Wrong secret code.". Sin código configurado SHALL mostrar "Set a secret code for locked chats in WhatsApp on your phone.".

#### Scenario: Código correcto
- **WHEN** el usuario pulsa Alt-l y escribe su código secreto de WhatsApp
- **THEN** aparece `▾ LOCKED` con los chats restringidos

#### Scenario: Ocultar de nuevo
- **WHEN** la sección está visible y el usuario pulsa Alt-l
- **THEN** la sección desaparece y, si el chat abierto era restringido, se abre el primer chat de la lista

#### Scenario: Reinicio
- **WHEN** el usuario cierra y abre nchat
- **THEN** los chats restringidos están ocultos
