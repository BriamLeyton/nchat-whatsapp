// mediainfo.cpp
//
// Copyright (c) 2026 Briam Leyton
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "mediainfo.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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

  // link preview fetching, separate thread as a fetch may take several seconds
  struct LinkTask
  {
    std::string profileId;
    std::string msgId;
    std::string url;
  };

  struct LinkResult
  {
    std::string key;
    std::string title;
    std::string description;
    std::string thumbPath;
  };

  struct LinkCacheEntry
  {
    std::string title;
    std::string description;
    std::string thumbPath; // thumbnail of first message with url, copied for other messages
  };

  std::unordered_set<std::string> s_LinkRequests;
  std::unordered_map<std::string, LinkCacheEntry> s_LinkUrls; // only accessed from link thread
  std::vector<LinkResult> s_LinkResults;
  std::deque<LinkTask> s_LinkQueue;
  std::condition_variable s_LinkCond;
  std::thread s_LinkThread;
  bool s_LinkRunning = false;
  pid_t s_LinkPid = 0; // process group of running link command, killed on cleanup

  std::mutex s_SavedMutex;
  std::unordered_map<std::string, std::pair<time_t, std::unordered_set<std::string>>> s_SavedContacts;
  std::unordered_map<std::string, time_t> s_SavedChecked;

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

  // add keys missing in existing sidecar meta file, or create it
  void AddMetaKeys(const std::string& p_MetaPath, const std::vector<std::pair<std::string, std::string>>& p_Keys)
  {
    std::string data = FileUtil::Exists(p_MetaPath) ? FileUtil::ReadFile(p_MetaPath) : "";
    if (!data.empty() && (data.back() != '\n'))
    {
      data += "\n";
    }

    for (const auto& key : p_Keys)
    {
      if (("\n" + data).find("\n" + key.first + "=") != std::string::npos) continue;

      data += key.first + "=" + key.second + "\n";
    }

    FileUtil::MkDir(FileUtil::DirName(p_MetaPath));
    FileUtil::WriteFile(p_MetaPath, data);
  }

  // run shell command in own process group (to allow kill on cleanup), returns exit status, -1 on error
  int RunLinkCommand(const std::string& p_Cmd, std::string& p_Output, size_t p_MaxOutput)
  {
    int fds[2];
    if (pipe(fds) != 0) return -1;

    const pid_t pid = fork();
    if (pid < 0)
    {
      close(fds[0]);
      close(fds[1]);
      return -1;
    }

    if (pid == 0)
    {
      setpgid(0, 0);
      dup2(fds[1], STDOUT_FILENO);
      close(fds[0]);
      close(fds[1]);
      execl("/bin/sh", "sh", "-c", p_Cmd.c_str(), (char*)nullptr);
      _exit(127);
    }

    setpgid(pid, pid);
    close(fds[1]);
    bool running = true;
    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      running = s_LinkRunning;
      s_LinkPid = pid;
    }

    if (!running)
    {
      kill(-pid, SIGKILL);
    }

    char buf[4096];
    ssize_t len = 0;
    while ((len = read(fds[0], buf, sizeof(buf))) > 0)
    {
      if (p_Output.size() < p_MaxOutput)
      {
        p_Output.append(buf, std::min((size_t)len, p_MaxOutput - p_Output.size()));
      }
    }

    close(fds[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      s_LinkPid = 0;
    }

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
  }

  std::string ToLower(const std::string& p_Str)
  {
    std::string str = p_Str;
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
  }

  std::string CodepointToUtf8(uint32_t p_Cp)
  {
    std::string str;
    if ((p_Cp == 0) || (p_Cp > 0x10ffff) || ((p_Cp >= 0xd800) && (p_Cp <= 0xdfff))) return str;

    if (p_Cp < 0x80)
    {
      str += (char)p_Cp;
    }
    else if (p_Cp < 0x800)
    {
      str += (char)(0xc0 | (p_Cp >> 6));
      str += (char)(0x80 | (p_Cp & 0x3f));
    }
    else if (p_Cp < 0x10000)
    {
      str += (char)(0xe0 | (p_Cp >> 12));
      str += (char)(0x80 | ((p_Cp >> 6) & 0x3f));
      str += (char)(0x80 | (p_Cp & 0x3f));
    }
    else
    {
      str += (char)(0xf0 | (p_Cp >> 18));
      str += (char)(0x80 | ((p_Cp >> 12) & 0x3f));
      str += (char)(0x80 | ((p_Cp >> 6) & 0x3f));
      str += (char)(0x80 | (p_Cp & 0x3f));
    }

    return str;
  }

  // decode html entities, collapse whitespace (incl. nbsp) and trim to max number of characters
  std::string CleanHtmlText(const std::string& p_Str, size_t p_MaxChars)
  {
    static const std::unordered_map<std::string, uint32_t> named =
    {
      { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' }, { "nbsp", ' ' },
      { "iexcl", 0xa1 }, { "cent", 0xa2 }, { "pound", 0xa3 }, { "curren", 0xa4 }, { "yen", 0xa5 },
      { "brvbar", 0xa6 }, { "sect", 0xa7 }, { "uml", 0xa8 }, { "copy", 0xa9 }, { "ordf", 0xaa }, { "laquo", 0xab },
      { "not", 0xac }, { "shy", 0xad }, { "reg", 0xae }, { "macr", 0xaf }, { "deg", 0xb0 }, { "plusmn", 0xb1 },
      { "sup2", 0xb2 }, { "sup3", 0xb3 }, { "acute", 0xb4 }, { "micro", 0xb5 }, { "para", 0xb6 }, { "middot", 0xb7 },
      { "cedil", 0xb8 }, { "sup1", 0xb9 }, { "ordm", 0xba }, { "raquo", 0xbb }, { "frac14", 0xbc },
      { "frac12", 0xbd }, { "frac34", 0xbe }, { "iquest", 0xbf }, { "Agrave", 0xc0 }, { "Aacute", 0xc1 },
      { "Acirc", 0xc2 }, { "Atilde", 0xc3 }, { "Auml", 0xc4 }, { "Aring", 0xc5 }, { "AElig", 0xc6 },
      { "Ccedil", 0xc7 }, { "Egrave", 0xc8 }, { "Eacute", 0xc9 }, { "Ecirc", 0xca }, { "Euml", 0xcb },
      { "Igrave", 0xcc }, { "Iacute", 0xcd }, { "Icirc", 0xce }, { "Iuml", 0xcf }, { "ETH", 0xd0 },
      { "Ntilde", 0xd1 }, { "Ograve", 0xd2 }, { "Oacute", 0xd3 }, { "Ocirc", 0xd4 }, { "Otilde", 0xd5 },
      { "Ouml", 0xd6 }, { "times", 0xd7 }, { "Oslash", 0xd8 }, { "Ugrave", 0xd9 }, { "Uacute", 0xda },
      { "Ucirc", 0xdb }, { "Uuml", 0xdc }, { "Yacute", 0xdd }, { "THORN", 0xde }, { "szlig", 0xdf },
      { "agrave", 0xe0 }, { "aacute", 0xe1 }, { "acirc", 0xe2 }, { "atilde", 0xe3 }, { "auml", 0xe4 },
      { "aring", 0xe5 }, { "aelig", 0xe6 }, { "ccedil", 0xe7 }, { "egrave", 0xe8 }, { "eacute", 0xe9 },
      { "ecirc", 0xea }, { "euml", 0xeb }, { "igrave", 0xec }, { "iacute", 0xed }, { "icirc", 0xee },
      { "iuml", 0xef }, { "eth", 0xf0 }, { "ntilde", 0xf1 }, { "ograve", 0xf2 }, { "oacute", 0xf3 },
      { "ocirc", 0xf4 }, { "otilde", 0xf5 }, { "ouml", 0xf6 }, { "divide", 0xf7 }, { "oslash", 0xf8 },
      { "ugrave", 0xf9 }, { "uacute", 0xfa }, { "ucirc", 0xfb }, { "uuml", 0xfc }, { "yacute", 0xfd },
      { "thorn", 0xfe }, { "yuml", 0xff }, { "ensp", 0x2002 }, { "emsp", 0x2003 }, { "thinsp", 0x2009 },
      { "ndash", 0x2013 }, { "mdash", 0x2014 }, { "lsquo", 0x2018 }, { "rsquo", 0x2019 }, { "sbquo", 0x201a },
      { "ldquo", 0x201c }, { "rdquo", 0x201d }, { "bdquo", 0x201e }, { "dagger", 0x2020 }, { "bull", 0x2022 },
      { "hellip", 0x2026 }, { "prime", 0x2032 }, { "euro", 0x20ac }, { "trade", 0x2122 }
    };

    std::string decoded;
    for (size_t i = 0; i < p_Str.size(); ++i)
    {
      const size_t end = (p_Str[i] == '&') ? p_Str.find(';', i) : std::string::npos;
      if ((end != std::string::npos) && ((end - i) <= 10))
      {
        const std::string entity = p_Str.substr(i + 1, end - i - 1);
        std::string replacement;
        bool found = false;
        if ((entity.size() > 1) && (entity[0] == '#'))
        {
          const bool isHex = ((entity[1] == 'x') || (entity[1] == 'X'));
          const std::string digits = entity.substr(isHex ? 2 : 1);
          char* endPtr = nullptr;
          const unsigned long cp = strtoul(digits.c_str(), &endPtr, isHex ? 16 : 10);
          if (!digits.empty() && (endPtr != nullptr) && (*endPtr == '\0'))
          {
            found = true;
            replacement = (cp == 0xa0) ? " " : CodepointToUtf8((uint32_t)cp);
          }
        }
        else
        {
          auto namedIt = named.find(entity);
          if (namedIt != named.end())
          {
            found = true;
            replacement = CodepointToUtf8(namedIt->second);
          }
        }

        if (found)
        {
          decoded += replacement;
          i = end;
          continue;
        }
      }

      decoded += p_Str[i];
    }

    StrUtil::ReplaceString(decoded, "\xc2\xa0", " ");

    std::string collapsed;
    for (char c : decoded)
    {
      if (std::isspace((unsigned char)c))
      {
        if (!collapsed.empty() && (collapsed.back() != ' '))
        {
          collapsed += ' ';
        }
      }
      else
      {
        collapsed += c;
      }
    }

    if (!collapsed.empty() && (collapsed.back() == ' '))
    {
      collapsed.pop_back();
    }

    std::wstring wstr = StrUtil::ToWString(collapsed);
    if (wstr.size() > p_MaxChars)
    {
      wstr = wstr.substr(0, p_MaxChars);
      return StrUtil::ToString(wstr);
    }

    return collapsed;
  }

  bool IsHttpUrl(const std::string& p_Url)
  {
    const std::string lower = ToLower(p_Url.substr(0, 8));
    return (lower.rfind("http://", 0) == 0) || (lower.rfind("https://", 0) == 0);
  }

  // resolve possibly relative url against page url, returns empty if not http(s)
  std::string ResolveUrl(const std::string& p_Url, const std::string& p_PageUrl)
  {
    if (p_Url.empty()) return "";

    if (IsHttpUrl(p_Url)) return p_Url;

    const size_t schemeEnd = p_PageUrl.find("://");
    if ((schemeEnd == std::string::npos) || !IsHttpUrl(p_PageUrl)) return "";

    const std::string scheme = p_PageUrl.substr(0, schemeEnd);
    if (p_Url.rfind("//", 0) == 0) return scheme + ":" + p_Url;

    if (p_Url.find("://") != std::string::npos) return ""; // other scheme

    const size_t hostEnd = p_PageUrl.find_first_of("/?#", schemeEnd + 3);
    const std::string origin = p_PageUrl.substr(0, hostEnd);
    if (p_Url[0] == '/') return origin + p_Url;

    std::string path = (hostEnd == std::string::npos) ? "/" : p_PageUrl.substr(hostEnd);
    path = path.substr(0, path.find_first_of("?#"));
    path = path.substr(0, path.rfind('/') + 1);
    if (path.empty())
    {
      path = "/";
    }

    return origin + path + p_Url;
  }

  // parse attributes of a html tag, keys lower case
  std::unordered_map<std::string, std::string> ParseTagAttributes(const std::string& p_Tag)
  {
    std::unordered_map<std::string, std::string> attrs;
    size_t i = 0;
    const size_t len = p_Tag.size();
    while (i < len)
    {
      while ((i < len) && (std::isspace((unsigned char)p_Tag[i]) || (p_Tag[i] == '/'))) ++i;

      const size_t nameStart = i;
      while ((i < len) && !std::isspace((unsigned char)p_Tag[i]) && (p_Tag[i] != '=') && (p_Tag[i] != '/')) ++i;

      const std::string name = ToLower(p_Tag.substr(nameStart, i - nameStart));
      while ((i < len) && std::isspace((unsigned char)p_Tag[i])) ++i;

      std::string value;
      if ((i < len) && (p_Tag[i] == '='))
      {
        ++i;
        while ((i < len) && std::isspace((unsigned char)p_Tag[i])) ++i;

        if ((i < len) && ((p_Tag[i] == '"') || (p_Tag[i] == '\'')))
        {
          const char quote = p_Tag[i++];
          const size_t valueEnd = p_Tag.find(quote, i);
          value = p_Tag.substr(i, (valueEnd == std::string::npos) ? std::string::npos : (valueEnd - i));
          i = (valueEnd == std::string::npos) ? len : (valueEnd + 1);
        }
        else
        {
          const size_t valueStart = i;
          while ((i < len) && !std::isspace((unsigned char)p_Tag[i])) ++i;

          value = p_Tag.substr(valueStart, i - valueStart);
        }
      }

      if (!name.empty() && (attrs.find(name) == attrs.end()))
      {
        attrs[name] = value;
      }

      if (i == nameStart) ++i; // safeguard against no progress
    }

    return attrs;
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

  {
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_LinkRunning = false;
    s_LinkQueue.clear();
    if (s_LinkPid > 0)
    {
      kill(-s_LinkPid, SIGKILL);
    }
  }

  s_LinkCond.notify_all();
  if (s_LinkThread.joinable())
  {
    s_LinkThread.join();
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
      else if (param == "link_fetched")
      {
        meta.linkFetched = (value == "1");
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

void MediaInfo::RequestLinkPreview(const std::string& p_ProfileId, const std::string& p_MsgId,
                                   const std::string& p_Url)
{
  static const std::string command = UiConfig::GetStr("link_preview_command");
  if (command.empty() || !IsHttpUrl(p_Url)) return;

  std::lock_guard<std::mutex> lock(s_Mutex);
  const std::string key = GetMetaKey(p_ProfileId, p_MsgId);
  if (!s_LinkRequests.insert(key).second) return;

  LOG_DEBUG("request link preview %s %s", p_MsgId.c_str(), p_Url.c_str());
  LinkTask task;
  task.profileId = p_ProfileId;
  task.msgId = p_MsgId;
  task.url = p_Url;
  s_LinkQueue.push_back(task);
  if (!s_LinkThread.joinable())
  {
    s_LinkRunning = true;
    s_LinkThread = std::thread(&MediaInfo::LinkWorker);
  }

  s_LinkCond.notify_one();
}

bool MediaInfo::IsSavedContact(const std::string& p_ProfileId, const std::string& p_UserId)
{
  std::lock_guard<std::mutex> lock(s_SavedMutex);
  auto& saved = s_SavedContacts[p_ProfileId];
  const time_t now = time(nullptr);
  time_t& checked = s_SavedChecked[p_ProfileId];
  if ((now - checked) >= 5)
  {
    // reload if file changed, checked at most every 5 sec
    checked = now;
    const std::string path = FileUtil::GetApplicationDir() + "/profiles/" + p_ProfileId + "/saved_contacts";
    struct stat st;
    const time_t mtime = (stat(path.c_str(), &st) == 0) ? st.st_mtime : 0;
    if (mtime != saved.first)
    {
      saved.first = mtime;
      saved.second.clear();
      if (mtime != 0)
      {
        for (const auto& line : StrUtil::Split(FileUtil::ReadFile(path), '\n'))
        {
          if (!line.empty())
          {
            saved.second.insert(line);
          }
        }
      }

      LOG_DEBUG("saved contacts loaded %zu", saved.second.size());
    }
  }

  return saved.second.count(p_UserId) > 0;
}

std::string MediaInfo::GetFirstUrl(const std::string& p_Text)
{
  const std::string lower = ToLower(p_Text);
  size_t pos = 0;
  while ((pos = lower.find("http", pos)) != std::string::npos)
  {
    if (((pos == 0) || !std::isalnum((unsigned char)p_Text[pos - 1])) && IsHttpUrl(p_Text.substr(pos)))
    {
      size_t end = pos;
      while ((end < p_Text.size()) && !std::isspace((unsigned char)p_Text[end])) ++end;

      std::string url = p_Text.substr(pos, end - pos);
      static const std::string trailing = ".,;:!?)]}>\"'";
      while (!url.empty() && (trailing.find(url.back()) != std::string::npos))
      {
        url.pop_back();
      }

      if (url.find("://") + 3 < url.size()) return url;
    }

    pos += 4;
  }

  return "";
}

LinkPreview MediaInfo::ParseLinkPreview(const std::string& p_Html, const std::string& p_PageUrl)
{
  static const size_t maxTitle = 200;
  static const size_t maxDescription = 500;

  // meta values by priority, lower index is preferred
  std::string titles[3];
  std::string descriptions[3];
  std::string images[2];

  const std::string lower = ToLower(p_Html);
  size_t pos = 0;
  while ((pos = lower.find("<meta", pos)) != std::string::npos)
  {
    const size_t end = lower.find('>', pos);
    if (end == std::string::npos) break;

    const auto attrs = ParseTagAttributes(p_Html.substr(pos + 5, end - pos - 5));
    pos = end;

    auto contentIt = attrs.find("content");
    if (contentIt == attrs.end()) continue;

    auto propIt = attrs.find("property");
    if ((propIt == attrs.end()) || propIt->second.empty())
    {
      propIt = attrs.find("name");
    }

    if (propIt == attrs.end()) continue;

    const std::string prop = ToLower(propIt->second);
    const std::string& content = contentIt->second;
    auto setFirst = [&](std::string& p_Dst)
    {
      if (p_Dst.empty())
      {
        p_Dst = content;
      }
    };

    if (prop == "og:title") setFirst(titles[0]);
    else if (prop == "twitter:title") setFirst(titles[1]);
    else if (prop == "og:description") setFirst(descriptions[0]);
    else if (prop == "twitter:description") setFirst(descriptions[1]);
    else if (prop == "description") setFirst(descriptions[2]);
    else if ((prop == "og:image") || (prop == "og:image:url") || (prop == "og:image:secure_url")) setFirst(images[0]);
    else if ((prop == "twitter:image") || (prop == "twitter:image:src")) setFirst(images[1]);
  }

  const size_t titleStart = lower.find("<title");
  if (titleStart != std::string::npos)
  {
    const size_t textStart = lower.find('>', titleStart);
    const size_t textEnd = (textStart != std::string::npos) ? lower.find("</title", textStart) : std::string::npos;
    if (textEnd != std::string::npos)
    {
      titles[2] = p_Html.substr(textStart + 1, textEnd - textStart - 1);
    }
  }

  LinkPreview preview;
  for (const auto& title : titles)
  {
    preview.title = CleanHtmlText(title, maxTitle);
    if (!preview.title.empty()) break;
  }

  for (const auto& description : descriptions)
  {
    preview.description = CleanHtmlText(description, maxDescription);
    if (!preview.description.empty()) break;
  }

  for (const auto& image : images)
  {
    preview.imageUrl = ResolveUrl(CleanHtmlText(image, 2048), p_PageUrl);
    if (!preview.imageUrl.empty()) break;
  }

  return preview;
}

bool MediaInfo::TakePosterUpdated()
{
  std::vector<std::tuple<std::string, uint32_t, uint64_t>> results;
  std::vector<LinkResult> linkResults;
  {
    std::lock_guard<std::mutex> lock(s_Mutex);
    linkResults.swap(s_LinkResults);
    for (const auto& linkResult : linkResults)
    {
      auto it = s_Metas.find(linkResult.key);
      if (it == s_Metas.end()) continue;

      it->second.linkFetched = true;
      if (it->second.title.empty() && it->second.description.empty())
      {
        it->second.title = linkResult.title;
        it->second.description = linkResult.description;
      }

      if (it->second.thumbPath.empty())
      {
        it->second.thumbPath = linkResult.thumbPath;
      }
    }

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
        std::vector<std::pair<std::string, std::string>> keys;
        if (seconds > 0)
        {
          keys.push_back(std::make_pair("seconds", std::to_string(seconds)));
        }

        if (fileLength > 0)
        {
          keys.push_back(std::make_pair("file_length", std::to_string(fileLength)));
        }

        AddMetaKeys(GetBasePath(task.profileId, task.msgId) + ".meta", keys);

        {
          std::lock_guard<std::mutex> lock(s_Mutex);
          s_ProbeResults.push_back(std::make_tuple(GetMetaKey(task.profileId, task.msgId), seconds, fileLength));
        }

        s_PosterUpdated = true;
      }
    }
  }
}

void MediaInfo::LinkWorker()
{
  static const std::string pageCommand = UiConfig::GetStr("link_preview_command");
  static const std::string thumbCommand = UiConfig::GetStr("link_thumbnail_command");
  static const size_t maxPageSize = 1024 * 1024;

  while (true)
  {
    LinkTask task;
    {
      std::unique_lock<std::mutex> lock(s_Mutex);
      s_LinkCond.wait(lock, []() { return !s_LinkRunning || !s_LinkQueue.empty(); });
      if (!s_LinkRunning) return;

      task = s_LinkQueue.front();
      s_LinkQueue.pop_front();
    }

    const std::string basePath = GetBasePath(task.profileId, task.msgId);
    const std::string thumbPath = basePath + ".thumb.jpg";
    LinkCacheEntry entry;
    auto cacheIt = s_LinkUrls.find(task.url);
    if (cacheIt != s_LinkUrls.end())
    {
      // url already fetched this session, reuse result without network
      entry = cacheIt->second;
      if (!entry.thumbPath.empty() && (entry.thumbPath != thumbPath) && !FileUtil::Exists(thumbPath))
      {
        FileUtil::CopyFile(entry.thumbPath, thumbPath);
      }
    }
    else
    {
      std::string cmd = pageCommand;
      StrUtil::ReplaceString(cmd, "%1", ShellQuoteContent(task.url));
      LOG_DEBUG("fetch link preview: %s", task.url.c_str());
      std::string html;
      const int rv = RunLinkCommand(cmd, html, maxPageSize);
      const LinkPreview preview = ParseLinkPreview(html, task.url);
      entry.title = preview.title;
      entry.description = preview.description;
      LOG_DEBUG("fetch link preview rv %d size %zu title \"%s\" image \"%s\"", rv, html.size(),
                preview.title.c_str(), preview.imageUrl.c_str());

      {
        std::lock_guard<std::mutex> lock(s_Mutex);
        if (!s_LinkRunning) return;
      }

      if (rv != 0)
      {
        // network error, not marked as fetched to allow retry next session
        LOG_WARNING("fetch link preview failed (%d) for %s", rv, task.url.c_str());
        continue;
      }

      if (!preview.imageUrl.empty() && !thumbCommand.empty() && !FileUtil::Exists(thumbPath))
      {
        FileUtil::MkDir(FileUtil::DirName(thumbPath));
        std::string thumbCmd = thumbCommand;
        StrUtil::ReplaceString(thumbCmd, "%1", ShellQuoteContent(preview.imageUrl));
        StrUtil::ReplaceString(thumbCmd, "%2", ShellQuoteContent(thumbPath));
        std::string output;
        const int thumbRv = RunLinkCommand(thumbCmd, output, 0);
        if ((thumbRv != 0) || !FileUtil::Exists(thumbPath))
        {
          LOG_WARNING("link thumbnail failed (%d) for %s", thumbRv, preview.imageUrl.c_str());
          FileUtil::RmFile(thumbPath);
        }
      }

      if (FileUtil::Exists(thumbPath))
      {
        entry.thumbPath = thumbPath;
      }

      s_LinkUrls[task.url] = entry;
    }

    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      if (!s_LinkRunning) return;
    }

    std::vector<std::pair<std::string, std::string>> keys;
    if (!entry.title.empty())
    {
      keys.push_back(std::make_pair("title", entry.title));
    }

    if (!entry.description.empty())
    {
      keys.push_back(std::make_pair("description", entry.description));
    }

    keys.push_back(std::make_pair("link_fetched", "1"));
    AddMetaKeys(basePath + ".meta", keys);

    {
      std::lock_guard<std::mutex> lock(s_Mutex);
      LinkResult result;
      result.key = GetMetaKey(task.profileId, task.msgId);
      result.title = entry.title;
      result.description = entry.description;
      result.thumbPath = FileUtil::Exists(thumbPath) ? thumbPath : "";
      s_LinkResults.push_back(result);
    }

    s_PosterUpdated = true;
  }
}
