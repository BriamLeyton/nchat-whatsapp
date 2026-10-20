## 1. Detección

- [x] 1.1 `ParseWhatsAppLink`: links `wa.me`, `api.whatsapp.com/send`, `whatsapp://send` y número de 8 a 15 dígitos; decodificar `text` (`+`, `%XX`).
- [x] 1.2 `UiContactListDialog`: fila `Open chat with +<número>` al inicio cuando el filtro es número o link; `UiContactListItem` con `phone` y `text`.

## 2. Abrir o crear

- [x] 2.1 `protocol.h`: `CreateChatRequest.phone`; `CreateChatNotify.phone` y `result`.
- [x] 2.2 `wmchat.cpp`, `cgowm.go`, `gowm.go`: `WmCreateChat` con `IsOnWhatsApp` en goroutine (timeout 10 s) y `WmCreateChatNotify`.
- [x] 2.3 `uimodel`: `OpenChatByPhone` (chat existente sin red, sin conexión aviso, si no `CreateChatRequest`); manejo de `CreateChatNotify` con chat ya existente y avisos de error.
- [x] 2.4 Texto pendiente: aplicarlo a la caja si está vacía, o aviso "Draft kept, link text not inserted.".

- [x] 2.5 `WmCreateChat`: crear el chat con el número (`PhoneNumber`), no con el LID que devuelve `IsOnWhatsApp`. Compila; verificar en vivo con 4.3.

## 3. Documentación

- [x] 3.1 Ayuda (`uihelpdialog.cpp`) y README: números y links en Alt-n.

## 4. Verificación

- [x] 4.1 Sandbox sin red: parseo de los formatos de link y número; fila en el diálogo; chat existente se abre con el texto; borrador existente se mantiene; número nuevo sin conexión muestra el aviso.
- [x] 4.2 En vivo en Ghostty: link de ACME crea el chat con el texto listo; número no registrado muestra el aviso.
- [x] 4.3 En vivo: chat nuevo por número y primer mensaje; el log muestra `chat <número>@s.whatsapp.net`, el mensaje se ve en el chat abierto y no aparece un segundo chat.
