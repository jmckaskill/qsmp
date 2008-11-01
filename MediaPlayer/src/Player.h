#ifndef QSMP_PLAYER_H_
#define QSMP_PLAYER_H_

#include "common.h"

QSMP_BEGIN

class Entry;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Player : public QObject
{
  Q_OBJECT
public:
  Player(boost::function<Entry* ()> get_next = boost::function<Entry* ()>());

  Phonon::State        status()const{return media_.state();}
  Phonon::AudioOutput* audio(){return &audio_;}
  Phonon::MediaObject* media(){return &media_;}

public Q_SLOTS:
  void PlayFile(Entry* entry, bool play_file = true);
  void Play();
  void Pause();
  void Stop();
  //Volume is from 0-1000
  void SetVolume(int volume);

Q_SIGNALS:
  void OnProgress(QTime progress, QTime total);
  void OnStatus(Phonon::State state);
  void OnSourceChanged();

private Q_SLOTS:
  void EnqueueNext();
private:
  boost::function<Entry* ()> get_next_;
	Phonon::MediaObject        media_;
	Phonon::AudioOutput        audio_;
	Phonon::Path               audio_path_;
  bool                       loaded_file_;
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

  //Index values:
  // -ve : already played
  // 0   : currently playing / next to be played if stopped
  // +ve : next to be played starting with entries in the queue and then however many items have 
  //       been pulled into the cache
  tuple<Entry*, uint /*queue_index*/>      GetEntry(int index);

  void PlayFile(Entry* entry);
  void SetNextCallback(boost::function<Entry* ()> callback);
  void InvalidateCache();
  void SetPlayer(Player* player);

  void InsertToQueue(uint queue_index, Entry* entry);
  void RemoveFromQueue(uint queue_index);

  // Used by the player to request the next item, but will call CurrentSourceChanged
  // back sometime after requesting to indicate when it has in fact switched over
  Entry* PlayerNext();

public Q_SLOTS:
  Entry* Next(){return Next(true);}
  Entry* Next(bool force_play);
  Entry* Previous(){return Previous(false);}
  Entry* Previous(bool force_play);

  void   CurrentSourceChanged();

Q_SIGNALS:
  void OnPlayFile(Entry* entry, bool play_file);
  void OnCacheUpdate();
  void OnQueueUpdate();
private:
  void   Init();
  Entry* CurrentUpdated(bool force_play);
  typedef std::list<Entry*> cache_t;
  cache_t           cache_;
  cache_t::iterator current_;
  cache_t::iterator queue_end_;
  bool              next_enqueued_;
  bool              current_played_;
  boost::function<Entry* ()> get_next_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif
