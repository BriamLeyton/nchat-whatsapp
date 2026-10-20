## Purpose

Prevent nchat from generating traffic to WhatsApp for expired status updates, and prevent publishing status updates by accident from the status chat.

## ADDED Requirements

### Requirement: Expired status
The system SHALL consider expired every message in the status chat (`status@broadcast`) whose publication time is more than 24 hours before the current time.

#### Scenario: Status from yesterday
- **WHEN** a status was published 25 hours ago
- **THEN** it is considered expired

#### Scenario: Status from today
- **WHEN** a status was published 3 hours ago
- **THEN** it is not considered expired and works as it does today

### Requirement: Local-only seen
When showing an unread expired status, the system SHALL mark it as read in memory and in the local cache, and MUST NOT send the read receipt to WhatsApp.

#### Scenario: Browse old unread statuses
- **WHEN** the user browses unread statuses from 5 days ago
- **THEN** they become read in nchat, also after a restart, and no read receipt is sent

#### Scenario: Current status
- **WHEN** the user views an unread status from 2 hours ago
- **THEN** the read receipt is sent as it is today

### Requirement: No downloads of expired statuses
The system MUST NOT ask WhatsApp to download the attachment of an expired status for any reason: automatic download, inline image, selection, open, save or view in an external viewer. If the attachment is already downloaded, the local file SHALL be used. Otherwise, the history MUST show the `📎` line without a download-in-progress indicator, and open or save MUST show the notice "Expired status, attachment not available."

#### Scenario: Expired inline photo not downloaded
- **WHEN** an expired status with a photo that was never downloaded is shown
- **THEN** the `📎` line is shown without an image and no download request is made

#### Scenario: Expired photo already downloaded
- **WHEN** an expired status whose photo is already on disk is shown
- **THEN** the photo is shown inline as it is today

#### Scenario: Open expired attachment
- **WHEN** the user selects an expired status without a downloaded file and opens it
- **THEN** the notice is shown and no download request is made

### Requirement: No reactions to expired statuses
Reacting to an expired status SHALL NOT send anything to WhatsApp, and MUST show the notice "Expired status, cannot react."

#### Scenario: React to an old status
- **WHEN** the user selects a status from 3 days ago and presses `Alt-s`
- **THEN** the notice is shown and the reaction is not sent

### Requirement: Read-only status chat
In the status chat, sending text, sending files and editing messages SHALL NOT send anything to WhatsApp, and MUST show the notice "Sending to Status Updates is not supported." The text typed in the input box MUST be kept.

#### Scenario: Enter in Status
- **WHEN** the user types "hello" in the status chat and presses `Enter`
- **THEN** the notice is shown, no status is published and "hello" stays in the input box

#### Scenario: Send a file in Status
- **WHEN** the user tries to send a file in the status chat
- **THEN** the notice is shown and nothing is sent
