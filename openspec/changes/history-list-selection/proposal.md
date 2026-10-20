## Why

Hoy, al entrar a la selección de mensajes con `↑` desde la caja de texto, el mensaje seleccionado siempre queda abajo de todo. Cada `↑` desplaza el historial completo un mensaje hacia abajo. El usuario pierde de vista los mensajes recientes, y el chat "salta" con cada tecla en vez de comportarse como una lista normal.

## What Changes

- Al seleccionar con `↑`, el historial queda quieto y la selección sube por los mensajes visibles.
- El historial solo se desplaza cuando la selección pasa del borde:
  - **Arriba:** el mensaje seleccionado entra completo por arriba.
  - **Abajo, con `↓`:** el historial baja lo necesario para mostrarlo.
- Al salir de la selección (`↓` en el último mensaje), el historial vuelve al final como hoy.
- Las acciones que saltan a un mensaje fuera de la vista (ir al citado, buscar, ir al fijado) desplazan el historial para mostrarlo. Si el mensaje ya se ve, no se desplaza nada.
- `PgUp`/`PgDn` siguen igual: desplazan el historial y salen de la selección.

## Capabilities

### New Capabilities
- `history-list-selection`: selección de mensajes en el historial con comportamiento de lista (vista fija, selección móvil).

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: nuevo desplazamiento de la vista por chat, separado del índice del mensaje seleccionado (`m_MessageOffset`, que se mantiene), y su reinicio al entrar y salir de la selección.
- `src/uihistoryview.cpp`: dibuja desde el desplazamiento de la vista, resalta el mensaje seleccionado esté donde esté y ajusta la vista si el seleccionado no se ve completo.
- Sin cambios de protocolo, caché ni configuración.
