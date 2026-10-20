## Purpose

Tener los estados vigentes descargados antes de verlos, sin bloquear el envío de mensajes, y acotar el disco que ocupan los videos de estados viejos.

## ADDED Requirements

### Requirement: Precarga de estados vigentes
Con `status_prefetch=1`, el sistema SHALL descargar en segundo plano el adjunto de cada estado vigente no descargado que se cargue, ya sea recibido o leído de la caché:
- imágenes siempre;
- videos solo si su tamaño conocido no supera `status_prefetch_video_max_mb` MB.

El sistema MUST NOT precargar estados vencidos, ni videos de tamaño desconocido.

#### Scenario: Llega una foto de estado
- **WHEN** llega un estado con foto
- **THEN** la foto se descarga sin abrir el chat de estados

#### Scenario: Video grande
- **WHEN** llega un estado con un video de 40 MB y el tope es 16
- **THEN** no se precarga, y se descarga solo si el usuario lo abre

#### Scenario: Estado vencido en caché
- **WHEN** se lee de la caché un estado de hace 3 días sin descargar
- **THEN** no se precarga

#### Scenario: Desactivado
- **WHEN** `status_prefetch=0`
- **THEN** los estados se descargan como hoy

### Requirement: Precarga de a una
Las precargas SHALL enviarse de a una: la siguiente solo después de que termine la anterior, o tras 120 s sin respuesta. Antes de enviar cada una, el sistema SHALL saltarla si ya está descargada o si venció.

#### Scenario: Muchos estados y un mensaje
- **WHEN** hay 30 estados encolados y el usuario envía un mensaje
- **THEN** el mensaje sale después de la descarga en curso, no después de las 30

### Requirement: Retención de videos de estados
El default de `status_video_retention_days` SHALL ser 0. Con un valor mayor que 0, al iniciar el sistema SHALL borrar en segundo plano los archivos de video de estados publicados hace más de esos días, y marcar su adjunto como no descargado en la caché. MUST conservar imágenes, fotogramas, miniaturas y metadatos. Con valor 0 MUST NOT borrar nada.

#### Scenario: Default
- **WHEN** nchat inicia sin cambiar `status_video_retention_days`
- **THEN** no se borra ningún archivo

#### Scenario: Video de hace 10 días
- **WHEN** nchat inicia con retención 7 y hay un video de estado de hace 10 días en disco
- **THEN** el archivo de video se borra, y el estado se sigue viendo con su fotograma o miniatura sin pedir descarga

#### Scenario: Imagen vieja
- **WHEN** hay una foto de estado de hace 30 días
- **THEN** no se borra

#### Scenario: Abrir video limpiado
- **WHEN** el usuario abre un video de estado ya limpiado
- **THEN** ve el aviso "Expired status, attachment not available." y no se descarga
