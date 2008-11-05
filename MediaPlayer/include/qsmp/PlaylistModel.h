#ifndef QSMP_PLAYLISTMODEL_H_
#define QSMP_PLAYLISTMODEL_H_
#include "qsmp/common.h"

QSMP_BEGIN

class PlaylistModelBase : public QAbstractTableModel
{
  Q_OBJECT
public Q_SLOTS:
  virtual void onDoubleClicked(const QModelIndex& index)=0;
  virtual void Reset()=0;
Q_SIGNALS:
  void itemSelected(QString path);
};

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

template<class RangeFunction>
shared_ptr<PlaylistModel<RangeFunction> > NewPlaylist(RangeFunction function)
{return shared_ptr<PlaylistModel<RangeFunction> >(new PlaylistModel<RangeFunction>(function));}

QSMP_END

#include "PlaylistModel.inl"

#endif