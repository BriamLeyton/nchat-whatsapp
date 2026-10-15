## Purpose

Seleccionar mensajes del historial como en una lista normal: la vista queda fija y la selección se mueve, desplazando solo cuando la selección sale de la vista.

## ADDED Requirements

### Requirement: Vista fija al mover la selección
Mientras la selección de mensajes está activa, mover la selección con `↑`/`↓` hacia un mensaje que ya se ve completo MUST NOT desplazar el historial. El mensaje seleccionado SHALL resaltarse en su posición actual.

#### Scenario: Entrar a la selección
- **WHEN** el historial muestra los últimos 6 mensajes y el usuario presiona `↑` desde la caja de texto vacía
- **THEN** se selecciona el último mensaje y el historial no se mueve

#### Scenario: Subir dentro de la vista
- **WHEN** está seleccionado el último mensaje y el usuario presiona `↑` 3 veces
- **THEN** queda seleccionado el cuarto mensaje desde abajo y los 6 mensajes siguen en la misma posición

### Requirement: Desplazamiento en los bordes
Si la selección pasa a un mensaje que no se ve completo, el sistema SHALL desplazar el historial lo mínimo necesario para mostrarlo completo:
- **Hacia arriba:** el mensaje queda arriba de la vista.
- **Hacia abajo:** el mensaje queda abajo de la vista.

Si el mensaje seleccionado no cabe en la vista, se muestra recortado con `[...]`, como hoy.

#### Scenario: Pasar el borde superior
- **WHEN** está seleccionado el mensaje de más arriba que se ve completo y el usuario presiona `↑`
- **THEN** el historial se desplaza y el mensaje anterior queda seleccionado y visible completo arriba

#### Scenario: Volver hacia abajo
- **WHEN** el historial se desplazó hacia arriba y el usuario presiona `↓` hasta pasar el borde inferior
- **THEN** el historial se desplaza lo necesario para mostrar el mensaje seleccionado abajo

### Requirement: Salir de la selección
Al salir de la selección con `↓` desde el mensaje más reciente, con `Esc`/cancelar o al enviar, el historial SHALL volver a mostrar los mensajes más recientes, como hoy.

#### Scenario: Salir con flecha abajo
- **WHEN** está seleccionado el mensaje más reciente y el usuario presiona `↓`
- **THEN** la selección termina y el historial muestra el final del chat

### Requirement: Saltos a mensajes
Las acciones que seleccionan un mensaje específico (ir al citado, buscar, ir al fijado) SHALL mostrarlo completo, desplazando la vista solo si no se ve. `PgUp`/`PgDn` MUST mantener su comportamiento actual.

#### Scenario: Ir al mensaje citado visible
- **WHEN** el mensaje citado ya se ve completo en pantalla y el usuario usa `jump_quoted`
- **THEN** la selección pasa a ese mensaje sin desplazar el historial

#### Scenario: Ir al mensaje citado fuera de vista
- **WHEN** el mensaje citado está más arriba de la vista
- **THEN** el historial se desplaza y el mensaje queda seleccionado y visible completo
