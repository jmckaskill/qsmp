#ifndef QSMP_PLAYLISTVIEW_H_
#define QSMP_PLAYLISTVIEW_H_

#include "qsmp/common.h"
#include "qsmp/Player.h"

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class PlaylistView : public QWidget
{
public:
  PlaylistView();
  PlaylistView(QAbstractItemModel* model);

  void SetModel(QAbstractItemModel* model);

private:
  void Init();
  QTreeView* view_;
  QSortFilterProxyModel* proxy_;
};

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