// uiimage.cpp
//
// Copyright (c) 2019-2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uiimage.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iterator>

#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <webp/decode.h>
#include <webp/demux.h>

#include "stb_image.h"
#include "stb_image_resize2.h"

#include "log.h"
#include "strutil.h"
#include "uiconfig.h"

static const size_t s_MaxCacheEntries = 64;
static const size_t s_ChunkSize = 4096;
static const int s_DetectTimeoutMs = 200;

std::mutex UiImage::s_Mutex;
bool UiImage::s_Enabled = false;
int UiImage::s_FallbackCellW = 8;
int UiImage::s_FallbackCellH = 16;
uint32_t UiImage::s_NextId = 0;
std::unordered_map<std::string, UiImage::CacheEntry> UiImage::s_Cache;
std::list<std::string> UiImage::s_CacheLru;
std::unordered_map<uint32_t, std::string> UiImage::s_IdKeys;
std::vector<UiImagePlacement> UiImage::s_Placements;
std::vector<UiImagePlacement> UiImage::s_Requested;
int UiImage::s_SuspendCount = 0;

static std::string Base64Encode(const uint8_t* p_Data, size_t p_Len)
{
  static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((p_Len + 2) / 3) * 4);
  size_t i = 0;
  for (; i + 2 < p_Len; i += 3)
  {
    uint32_t n = (p_Data[i] << 16) | (p_Data[i + 1] << 8) | p_Data[i + 2];
    out += table[(n >> 18) & 63];
    out += table[(n >> 12) & 63];
    out += table[(n >> 6) & 63];
    out += table[n & 63];
  }

  if (i < p_Len)
  {
    uint32_t n = p_Data[i] << 16;
    if ((i + 1) < p_Len) n |= (p_Data[i + 1] << 8);
    out += table[(n >> 18) & 63];
    out += table[(n >> 12) & 63];
    out += ((i + 1) < p_Len) ? table[(n >> 6) & 63] : '=';
    out += '=';
  }

  return out;
}

void UiImage::Init()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  s_Enabled = false;

  if (!UiConfig::GetBool("inline_images_enabled"))
  {
    LOG_INFO("inline images disabled by config");
    return;
  }

  if (getenv("TMUX") != nullptr)
  {
    LOG_INFO("inline images disabled under tmux");
    return;
  }

  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
  {
    LOG_INFO("inline images disabled, not a tty");
    return;
  }

  // ids are global per terminal screen, offset by pid to reduce clashes with other programs
  s_NextId = 0x4e430000 + ((static_cast<uint32_t>(getpid()) & 0xff) << 8);
  s_Enabled = DetectSupport();
  LOG_INFO("inline images supported=%d cell=%dx%d", s_Enabled, s_FallbackCellW, s_FallbackCellH);
}

bool UiImage::DetectSupport()
{
  struct termios origTermios;
  if (tcgetattr(STDIN_FILENO, &origTermios) != 0) return false;

  struct termios rawTermios = origTermios;
  rawTermios.c_lflag &= ~(ICANON | ECHO);
  rawTermios.c_cc[VMIN] = 0;
  rawTermios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &rawTermios);

  // graphics query, cell size query, primary device attributes (answered by all terminals)
  WriteOut("\033_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA\033\\\033[16t\033[c");

  std::string response;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(s_DetectTimeoutMs);
  while (true)
  {
    size_t daPos = response.find("\033[?");
    if ((daPos != std::string::npos) && (response.find('c', daPos) != std::string::npos)) break;

    int remainMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                      deadline - std::chrono::steady_clock::now()).count());
    if (remainMs <= 0) break;

    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    if (poll(&pfd, 1, remainMs) <= 0) break;

    char buf[256];
    ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
    if (len <= 0) break;

    response.append(buf, len);
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &origTermios);

  size_t sizePos = response.find("\033[6;");
  if (sizePos != std::string::npos)
  {
    int cellH = 0;
    int cellW = 0;
    if ((sscanf(response.c_str() + sizePos, "\033[6;%d;%dt", &cellH, &cellW) == 2) &&
        (cellH > 0) && (cellW > 0))
    {
      s_FallbackCellW = cellW;
      s_FallbackCellH = cellH;
    }
  }

  return (response.find("\033_Gi=31;OK") != std::string::npos);
}

void UiImage::Cleanup()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled) return;

  // delete all placements and free image data
  WriteOut("\033_Ga=d,d=A,q=2\033\\");
  s_Placements.clear();
  s_Requested.clear();
  s_Cache.clear();
  s_CacheLru.clear();
  s_IdKeys.clear();
}

bool UiImage::IsEnabled()
{
  return s_Enabled;
}

bool UiImage::IsImageFile(const std::string& p_Path)
{
  size_t dotPos = p_Path.find_last_of('.');
  if (dotPos == std::string::npos) return false;

  std::string ext = StrUtil::ToLower(p_Path.substr(dotPos + 1));
  return (ext == "jpg") || (ext == "jpeg") || (ext == "png") || (ext == "webp") || (ext == "gif");
}

void UiImage::GetCellSize(int& p_CellW, int& p_CellH)
{
  struct winsize ws;
  if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) && (ws.ws_col > 0) && (ws.ws_row > 0) &&
      (ws.ws_xpixel > 0) && (ws.ws_ypixel > 0))
  {
    p_CellW = ws.ws_xpixel / ws.ws_col;
    p_CellH = ws.ws_ypixel / ws.ws_row;
    if ((p_CellW > 0) && (p_CellH > 0)) return;
  }

  p_CellW = s_FallbackCellW;
  p_CellH = s_FallbackCellH;
}

bool UiImage::Decode(const std::string& p_Path, std::vector<uint8_t>& p_Rgba, int& p_W, int& p_H)
{
  std::ifstream file(p_Path, std::ios::binary);
  if (!file) return false;

  std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  if (data.size() < 12) return false;

  const bool isWebp = (memcmp(data.data(), "RIFF", 4) == 0) && (memcmp(data.data() + 8, "WEBP", 4) == 0);
  if (isWebp)
  {
    int w = 0;
    int h = 0;
    uint8_t* pixels = WebPDecodeRGBA(data.data(), data.size(), &w, &h);
    if (pixels != nullptr)
    {
      p_Rgba.assign(pixels, pixels + (static_cast<size_t>(w) * h * 4));
      WebPFree(pixels);
      p_W = w;
      p_H = h;
      return true;
    }

    // animated webp (common for stickers), use first frame
    WebPData webpData = { data.data(), data.size() };
    WebPAnimDecoderOptions options;
    if (!WebPAnimDecoderOptionsInit(&options)) return false;

    options.color_mode = MODE_RGBA;
    WebPAnimDecoder* decoder = WebPAnimDecoderNew(&webpData, &options);
    if (decoder == nullptr) return false;

    bool rv = false;
    WebPAnimInfo animInfo;
    uint8_t* frame = nullptr;
    int timestamp = 0;
    if (WebPAnimDecoderGetInfo(decoder, &animInfo) && WebPAnimDecoderGetNext(decoder, &frame, &timestamp))
    {
      p_W = static_cast<int>(animInfo.canvas_width);
      p_H = static_cast<int>(animInfo.canvas_height);
      p_Rgba.assign(frame, frame + (static_cast<size_t>(p_W) * p_H * 4));
      rv = true;
    }

    WebPAnimDecoderDelete(decoder);
    return rv;
  }

  int w = 0;
  int h = 0;
  int channels = 0;
  uint8_t* pixels = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &w, &h, &channels, 4);
  if (pixels == nullptr) return false;

  p_Rgba.assign(pixels, pixels + (static_cast<size_t>(w) * h * 4));
  stbi_image_free(pixels);
  p_W = w;
  p_H = h;
  return true;
}

bool UiImage::Prepare(const std::string& p_Path, int p_MaxCols, int p_MaxRows, UiImageInfo& p_Info)
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled || (p_MaxCols <= 0) || (p_MaxRows <= 0)) return false;

  int cellW = 0;
  int cellH = 0;
  GetCellSize(cellW, cellH);

  const std::string key = p_Path + "|" + std::to_string(p_MaxCols) + "x" + std::to_string(p_MaxRows) +
    "|" + std::to_string(cellW) + "x" + std::to_string(cellH);
  auto cacheIt = s_Cache.find(key);
  if (cacheIt != s_Cache.end())
  {
    s_CacheLru.remove(key);
    s_CacheLru.push_front(key);
    if (cacheIt->second.ok && !cacheIt->second.transmitted)
    {
      EnsureTransmitted(cacheIt->second.info.id);
    }

    p_Info = cacheIt->second.info;
    return cacheIt->second.ok;
  }

  CacheEntry entry;
  std::vector<uint8_t> rgba;
  int imgW = 0;
  int imgH = 0;
  if (!Decode(p_Path, rgba, imgW, imgH) || (imgW <= 0) || (imgH <= 0))
  {
    LOG_WARNING("inline image decode failed %s", p_Path.c_str());
  }
  else
  {
    // fit within max box, preserving aspect ratio and never upscaling
    const double maxW = static_cast<double>(p_MaxCols * cellW);
    const double maxH = static_cast<double>(p_MaxRows * cellH);
    const double scale = std::min({ 1.0, maxW / imgW, maxH / imgH });
    const int dstW = std::max(1, static_cast<int>(std::floor(imgW * scale)));
    const int dstH = std::max(1, static_cast<int>(std::floor(imgH * scale)));

    std::vector<uint8_t> scaled;
    if ((dstW != imgW) || (dstH != imgH))
    {
      scaled.resize(static_cast<size_t>(dstW) * dstH * 4);
      if (stbir_resize_uint8_srgb(rgba.data(), imgW, imgH, 0, scaled.data(), dstW, dstH, 0,
                                  STBIR_RGBA) == nullptr)
      {
        LOG_WARNING("inline image resize failed %s", p_Path.c_str());
        scaled.clear();
      }
    }
    else
    {
      scaled.swap(rgba);
    }

    if (!scaled.empty())
    {
      entry.ok = true;
      entry.info.id = s_NextId++;
      entry.info.pxW = dstW;
      entry.info.pxH = dstH;
      entry.info.cellH = cellH;
      entry.info.cols = (dstW + cellW - 1) / cellW;
      entry.info.rows = (dstH + cellH - 1) / cellH;
      Transmit(entry.info.id, scaled, dstW, dstH);
      entry.transmitted = true;
      entry.rgba.swap(scaled);
      LOG_DEBUG("inline image %s id %u %dx%d px %dx%d cells", p_Path.c_str(), entry.info.id,
                dstW, dstH, entry.info.cols, entry.info.rows);
    }
  }

  // evict least recently used, freeing terminal memory
  while (s_CacheLru.size() >= s_MaxCacheEntries)
  {
    const std::string& oldKey = s_CacheLru.back();
    auto oldIt = s_Cache.find(oldKey);
    if (oldIt != s_Cache.end())
    {
      if (oldIt->second.ok)
      {
        Delete(oldIt->second.info.id);
        s_IdKeys.erase(oldIt->second.info.id);
      }

      s_Cache.erase(oldIt);
    }

    s_CacheLru.pop_back();
  }

  if (entry.ok)
  {
    s_IdKeys[entry.info.id] = key;
  }

  s_Cache[key] = std::move(entry);
  s_CacheLru.push_front(key);
  p_Info = s_Cache[key].info;
  return s_Cache[key].ok;
}

void UiImage::Transmit(uint32_t p_Id, const std::vector<uint8_t>& p_Rgba, int p_W, int p_H)
{
  const std::string encoded = Base64Encode(p_Rgba.data(), p_Rgba.size());
  std::string out;
  out.reserve(encoded.size() + ((encoded.size() / s_ChunkSize) + 1) * 64);
  for (size_t pos = 0; pos < encoded.size(); pos += s_ChunkSize)
  {
    const bool isFirst = (pos == 0);
    const bool isLast = ((pos + s_ChunkSize) >= encoded.size());
    out += "\033_G";
    if (isFirst)
    {
      out += "a=t,t=d,f=32,q=2,i=" + std::to_string(p_Id) + ",s=" + std::to_string(p_W) +
        ",v=" + std::to_string(p_H) + ",";
    }

    out += std::string("m=") + (isLast ? "0" : "1") + ";";
    out += encoded.substr(pos, s_ChunkSize);
    out += "\033\\";
  }

  WriteOut(out);
}

void UiImage::Delete(uint32_t p_Id)
{
  WriteOut("\033_Ga=d,d=I,q=2,i=" + std::to_string(p_Id) + "\033\\");
  s_Placements.erase(std::remove_if(s_Placements.begin(), s_Placements.end(),
                                    [&](const UiImagePlacement& p_Placement)
                                    {
                                      return p_Placement.info.id == p_Id;
                                    }), s_Placements.end());
}

void UiImage::ClearPlacements()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled) return;

  WriteOut("\033_Ga=d,d=a,q=2\033\\");
  s_Placements.clear();
}

void UiImage::PlaceAll(const std::vector<UiImagePlacement>& p_Placements)
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled) return;

  s_Requested = p_Placements;
  if (s_SuspendCount > 0) return;

  if (p_Placements == s_Placements) return;

  EmitPlacements(p_Placements);
}

void UiImage::Suspend()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled) return;

  if (s_SuspendCount++ == 0)
  {
    WriteOut("\033_Ga=d,d=a,q=2\033\\");
    s_Placements.clear();
  }
}

void UiImage::Resume()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled || (s_SuspendCount == 0)) return;

  if ((--s_SuspendCount == 0) && !s_Requested.empty())
  {
    EmitPlacements(s_Requested);
  }
}

void UiImage::Invalidate()
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  if (!s_Enabled) return;

  for (auto& cacheEntry : s_Cache)
  {
    cacheEntry.second.transmitted = false;
  }

  s_Placements.clear();
  LOG_DEBUG("inline images invalidated");
}

void UiImage::EnsureTransmitted(uint32_t p_Id)
{
  auto keyIt = s_IdKeys.find(p_Id);
  if (keyIt == s_IdKeys.end()) return;

  auto cacheIt = s_Cache.find(keyIt->second);
  if ((cacheIt == s_Cache.end()) || !cacheIt->second.ok || cacheIt->second.transmitted) return;

  CacheEntry& entry = cacheIt->second;
  Transmit(entry.info.id, entry.rgba, entry.info.pxW, entry.info.pxH);
  entry.transmitted = true;
  LOG_DEBUG("inline image id %u re-transmitted", entry.info.id);
}

void UiImage::EmitPlacements(const std::vector<UiImagePlacement>& p_Placements)
{
  for (const auto& placement : p_Placements)
  {
    EnsureTransmitted(placement.info.id);
  }

  std::string out = "\0337"; // save cursor
  out += "\033_Ga=d,d=a,q=2\033\\";
  uint32_t placementId = 1;
  for (const auto& placement : p_Placements)
  {
    const int clipY = placement.skipRows * placement.info.cellH;
    const int clipH = std::min(placement.visibleRows * placement.info.cellH, placement.info.pxH - clipY);
    if (clipH <= 0) continue;

    out += "\033[" + std::to_string(placement.screenY + 1) + ";" + std::to_string(placement.screenX + 1) + "H";
    out += "\033_Ga=p,C=1,q=2,i=" + std::to_string(placement.info.id) + ",p=" + std::to_string(placementId++);
    if (placement.skipRows > 0)
    {
      out += ",y=" + std::to_string(clipY) + ",h=" + std::to_string(clipH);
    }

    out += "\033\\";
  }

  out += "\0338"; // restore cursor
  WriteOut(out);
  s_Placements = p_Placements;
}

void UiImage::WriteOut(const std::string& p_Data)
{
  fflush(stdout);
  const char* data = p_Data.data();
  size_t remain = p_Data.size();
  while (remain > 0)
  {
    ssize_t written = write(STDOUT_FILENO, data, remain);
    if (written < 0)
    {
      if (errno == EINTR || errno == EAGAIN) continue;

      LOG_WARNING("inline image write failed %d", errno);
      return;
    }

    data += written;
    remain -= written;
  }
}
