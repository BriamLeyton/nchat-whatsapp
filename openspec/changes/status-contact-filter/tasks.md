## 1. Modelo

- [x] 1.1 Agregar `m_StatusFilterSenderId`, `m_StatusFilterVec`, `GetShownMessageVec` y la reconstrucción del filtro. Migrar a la lista mostrada los lectores de `m_MessageVec` que usan `m_MessageOffset`, y `GetMessageVecLocked`.
- [x] 1.2 Reconstruir el filtro tras cambios en los mensajes del chat de estados, y quitarlo en `OnCurrentChatChanged`.
- [x] 1.3 `RequestMessages` con filtro: pedir con `FetchSenderMessagesSync` bloques de 50 del contacto hasta llenar el historial filtrado o agotar la caché. Al quitar el filtro, liberar los estados cargados solo para el filtro.

## 2. Comando y diálogo

- [x] 2.1 `MessageCache::GetSenderLastTimesSync` (consulta agrupada por `senderId`) y `MessageCache::FetchSenderMessagesSync`.
- [x] 2.2 `UiStatusContactDialog` (nuevo) basado en `UiListDialog`, con búsqueda.
- [x] 2.3 Tecla `status_filter=\33\165` y `UiModel::OnKeyStatusFilter`: quitar filtro si está activo; si no, filtrar por el autor seleccionado, o abrir el diálogo sin selección. No hacer nada fuera de estados.

## 3. Vista y documentación

- [x] 3.1 `UiListView`: ` · <nombre>` en la fila de estados con filtro.
- [x] 3.2 Modal de ayuda y README.

## 4. Verificación

- [x] 4.1 Sandbox offline:
  - [x] `Alt-u` sobre un estado seleccionado filtra y mantiene la selección;
  - [x] `Alt-u` sin selección abre el diálogo ordenado por fecha, con búsqueda, y `Esc` no filtra;
  - [x] elegir un contacto filtra y carga sus estados viejos;
  - [x] `↑`/`↓`, `PgUp`/`PgDn` y abrir adjunto sobre la lista filtrada;
  - [x] `Alt-u` de nuevo y cambiar de chat quitan el filtro;
  - [x] RAM: filtrar un contacto con estados viejos carga solo los suyos, y quitar el filtro la devuelve;
  - [x] la fila muestra `◉ Status · <nombre>`;
  - [x] `Alt-u` en otro chat no hace nada.
- [ ] 4.2 En vivo en Ghostty: un estado nuevo de otro contacto no aparece con el filtro activo, y uno del contacto filtrado sí.
