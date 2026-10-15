// uihistoryview.cpp
//
// Copyright (c) 2019-2025 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uihistoryview.h"

#include "appconfig.h"
#include "apputil.h"
#include "fileutil.h"
#include "log.h"
#include "protocolutil.h"
#include "strutil.h"
#include "timeutil.h"
#include "uicolorconfig.h"
#include "uiconfig.h"
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
  static std::wstring quoteIndicator = L"> ";

  std::pair<std::string, std::string>& currentChat = m_Model->GetCurrentChatLocked();
  const bool emojiEnabled = m_Model->GetEmojiEnabledLocked();
  static const bool developerMode = AppUtil::GetDeveloperMode();

  std::vector<std::string>& messageVec =
    m_Model->GetMessageVecLocked(currentChat.first, currentChat.second);
  std::unordered_map<std::string, ChatMessage>& messages =
    m_Model->GetMessagesLocked(currentChat.first, currentChat.second);
  int messageOffset = std::max(m_Model->GetMessageOffsetLocked(currentChat.first, currentChat.second), 0);

  werase(m_PaddedWin);
  wbkgd(m_PaddedWin, attributeTextNormal | colorPairTextRecv | ' ');

  m_HistoryShowCount = 0;

  static const bool inlineImagesEnabled = UiImage::IsEnabled();
  static const int inlineImageMaxHeight = std::max(1, UiConfig::GetNum("inline_image_max_height"));
  std::vector<UiImagePlacement> imagePlacements;
  int winBegY = 0;
  int winBegX = 0;
  getbegyx(m_PaddedWin, winBegY, winBegX);

  bool firstMessage = true;
  int y = m_PaddedH - 1;
  for (auto it = std::next(messageVec.begin(), messageOffset); it != messageVec.end(); ++it)
  {
    bool isSelectedMessage = firstMessage && m_Model->GetSelectMessageActiveLocked();

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

    // Quoted message
    if (!msg.quotedId.empty())
    {
      std::string quotedText;
      auto quotedIt = messages.find(msg.quotedId);
      if (quotedIt != messages.end())
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
      else
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
    int imageLineIndex = -1; // index in wlines of first inline image row
    UiImageInfo imageInfo;
    if (!msg.fileInfo.empty())
    {
      FileInfo fileInfo = ProtocolUtil::FileInfoFromHex(msg.fileInfo);
      const bool isInlineImage = inlineImagesEnabled && UiImage::IsImageFile(fileInfo.filePath);

      // special case handling selection-triggered download, and handling cache's old setting
      static const bool isAttachmentPrefetchAll =
        (AppConfig::GetNum("attachment_prefetch") == AttachmentPrefetchAll);
      static const bool isAttachmentPrefetchSelected =
        (AppConfig::GetNum("attachment_prefetch") == AttachmentPrefetchSelected);
      if (isAttachmentPrefetchAll || (isSelectedMessage && isAttachmentPrefetchSelected) || isInlineImage)
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
      if (fileInfo.fileStatus == FileStatusNone)
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

      std::wstring fileStr = attachmentIndicator + StrUtil::ToWString(fileName + fileStatus);
      wlines.insert(wlines.begin(), fileStr);

      // Inline image rows reserved below attachment line, image placed after refresh
      if (isInlineImage && (fileInfo.fileStatus == FileStatusDownloaded) &&
          UiImage::Prepare(fileInfo.filePath, m_PaddedW, inlineImageMaxHeight, imageInfo))
      {
        imageLineIndex = 1;
        wlines.insert(std::next(wlines.begin(), imageLineIndex), imageInfo.rows, std::wstring());
      }
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

    const int maxMessageLines = (m_PaddedH - 1);
    if (firstMessage && ((int)wlines.size() > maxMessageLines))
    {
      // drop inline image if it does not fit, keeping the attachment line
      if ((imageLineIndex >= 0) && ((imageLineIndex + imageInfo.rows) > (maxMessageLines - 1)))
      {
        wlines.erase(std::next(wlines.begin(), imageLineIndex),
                     std::next(wlines.begin(), imageLineIndex + imageInfo.rows));
        imageLineIndex = -1;
      }

      wlines.resize(maxMessageLines - 1);
      wlines.push_back(L"[...]");
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

      bool isAttachment = (wline->rfind(attachmentIndicator, 0) == 0);
      bool isQuote = (wline->rfind(quoteIndicator, 0) == 0);
      bool isReaction = (reactionLines == 1) && (std::distance(wline, wlines.rbegin()) == 0);

      if (isAttachment)
      {
        wattron(m_PaddedWin, attributeText | colorPairTextAttachment);
      }
      else if (isQuote)
      {
        wattron(m_PaddedWin, attributeText | colorPairTextQuoted);
      }
      else if (isReaction)
      {
        wattron(m_PaddedWin, attributeTextNormal | colorPairTextReaction);
      }
      else
      {
        wattron(m_PaddedWin, attributeText | colorPairText);
      }

      const std::wstring wdisp = isReaction ? *wline : StrUtil::TrimPadWString(*wline, m_PaddedW);
      mvwaddnwstr(m_PaddedWin, y, 0, wdisp.c_str(), std::min((int)wdisp.size(), m_PaddedW));

      if (isAttachment)
      {
        wattroff(m_PaddedWin, attributeText | colorPairTextAttachment);
      }
      else if (isQuote)
      {
        wattroff(m_PaddedWin, attributeText | colorPairTextQuoted);
      }
      else if (isReaction)
      {
        wattroff(m_PaddedWin, attributeTextNormal | colorPairTextReaction);
      }
      else
      {
        wattroff(m_PaddedWin, attributeText | colorPairText);
      }

      if (--y < 0) break;
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

    if (y < 0) break;

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

    m_Model->MarkReadLocked(currentChat.first, currentChat.second, *it, (!msg.isOutgoing && !msg.isRead));

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

    if (--y < 0) break;

    if (--y < 0) break;

    firstMessage = false;
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
