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
#include <qsmp_gui/Player.h>
#include <qsmp_gui/Player.moc>

#include <qsmp_gui/utilities.h>
#include <qsmp_lib/Log.h>


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Player::Player(boost::function<Media ()> get_next)
: get_next_(get_next),
audio_(Phonon::MusicCategory),
file_active_(false)
{
  LOG("Player") << "Initialising";
  audio_path_ = Phonon::createPath(&media_,&audio_);
  connect(&media_, SIGNAL(aboutToFinish()), this, SLOT(EnqueueNext()));
  connect(&media_, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)), this, SIGNAL(OnSourceChanged()));
  connect(&media_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(StatusChanged(Phonon::State, Phonon::State)));
}

//-----------------------------------------------------------------------------

void Player::PlayFile(const Media& entry, bool play_file)
{
  media_.clearQueue();
  if (entry.valid())
  {
    if (play_file || file_active_)
    {
      LOG("Player") << "Play: " << entry;
      media_.setCurrentSource(entry.path());
      media_.play();
      file_active_ = true;
    }
  }
  else
  {
    Stop();
  }
}

//-----------------------------------------------------------------------------

void Player::Play()
{
  if (!file_active_)
  {
    if (!get_next_.empty())
      PlayFile(get_next_());
  }
  else
  {
    LOG("Player") << "Play";
    media_.play();
  }
}

//-----------------------------------------------------------------------------

void Player::Pause()
{
  if (file_active_)
  {
    LOG("Player") << "Pause";
    media_.pause();
  }
}

//-----------------------------------------------------------------------------

void Player::PlayPause()
{
  if (!file_active_)
  {
    Play();
  }
  else
  {
    switch (status())
    {
    case Phonon::PlayingState:
      Pause();
      break;
    case Phonon::PausedState:
    case Phonon::StoppedState:
      Play();
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------------------------------------

void Player::Stop()
{
  if (file_active_)
  {
    LOG("Player") << "Stop";
    file_active_ = false;
    media_.stop();
  }
}

//-----------------------------------------------------------------------------

void Player::SetVolume(int volume)
{
  audio_.setVolume(double(volume) / 1000.0);
}

//-----------------------------------------------------------------------------

namespace
{
  template<class CharT, class traits>
  std::basic_ostream<CharT, traits>& operator<<(std::basic_ostream<CharT,traits>& stream, Phonon::State state)
  {
    switch(state)
    {
    case Phonon::LoadingState:
      stream << "Loading";
      break;
    case Phonon::StoppedState:
      stream << "Stopped";
      break;
    case Phonon::PlayingState:
      stream << "Playing";
      break;
    case Phonon::BufferingState:
      stream << "Buffering";
      break;
    case Phonon::PausedState:
      stream << "Paused";
      break;
    case Phonon::ErrorState:
      stream << "Error";
      break;
    }
    return stream;
  }
}
//-----------------------------------------------------------------------------

void Player::StatusChanged(Phonon::State newState, Phonon::State oldState)
{
  LOG("Player") << "Status changed: " << oldState << " -> " << newState;
  switch (newState)
  {
  case Phonon::PlayingState:
    OnStatus(PlayerState_Playing);
    break;
  case Phonon::PausedState:
    OnStatus(PlayerState_Paused);
    break;
  case Phonon::StoppedState:
    OnStatus(PlayerState_Stopped);
    break;
  case Phonon::LoadingState:
  case Phonon::BufferingState:
    break;//ignore intermediatary loading states for now
  default:
    OnStatus(PlayerState_Invalid);
    break;
  }
}

//-----------------------------------------------------------------------------

void Player::EnqueueNext()
{
  if (!get_next_.empty())
  {
    QString next = get_next_().path();
    LOG("Player") << "Getting next: " << next;
    media_.enqueue(next);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlayerHistory::PlayerHistory()
{
  Init();
}

//-----------------------------------------------------------------------------

PlayerHistory::PlayerHistory(Player* player)
{
  Init();
  SetPlayer(player);
}

//-----------------------------------------------------------------------------

void PlayerHistory::Init()
{
  current_        = 0;
  queue_end_      = 0;
  next_enqueued_  = false;
  current_played_ = false;
  LOG("History") << "Initialising";
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetNextCallback(boost::function<Media ()> callback)
{
  bool init = get_next_.empty();
  get_next_ = callback;
  ResetCache();
  if (init)
  {
    //Force filling out of the current item
    queue_end_++;
    Lookup(current_);
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetPlayer(Player* player)
{
  connect(this, SIGNAL(OnPlayFile(Media,bool)), player, SLOT(PlayFile(Media,bool)));
  connect(player, SIGNAL(OnStatus(PlayerState)), this, SLOT(Status(PlayerState)));
  connect(player, SIGNAL(OnSourceChanged()), this, SLOT(SourceChanged()));
  OnPlayFile(Lookup(current_),false);//force through the current item to the player
}

//-----------------------------------------------------------------------------

void PlayerHistory::PlayFile(const Media& entry)
{
  LOG("History") << "PlayFile";
  if (entry == Lookup(current_))
  {
    //There is a bit of a special case if the current item has been re-requested
    //Causes it to play if we are stopped but otherwise ignores it
    if (!current_played_)
      OnPlayFile(Lookup(current_), true);
  }
  else
  {
    InsertToQueue(1, entry);
    Next(true);
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::InsertToQueue(int index, const Media& entry)
{
  //Note you can't remove the current (queue index 0) via this method
  if (0 < index && index <= (int)queue_size())
  {
    cache_.insert(cache_.begin() + index + current_, entry);
    queue_end_++;
    OnQueueInsert(index, entry);
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::RemoveFromQueue(int index)
{
  //Note you can't remove the current (queue index 0) via this method
  if (0 < index && index < (int) queue_size())
  {
    cache_.erase(cache_.begin() + index + current_);
    queue_end_--;
    OnQueueRemove(index);
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::RemoveFromCache(int index)
{
  if (0 <= index && index < (int)cache_size())
  {
    cache_.erase(cache_.begin() + index + queue_end_);
    OnCacheRemove(index);
  }
}

//-----------------------------------------------------------------------------

Media PlayerHistory::Next(bool force_play)
{
  LOG("History") << "Next";
  NextUpdate();
  OnPlayFile(Lookup(current_), force_play);
  return Lookup(current_);
}

//-----------------------------------------------------------------------------

Media PlayerHistory::Previous(bool force_play)
{
  LOG("History") << "Previous";
  PreviousUpdate();
  OnPlayFile(Lookup(current_), force_play);
  return Lookup(current_);
}

//-----------------------------------------------------------------------------

void PlayerHistory::SourceChanged()
{
  if (next_enqueued_ && current_played_)
  {
    NextUpdate();
  }
  next_enqueued_ = false;
  current_played_ = true;

  LOG("History") << "Playing: " << Lookup(current_).path();
}

//-----------------------------------------------------------------------------

void PlayerHistory::Status(PlayerState state)
{
  switch(state)
  {
  case PlayerState_Stopped:
    {
      current_played_ = false;
    }
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::ResetCache()
{
  LOG("History") << "Cache invalidated";

  size_t to_remove = cache_size();
  cache_.erase(cache_.begin() + queue_end_, cache_.end());

  OnCacheReset();

}

//-----------------------------------------------------------------------------

void PlayerHistory::PreviousUpdate()
{
  //1. This pushes the current into the queue (if had entries other than the old current) or the forward history
  //2. It then pulls the tail out of the back history and puts it into head of the queue (current)

  if (current_ > 0)
  {
    current_--;
    current_played_ = false;
    next_enqueued_  = false;

    //Remove the tail from back history
    OnHistoryRemove(-1);
    //Insert it at the head of the queue
    OnQueueInsert(0, Lookup(current_));

    //An empty queue has only old and new currents
    if (queue_end_ - current_ == 2)
    {
      //Remove the old current
      queue_end_--;
      OnQueueRemove(1);
      OnCacheInsert(0, Lookup(current_+1));
    }
  }
  else
  {
    OnPlayFile(Media(), false);
    current_played_ = false;
    next_enqueued_  = false;
  }
}

//-----------------------------------------------------------------------------

void PlayerHistory::NextUpdate()
{
  //1. This pushes the current into the back history
  //2. It either switches to the next item in the queue or pulls an item out of the forward history
  //   and into the first spot of the queue (current item)

  current_++;
  current_played_ = false;
  next_enqueued_  = false;

  //We insert the old current at the tail of the history
  OnHistoryInsert(0, Lookup(current_ - 1));
  //We remove the old current out of the head of the queue
  OnQueueRemove(0);

  if (queue_end_ == current_)
  {
    //Queue only had the old current item in it, so we pull the next item out of the forward history
    queue_end_++;
    OnQueueInsert(0, Lookup(current_));
    OnCacheRemove(0);
  }
}

//-----------------------------------------------------------------------------

Media PlayerHistory::GetPlayerNext()
{
  int next = current_;
  if (current_played_)
    next++;
  next_enqueued_ = true;
  return Lookup(next);
}

//-----------------------------------------------------------------------------

Media PlayerHistory::Lookup(int index)
{
  if (index < (int)cache_.size())
    return cache_[index];

  cache_.reserve(index+1);
  while((int)cache_.size() <= index)
  {
    cache_.push_back(get_next_());
  }
  return cache_[index];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END
