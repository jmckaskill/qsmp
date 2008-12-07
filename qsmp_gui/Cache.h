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

#ifndef QSMP_CACHE_H_
#define QSMP_CACHE_H_

#include <qsmp_gui/common.h>

#include <algorithm>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/unordered_map.hpp>
#include <qsmp_gui/ViewSelector.h>
#include <qsmp_gui/Process.h>


QSMP_BEGIN


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline uint8_t FromHex(char ch)
{
  uint8_t ret = 0;
  if ('0' <= ch && ch <= '9')
    ret = ch - '0';
  else if ('a' <= ch && ch <= 'f')
    ret = ch - 'a' + 10;
  else if ('A' <= ch && ch <= 'F')
    ret = ch - 'A' + 10;
  return ret;
}
inline char ToHexLow(uint8_t v)
{
  v &= 0x0F;
  if (v < 10)
    return '0' + v;
  else
    return 'a' - 10 + v;
}
inline char ToHexHigh(uint8_t v)
{
  v >>= 4;
  if (v < 10)
    return '0' + v;
  else
    return 'a' - 10 + v;
}

class CacheId
{
public:
  CacheId(){}
  CacheId(const char* string)
  {
    FromTextString(boost::as_literal(string));
  }

  const QString& string()const{return string_;}

  bool operator==(const CacheId& r)const
  {
    return boost::equals(data_,r.data_);
  }
  bool operator!=(const CacheId& r)const
  {
    return !(*this == r);
  }
  template<class Range1T>
  void FromTextString(const Range1T& range)
  {
    ASSERTE("Cache",boost::size(range) == 40);
    typename boost::range_iterator<Range1T>::type ii = boost::begin(range);
    string_ = QString::fromAscii(&*ii, 40);
    for(size_t i = 0; i < 20; ++i)
    {
      data_[i]  = 0xF0 & (FromHex(*ii++) << 4);
      data_[i] |= 0x0F & (FromHex(*ii++));
    }
  }
  template<class Range1T>
  void FromBinString(const Range1T& range)
  {
    ASSERTE("Cache",boost::size(range) == 20);
    std::copy(boost::begin(range),boost::end(range),data_.begin());
    char output[40];
    for(size_t i = 0; i < 20; ++i)
    {
      output[2*i]   = ToHexHigh(data_[i]);
      output[2*i+1] = ToHexLow(data_[i]);
    }
    string_ = QString::fromAscii(output,40);
  }
private:
  friend size_t hash_value(const CacheId& id);
  template<class CharT, class traits>
  friend std::basic_ostream<CharT, traits>& operator<<(std::basic_ostream<CharT, traits>& stream, const CacheId& r);
  template<class CharT, class InItrT>
  friend class CacheIdInputFacet;
  template<class CharT, class InItrT>
  friend class CacheIdOutputFacet;


  boost::array<uint8_t,20> data_;
  QString                  string_;
};

//-----------------------------------------------------------------------------

BOOST_STATIC_ASSERT(sizeof(uint8_t)*20 > sizeof(size_t));

inline size_t hash_value(const CacheId& id)
{
  return *reinterpret_cast<const size_t*>(&id.data_[0]);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class CharT, class traits>
std::basic_ostream<CharT, traits>& operator<<(std::basic_ostream<CharT, traits>& stream, const CacheId& r)
{
  char output[40];
  for(size_t i = 0; i < 20; ++i)
  {
    output[2*i]   = ToHexHigh(r.data_[i]);
    output[2*i+1] = ToHexLow(r.data_[i]);
  }
  stream.write(output,40);
  return stream;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace cache
{
  class CatFileDeclaration;
  class State
  {
  public:
    virtual char* ProcessData(char* start, char* end)=0;
    virtual void  Reset(const CatFileDeclaration& declaration)=0;
  };
  class CommitState;
  class BlobState;
  class TreeState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


template<class T>
class CacheEntry
{
public:
  CacheEntry():valid_(false){}
  friend class Cache;
  friend class cache::BlobState;
  friend class cache::TreeState;
  friend class cache::CommitState;

  explicit CacheEntry(const CacheId& id):id_(id),valid_(false){}
  explicit CacheEntry(const CacheId& id, const T& data):id_(id),data_(data),valid_(false){}

  CacheId     id_;
  bool        valid_;
  T           data_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef QString Blob;
typedef CacheEntry<Blob> CacheBlob;
typedef const CacheBlob* CacheBlobRef;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Tree;
typedef CacheEntry<Tree> CacheTree;
typedef const CacheTree* CacheTreeRef;
class Tree
{
public:
  struct Entry
  {
    Entry():blob_(NULL),tree_(NULL){}
    QString       name_;
    CacheBlobRef  blob_;
    CacheTreeRef  tree_;
  };
  typedef std::vector<Entry> Children;
  friend class cache::TreeState;
  CacheId            id_;
  std::vector<Entry> children_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Commit;
typedef CacheEntry<Commit> CacheCommit;
typedef const CacheCommit* CacheCommitRef;
class Commit
{
public:
  Commit():tree_(NULL){}
  explicit Commit(CacheTreeRef tree);
private:
  friend class cache::CommitState;
  CacheId                               id_;
  QString                               message_;
  CacheTreeRef                          tree_;
  std::vector<CacheCommitRef>           parents_;
  boost::posix_time::ptime              commit_date_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CacheThread
{
  QSMP_NON_COPYABLE(CacheThread);
public:
  CacheThread(LogContext log, const fs::path& path);

  typedef boost::function<void ()> FinishCallback;
  typedef boost::function<void (const CacheId&)> RequestCallback;
  struct Task
  {
    virtual void operator()(FinishCallback finish,RequestCallback request)=0;
    virtual void OnCommit(CacheCommitRef commit){}
    virtual void OnTree(CacheTreeRef tree){}
    virtual void OnBlob(CacheBlobRef blob){}
  };

  void AddTask(shared_ptr<Task> task);

  void OnCommit(CacheCommitRef commit);
  void OnTree(CacheTreeRef tree);
  void OnBlob(CacheBlobRef blob);

private:
  void ReadThread();
  void WriteThread();
  void RequestId(const CacheId& id);
  void Finish();

  typedef boost::lock_guard<boost::mutex> guard;

  const LogContext                      log_;
  const LogContext                      log_write_;
  const LogContext                      log_write_profile_;
  const LogContext                      log_read_;

  std::queue<shared_ptr<Task> >         task_queue_;
  shared_ptr<Task>                      current_task_;
  boost::mutex                          task_queue_lock_;
  boost::condition_variable             task_signal_;

  boost::mutex                          cache_queues_lock_;
  boost::condition_variable             cache_queues_signal_;
  std::deque<CacheCommitRef>            commit_queue_;
  std::deque<CacheTreeRef>              tree_queue_;
  std::deque<CacheBlobRef>              blob_queue_;


  Process                               git_;
  io::stream<io::file_descriptor_sink>  git_stdin_;

  boost::thread                         read_thread_;
  boost::thread                         write_thread_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Cache : public boost::singleton<Cache>
{
public:
  typedef boost::filesystem::path Path;


  Cache(boost::restricted);

  
  //The Lookup functions are split up into two sections and lookup two different data types:
  //1. LookupCacheX: only lookups the item from the cache and will not go out to git to get the data
  //                 thus the data may or may not be valid (look at the optional part of the return)
  //2. LookupX: will lookup in the cache and if that is not valid, will go out to git to get it
  //            even after that it may not have the data (ie if git returns an error)
  //            if that happens it will be logged, but the next time the data is requested, the data
  //            will try to be re-retrieved since the optional tag indicates the data has still not been
  //            gathered

  //As a side note: note that all data in the cache is _owned_ by the cache and thus will not be
  //valid if the cache is cleared/destroyed

  CacheBlobRef   LookupCacheBlob(const CacheId& id);
  CacheTreeRef   LookupCacheTree(const CacheId& id);
  CacheCommitRef LookupCacheCommit(const CacheId& id);

  CacheBlobRef   LookupBlob(const CacheId& id);
  CacheBlobRef   LookupBlob(const Path& repo, const Path& head, const Path& path);
  CacheTreeRef   LookupTree(const CacheId& id, size_t depth_to_load = 1);
  CacheTreeRef   LookupTree(const Path& repo, const Path& head, const Path& path, size_t depth_to_load = 1);
  CacheCommitRef LookupCommit(const CacheId& id, size_t depth_to_load = 0);
  CacheCommitRef LookupCommit(const Path& repo, const Path& head, size_t depth_to_load = 0);

  CacheBlobRef   SetBlob(const CacheId& id, const Blob& blob);
  CacheTreeRef   SetTree(const CacheId& id, const Tree& tree);
  CacheCommitRef SetCommit(const CacheId& id, const Commit& commit);

  void AddThreadTask(shared_ptr<CacheThread::Task> task)
  {cache_thread_.AddTask(task);}

private:
  typedef boost::unordered_map<CacheId,CacheBlob>   BlobCache;
  typedef boost::unordered_map<CacheId,CacheTree>   TreeCache;
  typedef boost::unordered_map<CacheId,CacheCommit> CommitCache;
  typedef boost::lock_guard<boost::mutex> lock_guard;

  LogContext    log_;

  CacheThread   cache_thread_;

  boost::mutex  cache_lock_;
  BlobCache     blob_cache_;
  TreeCache     tree_cache_;
  CommitCache   commit_cache_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif

