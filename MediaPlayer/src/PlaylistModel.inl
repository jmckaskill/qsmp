#include "common.h"
#include "PlaylistModel.h"

QSMP_BEGIN
template<class RandomAccessRange>
PlaylistModel<RandomAccessRange>::PlaylistModel(RandomAccessRange path_range)
: paths_ascending_(begin(path_range),end(path_range)),
paths_descending_(begin(path_range),end(path_range)),
paths_(&paths_ascending_)
{
  qsmp::sort(paths_range_t(paths_ascending_),MetadataType_FileName,SortingOrder_Ascending);
  qsmp::sort(paths_range_t(paths_descending_),MetadataType_FileName,SortingOrder_Descending);

}
template<class RandomAccessRange>
QVariant PlaylistModel<RandomAccessRange>::data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const 
{
  QVariant ret;
  if (role == Qt::DisplayRole &&
    index.row() < (int)boost::size(*paths_))
  {
    switch(index.column())
    {
    case 0:
      ret = QString::fromStdString((boost::begin(*paths_) + index.row())->path_.file_string());
      break;
    case 1:
      ret = QString::fromStdString((boost::begin(*paths_) + index.row())->artist_);
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
    return boost::size(*paths_);
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

template<class RandomAccessRange>
void PlaylistModel<RandomAccessRange>::onDoubleClicked(const QModelIndex &index)
{
  itemSelected(QString::fromStdString((boost::begin(*paths_) + index.row())->path_.file_string()));
}

template<class RandomAccessRange>
void PlaylistModel<RandomAccessRange>::sort(int column, Qt::SortOrder order /* = Qt::AscendingOrder */)
{
  switch (column)
  {
  case 0:
    paths_ = (order == Qt::AscendingOrder) ? &paths_ascending_ : &paths_descending_;
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

QSMP_END