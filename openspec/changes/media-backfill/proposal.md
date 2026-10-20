## Why

`media-preview` toma la duración y el tamaño de los datos que trae el mensaje. Eso solo existe para lo recibido desde ese cambio. Quedan sin datos:
- **Mensajes viejos**, recibidos o enviados desde el teléfono: los datos venían en el mensaje, pero nchat los descartó.
- **Archivos enviados desde nchat:** no pasan por el manejador de recepción. Su `fileInfo` apunta al archivo local, ya "descargado".

Para todo video o audio que ya esté en disco, la duración y el tamaño se pueden sacar del archivo, de forma local y sin red.

## What Changes

- **Relleno local:** al mostrar un video o audio descargado (de cualquier chat y dirección, viejo o enviado desde nchat) sin duración conocida, se obtiene en segundo plano:
  - la duración, con un comando configurable (`media_probe_command`, por defecto `ffprobe`);
  - el tamaño, del archivo en disco.
- **Persistencia:** el resultado se guarda en `<msgId>.meta`, con el mismo formato de `media-preview`, para no recalcularlo.
- Usa el mismo hilo de trabajo que el fotograma de video, uno a la vez, y redibuja al terminar.
- Un archivo que falla no se reintenta en la sesión.
- Al descargarse un video o audio que no tenía datos, también se rellena cuando se muestra.

Fuera de alcance:
- adjuntos no descargados (no hay datos locales ni en el mensaje guardado);
- miniaturas de imágenes viejas no descargadas;
- vista previa de links viejos o enviados (requiere consultar la web; sería otro change).

## Capabilities

### New Capabilities
- `media-backfill`: duración y tamaño de videos y audios ya descargados, calculados localmente.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/mediainfo.{h,cpp}`:
  - tarea de sondeo en el hilo de trabajo existente;
  - escritura de `.meta`;
  - actualización de la caché en memoria.
- `src/uihistoryview.cpp`: pedir el relleno para video o audio descargado sin `seconds`.
- `src/uiconfig.cpp`: `media_probe_command`.
- `README.md`.
