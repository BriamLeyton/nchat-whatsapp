## Context

El historial (`src/uihistoryview.cpp`) se dibuja de abajo hacia arriba en una ventana ncurses (`m_PaddedWin`). Primero arma un vector de líneas `wlines` por mensaje (adjunto, cita, texto, reacciones) y termina con `wrefresh`. ncurses no sabe nada de imágenes. Cualquier gráfico tiene que escribirse directo a la terminal después del `wrefresh`, en celdas que ncurses deje en blanco.

El código ya escribe secuencias crudas fuera de ncurses: `printf("\033[?1004h")` + `fflush` en `UiModel::Impl::TerminalControlResume`. Eso sirve de precedente.

Datos disponibles por mensaje: `FileInfo { fileStatus, fileId, filePath, fileType }`. En WhatsApp, `filePath` ya trae la extensión correcta (`.jpg`, `.webp`, …) antes de la descarga (`gowm.go`, `HandleImageMessage`/`HandleStickerMessage`). `fileType` no se llena en mensajes recibidos.

Los diálogos (`UiListDialog::Run`, etc.) se dibujan encima del historial con ventanas ncurses propias y, al cerrarse, llaman `ReinitView()` + `Draw()`.

Librerías del sistema presentes: `libwebp`, `libjpeg-turbo`, `libpng`, `libmagic`.

## Goals / Non-Goals

**Goals:**
- Dibujar miniaturas con el protocolo gráfico de kitty sin cambiar el modelo de dibujo ncurses existente.
- Aislar todo lo gráfico en un módulo nuevo para que `uihistoryview.cpp` solo reserve filas y registre posiciones.
- Que cada redibujado cueste poco: transmitir los píxeles una vez y luego solo reposicionar.

**Non-Goals:**
- Placeholders Unicode de kitty (U+10EEEE). Requieren diacríticos combinados y colores de 24 bits codificados en celdas, algo que ncurses no maneja de forma confiable.
- Decodificación asíncrona en un hilo aparte. Queda como mejora si se nota lag.
- Soporte para Sixel o iTerm2, y passthrough de tmux.

## Decisions

### 1. Colocación directa con IDs (`a=T`/`a=p`) en vez de placeholders Unicode
Cada imagen se transmite una vez con un ID numérico (`a=t,i=<id>,f=32,s=<w>,v=<h>`). Cada `Draw()` hace dos cosas:
1. Borra todas las colocaciones visibles sin liberar los datos (`a=d,d=a`).
2. Por cada miniatura registrada mueve el cursor (`ESC[<row>;<col>H`) y coloca la imagen (`a=p,i=<id>,c=<cols>,r=<rows>,C=1,q=2`).

`C=1` evita que el cursor se mueva. `q=2` suprime las respuestas de la terminal, que si no llegarían como input basura a `wgetch`. La secuencia va envuelta en `ESC 7` / `ESC 8` (guardar/restaurar cursor) para no desincronizar a ncurses.

*Alternativa descartada:* placeholders Unicode, ver Non-Goals. Su ventaja (scroll y recorte gratis) no compensa la fragilidad con ncurses.

### 2. Decodificar en el cliente y enviar RGBA (`f=32`)
El protocolo kitty solo acepta PNG (`f=100`) o píxeles crudos. Las fotos de WhatsApp son JPEG y los stickers WebP. Se decodifica localmente:
- JPEG, PNG y GIF (primer frame) con `stb_image.h`.
- WebP con `libwebp` (`WebPDecodeRGBA`). Los stickers WebP animados (~20% de los stickers en la cache real) usan el primer frame con `libwebpdemux` (`WebPAnimDecoder`), igual que con GIF.

Luego se escala con `stb_image_resize2.h` al tamaño exacto en píxeles de la miniatura y se envía en base64 en chunks de 4096 bytes (`m=1`/`m=0`), con transmisión directa (`t=d`).

*Alternativas descartadas:* `t=f` (la terminal lee el archivo) no sirve para JPEG ni WebP y falla si la terminal corre en otro host. Usar `libjpeg`/`libpng` directo implica más código. ImageMagick es una dependencia pesada.

### 3. Tamaño en celdas a partir del tamaño en píxeles de la celda
El tamaño de celda se obtiene con `ioctl(TIOCGWINSZ)` (`ws_xpixel/ws_col`, `ws_ypixel/ws_row`). Si viene en 0, se consulta `ESC[16t`. Si tampoco responde, se asume 8×16.

Con eso: `rows = min(max_height, ceil(h_px / cell_h))`, `cols = ceil(w_px * scale / cell_w)`, limitado al ancho del panel. Los píxeles se escalan a `cols*cell_w × rows*cell_h` respetando la proporción, nunca por encima de su tamaño natural.

### 4. Detección de soporte al inicio, antes de `initscr()`
En `src/ui.cpp`, antes de `initscr()`, la terminal se pone en modo raw y se envía una consulta gráfica (`ESC_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA ESC\`) seguida de DA1 (`ESC[c`). Se lee la respuesta con un timeout de ~200 ms:
- Si llega `ESC_Gi=31;OK` antes que la respuesta a DA1, hay soporte.
- Si llega solo DA1, o se cumple el timeout, no hay soporte y se usa el fallback.

Si la variable `TMUX` está definida, no se habilita. Esto es robusto frente a `TERM`, que en Ghostty a veces es `xterm-256color`.

*Alternativa descartada:* chequear `TERM_PROGRAM=ghostty`/`TERM=xterm-kitty`. Falla con multiplexores y con `ssh`.

### 5. Registro de miniaturas en `UiHistoryView::Draw`
Al armar `wlines` de un mensaje con imagen renderizable, se insertan `rows` líneas vacías marcadas, justo después de la línea `📎`. Al recorrer las líneas de abajo hacia arriba, se anota la fila de pantalla de la primera línea visible de la imagen y cuántas filas quedaron recortadas arriba.

Después de `wrefresh(m_PaddedWin)` se llama `UiImage::PlaceAll(placements, origin)`. Para el recorte superior se usan los parámetros de recorte en píxeles del protocolo (`y=<skip_rows*cell_h>`, `h=<visible_rows*cell_h>`) y `r=<visible_rows>`.

El truncado del primer mensaje (`maxMessageLines`) cuenta las filas de la imagen como líneas normales. Si la imagen no cabe, se omite y queda la línea `📎`.

### 6. Módulo `src/uiimage.{h,cpp}` (clase estática `UiImage`)
API:
- `Init()`: detección, se llama desde `ui.cpp`.
- `IsEnabled()`
- `IsImageFile(path)`: por extensión, `.jpg/.jpeg/.png/.webp/.gif`.
- `Prepare(path, maxCols, maxRows)`: devuelve `{id, cols, rows}` o un fallo. Decodifica, escala y transmite solo la primera vez y guarda el resultado en caché por `(path, maxCols, maxRows)`. Los fallos también se cachean para no reintentar en cada frame.
- `ClearPlacements()`, `PlaceAll(...)` y `Cleanup()`: `a=d,d=A` al salir, que libera los datos.

La escritura se hace con `write(STDOUT_FILENO)` después de `fflush(stdout)`, para no mezclarse con el buffer de ncurses.

### 7. Ocultar alrededor de diálogos y pausas de terminal
Se llama `UiImage::ClearPlacements()`:
- en el constructor de `UiDialog`;
- en `UiModel::Impl::TerminalControlPause()`, para el editor externo y `Ctrl-z`;
- al inicio de `UiHistoryView::Draw` (siempre, aunque no haya imágenes).

Al cerrarse un diálogo, el `ReinitView()` + `Draw()` existente vuelve a colocar las miniaturas. Hay que verificar que `m_Dirty` del historial quede en `true` en ese flujo.

### 8. Auto-descarga de imágenes
En el bloque `// File attachment` de `UiHistoryView::Draw` se amplía la condición de prefetch con `|| (UiImage::IsEnabled() && UiImage::IsImageFile(fileInfo.filePath))`. Se reutiliza `DownloadAttachmentLocked(..., DownloadFileActionNone)`. El redibujado al terminar la descarga ya existe.

### 9. Configuración
Se agregan a los defaults de `UiConfig` (`src/uiconfig.cpp`): `inline_images_enabled=1` e `inline_image_max_height=10`. El mecanismo existente de `UiConfig` ya escribe las claves faltantes al guardar.

## Risks / Trade-offs

- [Decodificar fotos de 12 MP en el hilo de UI bloquea ~50–150 ms la primera vez] → Mitigación: caché por sesión; se decodifica una vez por archivo. Si molesta, se pasa a un hilo worker con un placeholder `⇄` (follow-up).
- [Respuestas de la terminal filtradas a `wgetch` como teclas basura] → Mitigación: `q=2` en todas las secuencias de ejecución. La consulta de detección se hace antes de `initscr()` y se drena el input.
- [Desincronización del cursor de ncurses] → Mitigación: `ESC 7`/`ESC 8` alrededor de cada lote y `C=1` en las colocaciones.
- [Memoria en la terminal por muchas imágenes transmitidas] → Mitigación: límite LRU, por ejemplo 64 imágenes. Al expulsar una se envía `a=d,d=I,i=<id>`.
- [Restos gráficos si nchat crashea] → Riesgo aceptado: la terminal los limpia con `clear`/reset. `Cleanup()` se llama también desde los handlers de señal de salida existentes.
- [Ghostty con `TERM=xterm-256color` y ancho de emojis] → No afecta: las celdas de la imagen son espacios.
- [Bajo `herdr` o tmux no hay soporte gráfico] → Se usa el fallback automático; no es una regresión.

- [Ghostty libera los datos de las imágenes con `ESC[2J` (clear screen) y al reentrar a la pantalla alternativa; esto se verificó empíricamente con consultas `a=p`, que devuelven `ENOENT`. ncurses emite `ESC[H ESC[2J` en cada `ReinitView()` (`UiScreen` hace `wclear(stdscr)`), y `TerminalControlResume` reentra a la pantalla alternativa. Si la caché sigue usando IDs muertos, las imágenes desaparecen y queda el espacio reservado] → Mitigación: `UiImage::Invalidate()` después de cada clear o reentrada marca las imágenes como no transmitidas. La caché guarda el RGBA escalado (máx. 64 imágenes, ~20–40 MB) y las retransmite al colocarlas, sin volver a decodificar.

## Migration Plan

- Sin migración de datos. Se compila y el symlink `~/.local/bin/nchat` toma el binario nuevo.
- Para desactivar en caliente: `inline_images_enabled=0` en `~/.config/nchat/ui.conf`.
- Rollback: volver al commit anterior y recompilar, o restaurar `~/.local/bin/nchat.5.19.18.bak`.
