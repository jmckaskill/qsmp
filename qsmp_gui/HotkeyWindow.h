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

#ifndef QSMP_HOTKEYWINDOW_H_
#define QSMP_HOTKEYWINDOW_H_


#include "qsmp_gui/common.h"
#include <QtGui/qwidget.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HotkeyWindow : public QWidget
{
  Q_OBJECT
public:
  HotkeyWindow();
  virtual ~HotkeyWindow();

  bool RegisterHotkeys();
  bool UnregisterHotkeys();

protected:
  bool winEvent(MSG* message, long* result);

Q_SIGNALS:
  void OnPrevious();
  void OnNext();
  void OnPlayPause();
  void OnStop();

private:
  bool registered_hotkeys_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif /*QSMP_HOTKEYWINDOW_H_*/
