## Why

Los adjuntos y links se ven pobres en todos los chats (personas, grupos y estados):
- **Videos:**
  - se ven como una línea `📎 XXXX.f4v` sin vista previa, duración ni tamaño;
  - la extensión `.f4v` sale de la librería MIME del sistema, que la pone antes de `.mp4`.
- **Notas de voz y audios:** no muestran su duración.
- **Imágenes sin descargar** (descarga en curso, fallida, o estado vencido): solo muestran `📎`, aunque el mensaje trae una miniatura que no requiere red.
- **Links:** se ven como URL suelta, aunque el mensaje trae título, descripción y miniatura.
- **Estados de solo texto:** pierden su color de fondo.

Todos esos datos vienen dentro del mensaje recibido. Mostrarlos no genera ninguna petición a WhatsApp.

En la caché del usuario hay 1568 estados: 939 imágenes, 546 videos y 83 de texto o links.

## What Changes

- **Metadatos sin red:** al recibir un mensaje de cualquier chat, el protocolo guarda junto al adjunto:
  - la miniatura incrustada (`<msgId>.thumb.jpg`);
  - un archivo `<msgId>.meta` con duración y tamaño (video y audio), si es nota de voz, título y descripción del link, y colores de fondo y texto (estados).
- **Videos:**
  - se guardan como `.mp4`;
  - la línea del adjunto muestra `▶ 1:15 · 4.2 MB` cuando hay metadatos;
  - inline se ve un fotograma del video descargado, generado localmente con un comando configurable (por defecto `ffmpegthumbnailer`). Si no hay fotograma, se usa la miniatura incrustada.
- **Audio:** una nota de voz muestra `🎤 0:42`, y otro audio muestra `▶ 3:10 · 4.5 MB`.
- **Miniatura de respaldo:** una imagen o video sin descargar muestra inline la miniatura incrustada, de baja resolución, en vez de solo `📎`. Esto no dispara descargas, y respeta los estados vencidos.
- **Links:** debajo del texto se ven el título y la descripción, y la miniatura inline en tamaño reducido.
- **Estados de solo texto:** se dibujan con su color de fondo y de texto.
- Los mensajes recibidos antes de este cambio no tienen metadatos y se ven como hoy, salvo el fotograma de video, que solo necesita el archivo descargado.

Fuera de alcance:
- precarga y limpieza de disco de estados (change `status-prefetch-cleanup`);
- reproducir video o audio dentro de la terminal (abrir sigue usando el visor externo, por ejemplo mpv);
- obtener vista previa de links consultando la web.

## Capabilities

### New Capabilities
- `media-preview`: vista previa de videos, audios, miniaturas, links y estados de texto en todos los chats, sin tráfico de red.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`:
  - `ExtensionByType` prefiere `.mp4`;
  - los manejadores de imagen, video, audio y texto extendido escriben la miniatura y los metadatos.
- `src/uihistoryview.cpp`: línea de video y audio, imagen inline desde fotograma o miniatura, vista previa de link, y color de estados de texto.
- Nuevo `src/mediainfo.{h,cpp}`: lectura y caché de metadatos, y generación del fotograma en segundo plano.
- `src/uicolorconfig.{h,cpp}`: pares de color a demanda para colores ARGB.
- `src/uiconfig.cpp`: `video_thumbnail_command`.
- `README.md`.
- Sin cambios en la interfaz C del protocolo ni en el esquema de la caché.
