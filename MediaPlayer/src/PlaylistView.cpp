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

#include "stdafx.h"
#include <qsmp/PlaylistView.h>
#include <qsmp/PlaylistView.moc>

#include "qsmp/Player.h"
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qlabel.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qsortfilterproxymodel.h>
#include <QtGui/qtreeview.h>

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
  QVBoxLayout* vLayout = new QVBoxLayout;
  setLayout(vLayout);

  QHBoxLayout* hLayout1 = new QHBoxLayout;
  hLayout1->addWidget(previous_);
  hLayout1->addWidget(stop_);
  hLayout1->addWidget(play_pause_);
  hLayout1->addWidget(next_);
  hLayout1->addWidget(volume_);

  QHBoxLayout* hLayout2 = new QHBoxLayout;
  hLayout2->addWidget(progress_);
  hLayout2->addWidget(progress_text_);

  vLayout->addLayout(hLayout1);
  vLayout->addLayout(hLayout2);

  connect(next_, SIGNAL(clicked()), history, SLOT(Next()));
  connect(previous_, SIGNAL(clicked()), history, SLOT(Previous()));
  connect(play_pause_, SIGNAL(clicked()), player, SLOT(PlayPause()));
  connect(stop_, SIGNAL(clicked()), player, SLOT(Stop()));
  connect(player, SIGNAL(OnStatus(PlayerState)), this, SLOT(Status(PlayerState)));

  Status(PlayerState_Invalid);
}

//-----------------------------------------------------------------------------

void PlayerControl::Status(PlayerState state)
{
  switch(state)
  {
  case PlayerState_Playing:
    play_pause_->setText(tr("Pause"));
    break;
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
