## Why

- **Espera al abrir:** hoy un estado se descarga recién cuando se ve en pantalla (imágenes inline) o al abrirlo (videos). La app oficial los descarga apenas llegan, para que estén listos al verlos.
- **Disco:** los adjuntos de estados ya ocupan 1.1 GB en `tmp`, y 972 MB son videos (≈4.6 MB cada uno).
  - Llegan unos 55 videos por día. Precargarlos todos sumaría unos 250 MB/día sin límite.
  - Los videos vencidos ya no se pueden volver a descargar, por la regla de `status-expired-local-only`. Conservarlos para siempre solo acumula disco.

## What Changes

- **Precarga de estados** (`status_prefetch=1`): al llegar un estado vigente (menos de 24 h) con adjunto sin descargar, se descarga en segundo plano.
  - Las imágenes se precargan siempre.
  - Los videos se precargan si su tamaño, tomado de los metadatos de `media-preview`, no supera `status_prefetch_video_max_mb` (default 16). Sin metadatos de tamaño no se precargan.
  - Las precargas van en una cola propia, una a la vez: la siguiente se envía cuando termina la anterior. Así enviar mensajes no queda esperando detrás de 50 videos, porque el protocolo procesa las peticiones de a una.
  - Los estados vencidos nunca se precargan.
- **Limpieza de videos** (`status_video_retention_days`, default 0 = nunca; el usuario la activa después):
  - con un valor N mayor que 0, al iniciar, en segundo plano, se borran los archivos de video de estados con más de N días;
  - su `fileInfo` en la caché pasa a "no descargado";
  - se conservan imágenes, miniaturas, fotogramas y metadatos, así el estado se sigue viendo con su fotograma o miniatura.

Fuera de alcance: limpiar imágenes, y borrar mensajes de estados de la caché.

## Capabilities

### New Capabilities
- `status-prefetch-cleanup`: precarga de estados vigentes y retención limitada de videos de estados.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`:
  - cola de precarga alimentada desde `NewMessagesNotify` del chat de estados, que avanza con `NewMessageFileNotify`;
  - limpieza al iniciar el perfil.
- `lib/ncutil/src/messagecache.{h,cpp}`: consulta síncrona de estados con adjunto anteriores a una fecha.
- `src/uiconfig.cpp`: `status_prefetch`, `status_prefetch_video_max_mb`, `status_video_retention_days`.
- `README.md`.
- Depende de `media-preview` (tamaño del video y fotograma).
