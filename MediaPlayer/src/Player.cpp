#include "Player.h"
#include "utilities.h"

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Player::Player(boost::function<Entry *()> get_next)
: get_next_(get_next),
  audio_(Phonon::MusicCategory),
  loaded_file_(false)
{
  audio_path_ = Phonon::createPath(&media_,&audio_);
  connect(&media_, SIGNAL(aboutToFinish()), this, SLOT(EnqueueNext()));
  connect(&media_, SIGNAL(currentSourceChanged(const Phonon::MediaSource &)), this, SIGNAL(OnSourceChanged()));
}

//-----------------------------------------------------------------------------

void Player::PlayFile(Entry* entry, bool play_file)
{
  media_.clearQueue();
  if (entry != NULL)
  {
    media_.setCurrentSource(QString::fromStdString(entry->path_.file_string()));
    if (play_file || loaded_file_)
      media_.play();
    loaded_file_ = true;
  }
  else
  {
    loaded_file_ = false;
    media_.stop();
  }
}

//-----------------------------------------------------------------------------

void Player::Play()
{
  if (!loaded_file_)
  {
    if (!get_next_.empty())
      PlayFile(get_next_());
  }
  else
  {
    media_.play();
  }
}

//-----------------------------------------------------------------------------

void Player::Pause()
{
  if (!loaded_file_)
    media_.pause();
}

//-----------------------------------------------------------------------------

void Player::Stop()
{
  if (!loaded_file_)
    media_.stop();
}

//-----------------------------------------------------------------------------

void Player::SetVolume(int volume)
{
  audio_.setVolume(double(volume) / 1000.0);
}

//-----------------------------------------------------------------------------

void Player::EnqueueNext()
{
  if (!get_next_.empty())
    media_.enqueue(QString::fromStdString(get_next_()->path_.file_string()));
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
  cache_.push_back(NULL);
  current_ = cache_.begin();
  queue_end_ = current_;
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetPlayer(Player* player)
{
  connect(this, SIGNAL(OnPlayFile(Entry*,bool)), player, SLOT(PlayFile(Entry*,bool)));
  connect(player, SIGNAL(OnSourceChanged()), this, SLOT(CurrentSourceChanged()));
  CurrentUpdated(false);
}

//-----------------------------------------------------------------------------

void PlayerHistory::PlayFile(Entry* entry)
{
  cache_t::iterator i = current_;
  if (i != cache_.end())
    ++i;

  cache_.insert(i,entry);
  Next(true);
}

//-----------------------------------------------------------------------------

void PlayerHistory::SetNextCallback(boost::function<Entry* ()> callback)
{
  get_next_ = callback;
  InvalidateCache();
}

//-----------------------------------------------------------------------------

void PlayerHistory::InvalidateCache()
{
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
    current_ = cache_.end();
    CurrentUpdated(true);
  }
  OnCacheUpdate();
}

//-----------------------------------------------------------------------------

void PlayerHistory::CurrentSourceChanged()
{
  if (next_enqueued_)
  {
    bool shift_queue = (current_ == queue_end_);
    ++current_;
    next_enqueued_ = false;
    if (shift_queue)
      queue_end_ = current_;
  }
  current_played_ = true;
}

//-----------------------------------------------------------------------------

Entry* PlayerHistory::Next(bool force_play)
{
  bool shift_queue = (current_ == queue_end_);

  ++current_;
  if (current_ == cache_.end())
  {
    Entry* next = get_next_.empty() ? NULL : get_next_();
    if (next != NULL)
    {
      cache_.push_back(next);
      --current_;
    }
  }

  if (shift_queue)
  {
    queue_end_ = current_;
  }


  OnCacheUpdate();
  if (!shift_queue)
    OnQueueUpdate();

  return CurrentUpdated(force_play);
}

//-----------------------------------------------------------------------------

Entry* PlayerHistory::PlayerNext()
{
  next_enqueued_ = true;
  cache_t::iterator i = current_;
  assert(i != cache_.end());//this function should only be called if we are currently playing something
  ++i;
  if (i == cache_.end())
  {
    Entry* next = get_next_.empty() ? NULL : get_next_();
    if (next != NULL)
    {
      cache_.push_back(next);
      --i;
    }
  }
  return (i != cache_.end()) ? *i : NULL;
}

//-----------------------------------------------------------------------------

Entry* PlayerHistory::Previous(bool force_play)
{
  bool shift_queue = (queue_end_ == current_);

  if (current_ != cache_.begin())
    --current_;

  if (shift_queue)
    queue_end_ = current_;

  OnCacheUpdate();
  if (!shift_queue)
    OnQueueUpdate();

  return CurrentUpdated(force_play);
}

//-----------------------------------------------------------------------------

Entry* PlayerHistory::CurrentUpdated(bool force_play)
{
  Entry* ret = NULL;
  if (current_ != cache_.end())
    ret = *current_;
  current_played_ = false;
  next_enqueued_ = false;
  OnPlayFile(ret, force_play);
  return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END