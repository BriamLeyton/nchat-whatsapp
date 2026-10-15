// uihelpdialog.cpp
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// nchat is distributed under the MIT license, see LICENSE for details.

#include "uihelpdialog.h"

#include <algorithm>

#include "apputil.h"
#include "strutil.h"
#include "uicolorconfig.h"
#include "uicontroller.h"
#include "uihelpview.h"
#include "uikeyconfig.h"
#include "uimodel.h"

namespace
{
  struct HelpEntry
  {
    std::string func;
    std::string desc;
  };

  struct HelpCategory
  {
    std::string title;
    std::vector<HelpEntry> entries;
  };

  const std::vector<HelpCategory>& GetHelpCategories()
  {
    static const std::vector<HelpCategory> categories =
    {
      {
        "Chats",
        {
          { "next_chat", "Chat siguiente" },
          { "prev_chat", "Chat anterior" },
          { "unread_chat", "Ir a chat no leído" },
          { "goto_chat", "Ir a chat" },
          { "select_contact", "Buscar contactos" },
          { "pin", "Fijar/desfijar chat" },
          { "archive_chat", "Archivar/desarchivar chat" },
          { "delete_chat", "Eliminar/salir del chat" },
          { "toggle_others", "Colapsar/expandir Otros" },
          { "toggle_archived", "Colapsar/expandir Archivados" },
        },
      },
      {
        "Mensajes",
        {
          { "send_msg", "Enviar mensaje" },
          { "transfer", "Enviar archivo" },
          { "select_emoji", "Insertar emoji" },
          { "select_mention", "Insertar mención" },
          { "ext_edit", "Componer en editor externo" },
          { "auto_compose", "Autocompletar respuesta" },
          { "spell", "Corrector externo" },
          { "ext_call", "Llamada externa" },
          { "find", "Buscar en el chat" },
          { "find_next", "Buscar siguiente" },
          { "jump_pinned", "Ir a mensaje fijado" },
          { "prev_page", "Página anterior" },
          { "next_page", "Página siguiente" },
        },
      },
      {
        "Mensaje seleccionado",
        {
          { "up", "Seleccionar / mensaje anterior" },
          { "down", "Mensaje siguiente" },
          { "edit_msg", "Editar mensaje" },
          { "delete_msg", "Eliminar mensaje" },
          { "react", "Reaccionar" },
          { "forward_msg", "Reenviar mensaje" },
          { "jump_quoted", "Ir al mensaje citado" },
          { "open", "Abrir adjunto" },
          { "save", "Guardar adjunto" },
          { "open_link", "Abrir enlace" },
          { "open_msg", "Ver en visor externo" },
        },
      },
      {
        "Edición de texto",
        {
          { "cut", "Cortar" },
          { "copy", "Copiar" },
          { "paste", "Pegar" },
          { "begin_line", "Inicio de línea" },
          { "end_line", "Fin de línea" },
          { "backward_word", "Palabra anterior" },
          { "forward_word", "Palabra siguiente" },
          { "backward_kill_word", "Borrar palabra anterior" },
          { "kill_word", "Borrar palabra siguiente" },
          { "delete_line_before_cursor", "Borrar hasta inicio de línea" },
          { "delete_line_after_cursor", "Borrar hasta fin de línea" },
        },
      },
      {
        "Vista",
        {
          { "toggle_help", "Mostrar/cerrar esta ayuda" },
          { "toggle_list", "Mostrar/ocultar lista de chats" },
          { "toggle_top", "Mostrar/ocultar barra superior" },
          { "toggle_emoji", "Mostrar emojis como texto" },
          { "decrease_list_width", "Reducir ancho de la lista" },
          { "increase_list_width", "Aumentar ancho de la lista" },
          { "quit", "Salir" },
        },
      },
    };

    return categories;
  }
}

UiHelpDialog::UiHelpDialog(const UiDialogParams& p_Params)
  : UiDialog(p_Params)
{
  // esc is used to close, avoid waiting the default 1 sec for an escape sequence
  m_PrevEscDelay = get_escdelay();
  set_escdelay(50);
  Layout();
}

UiHelpDialog::~UiHelpDialog()
{
  set_escdelay(m_PrevEscDelay);
}

void UiHelpDialog::Run()
{
  Draw();
  while (m_Running && !AppUtil::IsTerminateRequested())
  {
    wint_t key = UiController::GetKey(50);
    if (key != 0)
    {
      KeyHandler(key);
    }
  }
}

void UiHelpDialog::KeyHandler(wint_t p_Key)
{
  static wint_t keyEsc = 27;
  static wint_t keyCancel = UiKeyConfig::GetKey("cancel");
  static wint_t keyQuit = UiKeyConfig::GetKey("quit");
  static wint_t keyToggleHelp = UiKeyConfig::GetKey("toggle_help");
  static wint_t keyUp = UiKeyConfig::GetKey("up");
  static wint_t keyDown = UiKeyConfig::GetKey("down");
  static wint_t keyPrevPage = UiKeyConfig::GetKey("prev_page");
  static wint_t keyNextPage = UiKeyConfig::GetKey("next_page");
  static wint_t keyTerminalFocusIn = UiKeyConfig::GetKey("terminal_focus_in");
  static wint_t keyTerminalFocusOut = UiKeyConfig::GetKey("terminal_focus_out");
  static wint_t keyTerminalResize = UiKeyConfig::GetKey("terminal_resize");

  if (p_Key == keyTerminalResize)
  {
    Cleanup();
    m_Model->ReinitView();
    m_Model->Draw();
    curs_set(0);
    Init();
    Layout();
  }
  else if (p_Key == keyTerminalFocusIn)
  {
    m_Model->SetTerminalActive(true);
    return;
  }
  else if (p_Key == keyTerminalFocusOut)
  {
    m_Model->SetTerminalActive(false);
    return;
  }
  else if ((p_Key == keyEsc) || (p_Key == keyCancel) || (p_Key == keyQuit) || (p_Key == keyToggleHelp))
  {
    m_Running = false;
    return;
  }
  else if (p_Key == keyUp)
  {
    --m_Offset;
  }
  else if (p_Key == keyDown)
  {
    ++m_Offset;
  }
  else if (p_Key == keyPrevPage)
  {
    m_Offset -= m_H;
  }
  else if (p_Key == keyNextPage)
  {
    m_Offset += m_H;
  }
  else
  {
    return;
  }

  Draw();
}

void UiHelpDialog::Layout()
{
  // build lines, skipping functions without a displayable key
  std::vector<std::vector<Line>> categoryLines;
  m_KeyW = 0;
  for (const auto& category : GetHelpCategories())
  {
    std::vector<Line> lines;
    lines.push_back(Line{ L"", StrUtil::ToWString(category.title), true });
    for (const auto& entry : category.entries)
    {
      const std::string keyDisplay = UiHelpView::GetKeyDisplay(entry.func);
      if (keyDisplay.empty()) continue;

      Line line{ StrUtil::ToWString(keyDisplay), StrUtil::ToWString(entry.desc), false };
      m_KeyW = std::max(m_KeyW, StrUtil::WStringWidth(line.key));
      lines.push_back(line);
    }

    if (lines.size() > 1)
    {
      categoryLines.push_back(lines);
    }
  }

  // use two columns if wide enough, splitting categories at about half the lines
  static const int minColW = 36;
  static const int colGap = 3;
  const int numCols = (((m_W - colGap) / 2) >= minColW) ? 2 : 1;
  m_ColW = (numCols == 2) ? ((m_W - colGap) / 2) : m_W;

  int totalLines = 0;
  for (const auto& lines : categoryLines)
  {
    totalLines += lines.size() + 1;
  }

  m_Columns.assign(numCols, std::vector<Line>());
  int col = 0;
  for (const auto& lines : categoryLines)
  {
    if ((numCols == 2) && (col == 0) && !m_Columns[0].empty() &&
        ((int)(m_Columns[0].size() + (lines.size() / 2)) > (totalLines / 2)))
    {
      col = 1;
    }

    std::vector<Line>& column = m_Columns[col];
    if (!column.empty())
    {
      column.push_back(Line{});
    }

    column.insert(column.end(), lines.begin(), lines.end());
  }

  m_Offset = 0;
}

void UiHelpDialog::Draw()
{
  static int colorPair = UiColorConfig::GetColorPair("dialog_color");
  static int attribute = UiColorConfig::GetAttribute("dialog_attr");

  int rows = 0;
  for (const auto& column : m_Columns)
  {
    rows = std::max(rows, (int)column.size());
  }

  const int maxOffset = std::max(0, rows - m_H);
  m_Offset = std::max(0, std::min(m_Offset, maxOffset));

  std::string footer = "Esc cerrar";
  if (maxOffset > 0)
  {
    footer += "  \xe2\x86\x91\xe2\x86\x93 desplazar";
    footer += (m_Offset < maxOffset) ? "  \xe2\x96\xbe m\xc3\xa1s" : "";
  }

  SetFooter(footer);

  werase(m_Win);
  wbkgd(m_Win, colorPair | ' ');
  wattron(m_Win, attribute | colorPair);

  for (int y = 0; y < m_H; ++y)
  {
    const int row = y + m_Offset;
    for (int col = 0; col < (int)m_Columns.size(); ++col)
    {
      if (row >= (int)m_Columns[col].size()) continue;

      DrawLine(y, col * (m_ColW + 3), m_ColW, m_Columns[col][row]);
    }
  }

  wattroff(m_Win, attribute | colorPair);
  wrefresh(m_Win);
}

void UiHelpDialog::DrawLine(int p_Y, int p_X, int p_W, const Line& p_Line)
{
  if (p_Line.isTitle)
  {
    std::wstring wtitle = StrUtil::TrimPadWString(p_Line.desc, p_W);
    wattron(m_Win, A_BOLD | A_UNDERLINE);
    mvwaddnwstr(m_Win, p_Y, p_X, wtitle.c_str(), wtitle.size());
    wattroff(m_Win, A_BOLD | A_UNDERLINE);
    return;
  }

  if (p_Line.key.empty() && p_Line.desc.empty()) return;

  const int keyW = std::min(m_KeyW + 2, p_W);
  std::wstring wkey = StrUtil::TrimPadWString(p_Line.key, keyW);
  wattron(m_Win, A_BOLD);
  mvwaddnwstr(m_Win, p_Y, p_X, wkey.c_str(), wkey.size());
  wattroff(m_Win, A_BOLD);

  if (p_W > keyW)
  {
    std::wstring wdesc = StrUtil::TrimPadWString(p_Line.desc, p_W - keyW);
    mvwaddnwstr(m_Win, p_Y, p_X + keyW, wdesc.c_str(), wdesc.size());
  }
}
