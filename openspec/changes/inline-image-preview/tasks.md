## 1. Dependencias y build

- [x] 1.1 Vendorizar `stb_image.h` y `stb_image_resize2.h` (versiones fijas, con licencia) en `ext/stb/`. Verificar que existan y que el header de licencia esté intacto.
- [x] 1.2 En `CMakeLists.txt`: agregar `ext/stb` a includes, `find_package`/`pkg_check_modules` para `libwebp` y linkear a `nchat`. Verificar que `cmake .. -DHAS_TELEGRAM=OFF && make -j$(nproc)` compila sin warnings nuevos.

## 2. Módulo UiImage

- [x] 2.1 Crear `src/uiimage.h/.cpp` con la API del design (`Init`, `IsEnabled`, `IsImageFile`, `Prepare`, `ClearPlacements`, `PlaceAll`, `Cleanup`) y agregarlo a `CMakeLists.txt`. Verificar que compila.
- [x] 2.2 Implementar la detección (consulta `a=q` + DA1, timeout ~200 ms, desactivar si hay `TMUX`) y el tamaño de celda (`TIOCGWINSZ`, `ESC[16t`, fallback 8×16), con log del resultado. Verificar en el log: `supported=1` en Ghostty y `supported=0` dentro de tmux.
- [x] 2.3 Implementar la decodificación: JPEG/PNG/GIF con stb y WebP con libwebp, a RGBA. Escalar al tamaño de celdas calculado sin ampliar. Verificar con un programa de prueba en `build/` sobre un `.jpg` y un `.webp` de `~/.config/nchat/profiles/*/tmp`: dimensiones esperadas y sin errores.
- [ ] 2.4 Implementar la transmisión (`a=t,f=32,t=d,q=2`, base64 en chunks de 4096, `m=1/0`), la caché por `(path,maxCols,maxRows)` con LRU de 64 (`a=d,d=I` al expulsar) y la caché de fallos. Verificar con log que la segunda llamada a `Prepare` del mismo archivo no retransmite.
- [x] 2.5 Implementar `ClearPlacements` (`a=d,d=a,q=2`), `PlaceAll` (cursor + `a=p,c,r,C=1,q=2`, recorte `y/h`, envuelto en `ESC7/ESC8`) y `Cleanup` (`a=d,d=A`). Usar `fflush(stdout)` + `write(STDOUT_FILENO)`. Verificar que la imagen aparece en Ghostty y el cursor de entrada queda en su lugar.

## 3. Integración en la UI

- [x] 3.1 Agregar `inline_images_enabled=1` e `inline_image_max_height=10` a los defaults de `src/uiconfig.cpp`. Verificar que tras arrancar y salir `~/.config/nchat/ui.conf` contiene ambas claves y el resto queda igual (comparar con `diff` contra el respaldo).
- [x] 3.2 Llamar `UiImage::Init()` antes de `initscr()` y `UiImage::Cleanup()` antes de `endwin()` en `src/ui.cpp`. Verificar que al salir con `Ctrl-q` no quedan imágenes en pantalla.
- [x] 3.3 En `UiHistoryView::Draw`: `ClearPlacements()` al inicio; para cada adjunto de imagen descargado llamar `Prepare` con el ancho del panel e insertar `rows` líneas reservadas después de la línea `📎`; registrar fila visible y filas recortadas; tras `wrefresh` llamar `PlaceAll`. Verificar en un chat con fotos que la miniatura queda entre la línea `📎` y el caption.
- [ ] 3.4 Manejar el truncado del primer mensaje (`maxMessageLines`): si la imagen no cabe se omite y queda la línea `📎`. Verificar seleccionando un mensaje con foto en una terminal de poca altura.
- [ ] 3.5 Ampliar la condición de prefetch para auto-descargar adjuntos de imagen cuando `UiImage::IsEnabled()`. Verificar con `attachment_prefetch=1` que las fotos visibles no seleccionadas se descargan y un PDF no.
- [x] 3.6 Llamar `ClearPlacements()` en el constructor de `UiDialog` y en `TerminalControlPause()`. Confirmar que al cerrar un diálogo el historial queda dirty y se redibuja. Verificar abriendo y cerrando `Ctrl-s` (emoji) y el editor externo: sin miniaturas encima del diálogo y redibujadas al cerrar.

- [x] 3.7 Retransmitir las imágenes después de un clear de pantalla (`UiScreen`, `ReinitView`) y de reentrar a la pantalla alternativa (`TerminalControlResume`, protocol UI control), usando el RGBA en memoria. Verificar en Ghostty: tras redimensionar solo el alto, cerrar un diálogo con Esc o suspender/reanudar, las imágenes siguen visibles sin reiniciar nchat.

## 4. Verificación integral

- [x] 4.1 Probar en Ghostty con la sesión real: scroll con PageUp/PageDn, cambio de chat con Tab, redimensionar la ventana, sticker WebP con transparencia y foto panorámica en panel angosto. Sin restos gráficos ni teclas basura en el input.
- [ ] 4.2 Probar el fallback: con `inline_images_enabled=0`, dentro de tmux y con un `.jpg` corrupto, se ve solo la línea `📎`, sin filas vacías, y el error aparece en `log.txt`.
- [x] 4.3 Verificar que `nchat-status` (`~/.local/bin/nchat-status`) sigue funcionando y que no hubo cambios de esquema en `db.sqlite`.
