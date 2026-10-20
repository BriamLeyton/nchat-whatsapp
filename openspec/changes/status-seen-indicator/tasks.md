## 1. Implementation

- [x] 1.1 Reviewed per profile: lazy reading of `<profile>/status_seen`, baseline on first start, pruning at 30 days, atomic write.
- [x] 1.2 `IsStatusNew` and `MarkStatusSeen` (without touching `isRead` or sending anything), with their `Locked` variants in `UiModel`.
- [x] 1.3 `UpdateChatInfoIsUnread`: for Status, unread if there is any new status.
- [x] 1.4 `UiHistoryView`: `status_new_indicator` before the name on new statuses; `MarkStatusSeenLocked` for the selected status with `p_Final`.
- [x] 1.5 `UiListView`: the Status row uses `status_new_indicator` instead of `unread_indicator`.
- [x] 1.6 `status_new_indicator=●` in `uiconfig.cpp` and README, with the meaning of the mark on statuses and on the Status row.

## 2. Verification

- [x] 2.1 Offline sandbox without network:
  - without `status_seen`, no status is shown marked;
  - with the baseline moved back, later statuses show `●` and so does the Status row;
  - browsing with the cursor removes `●` and, at the end, the one on the Status row, without sending a read receipt (log);
  - restarting keeps the reviewed ones.
- [x] 2.2 Live in Ghostty: a new status shows `●` on the status and on the Status row, and browsing it removes them.
