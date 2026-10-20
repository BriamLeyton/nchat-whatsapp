## 1. Modelo

- [x] 1.1 Agregar `m_HistoryViewOffset` con `GetHistoryViewOffsetLocked`/`SetHistoryViewOffsetLocked`. Inicializarlo con `messageOffset` al activar la selección. En `OnKeyPrevPage`, con la selección activa, partir de `viewOffset`.

## 2. Vista

- [x] 2.1 En `UiHistoryView::Draw`: anclar en `viewOffset` con la selección activa (o en `messageOffset` sin ella), resaltar por índice, registrar si el seleccionado se dibujó completo, ajustar `view` (`sel < view` → `view = sel`; no visible → `view+1` y redibujar) y guardar la vista. Colocar imágenes solo desde el dibujo final.

## 3. Verificación

- [x] 3.1 Sandbox offline:
  - [x] `↑` desde la entrada no mueve el historial;
  - [x] `↑` repetido sube la selección sin mover nada hasta el borde superior;
  - [x] pasado el borde, desplaza un mensaje;
  - [x] `↓` vuelve hasta el borde inferior y desplaza;
  - [x] `↓` desde el último sale y muestra el final;
  - [x] `PgUp`/`PgDn` igual que antes;
  - [x] mensaje más alto que la vista recortado con `[...]`.
- [x] 3.2 En vivo en Ghostty: selección con imágenes inline, saltar al citado, reaccionar y editar sobre el mensaje seleccionado.
