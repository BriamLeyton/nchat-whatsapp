## Why

Desde `status-read-on-open`, recorrer los estados ya no los marca vistos. Por eso el `*` de la fila Status no se quita aunque el usuario haya revisado toda la lista.

Hoy el `*` de Status se calcula como en cualquier chat, mirando solo el estado más reciente:
- si nunca se abre el más reciente, el `*` queda para siempre, incluso después de vencer;
- si se abre solo el más reciente, el `*` se quita aunque queden otros sin abrir.

Además, dentro de Status no hay forma de distinguir qué estados ya se revisaron en la lista.

El usuario quiere ver qué estados son nuevos, y que dejen de serlo al recorrerlos con el cursor, sin abrirlos.

## What Changes

- Cada estado entrante pasa por tres situaciones:
  - **nuevo:** se muestra con `●` antes del nombre del contacto;
  - **revisado:** sin `●`. Pasa a revisado cuando el cursor de selección lo selecciona. Es solo local: no se envía visto;
  - **abierto:** visto de verdad con `open` u `open_msg`, con aviso de visto si está vigente, como hoy. Un estado abierto tampoco lleva `●`.
- La fila Status de la lista muestra la misma marca `●` (en lugar de `*`) cuando hay al menos un estado nuevo.
- Los estados revisados se guardan en `<profile>/status_seen`, así no vuelven a ser nuevos al reiniciar.
- La primera vez que corre, sin `status_seen`, los estados que ya existían cuentan como revisados, para no marcar todo el historial.
- La marca es configurable con `status_new_indicator` en `ui.conf` (default `●`).
- Los demás chats no cambian.

## Capabilities

### New Capabilities
- `status-seen-indicator`: marca de estados nuevos que se quita al seleccionarlos, también en la fila Status.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: conjunto de estados revisados por perfil con su archivo; `IsStatusNew`, `MarkStatusSeen`; `UpdateChatInfoIsUnread` usa la marca para Status.
- `src/uihistoryview.cpp`: `●` antes del nombre en estados nuevos; marcar revisado el estado seleccionado.
- `src/uilistview.cpp`: la fila Status usa `status_new_indicator` en lugar de `unread_indicator`.
- `src/uiconfig.cpp`: `status_new_indicator`.
- `README.md`: `status_new_indicator` y significado de la marca en los estados y en la fila Status.
