// uistatuscontactdialog.h
//
// Copyright (c) 2026 Briam Leyton
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <string>
#include <vector>

#include "uilistdialog.h"

struct UiStatusContactListItem
{
  std::string senderId;
  std::string name;
  int64_t lastTime = 0;
};

// Lists contacts with status updates, most recent first
class UiStatusContactDialog : public UiListDialog
{
public:
  UiStatusContactDialog(const UiDialogParams& p_Params,
                        const std::string& p_ProfileId,
                        const std::string& p_ChatId);
  virtual ~UiStatusContactDialog();

  UiStatusContactListItem GetSelectedItem();

protected:
  virtual void OnSelect();
  virtual void OnBack();
  virtual bool OnTimer();
  virtual void UpdateList();

private:
  std::string m_ProfileId;
  std::string m_ChatId;
  std::vector<UiStatusContactListItem> m_AllItemVec;
  std::vector<UiStatusContactListItem> m_ItemVec;
  UiStatusContactListItem m_SelectedItem;
  int m_PrevEscDelay = 0;
};
