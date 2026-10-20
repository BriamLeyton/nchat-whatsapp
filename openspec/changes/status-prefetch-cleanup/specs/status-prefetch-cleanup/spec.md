## Purpose

Have current statuses downloaded before they are viewed, without blocking message sends, and bound the disk used by old status videos.

## ADDED Requirements

### Requirement: Prefetch of current statuses
With `status_prefetch=1`, the system SHALL download in the background the attachment of each current status not yet downloaded that is loaded, whether received or read from the cache:
- images always;
- videos only if their known size does not exceed `status_prefetch_video_max_mb` MB.

The system MUST NOT prefetch expired statuses, nor videos of unknown size.

#### Scenario: A status photo arrives
- **WHEN** a status with a photo arrives
- **THEN** the photo is downloaded without opening the statuses chat

#### Scenario: Large video
- **WHEN** a status with a 40 MB video arrives and the limit is 16
- **THEN** it is not prefetched, and it is downloaded only if the user opens it

#### Scenario: Expired status in the cache
- **WHEN** a status from 3 days ago that is not downloaded is read from the cache
- **THEN** it is not prefetched

#### Scenario: Disabled
- **WHEN** `status_prefetch=0`
- **THEN** statuses are downloaded as today

### Requirement: One prefetch at a time
Prefetches SHALL be sent one at a time: the next one only after the previous one finishes, or after 120 s without a response. Before sending each one, the system SHALL skip it if it is already downloaded or has expired.

#### Scenario: Many statuses and a message
- **WHEN** 30 statuses are queued and the user sends a message
- **THEN** the message goes out after the download in progress, not after the 30

### Requirement: Status video retention
The default of `status_video_retention_days` SHALL be 0. With a value greater than 0, at startup the system SHALL delete in the background the video files of statuses posted more than that many days ago, and mark their attachment as not downloaded in the cache. It MUST keep images, frames, thumbnails and metadata. With value 0 it MUST NOT delete anything.

#### Scenario: Default
- **WHEN** nchat starts without changing `status_video_retention_days`
- **THEN** no file is deleted

#### Scenario: Video from 10 days ago
- **WHEN** nchat starts with retention 7 and a status video from 10 days ago is on disk
- **THEN** the video file is deleted, and the status is still seen with its frame or thumbnail without requesting a download

#### Scenario: Old image
- **WHEN** there is a status photo from 30 days ago
- **THEN** it is not deleted

#### Scenario: Open a cleaned video
- **WHEN** the user opens a status video that has already been cleaned up
- **THEN** the notice "Expired status, attachment not available." appears and nothing is downloaded
