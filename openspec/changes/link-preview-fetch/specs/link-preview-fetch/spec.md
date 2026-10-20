## Purpose

Show the preview of links that do not carry it in the message (old ones, ones sent from nchat, or ones without a preview), by querying the page only for visible messages and storing the result so the query is not repeated.

## ADDED Requirements

### Requirement: Query visible links
When showing a message from an allowed origin (see *Origin filter*), without an attachment, containing a link and with no known preview or prior query, the system SHALL query the first link's page in the background and obtain title, description and image:
- from Open Graph;
- if absent, from Twitter;
- if absent, from `<title>` and `description`.

The query MUST NOT block the UI, and SHALL be done one at a time. Messages that are not shown MUST NOT be queried.

#### Scenario: Old Facebook link
- **WHEN** an old message with a `facebook.com/share/r/...` link is shown
- **THEN** shortly after, the title, description and thumbnail appear below the link

#### Scenario: Chat never opened
- **WHEN** a chat with links is never opened
- **THEN** none of its links are queried

#### Scenario: Disabled
- **WHEN** `link_preview_fetch=0`
- **THEN** no query is made

### Requirement: Origin filter
The system SHALL query links only from the origins allowed by `link_preview_fetch`:
- `0`: none;
- `1`: own messages, and 1:1 chats or statuses from contacts saved in the address book;
- `2` (default): the above, plus any message from a group;
- `3`: everyone, including unknown numbers and channels.

A contact that only has a public name, without being in the address book, MUST NOT be considered saved.

#### Scenario: Unknown sender at level 2
- **WHEN** `link_preview_fetch=2` and a number not in the address book sends a link in a 1:1 chat
- **THEN** the link is not queried, and only the link is shown

#### Scenario: Group at level 1
- **WHEN** `link_preview_fetch=1` and a link arrives in a group
- **THEN** the link is not queried

#### Scenario: Group at level 2
- **WHEN** `link_preview_fetch=2` and a link arrives in a group from any sender
- **THEN** the link is queried

#### Scenario: Own link
- **WHEN** `link_preview_fetch=1` and the user sends a link to an unknown number
- **THEN** the link is queried

### Requirement: Preview cache
The result SHALL be stored in `<msgId>.meta` and `<msgId>.thumb.jpg`, with the mark `link_fetched=1` even if the query fails. A marked message MUST NOT be queried again, not even after a restart. The same URL SHALL be queried only once per session, even if it appears in several messages.

#### Scenario: Restart
- **WHEN** nchat is restarted and an already queried link is shown
- **THEN** its preview is shown without a new query

#### Scenario: Page without metadata
- **WHEN** the page returns no title, description or image
- **THEN** only the link is shown, and it is not queried again

### Requirement: Safe text and thumbnail
The texts SHALL decode HTML entities, and the title SHALL be limited to 200 characters and the description to 500. The thumbnail SHALL be converted to a JPEG at most 320 px wide. The URL MUST be passed to the command in escaped single quotes.

#### Scenario: Entities
- **WHEN** the title is `D&#xed;a &amp; noche`
- **THEN** `Día & noche` is shown

#### Scenario: URL with a quote
- **WHEN** the link contains a single quote
- **THEN** the command receives the full URL as a single argument, without running anything extra
