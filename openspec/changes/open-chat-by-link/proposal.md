## Why

Las empresas y las páginas web comparten links como `https://api.whatsapp.com/send/?phone=15551234567&text=Hola...` o `https://wa.me/15551234567`. En la app, abrir el link lleva directo al chat con el texto ya escrito.

En nchat, el diálogo de contactos (Alt-n) solo lista contactos sincronizados. Para escribirle a un número que no está guardado, hay que guardarlo antes en el teléfono, y luego copiar el texto a mano.

## What Changes

- **El filtro del diálogo de contactos (Alt-n) acepta un número o un link de WhatsApp**:
  - número internacional con código de país: `+1 555 123 4567`, `15551234567`, `(+1) 555-123-4567`;
  - `https://wa.me/<número>[?text=...]`;
  - `https://api.whatsapp.com/send/?phone=<número>&text=...` (también `/send?`, sin `https://`, y `whatsapp://send?phone=...`).
- **Fila nueva al inicio de la lista**: `Open chat with +15551234567`, seguida de los contactos que coincidan como hoy. Enter sobre ella abre el chat.
  - Si ya hay un chat con ese número, se abre (y se desarchiva, como al elegir un contacto). No se consulta a WhatsApp.
  - Si no hay chat, nchat pregunta una vez a WhatsApp si el número está registrado, como hace la app al abrir el link. Si lo está, se crea el chat y se abre. Si no, aparece "+15551234567 is not on WhatsApp.".
  - Sin conexión aparece "Not connected, please try again later." y no se crea nada.
- **El texto del link queda escrito, sin enviar**: el `text=` se decodifica (`+` es espacio, `%C3%A1` es `á`) y se pone en la caja de mensaje del chat. El usuario lo revisa, lo edita y lo envía con Enter.
  - Si el chat ya tenía un borrador, no se reemplaza, y aparece "Draft kept, link text not inserted.".
- Un número sin código de país no se puede adivinar: se toma tal cual (por ejemplo, `5551234567` sería `+32...`). La fila muestra el número con `+` para que el usuario lo vea antes de Enter.
- Fuera de alcance: `nchat --open <link>` y registrar nchat como manejador de links `wa.me` en el sistema.

## Capabilities

### New Capabilities
- `open-chat-by-link`: abrir o crear un chat escribiendo o pegando un número o un link de WhatsApp en el diálogo de contactos, con el texto del link listo para enviar.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uicontactlistdialog.{h,cpp}`: detectar número o link en el filtro y agregar la fila `Open chat with +N`.
- `src/uimodel.{h,cpp}`: abrir el chat existente o pedir su creación por número, texto pendiente para la caja de mensaje, avisos.
- `lib/common/src/protocol.h`: `CreateChatRequest` con `phone`; `CreateChatNotify` con el número en caso de error.
- `lib/wmchat/src/wmchat.cpp`, `lib/wmchat/go/cgowm.go`, `lib/wmchat/go/gowm.go`: `WmCreateChat` con `IsOnWhatsApp`, en una goroutine.
- `src/uihelpdialog.cpp`, `README.md`: documentar números y links en Alt-n.
