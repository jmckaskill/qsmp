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

#ifndef QSMP_PLAYLISTMODEL_INL_
#define QSMP_PLAYLISTMODEL_INL_

#include <boost/range.hpp>
#include <string>
#include <QtCore/qstring.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class RangeFunction>
PlaylistModel<RangeFunction>::PlaylistModel(RangeFunction get_entries)
: get_entries_(get_entries),
  entries_(get_entries())
{
}

//-----------------------------------------------------------------------------

template<class RangeFunction>
QVariant PlaylistModel<RangeFunction>::data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const 
{
  QVariant ret;
  if (role == Qt::DisplayRole &&
      index.row() < boost::distance(entries_))
  {
    boost::range_iterator<Range>::type ii = boost::begin(entries_);
    std::advance(ii, index.row());

    switch(index.column())
    {
    case 0:
      {
        int queue_index = ii->queue_index();
        if (queue_index >= 0)
          ret = QString::number(queue_index);
      }
      break;
    case 1:
      ret = QString::fromStdString(ii->path().file_string());
      break;
    case 2:
      ret = QString::fromStdString(ii->artist());
      break;
    default:
      break;
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------

template<class RangeFunction>
int PlaylistModel<RangeFunction>::rowCount(const QModelIndex& parent /* = QModelIndex */)const 
{
  if (parent == QModelIndex())
  {
    return boost::distance(entries_);
  }
  else
  {
    return 0;
  }
}

//-----------------------------------------------------------------------------

template<class RangeFunction>
int PlaylistModel<RangeFunction>::columnCount(const QModelIndex& parent /* = QModelIndex */)const 
{
  if (parent == QModelIndex())
  {
    return 3;
  }
  else
  {
    return 0;
  }
}

//-----------------------------------------------------------------------------

template<class RangeFunction>
void PlaylistModel<RangeFunction>::onDoubleClicked(const QModelIndex &index)
{
 // itemSelected(QString::fromStdString((boost::begin(*paths_) + index.row())->path_.file_string()));
}

//-----------------------------------------------------------------------------

template<class RangeFunction>
void PlaylistModel<RangeFunction>::sort(int column, Qt::SortOrder order /* = Qt::AscendingOrder */)
{
  switch (column)
  {
  case 0:
    //paths_ = (order == Qt::AscendingOrder) ? &paths_ascending_ : &paths_descending_;
    //smp::sort(paths_,MetadataType_FileName,sortingOrder(order));
    break;
  case 1:
    //smp::sort(paths_,MetadataType_Artist,sortingOrder(order));
    break;
  default:
    break;
  }
  reset();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif