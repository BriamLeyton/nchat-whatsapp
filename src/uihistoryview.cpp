// uihistoryview.cpp
//
// Copyright (c) 2019-2025 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uihistoryview.h"

#include <map>

#include "appconfig.h"
#include "apputil.h"
#include "fileutil.h"
#include "log.h"
#include "mediainfo.h"
#include "protocolutil.h"
#include "strutil.h"
#include "timeutil.h"
#include "uicolorconfig.h"
#include "uiconfig.h"
#include "uihelpview.h"
#include "uiimage.h"
#include "uimodel.h"

UiHistoryView::UiHistoryView(const UiViewParams& p_Params)
  : UiViewBase(p_Params)
{
  if (m_Enabled)
  {
    int hpad = (m_X == 0) ? 0 : 1;
    int vpad = 1;
    int paddedY = m_Y + vpad;
    int paddedX = m_X + hpad;
    m_PaddedH = m_H - (vpad * 2);
    m_PaddedW = m_W - (hpad * 2);
    m_PaddedWin = newwin(m_PaddedH, m_PaddedW, paddedY, paddedX);

    static int attributeTextNormal = UiColorConfig::GetAttribute("history_text_attr");
    static int colorPairTextRecv = UiColorConfig::GetColorPair("history_text_recv_color");
    werase(m_Win);
    wbkgd(m_Win, attributeTextNormal | colorPairTextRecv | ' ');
    wrefresh(m_Win);

    // view (re)created, any previous placements may be stale
    UiImage::ClearPlacements();
  }
}

UiHistoryView::~UiHistoryView()
{
  if (m_PaddedWin != nullptr)
  {
    delwin(m_PaddedWin);
    m_PaddedWin = nullptr;
  }
}

void UiHistoryView::Draw()
{
  if (!m_Enabled || !m_Dirty) return;
  m_Dirty = false;

  curs_set(0);

  static int colorPairTextSent = UiColorConfig::GetColorPair("history_text_sent_color");
  static int colorPairTextRecv = UiColorConfig::GetColorPair("history_text_recv_color");
  static int colorPairTextQuoted = UiColorConfig::GetColorPair("history_text_quoted_color");
  static int colorPairTextReaction = UiColorConfig::GetColorPair("history_text_reaction_color");
  static int colorPairTextAttachment = UiColorConfig::GetColorPair("history_text_attachment_color");
  static int attributeTextNormal = UiColorConfig::GetAttribute("history_text_attr");
  static int attributeTextSelected = UiColorConfig::GetAttribute("history_text_attr_selected");

  static int colorPairNameSent = UiColorConfig::GetColorPair("history_name_sent_color");
  static int colorPairNameRecv = UiColorConfig::GetColorPair("history_name_recv_color");
  static int attributeNameNormal = UiColorConfig::GetAttribute("history_name_attr");
  static int attributeNameSelected = UiColorConfig::GetAttribute("history_name_attr_selected");

  static std::wstring attachmentIndicator =
    StrUtil::ToWString(UiConfig::GetStr("attachment_indicator") + " ");
  static const bool attachmentTypeIconsEnabled = UiConfig::GetBool("attachment_type_icons_enabled");
  static const std::map<FileCategory, std::wstring> attachmentTypeIndicators = []()
  {
    const std::map<FileCategory, std::string> keys =
    {
      { FileCategoryImage, "attachment_indicator_image" },
      { FileCategoryVideo, "attachment_indicator_video" },
      { FileCategoryAudio, "attachment_indicator_audio" },
      { FileCategoryPdf, "attachment_indicator_pdf" },
      { FileCategoryDocument, "attachment_indicator_document" },
      { FileCategorySpreadsheet, "attachment_indicator_spreadsheet" },
      { FileCategoryArchive, "attachment_indicator_archive" },
    };

    // empty icon falls back to generic attachment indicator
    std::map<FileCategory, std::wstring> indicators;
    for (const auto& key : keys)
    {
      const std::string icon = UiConfig::GetStr(key.second);
      indicators[key.first] = icon.empty() ? attachmentIndicator : StrUtil::ToWString(icon + " ");
    }

    return indicators;
  }();
  static const std::wstring attachmentVoiceIndicator = []()
  {
    const std::string icon = UiConfig::GetStr("attachment_indicator_voice");
    return icon.empty() ? attachmentIndicator : StrUtil::ToWString(icon + " ");
  }();
  static std::wstring quoteIndicator = L"> ";

  std::pair<std::string, std::string>& currentChat = m_Model->GetCurrentChatLocked();
  const bool emojiEnabled = m_Model->GetEmojiEnabledLocked();
  static const bool developerMode = AppUtil::GetDeveloperMode();

  std::vector<std::string>& messageVec =
    m_Model->GetMessageVecLocked(currentChat.first, currentChat.second);
  std::unordered_map<std::string, ChatMessage>& messages =
    m_Model->GetMessagesLocked(currentChat.first, currentChat.second);
  const int messageOffset = std::max(m_Model->GetMessageOffsetLocked(currentChat.first, currentChat.second), 0);
  const bool selectActive = m_Model->GetSelectMessageActiveLocked();


  static const bool inlineImagesEnabled = UiImage::IsEnabled();
  static const int inlineImageMaxHeight = std::max(1, UiConfig::GetNum("inline_image_max_height"));
  std::vector<UiImagePlacement> imagePlacements;

  // Chat banner on top rows (contact or group), messages are drawn below it
  static const int bannerRows = 3;
  ChatBanner chatBanner;
  const bool hasBanner = m_Model->GetChatBannerLocked(currentChat.first, currentChat.second, chatBanner) &&
    (m_PaddedH > (bannerRows * 3));
  const int topY = hasBanner ? bannerRows : 0;
  int winBegY = 0;
  int winBegX = 0;
  getbegyx(m_PaddedWin, winBegY, winBegX);

  // Draws messages bottom-up starting at p_ViewOffset, returns whether the selected message header
  // was shown. Side effects (mark read, downloads, fetch) only for the final draw.
  auto drawMessages = [&](int p_ViewOffset, bool p_Final) -> bool
  {
    werase(m_PaddedWin);
    wbkgd(m_PaddedWin, attributeTextNormal | colorPairTextRecv | ' ');

    m_HistoryShowCount = 0;
    imagePlacements.clear();
    bool selectedShown = false;

    bool firstMessage = true;
    int y = m_PaddedH - 1;
    for (auto it = std::next(messageVec.begin(), p_ViewOffset); it != messageVec.end(); ++it)
    {
      const int index = (int)std::distance(messageVec.begin(), it);
      bool isSelectedMessage = selectActive && (index == messageOffset);

      auto msgIt = messages.find(*it);
      if (msgIt == messages.end())
      {
        LOG_WARNING("message %s missing", it->c_str());
        continue;
      }

      ChatMessage& msg = msgIt->second;

      int attributeText = isSelectedMessage ? attributeTextSelected : attributeTextNormal;
      int colorPairText = [&]()
      {
        if (msg.isOutgoing) return colorPairTextSent;

        if (msg.senderId == currentChat.second) return colorPairTextRecv;

        static bool isUserColor = UiColorConfig::IsUserColor("history_text_recv_group_color");
        if (!isUserColor)
        {
          static int colorPairGroup = UiColorConfig::GetColorPair("history_text_recv_group_color");
          return colorPairGroup;
        }

        int colorPairGroup = UiColorConfig::GetUserColorPair("history_text_recv_group_color",
                                                             msg.senderId);
        return colorPairGroup;
      }();

      std::vector<std::wstring> wlines;
      if (!msg.text.empty())
      {
        std::string text = msg.text;
        StrUtil::SanitizeMessageStr(text);
        if (!emojiEnabled)
        {
          text = StrUtil::Textize(text);
        }

        wlines = StrUtil::WordWrap(StrUtil::ToWString(text), m_PaddedW, false, false, false, 2);
      }

      const int textLineCount = (int)wlines.size();

      // Media metadata (sidecar files written on receipt), only looked up for attachments, links and status
      static const std::string statusChatId = "status@broadcast";
      const bool hasLink = (msg.text.find("http") != std::string::npos);
      const MediaMeta* meta = (!msg.fileInfo.empty() || hasLink || (currentChat.second == statusChatId)) ?
        &MediaInfo::Get(currentChat.first, msg.id) : nullptr;

      // Link without preview (old, sent from nchat, or none embedded), fetch from web page if origin allowed:
      // 1 = own messages and saved contacts, 2 = also groups, 3 = all
      if (p_Final && hasLink && (meta != nullptr) && msg.fileInfo.empty() && meta->title.empty() &&
          meta->description.empty() && !meta->linkFetched)
      {
        static const int linkPreviewFetch = UiConfig::GetNum("link_preview_fetch");
        static const std::string groupSuffix = "@g.us";
        const std::string& chatId = currentChat.second;
        const bool isGroup = (chatId.size() > groupSuffix.size()) &&
          (chatId.compare(chatId.size() - groupSuffix.size(), groupSuffix.size(), groupSuffix) == 0);
        bool allowed = false;
        if (linkPreviewFetch >= 3)
        {
          allowed = true;
        }
        else if (linkPreviewFetch >= 1)
        {
          allowed = msg.isOutgoing ||
            (isGroup && (linkPreviewFetch >= 2)) ||
            (!isGroup && MediaInfo::IsSavedContact(currentChat.first,
                                                   (chatId == statusChatId) ? msg.senderId : chatId));
        }

        if (allowed)
        {
          const std::string url = MediaInfo::GetFirstUrl(msg.text);
          if (!url.empty())
          {
            MediaInfo::RequestLinkPreview(currentChat.first, msg.id, url);
          }
        }
      }

      // Quoted message
      bool hasQuoteLine = false;
      if (!msg.quotedId.empty())
      {
        hasQuoteLine = true;
        std::string quotedText;
        auto quotedIt = messages.find(msg.quotedId);
        bool quotedFound = (quotedIt != messages.end());
        if (!quotedFound && (currentChat.second != statusChatId))
        {
          // reply to status update (sent or received), quoted status is in the status chat
          std::unordered_map<std::string, ChatMessage>& statusMessages =
            m_Model->GetMessagesLocked(currentChat.first, statusChatId);
          quotedIt = statusMessages.find(msg.quotedId);
          quotedFound = (quotedIt != statusMessages.end());
        }

        if (quotedFound)
        {
          if (!quotedIt->second.text.empty())
          {
            quotedText = StrUtil::Split(quotedIt->second.text, '\n').at(0);
            if (!emojiEnabled)
            {
              quotedText = StrUtil::Textize(quotedText);
            }
          }
          else if (!quotedIt->second.fileInfo.empty())
          {
            FileInfo fileInfo = ProtocolUtil::FileInfoFromHex(quotedIt->second.fileInfo);
            quotedText = FileUtil::BaseName(fileInfo.filePath);
          }
        }
        else if (p_Final)
        {
          m_Model->FetchCachedMessageLocked(currentChat.first, currentChat.second, msg.quotedId);
          quotedText = "";
        }

        int maxQuoteLen = m_PaddedW - 3;
        std::wstring quote = quoteIndicator + StrUtil::ToWString(quotedText);
        if (StrUtil::WStringWidth(quote) > maxQuoteLen)
        {
          quote = StrUtil::TrimPadWString(quote, maxQuoteLen) + L"...";
        }

        wlines.insert(wlines.begin(), quote);
      }

      // File attachment
      bool hasAttachmentLine = false; // attachment line is always first in wlines
      int imageLineIndex = -1; // index in wlines of first inline image row
      UiImageInfo imageInfo;
      if (!msg.fileInfo.empty())
      {
        FileInfo fileInfo = ProtocolUtil::FileInfoFromHex(msg.fileInfo);
        const bool isImageFile = UiImage::IsImageFile(fileInfo.filePath);
        const bool isVideoFile = UiImage::IsVideoFile(fileInfo.filePath);
        const bool isInlineImage = inlineImagesEnabled && isImageFile;

        // special case handling selection-triggered download, and handling cache's old setting
        static const bool isAttachmentPrefetchAll =
          (AppConfig::GetNum("attachment_prefetch") == AttachmentPrefetchAll);
        static const bool isAttachmentPrefetchSelected =
          (AppConfig::GetNum("attachment_prefetch") == AttachmentPrefetchSelected);
        // attachments of expired status updates are never downloaded
        const bool isExpiredStatus = m_Model->IsExpiredStatusLocked(currentChat.second, msg);
        if (p_Final && !isExpiredStatus &&
            (isAttachmentPrefetchAll || (isSelectedMessage && isAttachmentPrefetchSelected) || isInlineImage))
        {
          if (!UiModel::IsAttachmentDownloaded(fileInfo) && UiModel::IsAttachmentDownloadable(fileInfo))
          {
            m_Model->DownloadAttachmentLocked(currentChat.first, currentChat.second, *it,
                                              fileInfo.fileId, DownloadFileActionNone);
            fileInfo = ProtocolUtil::FileInfoFromHex(msg.fileInfo);
          }
        }

        std::string fileName = FileUtil::BaseName(fileInfo.filePath);
        std::string fileStatus;
        if (isExpiredStatus && (fileInfo.fileStatus != FileStatusDownloaded))
        {
          // expired status attachment not downloaded, show plain attachment line
        }
        else if (fileInfo.fileStatus == FileStatusNone)
        {
          // should not happen
          static const std::string statusNone = " -";
          fileStatus = statusNone;
        }
        else if (fileInfo.fileStatus == FileStatusNotDownloaded)
        {
          static const std::string statusNotDownloaded = " " + UiConfig::GetStr("downloadable_indicator");
          fileStatus = statusNotDownloaded;
        }
        else if (fileInfo.fileStatus == FileStatusDownloaded)
        {
          static const std::string statusDownloaded = "";
          fileStatus = statusDownloaded;
        }
        else if (fileInfo.fileStatus == FileStatusDownloading)
        {
          static const std::string statusDownloading = " " + UiConfig::GetStr("syncing_indicator");
          fileStatus = statusDownloading;
        }
        else if (fileInfo.fileStatus == FileStatusDownloadFailed)
        {
          static const std::string statusDownloadFailed = " " + UiConfig::GetStr("failed_indicator");
          fileStatus = statusDownloadFailed;
        }

        // Downloaded video / audio without known duration (old or sent from nchat), probe locally
        const bool isAudioFile = UiImage::IsAudioFile(fileInfo.filePath);
        if (p_Final && (meta != nullptr) && (meta->seconds == 0) && (isVideoFile || isAudioFile) &&
            (fileInfo.fileStatus == FileStatusDownloaded) && FileUtil::Exists(fileInfo.filePath))
        {
          MediaInfo::RequestProbe(currentChat.first, msg.id, fileInfo.filePath);
        }

        // Video / audio duration and size, ex: " ▶ 1:15 · 4.2 MB" or " 🎤 0:42"
        // (voice note mic omitted when already shown as attachment type icon)
        const bool isPtt = (meta != nullptr) && meta->isPtt;
        std::string mediaStr;
        if ((meta != nullptr) && !isImageFile)
        {
          if (meta->isPtt && (meta->seconds > 0))
          {
            static const std::string pttPrefix = attachmentTypeIconsEnabled ? " " : " \xf0\x9f\x8e\xa4 ";
            mediaStr = pttPrefix + MediaInfo::FormatDuration(meta->seconds);
          }
          else if ((meta->seconds > 0) || (meta->fileLength > 0))
          {
            mediaStr = " \xe2\x96\xb6 ";
            if (meta->seconds > 0)
            {
              mediaStr += MediaInfo::FormatDuration(meta->seconds);
            }

            if (meta->fileLength > 0)
            {
              mediaStr += std::string((meta->seconds > 0) ? " \xc2\xb7 " : "") + MediaInfo::FormatSize(meta->fileLength);
            }
          }

          if (!emojiEnabled)
          {
            mediaStr = StrUtil::Textize(mediaStr);
          }
        }

        std::wstring indicator = attachmentIndicator;
        if (attachmentTypeIconsEnabled)
        {
          auto typeIt = attachmentTypeIndicators.find(FileUtil::GetFileCategory(fileInfo.filePath));
          if (isPtt)
          {
            indicator = attachmentVoiceIndicator;
          }
          else if (typeIt != attachmentTypeIndicators.end())
          {
            indicator = typeIt->second;
          }
        }

        // nothing may be inserted before the attachment line after this, see hasAttachmentLine
        std::wstring fileStr = indicator + StrUtil::ToWString(fileName + fileStatus + mediaStr);
        wlines.insert(wlines.begin(), fileStr);
        hasAttachmentLine = true;

        // Inline image: downloaded image, video poster frame, or embedded thumbnail when not downloaded
        std::string inlinePath;
        bool inlineUpscale = false;
        if (inlineImagesEnabled)
        {
          const bool isDownloaded = (fileInfo.fileStatus == FileStatusDownloaded);
          if (isImageFile && isDownloaded)
          {
            inlinePath = fileInfo.filePath;
          }
          else if (isVideoFile && isDownloaded)
          {
            inlinePath = MediaInfo::GetPosterPath(fileInfo.filePath, p_Final);
          }

          if (inlinePath.empty() && (isImageFile || isVideoFile) && (meta != nullptr))
          {
            inlinePath = meta->thumbPath;
            inlineUpscale = true;
          }
        }

        // Inline image rows reserved below attachment line, image placed after refresh
        if (!inlinePath.empty() && UiImage::Prepare(inlinePath, m_PaddedW, inlineImageMaxHeight, imageInfo, inlineUpscale))
        {
          imageLineIndex = 1;
          wlines.insert(std::next(wlines.begin(), imageLineIndex), imageInfo.rows, std::wstring());
        }
      }

      // Link preview: title, description and thumbnail embedded in the message
      int previewLineIndex = -1;
      int previewLineEnd = -1;
      bool previewHasTitle = false;
      if ((meta != nullptr) && msg.fileInfo.empty() && (!meta->title.empty() || !meta->description.empty()))
      {
        static const std::wstring previewIndicator = L"\u258f ";
        static const std::wstring linkIndicator = []()
        {
          const std::string icon = UiConfig::GetStr("link_preview_indicator");
          return icon.empty() ? std::wstring() : StrUtil::ToWString(icon + " ");
        }();
        static const int linkIndicatorW = StrUtil::WStringWidth(linkIndicator);
        const int previewW = std::max(m_PaddedW - 2, 1);
        // link icon on first preview line only (title, or first description line if no title)
        const int firstLineW = std::max(previewW - linkIndicatorW, 1);
        auto previewStr = [&](const std::string& p_Str)
        {
          std::string str = p_Str;
          StrUtil::SanitizeMessageStr(str);
          if (!emojiEnabled)
          {
            str = StrUtil::Textize(str);
          }

          return StrUtil::ToWString(str);
        };

        previewLineIndex = (int)wlines.size();
        if (!meta->title.empty())
        {
          previewHasTitle = true;
          wlines.push_back(previewIndicator + linkIndicator +
                           StrUtil::TrimPadWString(previewStr(meta->title), firstLineW));
        }

        if (!meta->description.empty())
        {
          static const int maxDescLines = 2;
          const int descW = previewHasTitle ? previewW : firstLineW;
          std::vector<std::wstring> descLines =
            StrUtil::WordWrap(previewStr(meta->description), descW, false, false, false, 2);
          if ((int)descLines.size() > maxDescLines)
          {
            descLines.resize(maxDescLines);
            std::wstring& lastLine = descLines.back();
            lastLine = StrUtil::TrimPadWString(lastLine, descW - 1);
            lastLine.erase(lastLine.find_last_not_of(L' ') + 1);
            lastLine += L"\u2026";
          }

          for (size_t i = 0; i < descLines.size(); ++i)
          {
            const bool isFirstLine = !previewHasTitle && (i == 0);
            wlines.push_back(previewIndicator + (isFirstLine ? linkIndicator : std::wstring()) + descLines[i]);
          }
        }

        previewLineEnd = (int)wlines.size();

        if (inlineImagesEnabled && !meta->thumbPath.empty() &&
            UiImage::Prepare(meta->thumbPath, m_PaddedW, inlineImageMaxHeight, imageInfo, true))
        {
          imageLineIndex = (int)wlines.size();
          wlines.insert(wlines.end(), imageInfo.rows, std::wstring());
        }
      }

      // Status text background color, applied to the text lines
      int colorPairBackground = 0;
      int backgroundLineIndex = -1;
      if ((meta != nullptr) && meta->hasBackground && msg.fileInfo.empty() && (textLineCount > 0))
      {
        const uint32_t bgRgb = meta->backgroundArgb & 0xffffff;
        uint32_t fgRgb = meta->textArgb & 0xffffff;
        if (!meta->hasTextColor)
        {
          const uint32_t r = (bgRgb >> 16) & 0xff;
          const uint32_t g = (bgRgb >> 8) & 0xff;
          const uint32_t b = bgRgb & 0xff;
          const uint32_t luminance = ((r * 299) + (g * 587) + (b * 114)) / 1000;
          fgRgb = (luminance > 140) ? 0x000000 : 0xffffff;
        }

        colorPairBackground = UiColorConfig::GetRgbColorPair(fgRgb, bgRgb);
        backgroundLineIndex = hasQuoteLine ? 1 : 0;
      }

      // Reactions
      int reactionLines = 0;
      static bool reactionsEnabled = UiConfig::GetBool("reactions_enabled");
      if (reactionsEnabled)
      {
        std::string selfEmoji;
        auto sit = msg.reactions.senderEmojis.find(s_ReactionsSelfId);
        if (sit != msg.reactions.senderEmojis.end())
        {
          selfEmoji = sit->second;
        }

        // Allow also if we have self emoji, even if not yet consolidated into count
        if (!msg.reactions.emojiCounts.empty() || !selfEmoji.empty())
        {
          bool foundSelf = false;
          std::string reactionsText;
          std::multimap<float, std::string> emojiCountsSorted;
          for (const auto& emojiCount : msg.reactions.emojiCounts)
          {
            float count = emojiCount.second;
            if (emojiCount.first == selfEmoji)
            {
              count += 0.1; // for equal count, prioritize own selected reaction
              foundSelf = true;
            }

            emojiCountsSorted.insert(std::make_pair(count, emojiCount.first));
          }

          if (!foundSelf && !selfEmoji.empty())
          {
            LOG_DEBUG("insert missing reaction for self");
            emojiCountsSorted.insert(std::make_pair(1.1, selfEmoji));
          }

          bool firstReaction = true;
          for (auto emojiCount = emojiCountsSorted.rbegin(); emojiCount != emojiCountsSorted.rend(); ++emojiCount)
          {
            reactionsText += (firstReaction ? " " : "  ");
            if (emojiCount->second == selfEmoji)
            {
              // Highlight own reaction emoji
              reactionsText += "" + emojiCount->second + "*";
            }
            else
            {
              reactionsText += emojiCount->second;
            }

            if (emojiCount->first > 1.5)
            {
              reactionsText += " " + FileUtil::GetSuffixedCount(static_cast<ssize_t>(emojiCount->first));
            }

            firstReaction = false;
          }

          if (!reactionsText.empty())
          {
            if (!emojiEnabled)
            {
              reactionsText = StrUtil::Textize(reactionsText);
            }

            const int maxReactionsLen = m_PaddedW - 4;
            std::wstring reactions = StrUtil::ToWString(reactionsText);
            if (StrUtil::WStringWidth(reactions) > maxReactionsLen)
            {
              reactions = StrUtil::TrimPadWString(reactions, maxReactionsLen) + L"... ";
            }
            else
            {
              reactions += L" ";
            }

            wlines.insert(wlines.end(), reactions);
            reactionLines = 1;
          }
        }
      }

      const int maxMessageLines = (m_PaddedH - topY - 1);
      if (firstMessage && ((int)wlines.size() > maxMessageLines))
      {
        // drop inline image if it does not fit, keeping the attachment line
        if ((imageLineIndex >= 0) && ((imageLineIndex + imageInfo.rows) > (maxMessageLines - 1)))
        {
          wlines.erase(std::next(wlines.begin(), imageLineIndex),
                       std::next(wlines.begin(), imageLineIndex + imageInfo.rows));
          imageLineIndex = -1;
        }

        wlines.resize(std::max(maxMessageLines - 1, 0));
        wlines.push_back(L"[...]");
        hasAttachmentLine = hasAttachmentLine && (wlines.size() > 1);
        reactionLines = 0;
      }

      int imageTopY = -1;
      int imageVisibleRows = 0;
      for (auto wline = wlines.rbegin(); wline != wlines.rend(); ++wline)
      {
        const int lineIndex = (int)wlines.size() - 1 - (int)std::distance(wlines.rbegin(), wline);
        if ((imageLineIndex >= 0) && (lineIndex >= imageLineIndex) &&
            (lineIndex < (imageLineIndex + imageInfo.rows)))
        {
          imageTopY = y;
          ++imageVisibleRows;
        }

        bool isAttachment = hasAttachmentLine && (lineIndex == 0);
        bool isQuote = (wline->rfind(quoteIndicator, 0) == 0);
        bool isReaction = (reactionLines == 1) && (std::distance(wline, wlines.rbegin()) == 0);
        bool isPreview = (lineIndex >= previewLineIndex) && (lineIndex < previewLineEnd);
        bool isPreviewTitle = isPreview && previewHasTitle && (lineIndex == previewLineIndex);
        bool isBackground = (colorPairBackground != 0) && (lineIndex >= backgroundLineIndex) &&
          (lineIndex < (backgroundLineIndex + textLineCount));

        int lineAttribute = 0;
        if (isAttachment)
        {
          lineAttribute = attributeText | colorPairTextAttachment;
        }
        else if (isQuote || isPreview)
        {
          lineAttribute = attributeText | colorPairTextQuoted | (isPreviewTitle ? A_BOLD : 0);
        }
        else if (isReaction)
        {
          lineAttribute = attributeTextNormal | colorPairTextReaction;
        }
        else if (isBackground)
        {
          lineAttribute = attributeText | colorPairBackground;
        }
        else
        {
          lineAttribute = attributeText | colorPairText;
        }

        wattron(m_PaddedWin, lineAttribute);

        const std::wstring wdisp = isReaction ? *wline : StrUtil::TrimPadWString(*wline, m_PaddedW);
        mvwaddnwstr(m_PaddedWin, y, 0, wdisp.c_str(), std::min((int)wdisp.size(), m_PaddedW));

        wattroff(m_PaddedWin, lineAttribute);

        if (--y < topY) break;
      }

      if (imageVisibleRows > 0)
      {
        UiImagePlacement placement;
        placement.info = imageInfo;
        placement.screenY = winBegY + imageTopY;
        placement.screenX = winBegX;
        placement.skipRows = imageInfo.rows - imageVisibleRows;
        placement.visibleRows = imageVisibleRows;
        imagePlacements.push_back(placement);
      }

      if (y < topY) break;

      int attributeName = isSelectedMessage ? attributeNameSelected : attributeNameNormal;
      int colorPairName = [&]()
      {
        if (msg.isOutgoing) return colorPairNameSent;

        if (msg.senderId == currentChat.second) return colorPairNameRecv;

        static bool isUserColor = UiColorConfig::IsUserColor("history_name_recv_group_color");
        if (!isUserColor)
        {
          static int colorPairGroup = UiColorConfig::GetColorPair("history_name_recv_group_color");
          return colorPairGroup;
        }

        int colorPairGroup = UiColorConfig::GetUserColorPair("history_name_recv_group_color",
                                                             msg.senderId);
        return colorPairGroup;
      }();

      std::string name = m_Model->GetContactNameLocked(currentChat.first, msg.senderId);
      if (currentChat.second == "status@broadcast")
      {
        // selecting a new status update marks it seen in list (local only, not opened)
        if (p_Final && isSelectedMessage)
        {
          m_Model->MarkStatusSeenLocked(currentChat.first, currentChat.second, *it);
        }

        static const std::string statusNewIndicator = UiConfig::GetStr("status_new_indicator") + " ";
        if (m_Model->IsStatusNewLocked(currentChat.first, currentChat.second, msg))
        {
          name = statusNewIndicator + name;
        }
      }

      if (!emojiEnabled)
      {
        name = StrUtil::Textize(name);
      }

      std::wstring wsender = StrUtil::ToWString(name);
      std::wstring wtime;
      if (developerMode)
      {
        wtime = L" (" + StrUtil::ToWString(std::to_string(msg.timeSent)) + L")";
      }
      else
      {
        if (msg.timeSent != std::numeric_limits<int64_t>::max())
        {
          wtime = L" (" + StrUtil::ToWString(TimeUtil::GetTimeString(msg.timeSent, false /* p_IsExport */)) + L")";
        }
      }

      if (p_Final)
      {
        m_Model->MarkReadLocked(currentChat.first, currentChat.second, *it, (!msg.isOutgoing && !msg.isRead));
      }

      static const std::string editedIndicator = " " + UiConfig::GetStr("edited_indicator");
      static const std::string pinnedIndicator = " " + UiConfig::GetStr("pinned_indicator");
      std::wstring wedited = StrUtil::ToWString(msg.isEdited ? editedIndicator : "");
      std::wstring wpinned = StrUtil::ToWString(msg.isPinned ? pinnedIndicator : "");
      std::wstring wheader = wsender + wtime;
      std::wstring wtrailer = wedited + wpinned;

      if (developerMode)
      {
        wtrailer = wtrailer +
          L" msg " + StrUtil::ToWString(msg.id) +
          L" user " + StrUtil::ToWString(msg.senderId);
      }

      // Delivery status of outgoing (not pending) messages, inferred for messages cached before tracking
      std::wstring wreceipt;
      int colorPairReceipt = colorPairName;
      if (msg.isOutgoing && (msg.timeSent != std::numeric_limits<int64_t>::max()))
      {
        static const std::wstring sentIndicator =
          L" " + StrUtil::ToWString(UiConfig::GetStr("receipt_sent_indicator"));
        static const std::wstring deliveredIndicator =
          L" " + StrUtil::ToWString(UiConfig::GetStr("receipt_delivered_indicator"));
        static const std::wstring readIndicator =
          L" " + StrUtil::ToWString(UiConfig::GetStr("receipt_read_indicator"));
        static const int colorPairReceiptSent = UiColorConfig::GetColorPair("history_receipt_sent_color");
        static const int colorPairReceiptDelivered = UiColorConfig::GetColorPair("history_receipt_delivered_color");
        static const int colorPairReceiptRead = UiColorConfig::GetColorPair("history_receipt_read_color");

        int deliveryStatus = msg.deliveryStatus;
        if (deliveryStatus == DeliveryStatusUnknown)
        {
          deliveryStatus = msg.isRead ? DeliveryStatusRead : DeliveryStatusDelivered;
        }

        if (deliveryStatus >= DeliveryStatusRead)
        {
          wreceipt = readIndicator;
          colorPairReceipt = colorPairReceiptRead;
        }
        else if (deliveryStatus == DeliveryStatusDelivered)
        {
          wreceipt = deliveredIndicator;
          colorPairReceipt = colorPairReceiptDelivered;
        }
        else
        {
          wreceipt = sentIndicator;
          colorPairReceipt = colorPairReceiptSent;
        }
      }

      // Header segments: sender and time, delivery status (own color), and trailing indicators padded
      // to full width. Each segment is truncated to the remaining width.
      int x = 0;
      auto drawSegment = [&](const std::wstring& p_Segment, int p_ColorPair, bool p_Pad)
      {
        const int remainW = m_PaddedW - x;
        if (remainW <= 0) return;

        std::wstring wdisp = p_Segment;
        if (p_Pad || (StrUtil::WStringWidth(wdisp) > remainW))
        {
          wdisp = StrUtil::TrimPadWString(wdisp, remainW);
        }

        wattron(m_PaddedWin, attributeName | p_ColorPair);
        mvwaddnwstr(m_PaddedWin, y, x, wdisp.c_str(), std::min((int)wdisp.size(), remainW));
        wattroff(m_PaddedWin, attributeName | p_ColorPair);
        x += StrUtil::WStringWidth(wdisp);
      };

      drawSegment(wheader, colorPairName, false);
      if (!wreceipt.empty())
      {
        drawSegment(wreceipt, colorPairReceipt, false);
      }

      drawSegment(wtrailer, colorPairName, true);

      ++m_HistoryShowCount;
      selectedShown = selectedShown || isSelectedMessage;

      if (--y < topY) break;

      if (--y < topY) break;

      firstMessage = false;
    }

    return selectedShown;
  };

  // With selection active the view stays put, and only scrolls the minimum needed to fully show
  // the selected message.
  int viewOffset = messageOffset;
  if (selectActive)
  {
    viewOffset = std::max(std::min(m_Model->GetHistoryViewOffsetLocked(currentChat.first, currentChat.second),
                                   messageOffset), 0);
    while ((viewOffset < messageOffset) && !drawMessages(viewOffset, false /* p_Final */))
    {
      ++viewOffset;
    }

    m_Model->SetHistoryViewOffsetLocked(currentChat.first, currentChat.second, viewOffset);
  }

  drawMessages(viewOffset, true /* p_Final */);

  if (hasBanner)
  {
    DrawBanner(chatBanner, bannerRows, winBegY, winBegX, imagePlacements);
  }

  wrefresh(m_PaddedWin);

  if (inlineImagesEnabled)
  {
    UiImage::PlaceAll(imagePlacements);
  }
}

int UiHistoryView::GetHistoryShowCount()
{
  return m_HistoryShowCount;
}

void UiHistoryView::DrawBanner(const ChatBanner& p_ChatBanner, int p_Rows, int p_WinBegY, int p_WinBegX,
                               std::vector<UiImagePlacement>& p_ImagePlacements)
{
  static const int colorPairBanner = UiColorConfig::GetColorPair("history_banner_color");
  static const int attributeBanner = UiColorConfig::GetAttribute("history_banner_attr");
  static const int colorPairTyping = UiColorConfig::GetColorPair("history_banner_typing_color");
  static const int colorPairOnline = UiColorConfig::GetColorPair("history_banner_online_color");
  static const int colorPairAway = UiColorConfig::GetColorPair("history_banner_away_color");
  static const int colorPairSeen = UiColorConfig::GetColorPair("history_banner_seen_color");
  static const bool inlineImagesEnabled = UiImage::IsEnabled();
  const bool emojiEnabled = m_Model->GetEmojiEnabledLocked();

  // background
  wattron(m_PaddedWin, attributeBanner | colorPairBanner);
  for (int y = 0; y < p_Rows; ++y)
  {
    const std::wstring wblank(m_PaddedW, L' ');
    mvwaddnwstr(m_PaddedWin, y, 0, wblank.c_str(), wblank.size());
  }

  wattroff(m_PaddedWin, attributeBanner | colorPairBanner);

  // profile photo on the left, enlarged to banner height
  int textX = 1;
  static const int avatarMaxCols = 10;
  UiImageInfo avatarInfo;
  if (inlineImagesEnabled && !p_ChatBanner.avatarPath.empty() &&
      UiImage::Prepare(p_ChatBanner.avatarPath, avatarMaxCols, p_Rows, avatarInfo, true /*p_Upscale*/))
  {
    UiImagePlacement placement;
    placement.info = avatarInfo;
    placement.screenY = p_WinBegY;
    placement.screenX = p_WinBegX;
    placement.skipRows = 0;
    placement.visibleRows = std::min(avatarInfo.rows, p_Rows);
    p_ImagePlacements.push_back(placement);
    textX = avatarInfo.cols + 2;
  }

  // segments of text per row, each with own color
  auto drawRow = [&](int p_Y, const std::vector<std::pair<std::string, int>>& p_Segments)
  {
    int x = textX;
    for (const auto& segment : p_Segments)
    {
      if (x >= m_PaddedW) break;

      std::string text = emojiEnabled ? segment.first : StrUtil::Textize(segment.first);
      std::wstring wtext = StrUtil::ToWString(text);
      const int remainW = m_PaddedW - x;
      if (StrUtil::WStringWidth(wtext) > remainW)
      {
        wtext = StrUtil::TrimPadWString(wtext, remainW);
      }

      wattron(m_PaddedWin, segment.second);
      mvwaddnwstr(m_PaddedWin, p_Y, x, wtext.c_str(), wtext.size());
      wattroff(m_PaddedWin, segment.second);
      x += StrUtil::WStringWidth(wtext);
    }
  };

  const int attributeText = attributeBanner | colorPairBanner;
  const int attributeSeen = attributeBanner | colorPairSeen;
  static const std::string separator = " \xc2\xb7 ";
  const std::string mutedSuffix = p_ChatBanner.isMuted ? "muted" : "";

  // row 1: name and phone
  std::vector<std::pair<std::string, int>> row1 = { { p_ChatBanner.name, attributeText | A_BOLD } };
  if (!p_ChatBanner.isGroup && !p_ChatBanner.phone.empty())
  {
    std::string phone = p_ChatBanner.phone;
    if (phone.front() != '+')
    {
      phone = "+" + phone;
    }

    row1.push_back({ separator + phone, attributeSeen });
  }

  drawRow(0, row1);

  // row 2: presence (contact) or participants (group), and muted
  std::vector<std::pair<std::string, int>> row2;
  if (p_ChatBanner.isGroup)
  {
    if (p_ChatBanner.memberCount > 0)
    {
      row2.push_back({ std::to_string(p_ChatBanner.memberCount) + " participants", attributeText });
    }
  }
  else if (!p_ChatBanner.presence.empty())
  {
    int colorPairPresence = colorPairSeen;
    switch (p_ChatBanner.presenceType)
    {
      case PresenceTyping: colorPairPresence = colorPairTyping; break;
      case PresenceOnline: colorPairPresence = colorPairOnline; break;
      case PresenceAway: colorPairPresence = colorPairAway; break;
      default: break;
    }

    row2.push_back({ p_ChatBanner.presence, attributeBanner | colorPairPresence });
  }

  if (!mutedSuffix.empty())
  {
    row2.push_back({ (row2.empty() ? "" : separator) + mutedSuffix, attributeSeen });
  }

  drawRow(1, row2);

  // row 3: status updates of last 24 hours (contact)
  if (!p_ChatBanner.isGroup && (p_ChatBanner.statusTotal > 0))
  {
    static const std::string statusNewIndicator = UiConfig::GetStr("status_new_indicator");
    static const std::string statusKey = UiHelpView::GetKeyDisplay("status_filter");
    const int count = (p_ChatBanner.statusNew > 0) ? p_ChatBanner.statusNew : p_ChatBanner.statusTotal;
    std::string text = (p_ChatBanner.statusNew > 0) ? (statusNewIndicator + " " + std::to_string(count) + " new")
                                                    : ("\xe2\x97\xaf " + std::to_string(count));
    text += (count == 1) ? " status update" : " status updates";
    if (!statusKey.empty())
    {
      text += separator + statusKey;
    }

    drawRow(2, { { text, (p_ChatBanner.statusNew > 0) ? (attributeBanner | colorPairOnline) : attributeSeen } });
  }
}
