## 1. Implementación

- [x] 1.1 `MarkRead` con `p_Opened`: ignorar el chat de estados si es falso.
- [x] 1.2 `MarkStatusOpened()` y llamadas desde `OnKeyOpenAttachment` (acción del usuario) y `OnKeyOpenMsg`.
- [x] 1.3 README: visto en estados solo al abrir.

## 2. Verificación

- [x] 2.1 Sandbox sin red: entrar a Status y recorrer con el cursor no genera `mark message read` en el log, y la fila sigue no leída.
- [x] 2.2 Sandbox sin red: `open` u `open_msg` sobre un estado genera un solo `mark message read` (vigente) o la marca local (vencido).
- [x] 2.3 Sandbox sin red: un chat normal se sigue marcando al verse.
- [ ] 2.4 En vivo en Ghostty: entrar a Status no marca nada; abrir un estado lo marca y desaparece del conteo.
