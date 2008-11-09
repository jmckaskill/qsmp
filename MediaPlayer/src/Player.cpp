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
#include <qsmp/Player.h>
#include <qsmp/Player.moc>

#include <qsmp/Log.h>
#include <qsmp/utilities.h>


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Player::Player(boost::function<Media ()> get_next)
: get_next_(get_next),
audio_(Phonon::MusicCategory),
file_active_(false)
{
  QSMP_LOG("Player") << "Initialising";
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
      QSMP_LOG("Player") << "Play: " << entry;
      media_.setCurrentSource(QString::fromStdString(entry.path().file_string()));
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
    QSMP_LOG("Player") << "Play";
    media_.play();
  }
}

//-----------------------------------------------------------------------------

void Player::Pause()
{
  if (file_active_)
  {
    QSMP_LOG("Player") << "Pause";
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
    QSMP_LOG("Player") << "Stop";
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
  QSMP_LOG("Player") << "Status changed: " << oldState << " -> " << newState;
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
    std::string next = get_next_().path().file_string();
    QSMP_LOG("Player") << "Getting next: " << next;
    media_.enqueue(QString::fromStdString(next));
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlayerHistory::PlayerHistory()
: next_enqueued_(false),
current_played_(false)
{
  Init();
}

//-----------------------------------------------------------------------------

PlayerHistory::PlayerHistory(Player* player)
: next_enqueued_(false),
current_played_(false)
{
  Init();
  SetPlayer(player);
}

//-----------------------------------------------------------------------------

void PlayerHistory::Init()
{
  //We insert a dummy first entry, which makes the next/previous logic
  //much simpler since we can insert an item at the end of the list and even
  //when the list is empty, the current is before this
  cache_.push_back(Media());
  current_ = cache_.begin();
  queue_end_ = current_;
  QSMP_LOG("History") << "Initialising";
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetPlayer(Player* player)
{
  connect(this, SIGNAL(OnPlayFile(Media,bool)), player, SLOT(PlayFile(Media,bool)));
  connect(player, SIGNAL(OnStatus(PlayerState)), this, SLOT(Status(PlayerState)));
  connect(player, SIGNAL(OnSourceChanged()), this, SLOT(SourceChanged()));
  UpdateCurrent(current_,true,false);//force through the current item to the player
}

//-----------------------------------------------------------------------------

void PlayerHistory::PlayFile(const Media& entry)
{
  cache_t::iterator i = current_;
  if (i != cache_.end())
    ++i;

  cache_.insert(i,entry);
  Next(true);
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetNextCallback(boost::function<Media ()> callback)
{
  get_next_ = callback;
  InvalidateCache();
}

//-----------------------------------------------------------------------------

void PlayerHistory::InvalidateCache()
{
  QSMP_LOG("History") << "Cache invalidated";
  cache_t::iterator cache_begin = queue_end_;
  if (cache_begin == cache_.begin() ||
    ( current_ == queue_end_ && 
    current_played_ && 
    cache_begin != cache_.end()
    )
    )
  {
    ++cache_begin;
  }
  bool current_erased = (cache_begin == current_);
  bool queue_end_erased = current_erased || (cache_begin == queue_end_);
  cache_.erase(cache_begin,cache_.end());

  if (queue_end_erased)
  {
    queue_end_ = cache_.end();
  }
  if (current_erased)
  {
    UpdateCurrent(cache_.end(),true,false);
  }
  OnHistoryUpdated();
}

//-----------------------------------------------------------------------------

Media PlayerHistory::Next(bool force_play)
{
  QSMP_LOG("History") << "Next";
  return UpdateCurrent(GetNext(current_),true,force_play);
}

//-----------------------------------------------------------------------------

Media PlayerHistory::Previous(bool force_play)
{
  QSMP_LOG("History") << "Previous";
  cache_t::iterator i = current_;
  if (i != cache_.begin())
    --i;
  return UpdateCurrent(i,true,force_play);
}

//-----------------------------------------------------------------------------

Media PlayerHistory::UpdateCurrent(cache_t::iterator new_current, bool send_play, bool force_play)
{
  Media ret;

  //Pre change
  bool shift_queue = (current_ == queue_end_);
  if (current_ != cache_.end() && current_->valid())
    current_->set_queue_index(-1);

  current_ = new_current;

  //Post change
  if (current_ != cache_.end() && current_->valid())
  {
    current_->set_queue_index(0);
    ret = *current_;
  }
  if (shift_queue)
    queue_end_ = current_;

  //Update the queue_index's
  cache_t::iterator ii;
  int i = 0;
  for(ii = current_; ii != queue_end_; ++ii)
  {
    if (ii->valid())
      ii->set_queue_index(i++);
  }

  if (send_play)
  {
    current_played_ = false;
    next_enqueued_ = false;
  }

  OnHistoryUpdated();
  if (send_play)
    OnPlayFile(ret, force_play);
  return ret;
}

//-----------------------------------------------------------------------------

PlayerHistory::cache_t::iterator PlayerHistory::GetNext(cache_t::iterator ii)
{
  ++ii;
  if (ii == cache_.end())
  {
    Media next = get_next_.empty() ? Media() : get_next_();
    if (next.valid())
    {
      cache_.push_back(next);
      --ii;
    }
  }
  return ii;
}

//-----------------------------------------------------------------------------

void PlayerHistory::SourceChanged()
{
  if (next_enqueued_)
  {
    next_enqueued_ = false;

    UpdateCurrent(GetNext(current_),false,false);
  }
  current_played_ = true;
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

Media PlayerHistory::PlayerNext()
{
  bool inc_current = (current_played_ || (current_ == cache_.end()) || !current_->valid());
  next_enqueued_ = inc_current;
  cache_t::iterator i = inc_current ? GetNext(current_) : current_;
  return (i != cache_.end()) ? *i : Media();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END
