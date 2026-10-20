## Why

En WhatsApp, cada chat tiene arriba una cabecera con la foto, el nombre y la presencia del contacto. Además, un anillo alrededor de la foto avisa si el contacto tiene estados, y tocarlo los abre.

En nchat la barra inferior solo muestra nombre y presencia en una línea. No hay foto, y para ver los estados de un contacto hay que ir a Status, filtrar con Alt-u y volver a mano.

## What Changes

- **Banner de 3 filas en la parte superior del historial**, en chats de contactos y en grupos:
  - foto de perfil inline, en terminales con imágenes (Ghostty, kitty). Sin soporte de imágenes, el banner se muestra solo con texto;
  - contacto: `nombre · número`, presencia (escribiendo, en línea, última vez) y estados, una fila cada uno;
  - grupo: nombre y número de participantes;
  - sin línea separadora: el banner se distingue con su propio color, `history_banner_color`.
- **Estados del contacto** en la tercera fila del banner, solo en contactos, contando los de las últimas 24 h:
  - `● 3 new status updates · Alt-u` si hay estados sin revisar (con la marca `●` de `status-seen-indicator`);
  - `◯ 5 status updates · Alt-u` si ya los revisaste todos;
  - fila vacía si no tiene estados de las últimas 24 h.
- **Colores de presencia** en la fila de presencia, configurables:
  - escribiendo: verde;
  - en línea: verde;
  - ausente (`away`): amarillo;
  - última vez (`seen ...`): gris;
  - si el chat está silenciado, ` · muted` en gris al final.
- **Ctrl-v (`open`) sin mensaje seleccionado** abre la foto de perfil del chat con `attachment_open_command`, como las demás fotos.
  - Se abre la foto en tamaño completo. Se pide a WhatsApp solo al abrirla, y se guarda en `<profile>/avatars/<chatId>.full.jpg`.
  - Si la foto cambió (cambia el id de la preview), la copia completa se borra y se vuelve a pedir la próxima vez.
  - Sin foto, aparece "No profile photo.".
  - Con un mensaje seleccionado, Ctrl-v sigue abriendo su adjunto como hoy.
- **Alt-u en el chat de un contacto** pasa a Status filtrado por ese contacto. Alt-u otra vez lo cierra y vuelve al chat del contacto. Dentro del filtro todo funciona como hoy: seleccionar quita la marca, `open` marca visto, y se puede responder.
- **Foto de perfil**:
  - se pide a WhatsApp al abrir el chat, en tamaño reducido (preview), como la app;
  - se guarda en `<profile>/avatars/`, y se vuelve a consultar como máximo una vez cada 24 h por chat, enviando el id de la foto guardada para no descargarla si no cambió;
  - si el contacto oculta su foto o no tiene, se recuerda y no se pregunta de nuevo hasta las 24 h.
- `chat_banner_enabled=1` en `ui.conf` para poder desactivarlo.
- Status no tiene banner: su fila de lista y el filtro ya lo identifican.

## Capabilities

### New Capabilities
- `chat-banner`: cabecera del chat con foto, nombre, número o participantes, presencia y estados del contacto, y acceso a sus estados con Alt-u.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/common/src/protocol.h`: `GetAvatarRequest` (preview o completa) y `NewAvatarNotify`.
- `lib/wmchat/go/gowm.go`, `cgowm.go` y `lib/wmchat/src/wmchat.cpp`: `WmGetAvatar` con `GetProfilePictureInfo` (preview y `ExistingID`), descarga a `avatars/` y aviso a la UI.
- `src/uimodel.{h,cpp}`: pedir la foto al abrir un chat, presencia por tipo, conteo de estados por contacto, Alt-u en chats de contacto y regreso, Ctrl-v sin selección para abrir la foto.
- `src/uihistoryview.cpp`: dibujar el banner y la foto en las primeras filas del historial.
- `src/uiconfig.cpp`, `src/uicolorconfig.cpp`, `src/uihelpdialog.cpp` y `README.md`: `chat_banner_enabled`, `history_banner_color` y colores de presencia, Ctrl-v sin selección, Alt-u en chats.
