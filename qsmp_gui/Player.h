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

#ifndef QSMP_PLAYER_H_
#define QSMP_PLAYER_H_

#include <boost/function.hpp>
#include <iterator>
#include <list>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <qsmp_gui/common.h>
#include <qsmp_gui/utilities.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qobject.h>


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum PlayerState
{
  PlayerState_Playing,
  PlayerState_Paused,
  PlayerState_Stopped,
  PlayerState_Invalid
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Player : public QObject
{
  Q_OBJECT
public:
  Player(boost::function<Media ()> get_next = boost::function<Media ()>());

  Phonon::State        status()const{return media_.state();}
  Phonon::AudioOutput* audio(){return &audio_;}
  Phonon::MediaObject* media(){return &media_;}

public Q_SLOTS:
  void PlayFile(const Media& entry, bool play_file = true);
  void Play();
  void Pause();
  void PlayPause();
  void Stop();
  //Volume is from 0-1000
  void SetVolume(int volume);
  void StatusChanged(Phonon::State, Phonon::State);

Q_SIGNALS:
  void OnProgress(QTime progress, QTime total);
  void OnStatus(PlayerState state);
  void OnSourceChanged();

private Q_SLOTS:
  void EnqueueNext();
private:
  boost::function<Media ()>  get_next_;
	Phonon::MediaObject        media_;
	Phonon::AudioOutput        audio_;
	Phonon::Path               audio_path_;
  bool                       file_active_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


class PlayerHistory : public QObject
{
  Q_OBJECT
public:
  PlayerHistory();
  PlayerHistory(Player* player);

  class const_cache_iterator : public std::iterator<std::random_access_iterator_tag,const Media>
  {
  public:
    reference operator*()const
    {
      return history_->cache_[index_];
    }
    pointer operator->()const
    {
      return &history_->cache_[index_];
    }

    const_cache_iterator& operator++()//preincrement
    {
      index_ = history_->GetNext(index_);
      return *this;
    }
    const_cache_iterator  operator++(int)//postincrement
    {
      const_cache_iterator ret = *this;
      ++(*this);
      return ret;
    }
    const_cache_iterator& operator+=(int diff)
    {
      index_ = history_->GetNext(index_,diff);
      return *this;
    }
    const_cache_iterator& operator--()//predecrement
    {
      --index_;
      return *this;
    }
    const_cache_iterator  operator--(int)//postdecrement
    {
      return const_cache_iterator(history_,index_--);
    }
    const_cache_iterator& operator-=(int diff)
    {
      index_ -= diff;
      return *this;
    }
    ptrdiff_t operator-(const_cache_iterator r)
    {
      return index_ - r.index_;
    }

    bool operator==(const_cache_iterator r)
    {
      assert(history_ == r.history_);
      return (index_ == r.index_);
    }
    bool operator!=(const_cache_iterator r)
    {
      return !(*this == r);
    }
  private:
    friend class PlayerHistory;

    const_cache_iterator(PlayerHistory* history, int index)
      : history_(history),
        index_(index)
    {}

    PlayerHistory*               history_;
    int                          index_;
  };

  const_cache_iterator begin()
  {
    return const_cache_iterator(this,0);
  }

  const_cache_iterator begin(size_t max_old_cache)
  {
    int old_begin = int(current_) - max_old_cache;
    if (old_begin < 0)
      old_begin = 0;
    return const_cache_iterator(this,old_begin);
  }
  const_cache_iterator end(size_t cache_size)
  {
    return const_cache_iterator(this,cache_size + queue_end_);
  }

  void PlayFile(const Media& entry);
  void SetNextCallback(boost::function<Media ()> callback);
  void InvalidateCache();
  void SetPlayer(Player* player);

  void InsertToQueue(uint queue_index, const Media& entry);
  void RemoveFromQueue(uint queue_index);

  // Used by the player to request the next item, but will call CurrentSourceChanged
  // back sometime after requesting to indicate when it has in fact switched over
  Media PlayerNext();

public Q_SLOTS:
  Media Next(){return Next(false);}
  Media Next(bool force_play);
  Media Previous(){return Previous(false);}
  Media Previous(bool force_play);

  void   Status(PlayerState state);
  void   SourceChanged();

Q_SIGNALS:
  void OnPlayFile(const Media& entry, bool play_file);
  void OnHistoryUpdated();
private:
  friend class const_cache_iterator;
  typedef std::vector<Media> cache_t;
  void                Init();
  size_t              GetNext(size_t i, size_t offset = 1);
  Media               UpdateCurrent(size_t new_current, bool send_play, bool force_play);

  bool                current_valid()const{return current_!=cache_.size() && cache_[current_].valid();}
  Media*              current(){return &cache_[current_];}

  cache_t             cache_;
  size_t              current_;
  size_t              queue_end_;
  bool                next_enqueued_;
  bool                current_played_;
  boost::function<Media ()> get_next_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif
