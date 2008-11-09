/******************************************************************************
 * Copyright (C) 2008 Tim Boundy <gigaplex@gmail.com>                         *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "stdafx.h"

#include <qsmp/HotkeyWindow.h>
#include <qsmp/HotkeyWindow.moc>

#include <MMShellHook.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

HotkeyWindow::HotkeyWindow()
: registered_hotkeys_(false)
{
}

//-----------------------------------------------------------------------------

HotkeyWindow::~HotkeyWindow()
{
  UnregisterHotkeys();
}

//-----------------------------------------------------------------------------

bool HotkeyWindow::RegisterHotkeys()
{
  if (registered_hotkeys_)
    UnregisterHotkeys();

  registered_hotkeys_ = SetMMShellHook(internalWinId());

  return registered_hotkeys_;
}

//-----------------------------------------------------------------------------

bool HotkeyWindow::UnregisterHotkeys()
{
  if (registered_hotkeys_)
  {
    registered_hotkeys_ = !UnSetMMShellHook(internalWinId());

    return !registered_hotkeys_;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool HotkeyWindow::winEvent(MSG* message, long* /*result*/)
{
  //static bool next = false;
  //static bool previous = false;
  //static bool playpause = false;
  //static bool stop = false;

  if(message->message == WM_APPCOMMAND)
  {
    // HWND hack to make all hex
    std::cout << "message->message: " << (HWND)message->message << std::endl;
    std::cout << "message->hwnd: " << (HWND)message->hwnd << std::endl;
    std::cout << "message->lParam: " << (HWND)message->lParam << std::endl;
    std::cout << "message->time: " << message->time << std::endl;
    std::cout << "message->wParam: " << (HWND)message->wParam << std::endl;

    switch(GET_APPCOMMAND_LPARAM(message->lParam))
    {
    case APPCOMMAND_MEDIA_NEXTTRACK:
      //if (next)
      //{
      //  next = false;
      //  std::cout << "Next hotkey ignored: " << GetTickCount() << std::endl;
      //}
      //else
      //{
      //  next = true;
        std::cout << "Next hotkey detected: " << GetTickCount() << std::endl;
        OnNext();
      //}
      return true;
    case APPCOMMAND_MEDIA_PREVIOUSTRACK:
      //if (previous)
      //{
      //  previous = false;
      //  std::cout << "Previous hotkey ignored: " << GetTickCount() << std::endl;
      //}
      //else
      //{
      //  previous = true;
        std::cout << "Previous hotkey detected: " << GetTickCount() << std::endl;
        OnPrevious();
      //}
      return true;
    case APPCOMMAND_MEDIA_STOP:
      //if (stop)
      //{
      //  stop = false;
      //  std::cout << "Stop hotkey ignored: " << GetTickCount() << std::endl;
      //}
      //else
      //{
      //  stop = true;
        std::cout << "Stop hotkey detected: " << GetTickCount() << std::endl;
        OnStop();
      //}
      return true;
    case APPCOMMAND_MEDIA_PLAY_PAUSE:
      //if (playpause)
      //{
      //  playpause = false;
      //  std::cout << "Play/Pause hotkey ignored: " << GetTickCount() << std::endl;
      //}
      //else
      //{
      //  playpause = true;
        std::cout << "Play/Pause hotkey detected: " << GetTickCount() << std::endl;
        OnPlayPause();
      //}
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END
