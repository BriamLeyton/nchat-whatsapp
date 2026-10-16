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
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sys/stat.h>

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

  // background task, either a poster frame (video path) or a probe (profile id, msg id, file path)
  struct Task
  {
    bool isProbe = false;
    std::string filePath;
    std::string profileId;
    std::string msgId;
  };

  std::mutex s_Mutex;
  std::unordered_map<std::string, MediaMeta> s_Metas; // only modified from ui thread
  std::unordered_map<std::string, PosterState> s_Posters;
  std::unordered_set<std::string> s_Probes;
  std::vector<std::tuple<std::string, uint32_t, uint64_t>> s_ProbeResults; // meta key, seconds, size
  std::deque<Task> s_PosterQueue;
  std::condition_variable s_PosterCond;
  std::thread s_PosterThread;
  bool s_PosterRunning = false;
  std::atomic<bool> s_PosterUpdated(false);

  std::string GetMetaKey(const std::string& p_ProfileId, const std::string& p_MsgId)
  {
    return p_ProfileId + "/" + p_MsgId;
  }

  std::string GetBasePath(const std::string& p_ProfileId, const std::string& p_MsgId)
  {
    return FileUtil::GetApplicationDir() + "/profiles/" + p_ProfileId + "/tmp/" + p_MsgId;
  }

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
  const std::string key = GetMetaKey(p_ProfileId, p_MsgId);
  auto it = s_Metas.find(key);
  if (it != s_Metas.end()) return it->second;

  MediaMeta meta;
  const std::string basePath = GetBasePath(p_ProfileId, p_MsgId);
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
  Task task;
  task.filePath = p_VideoPath;
  s_PosterQueue.push_back(task);
  if (!s_PosterThread.joinable())
  {
    s_PosterRunning = true;
    s_PosterThread = std::thread(&MediaInfo::Worker);
  }

  s_PosterCond.notify_one();
  return "";
}

void MediaInfo::RequestProbe(const std::string& p_ProfileId, const std::string& p_MsgId,
                             const std::string& p_FilePath)
{
  std::lock_guard<std::mutex> lock(s_Mutex);
  const std::string key = GetMetaKey(p_ProfileId, p_MsgId);
  if (!s_Probes.insert(key).second) return;

  Task task;
  task.isProbe = true;
  task.filePath = p_FilePath;
  task.profileId = p_ProfileId;
  task.msgId = p_MsgId;
  s_PosterQueue.push_back(task);
  if (!s_PosterThread.joinable())
  {
    s_PosterRunning = true;
    s_PosterThread = std::thread(&MediaInfo::Worker);
  }

  s_PosterCond.notify_one();
}

bool MediaInfo::TakePosterUpdated()
{
  std::vector<std::tuple<std::string, uint32_t, uint64_t>> results;
  {
    std::lock_guard<std::mutex> lock(s_Mutex);
    results.swap(s_ProbeResults);
    for (const auto& result : results)
    {
      auto it = s_Metas.find(std::get<0>(result));
      if (it == s_Metas.end()) continue;

      if (it->second.seconds == 0)
      {
        it->second.seconds = std::get<1>(result);
      }

      if (it->second.fileLength == 0)
      {
        it->second.fileLength = std::get<2>(result);
      }
    }
  }

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

void MediaInfo::Worker()
{
  static const std::string posterCommand = UiConfig::GetStr("video_thumbnail_command");
  static const std::string probeCommand = UiConfig::GetStr("media_probe_command");

  while (true)
  {
    Task task;
    {
      std::unique_lock<std::mutex> lock(s_Mutex);
      s_PosterCond.wait(lock, []() { return !s_PosterRunning || !s_PosterQueue.empty(); });
      if (!s_PosterRunning) return;

      task = s_PosterQueue.front();
      s_PosterQueue.pop_front();
    }

    if (!task.isProbe)
    {
      const std::string& videoPath = task.filePath;
      const std::string posterPath = GetPosterPathFor(videoPath);
      std::string cmd = posterCommand;
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
    else
    {
      uint32_t seconds = 0;
      if (!probeCommand.empty())
      {
        std::string cmd = probeCommand;
        StrUtil::ReplaceString(cmd, "%1", ShellQuoteContent(task.filePath));
        LOG_DEBUG("probe media: %s", cmd.c_str());
        std::string output;
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe != nullptr)
        {
          char buf[256];
          while (fgets(buf, sizeof(buf), pipe) != nullptr)
          {
            output += buf;
          }

          const int rv = pclose(pipe);
          const double duration = strtod(output.c_str(), nullptr);
          if ((rv == 0) && (duration > 0))
          {
            seconds = (uint32_t)(duration + 0.5);
          }
        }

        if (seconds == 0)
        {
          LOG_WARNING("probe media failed for %s", task.filePath.c_str());
        }
      }

      struct stat st;
      const uint64_t fileLength = (stat(task.filePath.c_str(), &st) == 0) ? (uint64_t)st.st_size : 0;
      if ((seconds > 0) || (fileLength > 0))
      {
        // add missing keys to existing sidecar meta file, or create it
        const std::string metaPath = GetBasePath(task.profileId, task.msgId) + ".meta";
        std::string data = FileUtil::Exists(metaPath) ? FileUtil::ReadFile(metaPath) : "";
        if (!data.empty() && (data.back() != '\n'))
        {
          data += "\n";
        }

        const bool hasSeconds = (("\n" + data).find("\nseconds=") != std::string::npos);
        const bool hasFileLength = (("\n" + data).find("\nfile_length=") != std::string::npos);
        if ((seconds > 0) && !hasSeconds)
        {
          data += "seconds=" + std::to_string(seconds) + "\n";
        }

        if ((fileLength > 0) && !hasFileLength)
        {
          data += "file_length=" + std::to_string(fileLength) + "\n";
        }

        FileUtil::MkDir(FileUtil::DirName(metaPath));
        FileUtil::WriteFile(metaPath, data);

        {
          std::lock_guard<std::mutex> lock(s_Mutex);
          s_ProbeResults.push_back(std::make_tuple(GetMetaKey(task.profileId, task.msgId), seconds, fileLength));
        }

        s_PosterUpdated = true;
      }
    }
  }
}
