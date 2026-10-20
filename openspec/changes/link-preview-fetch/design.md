## Context

- **`MediaInfo`:**
  - `Get(profileId, msgId)` lee `.meta` y `.thumb.jpg` y los cachea en memoria;
  - un hilo de trabajo hace fotogramas y sondeos, y sus resultados se aplican en el hilo de la UI desde `TakePosterUpdated()`.
- **Vista:** la vista previa de link ya existe. Usa `title` y `description` del `.meta`, y la miniatura con un alto máximo de 4 filas, para mensajes sin adjunto.
- **Pruebas:**
  - `curl -A facebookexternalhit/1.1` sobre un link `facebook.com/share/r/...` devuelve `og:title`, `og:description` y `og:image`. Sin ese user agent, Facebook da una página de login;
  - los textos vienen con entidades HTML (`&#xed;`, `&#xa0;`, `&amp;`).
- **Decodificación:** `stb_image` no decodifica WebP, que es común en `og:image`. Por eso la miniatura se convierte a JPEG con `ffmpeg`.

## Goals / Non-Goals

**Goals:**
- Consultar solo lo que se dibuja, una vez por link, sin bloquear la UI ni el hilo de fotogramas.
- Que se desactive con una sola opción.

**Non-Goals:**
- Vista previa al enviar.
- Páginas que requieren JavaScript o login.

## Decisions

### 1. Disparo
- Se dispara en la pasada final de `UiHistoryView::Draw`, para un mensaje sin adjunto cuyo `MediaMeta` no tiene `title`, ni `description`, ni `link_fetched`.
- Se toma el primer URL `http://` o `https://` del texto: termina en espacio o salto de línea, y se le quitan `.,;:!?)]}>"'` finales.
- Se llama `MediaInfo::RequestLinkPreview(profileId, msgId, url)`.
- Antes se filtra por origen según `link_preview_fetch`:

  | Origen del mensaje | Nivel mínimo |
  |---|---|
  | enviado por uno mismo | 1 |
  | chat 1:1 o estado de un contacto guardado | 1 |
  | grupo (`@g.us`), cualquier remitente | 2 |
  | número desconocido, canal u otro | 3 |

  Con `0` no se consulta nada. Los estados se evalúan por el remitente.
- No es tráfico hacia WhatsApp, así que la regla de estados vencidos no aplica.

### 1b. Contactos guardados
- `ContactInfo` no distingue un contacto de la agenda de un nombre público (push name). Un contacto es guardado si whatsmeow tiene `FullName` o `FirstName`.
- `GetContacts` en Go escribe `<profile>/saved_contacts`, un id por línea, con el id de teléfono y su alias `@lid`. Se reescribe en cada sincronización, vía archivo temporal y `rename`.
- `MediaInfo::IsSavedContact(profileId, userId)` lee el archivo y lo recarga si cambió su `mtime`. Sin archivo, nadie es guardado: con nivel 1 solo se consultan links propios.

### 2. Hilo de links
- Es un hilo y una cola separados de los fotogramas, una consulta a la vez, porque una consulta puede tardar hasta 8 s.
- Se descartan pedidos repetidos por `profileId/msgId`.
- En la sesión se guarda un mapa `url → resultado`. Si un URL ya se consultó, el resultado se copia al nuevo mensaje sin red.

### 3. Consulta
- **HTML:** `link_preview_command`, con `%1` reemplazado por el URL entre comillas simples escapadas. Se lee la salida con `popen`, con un tope de 1 MB.
  - Default: `curl -s -L -m 8 --max-redirs 5 --compressed --proto =http,https --proto-redir =http,https -A 'facebookexternalhit/1.1' '%1' </dev/null 2>/dev/null`. Sin `| head`, para conservar el código de salida de `curl`; el tope de 1 MB lo aplica nchat.
  - El comando corre en su propio grupo de procesos. Al salir de nchat se mata, para no esperar hasta 8 s.
- **Parseo** sin dependencias:
  - se recorren las etiquetas `<meta ...>` y se leen sus atributos `property` o `name`, y `content`, en cualquier orden y con comillas simples o dobles;
  - **prioridad:** `og:title` > `twitter:title` > `<title>`; `og:description` > `twitter:description` > `description`; `og:image` > `twitter:image`;
  - se decodifican las entidades `&amp; &lt; &gt; &quot; &apos; &nbsp;`, las Latin-1 (`&oacute;`, `&iquest;`...), las tipográficas comunes (`&hellip;`, `&mdash;`...) y las numéricas `&#NNN;` y `&#xHH;` a UTF-8;
  - se reemplazan los espacios en blanco por espacio y se recorta;
  - el título se limita a 200 caracteres y la descripción a 500.
- **Miniatura:** si hay imagen, se resuelve contra el URL si es relativa (`//host/...` o `/path`). Se ejecuta `link_thumbnail_command` con `%1` = imagen y `%2` = `<msgId>.thumb.jpg`.
  - Default: `ffmpeg -nostdin -y -v error -protocol_whitelist http,https,tcp,tls -rw_timeout 8000000 -user_agent 'facebookexternalhit/1.1' -i '%1' -vf "scale='min(320,iw)':-2" -frames:v 1 '%2' >/dev/null 2>&1`.
  - Solo se aceptan URL de imagen `http` o `https`. Una `og:image` como `file:///...` se descarta.

### 4. Caché
- Se escribe `<msgId>.meta`, agregando las claves que falten:
  - `title`, `description`, si se obtuvieron;
  - siempre `link_fetched=1`, también si la página no trae metadatos, para no reintentar.
- **Error de red** (el comando sale con código distinto de 0, por ejemplo sin conexión o dominio inexistente): no se marca. Se reintenta en la próxima sesión, una vez por sesión. Así un arranque sin red no deja links marcados como fallidos.
- El resultado se encola y se aplica a la caché de `MediaInfo` en el hilo de la UI, con redibujo. Es el mismo patrón de los sondeos.
- `MediaMeta` gana `linkFetched` y `thumbPath`, este último actualizado si se creó la miniatura.

### 5. Seguridad y privacidad
- **Lo que no se hace:** no se ejecuta nada de la página. El URL va como argumento entre comillas, y las comillas simples se escapan.
- **Riesgo:** ver un chat consulta los links visibles. El sitio ve la IP y sabe que el link se vio. Por ejemplo, un link de spam con rastreo confirmaría que el número está activo.
  - El filtro por origen lo limita a contactos guardados y grupos por defecto.
  - El README lo advierte. `link_preview_fetch=0` lo desactiva y `3` lo abre a todos.
- **Default:** `link_preview_fetch=2`, a pedido del usuario.

## Risks / Trade-offs

- **User agent:** `facebookexternalhit` hace que Facebook y otros sitios devuelvan metadatos. Algunos sitios pueden responder distinto. Es configurable.
- **Sitios con login** (Instagram, algunos TikTok): sin metadatos. Se marcan como consultados y se ve el link solo.
- **Disco:** una miniatura de unos 20 KB y un `.meta` por link visto.
