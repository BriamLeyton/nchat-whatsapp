## 1. Implementación

- [x] 1.1 Revisados por perfil: lectura perezosa de `<profile>/status_seen`, línea base al primer arranque, poda a 30 días, escritura atómica.
- [x] 1.2 `IsStatusNew` y `MarkStatusSeen` (sin tocar `isRead` ni enviar nada), con sus variantes `Locked` en `UiModel`.
- [x] 1.3 `UpdateChatInfoIsUnread`: para Status, no leído si hay algún estado nuevo.
- [x] 1.4 `UiHistoryView`: `status_new_indicator` antes del nombre en estados nuevos; `MarkStatusSeenLocked` para el estado seleccionado con `p_Final`.
- [x] 1.5 `UiListView`: la fila Status usa `status_new_indicator` en lugar de `unread_indicator`.
- [x] 1.6 `status_new_indicator=●` en `uiconfig.cpp` y README, con el significado de la marca en los estados y en la fila Status.

## 2. Verificación

- [x] 2.1 Sandbox sin red:
  - sin `status_seen`, ningún estado sale marcado;
  - con la línea base movida atrás, los estados posteriores salen con `●` y la fila Status también;
  - recorrer con el cursor quita `●` y, al final, el de la fila Status, sin enviar visto (log);
  - reiniciar conserva los revisados.
- [ ] 2.2 En vivo en Ghostty: un estado nuevo sale con `●` en el estado y en la fila Status, y recorrerlo los quita.
