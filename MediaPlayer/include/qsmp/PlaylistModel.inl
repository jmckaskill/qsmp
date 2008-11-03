#include "common.h"
#include "PlaylistModel.h"

QSMP_BEGIN
template<class RangeFunction>
PlaylistModel<RangeFunction>::PlaylistModel(RangeFunction get_entries)
: get_entries_(get_entries),
  entries_(get_entries())
{
}
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

template<class RangeFunction>
void PlaylistModel<RangeFunction>::onDoubleClicked(const QModelIndex &index)
{
 // itemSelected(QString::fromStdString((boost::begin(*paths_) + index.row())->path_.file_string()));
}

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

QSMP_END