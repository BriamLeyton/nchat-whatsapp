## 1. Recuperación de estado de WhatsApp

- [x] 1.1 Agregar `ResyncAppState`, `RequestAppStateRecovery` y `SendAppStateWithResync` en `gowm.go`; usarlos en `WmPinChat` y `WmArchiveChat`; manejar `events.AppStateSyncError` y `AppStateSyncComplete{Recovery}`. Verificar que compila y que `gofmt -l` está limpio.
- [x] 1.2 En vivo: el log muestra `recovered from phone`; los pins del teléfono aparecen en nchat; `Alt-p` fija en nchat y en el teléfono.

## 2. Configuración

- [x] 2.1 Agregar las claves de `ui.conf` (`list_pinned_header`, `list_others_header`, `list_archived_header`, `list_others_collapsed`, `list_archived_collapsed`), de `key.conf` (`toggle_others=\33\157`, `toggle_archived=\33\150`) y de `color.conf` (`list_color_header_fg/_bg`). Verificar con `diff` sobre copias que solo se agregan claves.

## 3. Modelo

- [x] 3.1 Incluir los archivados en `m_ChatVec` (`NewChatsNotify`, `UpdateArchivedNotify`, selección de contacto) y dejar de descartar sus mensajes en `NewMessagesNotify`, sin notificarlos. Mantener el chat actual al archivar o desarchivar.
- [x] 3.2 Ordenar por sección en `SortChats` (fijados, otros, archivados por tiempo).
- [x] 3.3 Agregar `GetChatSection`, `IsChatHiddenLocked`, el estado colapsado persistido, `OnKeyToggleOthers` y `OnKeyToggleArchived`, la navegación que salta ocultos y la expansión automática en no leído, ir a chat y contacto.

## 4. Vista

- [x] 4.1 En `UiListView::Draw`: filas virtuales con títulos, `(N)` y `*` en secciones colapsadas, desplazamiento basado en la fila seleccionada, sin títulos cuando solo hay `Otros`.
- [x] 4.2 Documentar `Alt-o`, `Alt-h` y las claves nuevas en `README.md`.

## 5. Verificación

- [x] 5.1 En el sandbox offline, con fijados y archivados en una copia de la DB:
  - [x] el orden y los títulos son correctos;
  - [x] Tab salta los títulos y los chats ocultos;
  - [x] `Alt-o` y `Alt-h` colapsan y expanden, y el título muestra `(N)` y `*`;
  - [x] `Ctrl-f` hacia un chat oculto expande su sección;
  - [x] el estado colapsado sobrevive al reinicio;
  - [x] sin fijados ni archivados no hay títulos.
- [ ] 5.2 En vivo en Ghostty:
  - [ ] `Alt-a` mueve el chat entre secciones;
  - [ ] un mensaje a un chat archivado aparece con `*` y sin notificación;
  - [ ] archivar o desarchivar desde el teléfono se refleja en vivo.
