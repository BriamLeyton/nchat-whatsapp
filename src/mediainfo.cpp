// mediainfo.cpp
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "mediainfo.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "fileutil.h"
#include "log.h"
#include "strutil.h"
#include "uiconfig.h"

namespace
{
  enum PosterState
  {
    PosterPending = 0,
    PosterReady,
    PosterFailed,
  };

  std::mutex s_Mutex;
  std::unordered_map<std::string, MediaMeta> s_Metas;
  std::unordered_map<std::string, PosterState> s_Posters;
  std::deque<std::string> s_PosterQueue;
  std::condition_variable s_PosterCond;
  std::thread s_PosterThread;
  bool s_PosterRunning = false;
  std::atomic<bool> s_PosterUpdated(false);

  std::string GetPosterPathFor(const std::string& p_VideoPath)
  {
    return FileUtil::RemoveFileExt(p_VideoPath) + ".poster.jpg";
  }

  std::string ShellQuoteContent(const std::string& p_Str)
  {
    // escape for use inside single quotes
    std::string str = p_Str;
    StrUtil::ReplaceString(str, "'", "'\\''");
    return str;
  }
}

void MediaInfo::Cleanup()
{
  {
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_PosterRunning = false;
    s_PosterQueue.clear();
  }

  s_PosterCond.notify_all();
  if (s_PosterThread.joinable())
  {
    s_PosterThread.join();
  }
}

const MediaMeta& MediaInfo::Get(const std::string& p_ProfileId, const std::string& p_MsgId)
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  const std::string key = p_ProfileId + "/" + p_MsgId;
  auto it = s_Metas.find(key);
  if (it != s_Metas.end()) return it->second;

  MediaMeta meta;
  const std::string basePath = FileUtil::GetApplicationDir() + "/profiles/" + p_ProfileId + "/tmp/" + p_MsgId;
  const std::string thumbPath = basePath + ".thumb.jpg";
  if (FileUtil::Exists(thumbPath))
  {
    meta.thumbPath = thumbPath;
  }

  const std::string metaPath = basePath + ".meta";
  if (FileUtil::Exists(metaPath))
  {
    const std::vector<std::string> lines = StrUtil::Split(FileUtil::ReadFile(metaPath), '\n');
    for (const auto& line : lines)
    {
      const size_t sep = line.find('=');
      if (sep == std::string::npos) continue;

      const std::string param = line.substr(0, sep);
      const std::string value = line.substr(sep + 1);
      if (param == "seconds")
      {
        meta.seconds = strtoul(value.c_str(), nullptr, 10);
      }
      else if (param == "file_length")
      {
        meta.fileLength = strtoull(value.c_str(), nullptr, 10);
      }
      else if (param == "ptt")
      {
        meta.isPtt = (value == "1");
      }
      else if (param == "title")
      {
        meta.title = value;
      }
      else if (param == "description")
      {
        meta.description = value;
      }
      else if (param == "background_argb")
      {
        meta.hasBackground = true;
        meta.backgroundArgb = strtoul(value.c_str(), nullptr, 10);
      }
      else if (param == "text_argb")
      {
        meta.hasTextColor = true;
        meta.textArgb = strtoul(value.c_str(), nullptr, 10);
      }
    }
  }

  return s_Metas.emplace(key, meta).first->second;
}

std::string MediaInfo::GetPosterPath(const std::string& p_VideoPath, bool p_Generate)
{
  static const std::string command = UiConfig::GetStr("video_thumbnail_command");

  std::lock_guard<std::mutex> lock(s_Mutex);
  const std::string posterPath = GetPosterPathFor(p_VideoPath);
  auto it = s_Posters.find(p_VideoPath);
  if (it != s_Posters.end())
  {
    return (it->second == PosterReady) ? posterPath : "";
  }

  if (FileUtil::Exists(posterPath))
  {
    s_Posters[p_VideoPath] = PosterReady;
    return posterPath;
  }

  if (!p_Generate || command.empty()) return "";

  s_Posters[p_VideoPath] = PosterPending;
  s_PosterQueue.push_back(p_VideoPath);
  if (!s_PosterThread.joinable())
  {
    s_PosterRunning = true;
    s_PosterThread = std::thread(&MediaInfo::PosterWorker);
  }

  s_PosterCond.notify_one();
  return "";
}

bool MediaInfo::TakePosterUpdated()
{
  return s_PosterUpdated.exchange(false);
}

std::string MediaInfo::FormatDuration(uint32_t p_Seconds)
{
  const uint32_t hours = p_Seconds / 3600;
  const uint32_t mins = (p_Seconds / 60) % 60;
  const uint32_t secs = p_Seconds % 60;
  char buf[32];
  if (hours > 0)
  {
    snprintf(buf, sizeof(buf), "%u:%02u:%02u", hours, mins, secs);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%u:%02u", mins, secs);
  }

  return std::string(buf);
}

std::string MediaInfo::FormatSize(uint64_t p_Bytes)
{
  char buf[32];
  if (p_Bytes >= (1024 * 1024))
  {
    snprintf(buf, sizeof(buf), "%.1f MB", (double)p_Bytes / (1024.0 * 1024.0));
  }
  else
  {
    snprintf(buf, sizeof(buf), "%llu KB", (unsigned long long)((p_Bytes + 1023) / 1024));
  }

  return std::string(buf);
}

void MediaInfo::PosterWorker()
{
  static const std::string command = UiConfig::GetStr("video_thumbnail_command");

  while (true)
  {
    std::string videoPath;
    {
      std::unique_lock<std::mutex> lock(s_Mutex);
      s_PosterCond.wait(lock, []() { return !s_PosterRunning || !s_PosterQueue.empty(); });
      if (!s_PosterRunning) return;

      videoPath = s_PosterQueue.front();
      s_PosterQueue.pop_front();
    }

    const std::string posterPath = GetPosterPathFor(videoPath);
    std::string cmd = command;
    StrUtil::ReplaceString(cmd, "%1", ShellQuoteContent(videoPath));
    StrUtil::ReplaceString(cmd, "%2", ShellQuoteContent(posterPath));
    LOG_DEBUG("generate poster: %s", cmd.c_str());
    const int rv = system(cmd.c_str());
    const bool ok = (rv == 0) && FileUtil::Exists(posterPath);
    if (!ok)
    {
      LOG_WARNING("generate poster failed (%d) for %s", rv, videoPath.c_str());
    }

    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      s_Posters[videoPath] = ok ? PosterReady : PosterFailed;
    }

    if (ok)
    {
      s_PosterUpdated = true;
    }
  }
}
