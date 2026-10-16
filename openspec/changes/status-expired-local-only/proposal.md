## Why

La app oficial de WhatsApp no deja ver estados de más de 24 horas. nchat los guarda en su caché local y permite recorrerlos, lo cual es útil, pero al mostrarlos genera tráfico que ningún cliente oficial produce:
- envía confirmaciones de visto de estados de hace días (hay 602 sin leer en la caché del usuario);
- intenta descargar fotos y videos de estados vencidos.

Además, el chat de estados acepta texto: enviar desde ahí manda el mensaje a `status@broadcast`, que puede publicarse como un estado propio.

El usuario quiere evitar cualquier patrón que pueda llamar la atención de WhatsApp sobre su cuenta.

## What Changes

- **Estados vencidos** (publicados hace más de 24 horas) son solo locales:
  - al verlos se marcan como leídos solo en nchat, sin enviar la confirmación a WhatsApp;
  - por ningún motivo se descargan sus adjuntos: ni la descarga automática, ni las imágenes inline, ni al abrir o guardar. Si el archivo ya está en disco, se usa; si no, queda la línea `📎` y, al abrir o guardar, un aviso;
  - no se puede reaccionar a ellos.
- **Envío bloqueado en el chat de estados:** enviar texto, enviar archivos y editar no hacen nada en Status y muestran un aviso. Así nunca se publica un estado por accidente.
- Los estados de menos de 24 horas siguen como hoy: confirmación de visto, descargas y reacciones.

Fuera de alcance: publicar estados y responder a un estado por chat privado.

## Capabilities

### New Capabilities
- `status-expired-local-only`: estados vencidos sin tráfico hacia WhatsApp, y chat de estados de solo lectura.

### Modified Capabilities
<!-- No hay specs existentes en openspec/specs/. -->

## Impact

- `src/uimodel.{h,cpp}`: chequeo de estado vencido, y bloqueos en `MarkRead`, `DownloadAttachment` (único punto de descarga), abrir/guardar, reaccionar, enviar texto, enviar archivo y editar.
- `README.md`: documenta el comportamiento.
- Sin cambios de protocolo ni de caché.
