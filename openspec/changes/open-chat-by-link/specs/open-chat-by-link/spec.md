## Purpose

Open or create a chat from the contacts dialog by typing or pasting a WhatsApp number or link, with the link's text ready to send.

## ADDED Requirements

### Requirement: Number or link in the contacts filter
When the filter of the contacts dialog contains a WhatsApp link (`wa.me`, `api.whatsapp.com/send`, `whatsapp://send`) or an international number of 8 to 15 digits, the dialog SHALL show as its first row `Open chat with +<number>`. The number SHALL come from the path or the `phone` parameter, without any non-digit characters.

#### Scenario: api.whatsapp.com link
- **WHEN** the user pastes `https://api.whatsapp.com/send/?phone=15551234567&text=Hello+ACME.&type=phone_number&app_absent=0` in Alt-n
- **THEN** the first row is `Open chat with +15551234567`

#### Scenario: Number typed with formatting
- **WHEN** the user types `+1 555 123 4567`
- **THEN** the first row is `Open chat with +15551234567`

#### Scenario: Normal text
- **WHEN** the user types a name
- **THEN** no row appears and the list filters contacts as before

### Requirement: Open or create the chat
When the row is chosen, if a chat with that number exists it SHALL be opened (unarchiving it if it was archived) without querying WhatsApp. If it does not exist, nchat SHALL query once whether the number is registered; if it is, SHALL create and open the chat with the number WhatsApp returns (`<number>@s.whatsapp.net`), never with its LID, so that sent and received messages end up in that same chat. If it is not registered it SHALL show "+<number> is not on WhatsApp.". Without a connection it SHALL show "Not connected, please try again later." and MUST NOT query. The query MUST NOT be made while the user types in the filter.

#### Scenario: Existing chat
- **WHEN** the user chooses the row of a number that has a chat
- **THEN** that chat is opened and no query is sent

#### Scenario: New registered number
- **WHEN** the user chooses the row of a number with no chat that is on WhatsApp
- **THEN** the chat is created and left open

#### Scenario: First message to a new number
- **WHEN** the user creates the chat of a new number and sends a message
- **THEN** the message appears in that chat, which moves to the top of its section and stays selected, without creating a second chat

#### Scenario: Unregistered number
- **WHEN** the number is not on WhatsApp
- **THEN** "+<number> is not on WhatsApp." appears and no chat is created

### Requirement: Link text in the message box
If the link has `text`, when the chat is opened the decoded text SHALL be placed in the message box with the cursor at the end, without being sent. If the chat already had a draft, the draft SHALL be kept and "Draft kept, link text not inserted." SHALL appear.

#### Scenario: Text with accents
- **WHEN** the link has `text=Hello+ACME.+Quiero+m%C3%A1s+informaci%C3%B3n`
- **THEN** the message box shows `Hello ACME. Quiero más información` and nothing is sent

#### Scenario: Existing draft
- **WHEN** the chat already has text typed in the box
- **THEN** the typed text does not change and the notice appears
