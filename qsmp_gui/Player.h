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

  void SetNextCallback(boost::function<Media ()> callback);
  void SetPlayer(Player* player);

  void PlayFile(const Media& entry);
  void InsertToQueue(int index, const Media& entry);
  void RemoveFromQueue(int index);
  void RemoveFromCache(int index);

  // Used by the player to request the next item, but will call SourceChanged
  // back sometime after to indicate when it has in fact switched over
  Media GetPlayerNext();

  size_t history_size()const{return current_;}
  size_t queue_size()const{return queue_end_ - current_;}
  size_t cache_size()const{return cache_.size() - queue_end_;}

  Media LookupHistory(int index){return Lookup(current_ + index);}
  Media LookupQueue(int index){return Lookup(current_ + index);}
  Media LookupCache(int index){return Lookup(queue_end_ + index);}

public Q_SLOTS:
  Media Next(){return Next(false);}
  Media Next(bool force_play);
  Media Previous(){return Previous(false);}
  Media Previous(bool force_play);

  void   Status(PlayerState state);
  void   SourceChanged();

Q_SIGNALS:
  void OnPlayFile(const Media& entry, bool play_file);
  void OnCacheReset();

  //The insert and remove signals follow insertions and removals STL style
  //Thus an insert at index x for count 1 means that after insertion the new item
  //sits at index x (ie _before_ current item at x)
  //A removal at x for count y means removal of [x,x+y)

  void OnHistoryInsert(int index, const Media& media);
  void OnHistoryRemove(int index);

  //Queue indexes are positive starting at 0 (0 is a special value for the current item)
  void OnQueueInsert(int index, const Media& media);
  void OnQueueRemove(int index);

  //Cache indexes are positive starting from 0 which is the first item in the cache
  void OnCacheInsert(int index, const Media& media);
  void OnCacheRemove(int index);

private:
  friend class const_cache_iterator;
  typedef std::vector<Media> cache_t;
  void                Init();
  Media               Lookup(int index);

  void                ResetCache();
  void                PreviousUpdate();
  void                NextUpdate();

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
