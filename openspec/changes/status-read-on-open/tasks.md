## 1. Implementation

- [x] 1.1 `MarkRead` with `p_Opened`: ignore the statuses chat if it is false.
- [x] 1.2 `MarkStatusOpened()` and calls from `OnKeyOpenAttachment` (user action) and `OnKeyOpenMsg`.
- [x] 1.3 README: statuses are seen only when opened.

## 2. Verification

- [x] 2.1 Offline sandbox without network: entering Status and moving the cursor through it does not produce `mark message read` in the log, and the row stays unread.
- [x] 2.2 Offline sandbox without network: `open` or `open_msg` on a status produces a single `mark message read` (current) or the local mark (expired).
- [x] 2.3 Offline sandbox: a normal chat is still marked when seen.
- [ ] 2.4 Live in Ghostty: entering Status marks nothing; opening a status marks it and it leaves the count.
