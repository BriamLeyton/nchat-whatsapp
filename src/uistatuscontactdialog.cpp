// uistatuscontactdialog.cpp
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uistatuscontactdialog.h"

#include <algorithm>

#include "messagecache.h"
#include "strutil.h"
#include "timeutil.h"
#include "uimodel.h"

UiStatusContactDialog::UiStatusContactDialog(const UiDialogParams& p_Params,
                                             const std::string& p_ProfileId,
                                             const std::string& p_ChatId)
  : UiListDialog(p_Params, false /*p_ShadeHidden*/)
  , m_ProfileId(p_ProfileId)
  , m_ChatId(p_ChatId)
{
  // esc is used to close, avoid waiting the default 1 sec for an escape sequence
  m_EscCancels = true;
  m_PrevEscDelay = get_escdelay();
  set_escdelay(50);

  // senders from cache (not only loaded messages), already sorted by most recent
  const bool emojiEnabled = m_Model->GetEmojiEnabled();
  for (const auto& senderLastTime : MessageCache::GetSenderLastTimesSync(m_ProfileId, m_ChatId))
  {
    UiStatusContactListItem item;
    item.senderId = senderLastTime.first;
    item.lastTime = senderLastTime.second;
    item.name = m_Model->GetContactListName(m_ProfileId, item.senderId, true /*p_AllowId*/,
                                            true /*p_AllowAlias*/);
    if (!emojiEnabled)
    {
      item.name = StrUtil::Textize(item.name);
    }

    m_AllItemVec.push_back(item);
  }

  UpdateList();
}

UiStatusContactDialog::~UiStatusContactDialog()
{
  set_escdelay(m_PrevEscDelay);
}

UiStatusContactListItem UiStatusContactDialog::GetSelectedItem()
{
  return m_SelectedItem;
}

void UiStatusContactDialog::OnSelect()
{
  if (m_ItemVec.empty()) return;

  m_SelectedItem = m_ItemVec[m_Index];
  m_Result = true;
  m_Running = false;
}

void UiStatusContactDialog::OnBack()
{
}

bool UiStatusContactDialog::OnTimer()
{
  return false;
}

void UiStatusContactDialog::UpdateList()
{
  m_Index = 0;
  m_Items.clear();
  m_ItemVec.clear();

  const std::string filterStr = StrUtil::ToFold(StrUtil::ToString(m_FilterStr));
  for (const auto& item : m_AllItemVec)
  {
    if (!filterStr.empty() && (StrUtil::ToFold(item.name).find(filterStr) == std::string::npos)) continue;

    // name left, time of most recent status right
    const std::wstring wtime = StrUtil::ToWString(" " + TimeUtil::GetTimeString(item.lastTime, false));
    const int timeW = StrUtil::WStringWidth(wtime);
    std::wstring witem = StrUtil::TrimPadWString(StrUtil::ToWString(item.name), std::max(0, m_W - timeW));
    if (m_W > timeW)
    {
      witem += wtime;
    }

    m_Items.push_back(StrUtil::TrimPadWString(witem, m_W));
    m_ItemVec.push_back(item);
  }
}
