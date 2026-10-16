// mediainfo.h
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <cstdint>
#include <string>

// Media metadata from sidecar files written by the protocol on message receipt, based on data embedded
// in the message (no network): <profile>/tmp/<msgId>.thumb.jpg and <profile>/tmp/<msgId>.meta
struct MediaMeta
{
  std::string thumbPath; // empty if not available
  uint32_t seconds = 0;
  uint64_t fileLength = 0;
  bool isPtt = false;
  std::string title;
  std::string description;
  bool hasBackground = false;
  uint32_t backgroundArgb = 0;
  bool hasTextColor = false;
  uint32_t textArgb = 0;
};

class MediaInfo
{
public:
  static void Cleanup();

  // Get (cached) sidecar metadata for a message.
  static const MediaMeta& Get(const std::string& p_ProfileId, const std::string& p_MsgId);

  // Get poster frame path of a downloaded video, empty if not (yet) available. If p_Generate is set,
  // a missing poster is generated in background using video_thumbnail_command (once per session).
  static std::string GetPosterPath(const std::string& p_VideoPath, bool p_Generate);

  // Returns true (once) if a poster frame was generated since last call.
  static bool TakePosterUpdated();

  static std::string FormatDuration(uint32_t p_Seconds); // ex: 1:15
  static std::string FormatSize(uint64_t p_Bytes); // ex: 4.2 MB

private:
  static void PosterWorker();
};
