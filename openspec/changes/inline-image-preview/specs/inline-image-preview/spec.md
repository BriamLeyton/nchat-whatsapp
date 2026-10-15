## Purpose

Mostrar los adjuntos de imagen de WhatsApp como miniaturas dentro del historial de chat en terminales compatibles con el protocolo gráfico de kitty (p. ej. Ghostty), manteniendo la línea de adjunto como fallback.

## ADDED Requirements

### Requirement: Renderizado inline de adjuntos de imagen
El sistema SHALL renderizar como miniatura inline, en el historial, todo adjunto de mensaje cuyo archivo sea una imagen soportada (JPEG, PNG, WebP o el primer frame de un GIF) cuando la función está activa y la terminal la soporta. La miniatura MUST aparecer inmediatamente debajo de la línea de adjunto del mismo mensaje y encima del texto (caption) del mensaje.

#### Scenario: Foto recibida y descargada
- **WHEN** el chat actual contiene un mensaje con una foto JPEG ya descargada y el mensaje está dentro del área visible del historial
- **THEN** el historial muestra la línea `📎 <archivo>.jpg`, debajo la miniatura de la foto y debajo el caption si existe

#### Scenario: Sticker WebP
- **WHEN** el chat actual contiene un sticker `.webp` descargado y visible
- **THEN** el historial muestra el sticker como miniatura inline, conservando la transparencia sobre el fondo del panel

#### Scenario: Adjunto que no es imagen
- **WHEN** el mensaje tiene un adjunto PDF, audio o video
- **THEN** el historial muestra solo la línea de adjunto, igual que antes de este cambio

### Requirement: Tamaño de la miniatura
La miniatura SHALL ocupar como máximo `inline_image_max_height` filas de terminal (por defecto 10) y como máximo el ancho del panel de historial. MUST conservar la relación de aspecto de la imagen original y MUST NOT ampliar imágenes más pequeñas que ese máximo.

#### Scenario: Foto vertical grande
- **WHEN** se muestra una foto de 3000×4000 píxeles con `inline_image_max_height=10`
- **THEN** la miniatura ocupa exactamente 10 filas y el ancho proporcional en columnas

#### Scenario: Foto panorámica en panel angosto
- **WHEN** se muestra una foto de 4000×1000 píxeles y el ancho proporcional a 10 filas supera el ancho del panel
- **THEN** la miniatura se limita al ancho del panel y reduce sus filas para mantener la proporción

#### Scenario: Imagen pequeña
- **WHEN** se muestra una imagen cuyo tamaño natural cabe en menos filas que el máximo
- **THEN** la miniatura se dibuja a su tamaño natural sin ampliarla

### Requirement: Fallback a la línea de adjunto
Cuando no sea posible renderizar la imagen, el sistema SHALL mostrar solo la línea de adjunto (`📎 <archivo>` más el indicador de estado), sin reservar filas vacías. MUST aplicarse cuando la terminal no soporta el protocolo gráfico de kitty, cuando `inline_images_enabled=0`, cuando el archivo no se puede leer o decodificar, y mientras la imagen no está descargada.

#### Scenario: Terminal sin soporte gráfico
- **WHEN** nchat corre en una terminal que no responde a la consulta del protocolo gráfico de kitty
- **THEN** todos los adjuntos de imagen se muestran solo como línea `📎`, igual que antes de este cambio

#### Scenario: Función desactivada
- **WHEN** `ui.conf` contiene `inline_images_enabled=0`
- **THEN** no se emite ninguna secuencia gráfica y los adjuntos se muestran solo como línea `📎`

#### Scenario: Archivo corrupto
- **WHEN** el archivo descargado de una imagen no se puede decodificar
- **THEN** se muestra solo la línea `📎` y el error se registra en el log sin interrumpir la UI

#### Scenario: Imagen aún descargándose
- **WHEN** la imagen está en estado descargando
- **THEN** se muestra la línea `📎 <archivo> ⇄` y, cuando la descarga termina, el historial se redibuja con la miniatura

### Requirement: Auto-descarga de imágenes visibles
El sistema SHALL iniciar automáticamente la descarga de todo adjunto de imagen descargable que aparezca en el área visible del historial cuando la función está activa y la terminal la soporta, sin importar el valor de `attachment_prefetch`. Los adjuntos que no son imagen MUST seguir respetando `attachment_prefetch`.

#### Scenario: Prefetch solo seleccionado
- **WHEN** `attachment_prefetch=1` y el usuario abre un chat con tres fotos no descargadas visibles
- **THEN** las tres fotos empiezan a descargarse y se muestran como miniaturas al terminar

#### Scenario: Documento no seleccionado
- **WHEN** `attachment_prefetch=1` y hay un PDF visible que no está seleccionado
- **THEN** el PDF no se descarga

### Requirement: Coherencia visual con la UI de texto
Las miniaturas SHALL dibujarse solo dentro del área del panel de historial y MUST NOT tapar otros paneles ni diálogos. Al hacer scroll, cambiar de chat, redimensionar la terminal o cerrar un diálogo, las miniaturas MUST quedar en la posición del mensaje correspondiente, sin restos de dibujos anteriores. Una miniatura que solo cabe parcialmente en el borde superior del panel MUST recortarse a la parte visible.

#### Scenario: Abrir un diálogo
- **WHEN** hay miniaturas visibles y el usuario abre un diálogo (p. ej. lista de emojis o de contactos) que se superpone al historial
- **THEN** las miniaturas desaparecen mientras el diálogo está abierto y reaparecen en su lugar al cerrarlo

#### Scenario: Scroll en el historial
- **WHEN** el usuario sube una página en el historial
- **THEN** las miniaturas de los mensajes que salen de la vista desaparecen y las de los mensajes que entran se dibujan en su nueva posición

#### Scenario: Cambio de chat
- **WHEN** el usuario cambia a otro chat
- **THEN** no queda ninguna miniatura del chat anterior en pantalla

#### Scenario: Imagen recortada en el borde superior
- **WHEN** la parte superior de una miniatura quedaría por encima del panel de historial
- **THEN** solo se dibujan las filas de la miniatura que caen dentro del panel

#### Scenario: Salida de nchat
- **WHEN** el usuario sale de nchat
- **THEN** todas las imágenes transmitidas a la terminal se eliminan y la terminal queda sin restos gráficos

### Requirement: Configuración
El sistema SHALL leer `inline_images_enabled` (booleano, por defecto `1`) e `inline_image_max_height` (entero de filas, por defecto `10`) desde `ui.conf`. Las claves MUST agregarse con su valor por defecto a `ui.conf` cuando no existan, sin modificar otras claves existentes.

#### Scenario: Primera ejecución tras actualizar
- **WHEN** el usuario arranca la nueva versión con un `ui.conf` existente que no tiene las claves nuevas
- **THEN** `ui.conf` incluye `inline_images_enabled=1` e `inline_image_max_height=10` y el resto de la configuración queda igual

#### Scenario: Altura personalizada
- **WHEN** `inline_image_max_height=5`
- **THEN** ninguna miniatura ocupa más de 5 filas

### Requirement: Selección y apertura del mensaje sin cambios
La selección de mensajes, la apertura de adjuntos, el guardado, la respuesta y las reacciones SHALL comportarse igual que antes. Cada miniatura MUST contar como parte del mensaje al que pertenece para el scroll y la selección.

#### Scenario: Abrir imagen con miniatura
- **WHEN** el usuario selecciona un mensaje con miniatura y usa la tecla de abrir adjunto
- **THEN** la imagen se abre con el comando externo configurado, igual que antes
