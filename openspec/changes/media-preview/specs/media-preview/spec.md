## Purpose

Mostrar mejor los videos, audios, links y estados de texto en todos los chats, usando solo los datos que trae el mensaje y procesos locales, sin peticiones nuevas a WhatsApp.

## ADDED Requirements

### Requirement: Metadatos sin red
Al recibir un mensaje de cualquier chat, el sistema SHALL guardar junto al adjunto, en el directorio `tmp` del perfil, la miniatura incrustada del mensaje (`<msgId>.thumb.jpg`) y un archivo `<msgId>.meta`:
- video y audio: duración y tamaño;
- audio: si es nota de voz;
- texto extendido: título y descripción del link;
- estados: color de fondo y color de texto.

Un mensaje sin esos datos MUST NOT generar archivos.

Guardarlos MUST NOT generar peticiones de red. Si ya existen, MUST NOT sobrescribirse.

#### Scenario: Llega un video de estado
- **WHEN** llega un estado de video de 15 s y 4.2 MB con miniatura
- **THEN** existen `<msgId>.thumb.jpg` y `<msgId>.meta`, con `seconds=15` y el tamaño en bytes, y no se hizo ninguna descarga

#### Scenario: Link en un grupo
- **WHEN** llega a un grupo un mensaje con un link que trae título y miniatura
- **THEN** existen `<msgId>.thumb.jpg` y `<msgId>.meta` con el título

#### Scenario: Texto plano
- **WHEN** llega un mensaje de texto sin link
- **THEN** no se escribe ningún archivo

### Requirement: Extensión de video
El sistema SHALL guardar los videos `video/mp4` con extensión `.mp4`.

#### Scenario: Video nuevo
- **WHEN** llega un video `video/mp4`
- **THEN** su ruta de archivo termina en `.mp4`, no en `.f4v`

### Requirement: Línea de video y audio
Para un adjunto de video o audio con metadatos, la línea del adjunto SHALL mostrar `▶` con la duración en `m:ss` y el tamaño en MB con un decimal, cuando existan. Una nota de voz SHALL mostrar `🎤` con la duración. Sin metadatos SHALL verse como hoy.

#### Scenario: Video con metadatos
- **WHEN** se muestra un video de 75 s y 4 404 019 bytes, en un chat o en estados
- **THEN** la línea es `📎 <archivo> ▶ 1:15 · 4.2 MB`

#### Scenario: Nota de voz
- **WHEN** se muestra una nota de voz de 42 s
- **THEN** la línea es `📎 <archivo> 🎤 0:42`

#### Scenario: Mensaje viejo
- **WHEN** se muestra un video recibido antes de este cambio
- **THEN** la línea es igual que hoy

### Requirement: Fotograma de video
Con imágenes inline activas, un video descargado de cualquier chat SHALL mostrarse inline con un fotograma generado localmente por `video_thumbnail_command`:
- la generación MUST hacerse fuera del hilo de la UI, y el historial SHALL redibujarse al terminar;
- si el comando está vacío o falla, SHALL usarse la miniatura incrustada si existe;
- un video que falló MUST NOT reintentarse en la misma sesión.

#### Scenario: Video descargado
- **WHEN** se muestra un video descargado sin fotograma previo
- **THEN** la UI sigue respondiendo, se crea `<archivo>.poster.jpg` y luego se ve inline

#### Scenario: Sin ffmpegthumbnailer
- **WHEN** el comando falla
- **THEN** se ve la miniatura incrustada, o solo la línea si no hay miniatura, y no se reintenta

### Requirement: Miniatura de respaldo
Una imagen o video sin descargar, de cualquier chat, SHALL mostrar inline su miniatura incrustada, si existe. Esto MUST NOT disparar descargas, incluso para estados vencidos.

#### Scenario: Foto vencida sin descargar
- **WHEN** se muestra un estado vencido con foto no descargada y miniatura guardada
- **THEN** se ve la miniatura inline y no hay petición de descarga

### Requirement: Vista previa de link
Un mensaje de cualquier chat con título o descripción de link SHALL mostrar, debajo del texto:
- una línea `▏` con el título en negrita;
- hasta 2 líneas `▏` con la descripción;
- la miniatura inline, con un alto máximo de 4 filas.

#### Scenario: Link de Facebook
- **WHEN** se muestra un mensaje o estado con un link que trae título, descripción y miniatura
- **THEN** se ven la URL, el título, la descripción y la miniatura

### Requirement: Color de estados de texto
Un estado sin adjunto con color de fondo SHALL dibujar sus líneas de texto con ese fondo:
- el color de texto es el del estado; si no viene, blanco o negro según la luminancia del fondo;
- si no hay colores disponibles, o se superó el límite de pares de color, SHALL dibujarse sin color.

#### Scenario: Estado de texto morado
- **WHEN** se muestra un estado de texto con fondo `0xff8a2be2`
- **THEN** sus líneas de texto tienen fondo morado y el encabezado se ve normal
