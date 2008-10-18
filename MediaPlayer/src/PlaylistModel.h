#ifndef QSMP_PLAYLISTMODEL
#define QSMP_PLAYLISTMODEL


template<class RandomAccessRange>
class PlaylistModel : public QAbstractTableModel
{
public:
  PlaylistModel(RandomAccessRange path_range)
    : paths_(path_range)
  {}
  virtual int      rowCount(const QModelIndex& parent = QModelIndex())const;
  virtual int      columnCount(const QModelIndex& parent = QModelIndex())const;
  virtual QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole)const;
private:
  RandomAccessRange paths_;
};


#endif