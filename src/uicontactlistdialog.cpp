// uicontactlistdialog.cpp
//
// Copyright (c) 2019-2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uicontactlistdialog.h"

#include <algorithm>

#include "apputil.h"
#include "log.h"
#include "uimodel.h"
#include "strutil.h"

UiContactListDialog::UiContactListDialog(const UiDialogParams& p_Params)
  : UiListDialog(p_Params, false /*p_ShadeHidden*/)
{
  static bool s_ContactsRequested = false;
  if (!s_ContactsRequested)
  {
    m_Model->RequestContacts();
    s_ContactsRequested = true;
  }

  UpdateList();
}

UiContactListDialog::~UiContactListDialog()
{
}

UiContactListItem UiContactListDialog::GetSelectedContactItem()
{
  return m_SelectedContactItem;
}

static std::string UrlDecode(const std::string& p_Str)
{
  std::string out;
  for (size_t i = 0; i < p_Str.size(); ++i)
  {
    const char c = p_Str[i];
    if (c == '+')
    {
      out += ' ';
    }
    else if ((c == '%') && (i + 2 < p_Str.size()) && isxdigit((unsigned char)p_Str[i + 1]) &&
             isxdigit((unsigned char)p_Str[i + 2]))
    {
      out += (char)std::stoi(p_Str.substr(i + 1, 2), nullptr, 16);
      i += 2;
    }
    else
    {
      out += c;
    }
  }

  return out;
}

static std::string GetUrlParam(const std::string& p_Query, const std::string& p_Name)
{
  size_t pos = 0;
  while (pos < p_Query.size())
  {
    size_t end = p_Query.find('&', pos);
    if (end == std::string::npos) end = p_Query.size();

    const std::string param = p_Query.substr(pos, end - pos);
    if (param.rfind(p_Name + "=", 0) == 0)
    {
      return UrlDecode(param.substr(p_Name.size() + 1));
    }

    pos = end + 1;
  }

  return "";
}

// Parse WhatsApp click-to-chat link (wa.me, api.whatsapp.com/send, whatsapp://send) or international
// phone number, into phone number digits and (for links) message text.
bool UiContactListDialog::ParseWhatsAppLink(const std::string& p_Str, std::string& p_Phone, std::string& p_Text)
{
  static const size_t minDigits = 8;
  static const size_t maxDigits = 15;

  std::string str = p_Str;
  StrUtil::Trim(str);
  std::string phone;
  std::string text;

  std::string lower = StrUtil::ToLower(str);
  bool isLink = false;
  for (const std::string& prefix : { "https://", "http://" })
  {
    if (lower.rfind(prefix, 0) == 0)
    {
      str = str.substr(prefix.size());
      lower = lower.substr(prefix.size());
      break;
    }
  }

  if (lower.rfind("www.", 0) == 0)
  {
    str = str.substr(4);
    lower = lower.substr(4);
  }

  size_t queryPos = str.find('?');
  const std::string path = (queryPos == std::string::npos) ? lower : lower.substr(0, queryPos);
  const std::string query = (queryPos == std::string::npos) ? "" : str.substr(queryPos + 1);

  if (path.rfind("wa.me/", 0) == 0)
  {
    isLink = true;
    phone = path.substr(6);
    if (!phone.empty() && (phone.back() == '/')) phone.pop_back();
    text = GetUrlParam(query, "text");
  }
  else if ((path == "api.whatsapp.com/send") || (path == "api.whatsapp.com/send/") ||
           (path == "whatsapp://send") || (path == "whatsapp://send/"))
  {
    isLink = true;
    phone = GetUrlParam(query, "phone");
    text = GetUrlParam(query, "text");
  }
  else
  {
    // plain number: only digits and common separators
    if (str.find_first_not_of("0123456789 +-().") != std::string::npos) return false;

    phone = str;
  }

  std::string digits;
  for (const char c : phone)
  {
    if (isdigit((unsigned char)c))
    {
      digits += c;
    }
    else if (isLink && (c != '+') && (c != ' ') && (c != '-'))
    {
      return false;
    }
  }

  if ((digits.size() < minDigits) || (digits.size() > maxDigits)) return false;

  p_Phone = digits;
  p_Text = text;
  return true;
}

void UiContactListDialog::OnSelect()
{
  if (m_ContactListItemVec.empty()) return;

  m_SelectedContactItem = m_ContactListItemVec[m_Index];
  m_Result = true;
  m_Running = false;
}

void UiContactListDialog::OnBack()
{
}

bool UiContactListDialog::OnTimer()
{
  int64_t modelContactInfosUpdateTime = m_Model->GetContactInfosUpdateTime();
  if (m_DialogContactInfosUpdateTime != modelContactInfosUpdateTime)
  {
    UpdateList();
    return true;
  }

  return false;
}

void UiContactListDialog::UpdateList()
{
  int64_t modelContactInfosUpdateTime = m_Model->GetContactInfosUpdateTime();
  if (m_DialogContactInfosUpdateTime != modelContactInfosUpdateTime)
  {
    m_DialogContactInfosUpdateTime = modelContactInfosUpdateTime;
    m_DialogContactInfos = m_Model->GetContactInfos();
  }

  const bool emojiEnabled = m_Model->GetEmojiEnabled();

  // Get current item based on index
  UiContactListItem currentContactItem;
  if (m_Index < (int)m_ContactListItemVec.size())
  {
    currentContactItem = m_ContactListItemVec[m_Index];
  }

  m_Index = 0;
  m_Items.clear();
  m_ContactListItemVec.clear();

  // Use a local vector which is sorted before populating dialog members, which need to be in sync
  std::vector<UiContactListItem> localContactListItemVec;

  for (const auto& profileContactInfos : m_DialogContactInfos)
  {
    for (const auto& idContactInfo : profileContactInfos.second)
    {
      const std::string& profileId = profileContactInfos.first;
      const std::string& contactId = idContactInfo.first;
      const std::string& name = m_Model->GetContactListName(profileId, contactId, false /*p_AllowId*/,
                                                            false /*p_AllowAlias*/);;

      if (name.empty()) continue;

      // hidden locked chats are not offered
      if (m_Model->IsChatLockedHidden(profileId, contactId)) continue;

      if (m_FilterStr.empty() ||
          (StrUtil::ToFold(name).find(StrUtil::ToFold(StrUtil::ToString(m_FilterStr))) != std::string::npos))
      {
        std::string displayName = name + m_Model->GetProfileSuffix(profileId);

        if (!emojiEnabled)
        {
          displayName = StrUtil::Textize(displayName);
        }

        static const bool developerMode = AppUtil::GetDeveloperMode();
        if (developerMode)
        {
          displayName += " [" + contactId + "]";
        }

        UiContactListItem contactListItem;
        contactListItem.profileId = profileId;
        contactListItem.contactId = contactId;
        contactListItem.name = displayName;

        localContactListItemVec.push_back(contactListItem);
      }
    }
  }

  std::sort(localContactListItemVec.begin(), localContactListItemVec.end(),
            [&](const UiContactListItem& lhs, const UiContactListItem& rhs) -> bool
  {
    return lhs.name < rhs.name;
  });

  // number or link in filter is offered as first item
  std::string linkPhone;
  std::string linkText;
  if (ParseWhatsAppLink(StrUtil::ToString(m_FilterStr), linkPhone, linkText))
  {
    UiContactListItem phoneListItem;
    // profile is resolved by model on select
    phoneListItem.phone = linkPhone;
    phoneListItem.text = linkText;
    phoneListItem.name = "Open chat with +" + linkPhone;
    localContactListItemVec.insert(localContactListItemVec.begin(), phoneListItem);
  }

  for (const auto& contactListItem : localContactListItemVec)
  {
    m_Items.push_back(StrUtil::TrimPadWString(StrUtil::ToWString(contactListItem.name), m_W));
    m_ContactListItemVec.push_back(contactListItem);
  }

  // Set index based on current item
  if (!currentContactItem.profileId.empty() && !currentContactItem.contactId.empty())
  {
    for (auto it = m_ContactListItemVec.begin(); it != m_ContactListItemVec.end(); ++it)
    {
      if ((it->contactId == currentContactItem.contactId) &&
          (it->profileId == currentContactItem.profileId))
      {
        m_Index = std::distance(m_ContactListItemVec.begin(), it);
        break;
      }
    }
  }
}
