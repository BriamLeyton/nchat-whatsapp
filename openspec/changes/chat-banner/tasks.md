## 1. Foto de perfil

- [x] 1.1 `GetAvatarRequest` y `NewAvatarNotify` en `protocol.h`; manejo en `wmchat.cpp` y `CWmGetAvatar` en `cgowm.go`.
- [x] 1.2 `WmGetAvatar`: `GetProfilePictureInfo` (preview, `ExistingID`), descarga atómica, `.id` con `none` para oculta o inexistente, aviso a la UI.
- [x] 1.3 `uimodel`: pedir la foto al abrir un chat (24 h por mtime de `.id`, una vez por sesión) y redibujar con `NewAvatarNotify`.
- [x] 1.4 Foto completa: `full` en la petición, `<chatId>.full.jpg` (borrada si cambia la preview), Ctrl-v sin selección la abre o la pide y la abre al llegar; "No profile photo." si no hay.

## 2. Banner

- [x] 2.1 `UiHistoryView`: reservar 3 filas con `history_banner_color`, dibujar texto de contacto o grupo, y ubicar la foto con `imagePlacements` (clave de caché con mtime).
- [x] 2.2 Conteo de estados por contacto (24 h, `{total, nuevos}`) al abrir el chat, al llegar un estado y al volver del filtro; línea de estados en el banner.
- [x] 2.3 `GetChatPresence` (tipo y texto), usada por `GetChatStatus`; presencia coloreada y ` · muted` en el banner.
- [x] 2.4 `chat_banner_enabled=1` en `uiconfig.cpp`, `history_banner_color`/`history_banner_attr` y colores de presencia en `uicolorconfig.cpp`, y README.

## 3. Alt-u desde el chat

- [x] 3.1 `OnKeyStatusFilter` en chat de contacto: guardar regreso, ir a Status y filtrar; Alt-u en el filtro vuelve; cambiar de chat olvida el regreso; aviso si no hay estados.
- [x] 3.2 Ayuda (`uihelpdialog.cpp`) y README: Alt-u en chats de contacto y Ctrl-v sin selección.

## 4. Verificación

- [x] 4.1 Sandbox sin red: banner de contacto y de grupo con texto, conteo de estados con valores conocidos, Alt-u ida y vuelta, aviso sin estados; sin red no se pide la foto; Ctrl-v sin foto muestra "No profile photo.". Colores de presencia y fotos pasan a 4.2, porque requieren red.
- [x] 4.2 En vivo en Ghostty: colores de presencia (typing, online, seen); fotos de contacto y grupo; Ctrl-v abre la foto completa; contacto con foto oculta; reabrir el chat no vuelve a consultar (log); estados nuevos y revisados en el banner.
