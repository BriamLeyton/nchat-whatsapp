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
  bool linkFetched = false; // link preview fetched from web page (also set if it failed)
};

struct LinkPreview
{
  std::string title;
  std::string description;
  std::string imageUrl; // absolute http(s) url, empty if not available
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

  // Request duration and size of a downloaded video / audio without known duration to be determined
  // in background (media_probe_command and file size), stored in the sidecar meta file (once per session).
  static void RequestProbe(const std::string& p_ProfileId, const std::string& p_MsgId, const std::string& p_FilePath);

  // Request link preview of a message to be fetched in background from the web page (link_preview_command
  // and link_thumbnail_command), stored in the sidecar files (once per message, once per url per session).
  static void RequestLinkPreview(const std::string& p_ProfileId, const std::string& p_MsgId, const std::string& p_Url);

  // Returns true if user id is a contact saved in address book (based on <profile>/saved_contacts).
  static bool IsSavedContact(const std::string& p_ProfileId, const std::string& p_UserId);

  // Returns first http(s) url in text, empty if none.
  static std::string GetFirstUrl(const std::string& p_Text);

  // Parse Open Graph / Twitter / html title and description, image url resolved against page url.
  static LinkPreview ParseLinkPreview(const std::string& p_Html, const std::string& p_PageUrl);

  // Returns true (once) if a poster frame was generated or a probe completed since last call. Must be
  // called from the ui thread, as it applies probe results to cached metadata.
  static bool TakePosterUpdated();

  static std::string FormatDuration(uint32_t p_Seconds); // ex: 1:15
  static std::string FormatSize(uint64_t p_Bytes); // ex: 4.2 MB

private:
  static void Worker();
  static void LinkWorker();
};
