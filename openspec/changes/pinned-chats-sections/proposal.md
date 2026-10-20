## Why

nchat already lets you pin chats with `Alt-p` and archive them with `Alt-a`, but:
- **Synchronization was broken:** the local copy of WhatsApp's `regular_low` state (pins and archived) became corrupt (`mismatching LTHash`), and the server snapshot too. Because of that, `Alt-p` failed with `409 conflict` and the pins and archived chats from the phone did not arrive. It is already fixed in commit `3f567a41`.
- **There is no visual distinction:** the list does not show which chats are pinned.
- **Archived chats are invisible:** nchat hides them completely and discards their new messages in the UI. With synchronization fixed, there are 74 chats that disappeared from the list.

## What Changes

- **WhatsApp state recovery (done):** on `mismatching LTHash`, nchat does a full resync from the server. If the server snapshot is also inconsistent, it asks the phone for a recovery copy, as WhatsApp Web does, and retries the change.
- **Three sections in the list, in this order:**
  - `Fijados`: pinned chats.
  - `Otros`: chats that are neither pinned nor archived.
  - `Archivados`: archived chats, sorted by last message.

  Each title is shown only if its section has chats. With only the `Otros` section, the list looks as it does today, without titles.
- **Collapsible sections:** `Alt-o` collapses or expands `Otros`, and `Alt-h` does the same for `Archivados`. A collapsed section shows only its title, with the number of chats and `*` if any has unread messages. `Archivados` starts collapsed. The state is remembered across restarts.
- **Archived chats:**
  - They can be opened and read.
  - They receive new messages in the UI, but do not generate notifications, as today.
  - They stay archived when receiving messages, as in WhatsApp with "Keep chats archived".
  - `Alt-a` moves a chat between `Otros` and `Archivados` live.
- **Configuration:**
  - `ui.conf`: `list_pinned_header`, `list_others_header`, `list_archived_header`, `list_others_collapsed` and `list_archived_collapsed`.
  - `key.conf`: `toggle_others` and `toggle_archived`.
  - `color.conf`: `list_color_header`.

Out of scope: local-only pinning, manual ordering among pinned chats, and WhatsApp's limit of 3 pinned chats.

## Capabilities

### New Capabilities
- `chat-list-pinned-sections`: `Fijados`, `Otros` and `Archivados` sections in the chat list, collapsible, and reliable synchronization of pinned and archived chats even when WhatsApp's state has become corrupt.

### Modified Capabilities
<!-- No existing specs in openspec/specs/. -->

## Impact

- **Go (`lib/wmchat/go/gowm.go`, done):** `ResyncAppState`, `RequestAppStateRecovery` and `SendAppStateWithResync`, plus handling of `events.AppStateSyncError` and of `AppStateSyncComplete` with `Recovery`.
- **UI:**
  - `src/uimodel.{h,cpp}`: archived chats enter `m_ChatVec`, order by section, collapsed state, navigation between visible chats and new keys.
  - `src/uilistview.cpp`: title rows and omission of collapsed chats.
  - `src/uiconfig.cpp`, `src/uikeyconfig.cpp`, `src/uicolorconfig.cpp`: new options.
- No C++ protocol or schema changes (`chats2.isArchived` already exists).
