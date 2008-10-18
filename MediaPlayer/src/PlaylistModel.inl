#include "common.h"
#include "PlaylistModel.h"

template<class RandomAccessRange>
QVariant PlaylistModel<RandomAccessRange>::data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const 
{
  QVariant ret;
  if (role == Qt::DisplayRole &&
      index.row() < (int)boost::size(paths_))
  {
    switch(index.column())
    {
    case 0:
      ret = QString::fromStdString((boost::begin(paths_) + index.row())->path_.file_string());
      break;
    case 1:
      ret = QString::fromStdString((boost::begin(paths_) + index.row())->artist_);
      break;
    default:
      break;
    }
  }
  return ret;
}

template<class RandomAccessRange>
int PlaylistModel<RandomAccessRange>::rowCount(const QModelIndex& parent /* = QModelIndex */)const 
{
  if (parent == QModelIndex())
  {
    return boost::size(paths_);
  }
  else
  {
    return 0;
  }
}

template<class RandomAccessRange>
int PlaylistModel<RandomAccessRange>::columnCount(const QModelIndex& parent /* = QModelIndex */)const 
{
  if (parent == QModelIndex())
  {
    return 2;
  }
  else
  {
    return 0;
  }
}