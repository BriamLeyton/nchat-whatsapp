## 1. Implementación

- [x] 1.1 Configs `link_preview_fetch`, `link_preview_command` y `link_thumbnail_command`.
- [x] 1.2 `MediaInfo`: hilo y cola de links, mapa de URL por sesión, `RequestLinkPreview`, y `linkFetched` en `MediaMeta`.
- [x] 1.3 Parseo de HTML: etiquetas meta, `<title>`, entidades, espacios y límites de largo; resolución de URL de imagen relativa.
- [x] 1.4 Escritura de `.meta` y miniatura, resultados aplicados en el hilo de la UI con redibujo.
- [x] 1.5 `UiHistoryView::Draw`: extraer el primer URL y pedir la consulta en la pasada final.
- [x] 1.6 README con aviso de privacidad y niveles de `link_preview_fetch`.
- [x] 1.7 Go: escribir `<profile>/saved_contacts` en `GetContacts`; `MediaInfo::IsSavedContact` con recarga por `mtime`.
- [x] 1.8 Filtro por origen en `UiHistoryView::Draw` según el nivel (propio, contacto guardado, grupo, otro).

## 2. Verificación

- [x] 2.1 Prueba unitaria del parseo con HTML de ejemplo (Open Graph, Twitter, `<title>`, entidades, atributos en otro orden).
- [x] 2.2 Programa de prueba aislado, con red y sin lanzar nchat (no se puede abrir la sesión de WhatsApp dos veces): el pipeline de `MediaInfo` con un link de Facebook y uno de YouTube produce `.meta` y `.thumb.jpg`.
- [x] 2.3 Sandbox sin red, con esos `.meta` ya generados: se ve la vista previa, y al reiniciar no se piden consultas (log).
- [x] 2.4 Con `link_preview_fetch=0` no hay consultas, y una página sin metadatos no se reintenta.
- [x] 2.6 Sandbox sin red: con niveles 1, 2 y 3, el log muestra pedidos solo para los orígenes permitidos.
- [ ] 2.5 En vivo en Ghostty: links viejos y enviados muestran la vista previa con miniatura inline.
