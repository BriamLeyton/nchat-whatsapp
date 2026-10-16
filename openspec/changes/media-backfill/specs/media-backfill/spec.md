## Purpose

Mostrar duración y tamaño de videos y audios que ya están en disco y no tienen esos datos (mensajes viejos o enviados desde nchat), calculándolos localmente.

## ADDED Requirements

### Requirement: Relleno local de duración y tamaño
Al mostrar un adjunto de video o audio descargado sin duración conocida, el sistema SHALL obtener en segundo plano:
- la duración, con `media_probe_command`;
- el tamaño, del archivo en disco.

SHALL guardarlos en `<msgId>.meta` y redibujar al terminar. El relleno MUST NOT generar peticiones de red, y MUST NOT bloquear la UI.

#### Scenario: Video viejo descargado
- **WHEN** se muestra un video descargado antes de `media-preview`
- **THEN** poco después su línea muestra `▶ m:ss · N.N MB`, y al reiniciar se muestra sin volver a sondear

#### Scenario: Video enviado desde nchat
- **WHEN** se muestra un video que el usuario envió desde nchat
- **THEN** su línea muestra la duración y el tamaño

#### Scenario: Nota de voz vieja
- **WHEN** se muestra una nota de voz vieja descargada
- **THEN** su línea muestra `▶` con la duración y el tamaño

#### Scenario: Adjunto no descargado
- **WHEN** se muestra un video viejo no descargado
- **THEN** no se sondea, y la línea queda como hoy

### Requirement: Fallos de sondeo
Si `media_probe_command` está vacío o falla, el sistema SHALL guardar solo el tamaño, y MUST NOT reintentar ese archivo en la misma sesión.

#### Scenario: Sin ffprobe
- **WHEN** el comando falla
- **THEN** la línea muestra solo `▶ N.N MB`
