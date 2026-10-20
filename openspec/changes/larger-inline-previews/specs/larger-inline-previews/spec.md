## Purpose

Show thumbnails of images and videos not yet downloaded at the same size as downloaded images, and enlarge all inline previews a bit.

## ADDED Requirements

### Requirement: Thumbnails at image size
The thumbnail included in the message SHALL be enlarged, keeping its proportion, until it fills the same box as a downloaded image (history width × `inline_image_max_height` rows). Downloaded images MUST NOT be enlarged.

#### Scenario: Vertical video not downloaded
- **WHEN** a video not downloaded is shown with a 90×160 thumbnail
- **THEN** the thumbnail takes the same height as a downloaded vertical image

#### Scenario: Downloaded sticker
- **WHEN** a downloaded image smaller than the box is shown
- **THEN** it is shown at its real size, without enlarging

### Requirement: Larger default sizes
The default of `inline_image_max_height` SHALL be 12, and the thumbnail of a link preview SHALL use the same `inline_image_max_height` maximum.

#### Scenario: New config
- **WHEN** nchat starts without `ui.conf`
- **THEN** `inline_image_max_height=12`
