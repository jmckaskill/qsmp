#ifndef QSMP_PLAYLISTMODEL
#define QSMP_PLAYLISTMODEL
#include "common.h"

QSMP_BEGIN

class PlaylistModelBase : public QAbstractTableModel
{
  Q_OBJECT
public Q_SLOTS:
  virtual void onDoubleClicked(const QModelIndex& index)=0;
Q_SIGNALS:
  void itemSelected(QString path);
};

template<class RandomAccessRange>
class PlaylistModel : public PlaylistModelBase
{
public:
  PlaylistModel(RandomAccessRange path_range);
  virtual int      rowCount(const QModelIndex& parent = QModelIndex())const;
  virtual int      columnCount(const QModelIndex& parent = QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole)const;

  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  virtual void onDoubleClicked(const QModelIndex& index);
private:
  typedef std::vector<typename boost::range_value<RandomAccessRange>::type> paths_t;
  typedef boost::iterator_range<typename paths_t::iterator> paths_range_t; 
  paths_t* paths_;
  paths_t paths_ascending_;
  paths_t paths_descending_;
};

QSMP_END


#endif