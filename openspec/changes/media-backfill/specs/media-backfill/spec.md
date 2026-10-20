## Purpose

Show duration and size for videos and audios that are already on disk and lack that data (old messages or files sent from nchat), computed locally.

## ADDED Requirements

### Requirement: Local backfill of duration and size
When showing a downloaded video or audio attachment with no known duration, the system SHALL obtain in the background:
- the duration, with `media_probe_command`;
- the size, from the file on disk.

It SHALL store them in `<msgId>.meta` and redraw when done. The backfill MUST NOT generate network requests, and MUST NOT block the UI.

#### Scenario: Old downloaded video
- **WHEN** a video downloaded before `media-preview` is shown
- **THEN** shortly after, its line shows `▶ m:ss · N.N MB`, and after a restart it is shown without probing again

#### Scenario: Video sent from nchat
- **WHEN** a video that the user sent from nchat is shown
- **THEN** its line shows the duration and the size

#### Scenario: Old voice note
- **WHEN** an old downloaded voice note is shown
- **THEN** its line shows `▶` with the duration and the size

#### Scenario: Attachment not downloaded
- **WHEN** an old video not downloaded is shown
- **THEN** it is not probed, and the line stays as it is today

### Requirement: Probe failures
If `media_probe_command` is empty or fails, the system SHALL store only the size, and MUST NOT retry that file in the same session.

#### Scenario: Without ffprobe
- **WHEN** the command fails
- **THEN** the line shows only `▶ N.N MB`
