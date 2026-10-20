// uiimage.h
//
// Copyright (c) 2026 Briam Leyton
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Inline image rendering using the kitty graphics protocol (supported by kitty, Ghostty, etc).
// Images are transmitted once per file and size, and then placed by id after each ncurses refresh.
struct UiImageInfo
{
  uint32_t id = 0;
  int cols = 0;   // terminal columns covered
  int rows = 0;   // terminal rows covered
  int pxW = 0;    // transmitted pixel width
  int pxH = 0;    // transmitted pixel height
  int cellH = 0;  // cell pixel height used when sizing
};

struct UiImagePlacement
{
  UiImageInfo info;
  int screenY = 0;  // 0-based screen row of first visible image row
  int screenX = 0;  // 0-based screen column
  int skipRows = 0; // image rows clipped at the top
  int visibleRows = 0;

  bool operator==(const UiImagePlacement& p_Other) const
  {
    return (info.id == p_Other.info.id) && (screenY == p_Other.screenY) &&
           (screenX == p_Other.screenX) && (skipRows == p_Other.skipRows) &&
           (visibleRows == p_Other.visibleRows);
  }
};

class UiImage
{
public:
  // Must be called before initscr(), as it queries the terminal directly.
  static void Init();
  // Must be called before endwin(), deletes all transmitted images.
  static void Cleanup();

  static bool IsEnabled();
  static bool IsImageFile(const std::string& p_Path);
  static bool IsVideoFile(const std::string& p_Path);
  static bool IsAudioFile(const std::string& p_Path);

  // Decode, scale and transmit image (once), returns false if it cannot be rendered.
  // Small images are only enlarged to fill the box when p_Upscale is set (embedded thumbnails).
  static bool Prepare(const std::string& p_Path, int p_MaxCols, int p_MaxRows, UiImageInfo& p_Info,
                      bool p_Upscale = false);

  // Remove all visible placements (keeps transmitted image data).
  static void ClearPlacements();
  // Replace currently visible placements with the specified ones (no-op if unchanged).
  static void PlaceAll(const std::vector<UiImagePlacement>& p_Placements);

  // Hide placements while a dialog covers the screen, restore last requested ones on resume.
  static void Suspend();
  static void Resume();

  // Terminal discarded all image data (full screen clear or alternate screen re-entry),
  // images are re-transmitted from memory when next placed.
  static void Invalidate();

private:
  static bool DetectSupport();
  static void GetCellSize(int& p_CellW, int& p_CellH);
  static bool Decode(const std::string& p_Path, std::vector<uint8_t>& p_Rgba, int& p_W, int& p_H);
  static void Transmit(uint32_t p_Id, const std::vector<uint8_t>& p_Rgba, int p_W, int p_H);
  static void Delete(uint32_t p_Id);
  static void WriteOut(const std::string& p_Data);
  static void EmitPlacements(const std::vector<UiImagePlacement>& p_Placements);
  static void EnsureTransmitted(uint32_t p_Id);

private:
  struct CacheEntry
  {
    bool ok = false;
    bool transmitted = false;
    UiImageInfo info;
    std::vector<uint8_t> rgba; // kept for re-transmission after terminal discards image data
  };

  static std::mutex s_Mutex;
  static bool s_Enabled;
  static int s_FallbackCellW;
  static int s_FallbackCellH;
  static uint32_t s_NextId;
  static std::unordered_map<std::string, CacheEntry> s_Cache;
  static std::list<std::string> s_CacheLru; // most recently used first
  static std::unordered_map<uint32_t, std::string> s_IdKeys;
  static std::vector<UiImagePlacement> s_Placements; // currently visible
  static std::vector<UiImagePlacement> s_Requested;  // last requested by history view
  static int s_SuspendCount;
};
