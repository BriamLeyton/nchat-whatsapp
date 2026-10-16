## Purpose

Mostrar las miniaturas de imágenes y videos no descargados del mismo tamaño que las imágenes descargadas, y agrandar un poco todas las vistas previas inline.

## ADDED Requirements

### Requirement: Miniaturas al tamaño de imagen
La miniatura incluida en el mensaje SHALL agrandarse, manteniendo la proporción, hasta llenar el mismo recuadro que una imagen descargada (ancho del historial × `inline_image_max_height` filas). Las imágenes descargadas MUST NOT agrandarse.

#### Scenario: Video vertical no descargado
- **WHEN** se muestra un video no descargado con una miniatura de 90×160
- **THEN** la miniatura ocupa la misma altura que una imagen vertical descargada

#### Scenario: Sticker descargado
- **WHEN** se muestra una imagen descargada más chica que el recuadro
- **THEN** se ve en su tamaño real, sin agrandar

### Requirement: Tamaños por defecto mayores
El default de `inline_image_max_height` SHALL ser 14, y la miniatura de una vista previa de link SHALL ocupar hasta 6 filas, o `inline_image_max_height` si es menor.

#### Scenario: Config nueva
- **WHEN** nchat arranca sin `ui.conf`
- **THEN** `inline_image_max_height=14`
