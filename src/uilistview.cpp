// uilistview.cpp
//
// Copyright (c) 2019-2025 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uilistview.h"

#include "strutil.h"
#include "uicolorconfig.h"
#include "uiconfig.h"
#include "uimodel.h"

UiListView::UiListView(const UiViewParams& p_Params)
  : UiViewBase(p_Params)
{
  if (m_Enabled)
  {
    int paddedY = m_Y + 1;
    int paddedX = m_X + 1;
    m_PaddedH = m_H - 2;
    m_PaddedW = m_W - 2;
    m_PaddedWin = newwin(m_PaddedH, m_PaddedW, paddedY, paddedX);

    static int colorPair = UiColorConfig::GetColorPair("list_color");
    static int attribute = UiColorConfig::GetAttribute("list_attr");

    werase(m_Win);
    wbkgd(m_Win, attribute | colorPair | ' ');
    wrefresh(m_Win);
  }
}

UiListView::~UiListView()
{
  if (m_PaddedWin != nullptr)
  {
    delwin(m_PaddedWin);
    m_PaddedWin = nullptr;
  }
}

void UiListView::Draw()
{
  if (!m_Enabled || !m_Dirty) return;
  m_Dirty = false;

  curs_set(0);

  static int colorPair = UiColorConfig::GetColorPair("list_color");
  static int attribute = UiColorConfig::GetAttribute("list_attr");
  static int attributeSelected = UiColorConfig::GetAttribute("list_attr_selected");
  static int colorPairUnread = UiColorConfig::GetColorPair("list_color_unread");

  static int colorPairHeader = UiColorConfig::GetColorPair("list_color_header");
  static int attributeHeader = UiColorConfig::GetAttribute("list_attr_header");
  static const std::string unreadIndicator = " " + UiConfig::GetStr("unread_indicator");
  static const std::wstring wunread = StrUtil::ToWString(unreadIndicator);
  static const std::string sectionHeaders[ChatSectionCount] =
  {
    UiConfig::GetStr("list_pinned_header"),
    UiConfig::GetStr("list_others_header"),
    UiConfig::GetStr("list_archived_header"),
  };

  int index = std::max(0, m_Model->GetCurrentChatIndexLocked());
  const std::vector<std::pair<std::string, std::string>>& p_ChatVec = m_Model->GetChatVecLocked();

  // build rows: section headers (chatIndex -1) and visible chats, chat vec is sorted by section
  struct Row
  {
    int chatIndex = -1;
    int section = 0;
  };

  const bool emojiEnabled = m_Model->GetEmojiEnabledLocked();
  std::vector<std::string> names;
  std::vector<bool> unreads;
  std::vector<int> sections;
  int sectionCounts[ChatSectionCount] = { 0, 0, 0 };
  bool sectionUnreads[ChatSectionCount] = { false, false, false };
  for (auto& chatPair : p_ChatVec)
  {
    const std::string& name = m_Model->GetContactListNameLocked(chatPair.first, chatPair.second, true /*p_AllowId*/,
                                                                true /*p_AllowAlias*/);
    bool isUnread = m_Model->GetChatIsUnreadLocked(chatPair.first, chatPair.second);
    int section = m_Model->GetChatSectionLocked(chatPair.first, chatPair.second);
    names.push_back(name);
    unreads.push_back(isUnread);
    sections.push_back(section);
    ++sectionCounts[section];
    sectionUnreads[section] = sectionUnreads[section] || isUnread;
  }

  // headers are only shown when chats exist in other sections than "others"
  const bool showHeaders = (sectionCounts[ChatSectionOthers] < (int)p_ChatVec.size());
  std::vector<Row> rows;
  int selectedRow = 0;
  int prevSection = -1;
  for (int i = 0; i < (int)p_ChatVec.size(); ++i)
  {
    const int section = sections[i];
    if (showHeaders && (section != prevSection))
    {
      rows.push_back(Row{ -1, section });
      prevSection = section;
    }

    if (m_Model->IsChatHiddenLocked(p_ChatVec[i].first, p_ChatVec[i].second)) continue;

    if (i == index)
    {
      selectedRow = rows.size();
    }

    rows.push_back(Row{ i, section });
  }

  werase(m_PaddedWin);
  wbkgd(m_PaddedWin, attribute | colorPair | ' ');
  wattron(m_PaddedWin, attribute | colorPair);

  if (!rows.empty())
  {
    int height = m_PaddedH;
    int count = rows.size();
    int offset = std::min(std::max(0, selectedRow - ((height - 1) / 2)), std::max(0, count - height));
    int last = std::min((height + offset), count);
    for (int r = offset; r < last; ++r)
    {
      int y = r - offset;
      const Row& row = rows[r];
      if (row.chatIndex == -1)
      {
        const int section = row.section;
        const bool collapsed = m_Model->GetSectionCollapsedLocked(section) &&
          (sectionCounts[section] < (int)p_ChatVec.size());
        // "▾ TITLE ─────" when expanded, "▸ TITLE (N) ──── *" when collapsed
        std::string header = (collapsed ? "\xe2\x96\xb8 " : "\xe2\x96\xbe ") + sectionHeaders[section];
        if (collapsed)
        {
          header += " (" + std::to_string(sectionCounts[section]) + ")";
        }

        const bool headerUnread = collapsed && sectionUnreads[section];
        const int unreadW = headerUnread ? StrUtil::WStringWidth(wunread) : 0;
        const int headerW = std::max(0, m_PaddedW - unreadW);
        std::wstring wheader = StrUtil::ToWString(header + " ");
        const int ruleW = headerW - StrUtil::WStringWidth(wheader);
        if (ruleW > 0)
        {
          wheader += std::wstring(ruleW, L'\u2500');
        }

        wheader = StrUtil::TrimPadWString(wheader, headerW);

        wattroff(m_PaddedWin, attribute | colorPair);
        wattron(m_PaddedWin, attributeHeader | colorPairHeader);
        mvwaddnwstr(m_PaddedWin, y, 0, wheader.c_str(), wheader.size());
        wattroff(m_PaddedWin, attributeHeader | colorPairHeader);

        if (headerUnread)
        {
          wattron(m_PaddedWin, colorPairUnread);
          mvwaddnwstr(m_PaddedWin, y, (m_PaddedW - unreadW), wunread.c_str(), wunread.size());
          wattroff(m_PaddedWin, colorPairUnread);
        }

        wattron(m_PaddedWin, attribute | colorPair);
        continue;
      }

      const int i = row.chatIndex;
      if (i == index)
      {
        wattroff(m_PaddedWin, attribute);
        wattron(m_PaddedWin, attributeSelected);
      }

      std::string name = names[i];
      if (!emojiEnabled)
      {
        name = StrUtil::Textize(name);
      }

      std::wstring wname = StrUtil::ToWString(name).substr(0, m_PaddedW);
      wname = StrUtil::TrimPadWString(wname, m_PaddedW);

      if (unreads[i])
      {
        wattron(m_PaddedWin, colorPairUnread);
      }

      mvwaddnwstr(m_PaddedWin, y, 0, wname.c_str(), wname.size());

      if (unreads[i])
      {
        mvwaddnwstr(m_PaddedWin, y, (m_PaddedW - StrUtil::WStringWidth(wunread)), wunread.c_str(), wunread.size());

        wattron(m_PaddedWin, colorPair);
      }

      if (i == index)
      {
        wattroff(m_PaddedWin, attributeSelected);
        wattron(m_PaddedWin, attribute);
      }
    }
  }

  wattroff(m_PaddedWin, attribute | colorPair);
  wrefresh(m_PaddedWin);
}
