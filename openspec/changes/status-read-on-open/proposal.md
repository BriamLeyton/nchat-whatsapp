## Why

Al entrar a Status, cada estado que se dibuja en pantalla se marca como visto, y a los vigentes se les envía el aviso de visto a quien los publicó. Es el comportamiento normal de nchat para cualquier chat, pero en estados equivale a decir "vi todos tus estados" solo por pasar por la lista.

El usuario quiere que un estado quede como no visto hasta que lo abra.

## What Changes

- En el chat de estados, dibujar un estado ya no lo marca como visto.
- Un estado se marca como visto solo al abrirlo con:
  - `open`: abrir el adjunto en el visor externo;
  - `open_msg`: ver el texto en el visor externo, para estados de texto.
- Al marcarlo visto:
  - si está vigente, se envía el aviso de visto, como hoy;
  - si está vencido, se marca solo en local, sin aviso, como hoy.
- Los estados no abiertos siguen contando como no leídos en la fila de Status.
- Los demás chats no cambian: se marcan al verse.

## Capabilities

### New Capabilities
- `status-read-on-open`: estados marcados como vistos solo al abrirlos.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: `MarkRead` ignora el chat de estados salvo apertura explícita; `OnKeyOpenAttachment` y `OnKeyOpenMsg` marcan el estado seleccionado.
- `README.md`: comportamiento de visto en estados.
