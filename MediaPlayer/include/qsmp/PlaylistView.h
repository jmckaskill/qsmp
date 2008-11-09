/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
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

#ifndef QSMP_PLAYLISTVIEW_H_
#define QSMP_PLAYLISTVIEW_H_

#include <qsmp/common.h>
#include <qsmp/Player.h>
#include <QtCore/qdatetime.h.>
#include <QtGui/qwidget.h>


namespace Phonon {
class SeekSlider;
class VolumeSlider;
}
class QAbstractItemModel;
class QLabel;
class QPushButton;
class QSortFilterProxyModel;
class QTreeView;


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class PlaylistView : public QWidget
{
public:
  PlaylistView();
  PlaylistView(QAbstractItemModel* model);

  void SetModel(QAbstractItemModel* model);

private:
  void Init();
  QTreeView* view_;
  QSortFilterProxyModel* proxy_;
};

//-----------------------------------------------------------------------------

class PlayerControl : public QWidget
{
  Q_OBJECT
public:
  PlayerControl(Player* player, PlayerHistory* history);

public Q_SLOTS:
  void Progress(QTime progress, QTime total);
  void Status(PlayerState state);

private:
  Player*       player_;
  Phonon::SeekSlider*   progress_;
  Phonon::VolumeSlider* volume_;
  QLabel*       progress_text_;
  QPushButton*  play_pause_;
  QPushButton*  next_;
  QPushButton*  previous_;
  QPushButton*  stop_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif