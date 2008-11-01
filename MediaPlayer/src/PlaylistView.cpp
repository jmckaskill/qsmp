#include "PlaylistView.h"

#include "Player.h"

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlaylistView::PlaylistView()
{
  Init();
}

//-----------------------------------------------------------------------------

PlaylistView::PlaylistView(QAbstractItemModel* model)
{
  Init();
  SetModel(model);
}

//-----------------------------------------------------------------------------

void PlaylistView::Init()
{
  proxy_ = new QSortFilterProxyModel;
  proxy_->setFilterKeyColumn(-1);
  proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);

  view_ = new QTreeView;
  view_->setModel(proxy_);
  view_->setAlternatingRowColors(true);
  view_->setUniformRowHeights(true);
  //view_->setSortingEnabled(true);
  view_->header()->setMovable(true);
  view_->header()->setClickable(true);
  view_->setDragEnabled(true);
  view_->setDropIndicatorShown(true);
  view_->setDragDropMode(QAbstractItemView::InternalMove);
  view_->setRootIsDecorated(false);
  view_->setAlternatingRowColors(true);
  view_->setSelectionMode(QAbstractItemView::ExtendedSelection);

  QLineEdit* filter_line = new QLineEdit;
  QObject::connect(filter_line, SIGNAL(textChanged(const QString &)),
                   proxy_, SLOT(setFilterRegExp(const QString &)));

  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(filter_line);
  layout->addWidget(view_);
  layout->setSpacing(0);
  layout->setContentsMargins(0,0,0,0);
}

//-----------------------------------------------------------------------------

void PlaylistView::SetModel(QAbstractItemModel *model)
{
  proxy_->setSourceModel(model);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlayerControl::PlayerControl(Player *player, PlayerHistory *history)
: progress_(new Phonon::SeekSlider(player->media())),
  progress_text_(new QLabel),
  play_pause_(new QPushButton),
  next_(new QPushButton(tr("Next"))),
  previous_(new QPushButton(tr("Previous"))),
  stop_(new QPushButton(tr("Stop"))),
  volume_(new Phonon::VolumeSlider(player->audio()))
{
  QHBoxLayout* layout = new QHBoxLayout;
  setLayout(layout);

  layout->addWidget(stop_);
  layout->addWidget(previous_);
  layout->addWidget(play_pause_);
  layout->addWidget(next_);
  layout->addWidget(progress_);
  layout->addWidget(progress_text_);
  layout->addWidget(volume_);

  connect(next_, SIGNAL(clicked()), history, SLOT(Next()));
  connect(previous_, SIGNAL(clicked()), history, SLOT(Previous()));
  Status(player->status());
  connect(player, SIGNAL(OnStatus(Phonon::State)), this, SLOT(Status(Phonon::State)));
}

//-----------------------------------------------------------------------------

void PlayerControl::Status(Phonon::State state)
{
  switch(state)
  {
  case Phonon::PlayingState:
    play_pause_->setText(tr("Pause"));
    break;
  case Phonon::PausedState:
    play_pause_->setText(tr("Play"));
    break;
  case Phonon::StoppedState:
  default:
    play_pause_->setText(tr("Play"));
    break;
  }
}

//-----------------------------------------------------------------------------

void PlayerControl::Progress(QTime progress, QTime total)
{
  //progress_->setSliderPosition(int(double(progress.msec()) / double(total.msec()) * 1000 + 0.5));
  progress_text_->setText(tr("%1 / %2").arg(progress.toString(tr("m:ss"))).arg(total.toString(tr("m:ss"))));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


QSMP_END
