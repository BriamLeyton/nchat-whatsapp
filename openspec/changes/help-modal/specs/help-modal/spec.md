## Purpose

Mostrar todos los comandos de nchat en un modal legible, con atajos en notación `Alt+p` / `Ctrl+s`.

## ADDED Requirements

### Requirement: Modal de ayuda
La tecla `toggle_help` (por defecto `Ctrl-g`) SHALL abrir un modal centrado con la lista de comandos agrupada por categoría:
- Chats;
- Mensajes;
- Mensaje seleccionado;
- Edición de texto;
- Vista.

Cada entrada MUST mostrar el atajo configurado y una descripción en español. Las funciones sin tecla asignada MUST NOT aparecer.

#### Scenario: Abrir la ayuda
- **WHEN** el usuario presiona `Ctrl-g` en la vista principal
- **THEN** aparece un modal con los comandos, incluyendo `Alt+p  Fijar/desfijar chat`

#### Scenario: Tecla reasignada
- **WHEN** `key.conf` tiene `pin=\33\153`
- **THEN** el modal muestra `Alt+k  Fijar/desfijar chat`

### Requirement: Cerrar y desplazar
El modal SHALL cerrarse con `Esc`, `toggle_help` o `cancel` sin ejecutar ninguna otra acción. Si la lista no cabe en el modal, `↑`/`↓` MUST desplazarla una línea y `PgUp`/`PgDn` una página. Al cambiar el tamaño de la terminal, el modal MUST redibujarse con el nuevo tamaño.

#### Scenario: Cerrar con Esc
- **WHEN** el modal está abierto y el usuario presiona `Esc`
- **THEN** el modal se cierra en menos de 100 ms y la vista vuelve como estaba

#### Scenario: Lista larga
- **WHEN** la terminal es más baja que la lista de comandos
- **THEN** el modal muestra un indicador de que hay más contenido y `↓` lo desplaza

### Requirement: Notación de teclas
El modal y la barra de ayuda inferior SHALL mostrar las teclas con `Alt+<tecla>` y `Ctrl+<tecla>` (por ejemplo `Alt+p`, `Ctrl+s`). Los defaults de `help_prefix_meta` y `help_prefix_ctrl` MUST ser `Alt+` y `Ctrl+`. Los valores anteriores `M-` y `^` MUST migrarse a los nuevos. Cualquier otro valor personalizado MUST respetarse.

#### Scenario: Configuración existente
- **WHEN** `ui.conf` tiene `help_prefix_meta=M-` y `help_prefix_ctrl=^`
- **THEN** tras arrancar, `ui.conf` tiene `help_prefix_meta=Alt+` y `help_prefix_ctrl=Ctrl+`, y la ayuda muestra `Alt+p`

### Requirement: Barra inferior
La barra de ayuda inferior SHALL seguir mostrándose u ocultándose según `help_enabled` en `ui.conf`. `toggle_help` MUST NOT alternarla.

#### Scenario: Barra activa
- **WHEN** `help_enabled=1` y el usuario abre y cierra el modal
- **THEN** la barra inferior sigue visible
