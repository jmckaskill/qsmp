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
#include <qsmp_gui/PlaylistView.h>
#include <qsmp_gui/PlaylistView.moc>

#include "qsmp_gui/Player.h"
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qlabel.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qsortfilterproxymodel.h>
#include <QtGui/qtreeview.h>
#include <QtGui/qboxlayout.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QVariant HistoryModelNode::data(int column, int role)const
{
  if (role == Qt::DisplayRole)
  {
    switch(column)
    {
    case 0:
      return queue_field_;
    case 1:
      return media_.path();
    }
  }
  return QVariant();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

HistoryModel::HistoryModel(PlayerHistory* history, size_t back_cache, size_t forward_cache)
: Base(this),
  history_(history),
  back_cache_(back_cache),
  forward_cache_(forward_cache),
  current_(0)
{
  //Insert the current queue
  queue_size_ = history_->queue_size(); //Add one extra for current

  root()->BeginAddChildren(queue_size_);
  HistoryModelNode* node = root()->AddChildren(queue_size_);
  for(int i = 0; i < queue_size_; i++)
  {
    Media media = history_->LookupQueue(i);
    node[i].media_       = media;
    if (i == 0)
      node[i].queue_field_ = '*';
    else
      node[i].queue_field_.setNum(i);
  }
  HistoryModelNode::EndAddChildren(this);

  CheckCache();

  connect(history, SIGNAL(OnHistoryInsert(int,const Media&)), this, SLOT(HistoryInsert(int,const Media&)));
  connect(history, SIGNAL(OnHistoryRemove(int)), this, SLOT(HistoryRemove(int)));
  connect(history, SIGNAL(OnQueueInsert(int,const Media&)), this, SLOT(QueueInsert(int,const Media&)));
  connect(history, SIGNAL(OnQueueRemove(int)), this, SLOT(QueueRemove(int)));
  connect(history, SIGNAL(OnCacheInsert(int,const Media&)), this, SLOT(CacheInsert(int,const Media&)));
  connect(history, SIGNAL(OnCacheRemove(int)), this, SLOT(CacheRemove(int)));
  connect(history, SIGNAL(OnCacheReset()), this, SLOT(CacheReset()));
}

//-----------------------------------------------------------------------------

void HistoryModel::HistoryInsert(int begin, const Media& media)
{
  //The current item is the zero point for the history with begin being negative
  int local_begin = current_ + begin;
  if (local_begin >= 0 && begin <= 0)
  {
    //Insert the new section
    root()->BeginInsertChildren(local_begin, 1);
    HistoryModelNode* node = root()->InsertChildren(local_begin, 1);
    node->media_       = media;

    HistoryModelNode::EndInsertChildren(this);

    current_++;

    CheckCache();
  }
  FLOG("HistoryModel", "HistoryInsert: %1% %2%") % begin % media.path();
}

//-----------------------------------------------------------------------------

void HistoryModel::HistoryRemove(int begin)
{
  int local_begin = current_ + begin;

  if (local_begin >= 0)
  {
    root()->RemoveChildren(local_begin, 1);

    current_--;

    CheckCache();
  }
  FLOG("HistoryModel", "HistoryRemove: %1%") % begin;
}

//-----------------------------------------------------------------------------

void HistoryModel::QueueInsert(int begin, const Media& media)
{
  int local_begin = current_ + begin;

  //Insert the new section
  root()->BeginInsertChildren(local_begin, 1);
  HistoryModelNode* node = root()->InsertChildren(local_begin, 1);
  node->media_       = media;
  if (begin == 0)
    node->queue_field_ = '*';
  else
    node->queue_field_.setNum(begin);

  HistoryModelNode::EndInsertChildren(this);

  queue_size_++;

  //Update the queue indices after the insert
  int update_begin = local_begin + 1;
  int update_end = current_ + queue_size_;
  HistoryModelNode* r = root();
  for(int i = update_begin; i < update_end; i++)
  {
    r->child(i)->queue_field_.setNum(i - current_);
  }
  if (update_begin < update_end)
    r->ChildrenUpdated(update_begin, update_end - update_begin);

  FLOG("HistoryModel", "QueueInsert: %1% %2%") % begin % media.path();
}

//-----------------------------------------------------------------------------

void HistoryModel::QueueRemove(int begin)
{
  int local_begin = current_ + begin;

  //Remove the section
  root()->RemoveChildren(local_begin, 1);

  queue_size_--;

  //Update the queue indices after the remove section
  int update_begin = local_begin;
  int update_end = current_ + queue_size_;
  HistoryModelNode* r = root();
  for(int i = update_begin; i < update_end; i++)
  {
    if (i == current_)
      r->child(i)->queue_field_ = '*';
    else
      r->child(i)->queue_field_.setNum(i - current_);
  }
  if (update_begin < update_end)
    r->ChildrenUpdated(update_begin, update_end - update_begin);
  FLOG("HistoryModel", "QueueRemove: %1%") % begin;
}

//-----------------------------------------------------------------------------

void HistoryModel::CacheInsert(int begin, const Media& media)
{
  int local_begin = current_ + queue_size_ + begin;
  int total = root()->rowCount();
  if (local_begin <= total)
  {
    root()->BeginInsertChildren(local_begin, 1);
    HistoryModelNode* node = root()->InsertChildren(local_begin, 1);
    node->media_ = media;

    HistoryModelNode::EndInsertChildren(this);
  }
  FLOG("HistoryModel", "CacheInsert: %1% %2%") % begin % media.path();
}

//-----------------------------------------------------------------------------

void HistoryModel::CacheRemove(int begin)
{
  int local_begin = current_ + queue_size_ + begin;

  int total = root()->rowCount();

  if (local_begin < total)
  {
    //Remove the section
    root()->RemoveChildren(local_begin, 1);

    CheckCache();
  }
  FLOG("HistoryModel", "CacheRemove: %1%") % begin;
}

//-----------------------------------------------------------------------------

void HistoryModel::CacheReset()
{
  int total = root()->rowCount();
  int queue_end = current_ - queue_size_;
  if (total - queue_end > 0)
    root()->RemoveChildren(queue_end, total - queue_end);

  CheckCache();
}

//-----------------------------------------------------------------------------

void HistoryModel::CheckCache()
{
  //Check to see if we need to add/remove items from the back cache
  if (current_ < (int)back_cache_)
  {
    int have  = current_;
    int want  = back_cache_;
    int history_has = history_->history_size();
    int to_get = std::min(history_has, want) - have;
    int history_begin = -have - to_get;
    if (to_get > 0)
    {
      root()->BeginInsertChildren(0, to_get);
      HistoryModelNode* node = root()->InsertChildren(0, to_get);
      for(int i = 0; i < to_get; i++)
      {
        Media media = history_->LookupHistory(history_begin + i);
        node[i].media_ = media;
      }
      HistoryModelNode::EndInsertChildren(this);
      current_ += to_get;
    }
  }
  else if (current_ > (int)back_cache_)
  {
    root()->RemoveChildren(0, current_ - back_cache_);
    current_ = back_cache_;
  }


  //Check to see if we need to add/remove items from the forward cache
  int total = root()->rowCount();
  int have = total - current_ - queue_size_;
  int want = forward_cache_;
  if (want > have)
  {
    //Need to add some
    int begin = total;//insert at end
    int to_get = want - have;
    int cache_begin = have;
    root()->BeginInsertChildren(begin, to_get);
    HistoryModelNode* node = root()->InsertChildren(begin, to_get);
    for(int i = 0; i < to_get; i++)
    {
      Media media = history_->LookupCache(cache_begin + i);
      node[i].media_ = media;
    }
    HistoryModelNode::EndInsertChildren(this);
  }
  else if (have > want)
  {
    //Need to remove some
    int begin = current_ + queue_size_ + want;
    int count = total - begin;
    root()->RemoveChildren(begin, count);
  }
}

//-----------------------------------------------------------------------------

void HistoryModel::DoubleClicked(const QModelIndex& index)
{
  //Now double clicking an item involves
  //1. Setting up the item to be played
  //2. Removing the item from the queue or forward cache but not the back cache

  //Lets do the remove first since the playing will result in a callback which inserts an item
  //and thus index is no longer valid
  //Need to save out the media since the entry will soon be deleted
  Media media = FromIndex(index)->media_;

  if (current_ <= index.row() && index.row() < current_ + queue_size_)
    history_->RemoveFromQueue(index.row() - current_);
  else if (index.row() > current_ + queue_size_)
    history_->RemoveFromCache(index.row() - current_ - queue_size_);

  //Now we play the new file
  //Note if this happens to be the current item, the player should do the right thing
  history_->PlayFile(media);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

HistoryView::HistoryView(PlayerHistory* history)
: model_(history, 5, 4)
{
  setModel(&model_);
  //setAlternatingRowColors(true);
  //setUniformRowHeights(true);
  //header()->setMovable(true);
  //header()->setClickable(true);
  //setDragEnabled(true);
  //setDropIndicatorShown(true);
  //setDragDropMode(QAbstractItemView::InternalMove);
  //setRootIsDecorated(false);
  //setAlternatingRowColors(true);
  //setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(this, SIGNAL(doubleClicked(const QModelIndex&)), &model_, SLOT(DoubleClicked(const QModelIndex&)));
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
