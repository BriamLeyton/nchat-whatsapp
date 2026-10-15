## Context

- `m_MessageOffset[profile][chat]` es el índice (0 = más reciente) del mensaje seleccionado. Cuando no hay selección, es el mensaje anclado abajo tras `PgUp`.
- `UiHistoryView::Draw` dibuja de abajo hacia arriba empezando en `m_MessageOffset`, y resalta solo el primer mensaje dibujado (`firstMessage`). Por eso la selección siempre queda abajo.
- `m_MessageOffset` lo usan unas 10 acciones sobre el mensaje seleccionado (editar, borrar, reaccionar, abrir, fijar, …), además de saltos (citado, buscar, fijado) y `PgUp`/`PgDn`, que usan además `m_MessageOffsetStack`.
- La altura de cada mensaje depende del ancho, de las reacciones, de las imágenes inline y del recorte con `[...]`. Solo la vista la conoce.

## Goals / Non-Goals

**Goals:**
- Que la selección se mueva con la vista fija.
- No cambiar la semántica de `m_MessageOffset`, que siempre es el seleccionado.

**Non-Goals:**
- Desplazamiento por línea.
- Selección con mouse.
- Cambiar `PgUp`/`PgDn`.

## Decisions

### 1. Desplazamiento de la vista separado
- **Estado nuevo:** `m_HistoryViewOffset[profile][chat]`, el índice del mensaje anclado abajo de la vista.
- **Getter y setter `...Locked`:** la vista puede ajustarlo durante `Draw`.
- **Sin selección activa:** la vista usa `m_MessageOffset` como ancla, igual que hoy, así que `PgUp`/`PgDn` no cambian.
- **Al activar la selección** (`↑` desde la entrada): `viewOffset = messageOffset`. El seleccionado inicial es el de abajo, y lo que se ve no cambia.

*Alternativa descartada:* que `m_MessageOffset` sea el ancla de la vista y agregar un índice de selección aparte. Obligaría a tocar todas las acciones sobre el mensaje seleccionado.

### 2. Ajuste de la vista en `Draw`
Con la selección activa, `Draw` hace:
1. Si `sel < view`, pone `view = sel`, porque el seleccionado quedó debajo de la vista.
2. Dibuja desde `view` y registra si el seleccionado se dibujó completo (su encabezado está en `y >= 0`) o quedó fuera o cortado arriba.
3. Si no se vio completo y `view < sel`, suma 1 a `view` y redibuja. Repite hasta que se vea, o hasta `view == sel`, donde aplica el recorte con `[...]` de hoy.
4. Guarda `view` en el modelo.

La primera condición cubre el desplazamiento hacia abajo y la tercera hacia arriba. Así la vista se mueve lo mínimo. Cada iteración solo dibuja en memoria (ncurses no refresca hasta `wrefresh`), y en la práctica son 1 o 2 iteraciones.

El recorte con `[...]` hoy se aplica al primer mensaje dibujado. Se mantiene igual, porque solo afecta al mensaje de abajo, que es el seleccionado únicamente cuando `view == sel`.

Las imágenes inline solo se colocan desde el dibujo final.

### 3. Resaltado
`isSelectedMessage` pasa de `firstMessage && selectActive` a `index == messageOffset && selectActive`.

### 4. Salida y saltos
- **Al desactivar la selección:** `viewOffset` se ignora mientras no hay selección y se reinicia en la próxima activación, así que no hace falta limpiarlo.
- **Los saltos** (citado, buscar, fijado) ya solo cambian `m_MessageOffset`. La regla del punto 2 los muestra con desplazamiento mínimo.
- **`PgUp` con selección activa:** antes de calcular, pone `messageOffset = viewOffset`. Así pagina desde lo que se ve y no desde el seleccionado.

## Risks / Trade-offs

- **Mensajes nuevos con la selección activa:** los índices se cuentan desde el más reciente, así que la vista y la selección bajan juntas un mensaje. Hoy ya pasa lo mismo con la selección, y la vista se mantiene coherente.
- **Bucle de redibujo:** acotado a `sel - view` iteraciones. En el peor caso, con un salto lejano, son decenas de dibujos en memoria, aceptable.
