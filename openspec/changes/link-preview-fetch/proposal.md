## Why

`media-preview` muestra la vista previa de un link solo si el mensaje la trae. Quedan sin vista previa:
- **links viejos:** se recibieron antes de ese cambio;
- **links enviados desde nchat:** nchat no la genera, la arma la app oficial al enviar;
- **links que llegan sin vista previa:** pasa cuando el emisor no la generó.

La única forma de obtenerla es consultar la página del link y leer sus etiquetas Open Graph (`og:title`, `og:description`, `og:image`). Esto no es tráfico hacia WhatsApp, pero sí hacia el sitio del link.

## What Changes

- **Consulta solo de lo visible:** al mostrar un mensaje con link, sin adjunto y sin título ni descripción conocidos, se consulta la página en segundo plano:
  - una consulta a la vez, en un hilo propio, para no frenar fotogramas ni sondeos;
  - se toman título, descripción e imagen de Open Graph. Si no existen, se usan `<title>` y `meta description`;
  - se descarga la imagen y se reduce a una miniatura JPEG.
- **Caché:**
  - el resultado se guarda en los mismos archivos de `media-preview` (`<msgId>.meta` y `<msgId>.thumb.jpg`), así cada link se consulta una sola vez, también entre reinicios;
  - un link que falla se marca como consultado y no se reintenta;
  - el mismo URL en varios mensajes se consulta una sola vez por sesión.
- **Configuración:**
  - `link_preview_fetch` define de quién se consultan links, porque un link de un desconocido puede ser de rastreo:
    - `0`: nunca;
    - `1`: solo contactos guardados en la agenda;
    - `2`: contactos guardados y grupos (default);
    - `3`: todos, también números desconocidos;
  - los links enviados por uno mismo se consultan con cualquier valor distinto de 0;
  - `link_preview_command` y `link_thumbnail_command` son configurables. Por defecto usan `curl` y `ffmpeg`, con límite de tiempo y de tamaño.

Fuera de alcance:
- generar la vista previa al enviar, para que la vea el destinatario;
- ejecutar JavaScript, o sitios que exigen login para dar metadatos (por ejemplo Instagram).

## Capabilities

### New Capabilities
- `link-preview-fetch`: vista previa de links consultando la página, solo de lo visible, con caché en disco.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `lib/wmchat/go/gowm.go`: escribir `<profile>/saved_contacts` con los ids de contactos guardados en la agenda (sin cambiar la interfaz C).
- `src/mediainfo.{h,cpp}`:
  - hilo de links;
  - parseo de HTML (etiquetas meta y entidades);
  - escritura de `.meta` y miniatura;
  - aplicación de resultados en el hilo de la UI.
- `src/uihistoryview.cpp`: pedir la consulta para el primer link de un mensaje visible.
- `src/uiconfig.cpp`: `link_preview_fetch`, `link_preview_command`, `link_thumbnail_command`.
- `README.md`: comportamiento y aviso de privacidad.
