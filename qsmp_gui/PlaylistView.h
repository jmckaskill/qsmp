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

#include <qsmp_gui/common.h>
#include <qsmp_gui/Player.h>
#include <qsmp_gui/TreeModel.h>
#include <QtCore/qdatetime.h>
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

class HistoryModel;

//-----------------------------------------------------------------------------

class HistoryModelNode : public Model::TreeModelNode<HistoryModelNode, HistoryModel>
{
public:
  HistoryModelNode(HistoryModel* model)
    : TreeModelNode(model)
  {}

  int       columnCount()const{return 2;}
  QVariant  data(int column, int role)const;

private:
  friend class HistoryModel;
  QString   queue_field_;
  Media     media_;
};

//-----------------------------------------------------------------------------

class HistoryModel : public Model::TreeModel<HistoryModelNode, HistoryModel>
{
  Q_OBJECT
public:
  HistoryModel(PlayerHistory* history, size_t back_cache, size_t forward_cache);

public Q_SLOTS:
  void  HistoryInsert(int begin, const Media& media);
  void  HistoryRemove(int begin);

  void  QueueInsert(int begin, const Media& media);
  void  QueueRemove(int begin);

  void  CacheInsert(int begin, const Media& media);
  void  CacheRemove(int begin);
  void  CacheReset();

  void  DoubleClicked(const QModelIndex& index);
private:
  void              CheckCache();
  int               queue_size_;
  int               current_;
  PlayerHistory*    history_;
  size_t            back_cache_;
  size_t            forward_cache_;
};

//-----------------------------------------------------------------------------

class HistoryView : public QTreeView
{
public:
  HistoryView(PlayerHistory* history);
private:
  HistoryModel model_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
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