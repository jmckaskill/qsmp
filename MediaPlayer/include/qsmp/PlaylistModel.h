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

#ifndef QSMP_PLAYLISTMODEL_H_
#define QSMP_PLAYLISTMODEL_H_

#include <qsmp/common.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtCOre/qvariant.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class PlaylistModelBase : public QAbstractTableModel
{
  Q_OBJECT
public Q_SLOTS:
  virtual void onDoubleClicked(const QModelIndex& index)=0;
  virtual void Reset()=0;
Q_SIGNALS:
  void itemSelected(QString path);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class RangeFunction>
class PlaylistModel : public PlaylistModelBase
{
public:
  PlaylistModel(RangeFunction get_entries);
  virtual int      rowCount(const QModelIndex& parent = QModelIndex())const;
  virtual int      columnCount(const QModelIndex& parent = QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole)const;

  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
  virtual void Reset()
  {
    entries_ = get_entries_();
    QAbstractTableModel::reset();
  }
  virtual void onDoubleClicked(const QModelIndex& index);
private:
  typedef typename RangeFunction::result_type Range;
  RangeFunction get_entries_;
  Range         entries_;
};

//-----------------------------------------------------------------------------

template<class RangeFunction>
boost::shared_ptr<PlaylistModel<RangeFunction> > NewPlaylist(RangeFunction function)
{return boost::shared_ptr<PlaylistModel<RangeFunction> >(new PlaylistModel<RangeFunction>(function));}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#include "PlaylistModel.inl"

#endif