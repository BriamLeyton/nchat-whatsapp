// uihelpdialog.h
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <string>
#include <vector>

#include "uidialog.h"

// Modal dialog listing all commands and their configured keys
class UiHelpDialog : public UiDialog
{
public:
  UiHelpDialog(const UiDialogParams& p_Params);
  virtual ~UiHelpDialog();

  void Run();
  void KeyHandler(wint_t p_Key);

private:
  struct Line
  {
    std::wstring key;
    std::wstring desc;
    bool isTitle = false;
  };

  void Layout();
  void Draw();
  void DrawLine(int p_Y, int p_X, int p_W, const Line& p_Line);

private:
  bool m_Running = true;
  int m_PrevEscDelay = 0;
  int m_Offset = 0;
  int m_KeyW = 0;
  int m_ColW = 0;
  std::vector<std::vector<Line>> m_Columns;
};
