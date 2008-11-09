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
#include <qsmp/common.h>
#include <qsmp/utilities.h>
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

  class const_cache_iterator : public std::iterator<std::bidirectional_iterator_tag,const Media>
  {
  public:
    reference operator*()const
    {
      assert(iter_ != history_->cache_.end());
      return *iter_;
    }
    pointer operator->()const
    {
      assert(iter_ != history_->cache_.end());
      return iter_.operator ->();
    }

    const_cache_iterator& operator++()//preincrement
    {
      iter_ = history_->GetNext(iter_);
      ++index_;
      return *this;
    }
    const_cache_iterator  operator++(int)//postincrement
    {
      const_cache_iterator ret = *this;
      ++(*this);
      return ret;
    }
    const_cache_iterator& operator--()//predecrement
    {
      --iter_;
      return *this;
    }
    const_cache_iterator  operator--(int)//postdecrement
    {
      return const_cache_iterator(history_,iter_--,index_--);
    }

    bool operator==(const_cache_iterator r)
    {
      assert(history_ == r.history_);
      return (iter_ == r.iter_) ||
             (index_ == r.index_);
    }
    bool operator!=(const_cache_iterator r)
    {
      return !(*this == r);
    }
  private:
    friend class PlayerHistory;

    const_cache_iterator(PlayerHistory* history, std::list<Media>::iterator iter, int index)
      : history_(history),
        iter_(iter),
        index_(index)
    {}

    PlayerHistory* history_;
    std::list<Media>::iterator iter_;
    int                         index_;
  };

  typedef std::list<Media>::const_reverse_iterator const_played_iterator;
  typedef std::list<Media>::const_iterator const_queue_iterator;
  typedef const_cache_iterator const_history_iterator;
  const_queue_iterator queue_begin()const{return current_;}
  const_queue_iterator queue_end()const{return queue_end_;}
  const_cache_iterator next_begin(){return const_cache_iterator(this,queue_end_,0);}
  const_cache_iterator next_end(size_t cache_size){return const_cache_iterator(this,cache_.end(),cache_size);}
  const_played_iterator played_begin()const{return const_played_iterator(current_);}
  const_played_iterator played_end()const
  {return const_played_iterator(cache_.begin());}//Note: this removes access to the first element, which is always NULL

  const_history_iterator begin(size_t max_old_cache)
  {
    const_history_iterator i(this,current_,0);
    while(i->valid() && max_old_cache > 0)
    {
      --i;
      --max_old_cache;
    }
    if (!i->valid())
      ++i;
    return i;
  }
  const_history_iterator end(size_t cache_size)
  {
    return const_history_iterator(this,cache_.end(),cache_size + std::distance(current_,queue_end_));
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
  typedef std::list<Media> cache_t;
  void                Init();
  cache_t::iterator   GetNext(cache_t::iterator ii);
  Media               UpdateCurrent(cache_t::iterator new_current, bool send_play, bool force_play);

  cache_t             cache_;
  cache_t::iterator   current_;
  cache_t::iterator   queue_end_;
  bool                next_enqueued_;
  bool                current_played_;
  boost::function<Media ()> get_next_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif
