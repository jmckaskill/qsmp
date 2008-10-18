#ifndef QSMP_PLAYLISTMODEL
#define QSMP_PLAYLISTMODEL
#include "common.h"

class PlaylistModelBase : public QAbstractTableModel
{
public:
  Q_OBJECT
public slots:
  virtual void onDoubleClicked(const QModelIndex& index)=0;
signals:
  void itemSelected(QString path);
};

template<class RandomAccessRange>
class PlaylistModel : public PlaylistModelBase
{
public:
  PlaylistModel(RandomAccessRange path_range)
    : paths_(path_range)
  {}
  virtual int      rowCount(const QModelIndex& parent = QModelIndex())const;
  virtual int      columnCount(const QModelIndex& parent = QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole)const;

  virtual void onDoubleClicked(const QModelIndex& index);
private:
  RandomAccessRange paths_;
};


#endif