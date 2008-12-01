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
#include <qsmp_gui/Cache.h>

#include <boost/iostreams/device/array.hpp>
#include <boost/range.hpp>
#include <boost/regex.hpp>
#include <qsmp_lib/Log.h>
#include <qsmp_gui/Cache.h>

namespace boost
{
  inline range_iterator<sub_match<char*> >::type
    range_begin(const sub_match<char*>& match)
  {
    return match.first;
  }

  inline range_iterator<sub_match<char*> >::type
    range_end(const sub_match<char*>& match)
  {
    return match.second;
  }
}

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


namespace cache 
{

  enum GitDataType
  {
    GitDataType_Commit,
    GitDataType_Blob,
    GitDataType_Tree,
    GitDataType_Tag,
  };

  const char* ToString(GitDataType type)
  {
    switch(type)
    {
    case GitDataType_Commit:
      return "commit";
    case GitDataType_Blob:
      return "blob";
    case GitDataType_Tag:
      return "tag";
    case GitDataType_Tree:
      return "tree";
    default:
      return "";
    }
  }

  template<class Range1T>
  GitDataType FromString(const Range1T& range)
  {
    if (boost::equals(range, boost::as_literal("commit")))
      return GitDataType_Commit;
    else if (boost::equals(range, boost::as_literal("blob")))
      return GitDataType_Blob;
    else if (boost::equals(range, boost::as_literal("tag")))
      return GitDataType_Tag;
    else if (boost::equals(range, boost::as_literal("tree")))
      return GitDataType_Tree;
    else
      return GitDataType_Blob;
  }

  enum TreeItemFlags
  {
    TIF_OtherExe   =      01,
    TIF_OtherWrite =      02,
    TIF_OtherRead  =      04,
    TIF_GroupExe   =     010,
    TIF_GroupWrite =     020,
    TIF_GroupRead  =     040,
    TIF_UserExe    =    0100,
    TIF_UserWrite  =    0200,
    TIF_UserRead   =    0400,
    TIF_Tree       =  040000,
    TIF_Blob       = 0100000,
  };

  class CatFileDeclaration
  {
  public:
    CacheId     id_;
    GitDataType type_;
    size_t      length_;
  };
  std::ostream& operator<<(std::ostream& stream, const CatFileDeclaration& decl)
  {
    stream << decl.id_ << ' ' << decl.type_ << ' ' << decl.length_;
    return stream;
  }

  class CatFileState : public State
  {
  public:
    CatFileState(LogContext parent)
      : log_(parent / "HeaderState"),
        regex_("\\A\\n?([[:xdigit:]]{40}) (\\w+) (\\d+)\\n"),
        missing_regex_("\\A\\n?([[:xdigit:]]{40}) missing\\n")
    {
    }
    boost::function<void (const CatFileDeclaration&)> OnDeclaration;

    virtual void  Reset(const CatFileDeclaration& declaration)
    {
    }
    virtual char* ProcessData(char* start, char* end)
    {
      boost::match_results<char*> matches;
      if (boost::regex_search(start,end,matches,regex_))
      {
        declaration_.id_.FromTextString(matches[1]);
        declaration_.type_ = FromString(matches[2]);
        io::stream<io::array_source> length(matches[3].first,matches[3].second);
        length >> declaration_.length_;

        start += boost::size(matches[0]);

        FLOG(log_, "Header: %1%") % declaration_;
        OnDeclaration(declaration_);
      }
      else if (boost::regex_search(start,end,matches,missing_regex_))
      {
        FLOG(log_, "Missing: %1%") % matches[1];
        start += boost::size(matches[0]);
      }
      return start;
    }
  private:
    const LogContext         log_;
    const boost::regex       regex_;
    const boost::regex       missing_regex_;
    CatFileDeclaration declaration_;
  };

  class CommitState : public State
  {
  public:
    CommitState(LogContext parent)
      : log_(parent/"CommitState"),
        tree_("\\Atree ([[:xdigit:]]{40})\\n"),
        parent_("\\Aparent ([[:xdigit:]]{40})\\n"),
        header_tail_("\\n\\n"),
        header_finished_(false)
    {
    }

    boost::function<void (CacheCommitRef)> OnCommit;

    virtual void  Reset(const CatFileDeclaration& declaration)
    {
      LOG(log_) << "Reset";
      commit_      = Commit();
      declaration_ = declaration;
      header_finished_ = false;
    }
    virtual char* ProcessData(char* start, char* end)
    {
      if (!header_finished_)
      {
        boost::match_results<char*> matches;
        if (boost::regex_search(start,end,matches,tree_))
        {
          CacheId id;
          id.FromTextString(matches[1]);
          commit_.tree_ = Cache::lease()->LookupCacheTree(id);
          start = matches[0].second;
          declaration_.length_ -= boost::size(matches[0]);
          FLOG(log_, "Tree: Id %1%") % id;
        }
        if (boost::regex_search(start,end,matches,parent_))
        {
          CacheId id;
          id.FromTextString(matches[1]);
          commit_.parents_.push_back(Cache::lease()->LookupCacheCommit(id));
          start = matches[0].second;
          declaration_.length_ -= boost::size(matches[0]);
          FLOG(log_, "Parent: Id %1%") % id;
        }
        if (boost::regex_search(start,end,matches,header_tail_))
        {
          header_finished_ = true;
          declaration_.length_ -= matches[0].second - start;
          start = matches[0].second;
          LOG(log_) << "Header finished";
        }
      }
      if (header_finished_)
      {
        size_t length = std::min(size_t(end - start),declaration_.length_);
        commit_.message_ += QString::fromUtf8(start,length);
        declaration_.length_ -= length;
        start += length;
      }
      if (declaration_.length_ == 0)
      {
        LOG(log_) << "Finished";
        //Remove a trailing newline if necessary
        if (end-start > 0 && *start == '\n')
          start++;
        CacheCommitRef commit = Cache::lease()->SetCommit(declaration_.id_,commit_);
        OnCommit(commit);
      }
      return start;
    }
  private:
    CatFileDeclaration declaration_;
    LogContext         log_;
    Commit             commit_;
    boost::regex       tree_;
    boost::regex       parent_;
    boost::regex       header_tail_;
    bool               header_finished_;
  };

  class TreeState : public State
  {
  public:
    TreeState(LogContext parent)
      : log_(parent/"TreeState"),
        item_("\\A([0-7]+) ([^\\x00]+)\\x00(.{20})")
        
    {}
    boost::function<void (CacheTreeRef)> OnTree;

    virtual void  Reset(const CatFileDeclaration& declaration)
    {
      LOG(log_) << "Reset";
      tree_        = Tree();
      declaration_ = declaration;
    }
    virtual char* ProcessData(char* start, char* end)
    {
      boost::match_results<char*> matches;
      while(declaration_.length_ > 0 && boost::regex_search(start,end,matches,item_))
      {
        //Grab the item type
        io::stream<io::array_source> item_type_source(matches[1].first,matches[1].second);
        uint item_type;
        std::oct(item_type_source);
        item_type_source >> item_type;

        //Grab the name
        Tree::Entry entry;
        entry.name_ = QString::fromUtf8(boost::begin(matches[2]),boost::size(matches[2]));

        //Grab the id
        CacheId id;
        id.FromBinString(matches[3]);
        if (item_type & TIF_Blob)
          entry.blob_ = Cache::lease()->LookupCacheBlob(id);
        else if (item_type & TIF_Tree)
          entry.tree_ = Cache::lease()->LookupCacheTree(id);

        //Insert into list
        tree_.children_.push_back(entry);
        declaration_.length_ -= boost::size(matches[0]);
        start += boost::size(matches[0]);

        FLOG(log_, "Processed item: Type %1%, Id %2%, Name %3%") 
          % boost::io::group(std::oct,item_type)
          % id
          % entry.name_;
      }
      if (declaration_.length_ == 0)
      {
        LOG(log_) << "Finish";
        //Remove a trailing newline if necessary
        if (end-start > 0 && *start == '\n')
          start++;
        CacheTreeRef tree = Cache::lease()->SetTree(declaration_.id_,tree_);
        OnTree(tree);
      }
      return start;
    }
  private:
    const LogContext         log_;
    const boost::regex       item_;
    CatFileDeclaration       declaration_;
    Tree                     tree_;
  };

  class BlobState : public State
  {
  public:
    BlobState(LogContext parent)
      : log_(parent/"BlobState")
    {}
    boost::function<void (CacheBlobRef)> OnBlob;

    virtual void  Reset(const CatFileDeclaration& declaration)
    {
      LOG(log_) << "Reset";
      declaration_ = declaration;
      blob_        = Blob();
    }
    virtual char* ProcessData(char* start, char* end)
    {
      size_t length_to_use = std::min(size_t(end-start), declaration_.length_);
      declaration_.length_ -= length_to_use;
      blob_ += QString::fromUtf8(start,length_to_use);
      start += length_to_use;
      if (declaration_.length_ == 0)
      {
        LOG(log_) << "Finish";
        CacheBlobRef blob = Cache::lease()->SetBlob(declaration_.id_,blob_);
        OnBlob(blob);
      }
      return start;
    }
  private:
    CatFileDeclaration declaration_;
    LogContext         log_;
    Blob               blob_;
  };

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------

  struct ProcessData
  {
  public:
    ProcessData(LogContext context, CacheThread* cache_thread);
    void operator()(HANDLE stdout_fd);
  private:
    void OnDeclaration(const cache::CatFileDeclaration& declaration);

    void OnCommit(CacheCommitRef commit);
    void OnTree(CacheTreeRef tree);
    void OnBlob(CacheBlobRef blob);

    LogContext log_;
    LogContext log_profile_;
    bool more_to_process_;
    CatFileDeclaration  declaration_;
    State* state_;
    CatFileState cat_file_state_;
    BlobState    blob_state_;
    TreeState    tree_state_;
    CommitState  commit_state_;
    CacheThread* cache_thread_;
  };

  //-----------------------------------------------------------------------------

  ProcessData::ProcessData(LogContext context, CacheThread* cache_thread)
    : log_(context),
      log_profile_(context,"Profile"),
      more_to_process_(false),
      cat_file_state_(log_),
      blob_state_(log_),
      tree_state_(log_),
      commit_state_(log_),
      cache_thread_(cache_thread)
  {
  }

  //-----------------------------------------------------------------------------

  void ProcessData::operator()(HANDLE stdout_fd)
  {
    LOG(log_) << "Starting";
    cat_file_state_.OnDeclaration = boost::bind(&ProcessData::OnDeclaration,this,_1);
    commit_state_.OnCommit = boost::bind(&ProcessData::OnCommit,this,_1);
    tree_state_.OnTree     = boost::bind(&ProcessData::OnTree,this,_1);
    blob_state_.OnBlob     = boost::bind(&ProcessData::OnBlob,this,_1);
    state_ = &cat_file_state_;

    boost::array<char,4096> buffer;
    DWORD read = 0;
    size_t already_read = 0;
    for(;;)
    {
      LOG(log_) << "Main loop";
      QSMP_PROFILE(log_profile_,"Main loop");
      if (!more_to_process_)
        ReadFile(stdout_fd,&buffer[already_read],buffer.size() - already_read,&read,NULL);
      FLOG(log_, "Git input: Read %1%, Already in buffer %2%") % read % already_read;
      more_to_process_ = false;
      char* start = &buffer[0];
      char* end   = start + already_read + read;
      char* new_start = state_->ProcessData(start,end);
      LOG(log_) << "Data processed: " << new_start - start;
      ASSERTE(log_, &*buffer.begin() <= new_start && new_start <= &*buffer.end());
      if (new_start < end)
        memmove(start,new_start,end - new_start);
      else
        more_to_process_ = false;
      already_read = end - new_start;
      read = 0;
    }
  }

  //-----------------------------------------------------------------------------

  void ProcessData::OnDeclaration(const CatFileDeclaration& declaration)
  {
    more_to_process_ = true;
    declaration_ = declaration;
    switch(declaration.type_)
    {
    case cache::GitDataType_Commit:
      state_ = &commit_state_;
      break;
    case cache::GitDataType_Tree:
      state_ = &tree_state_;
      break;
    case cache::GitDataType_Tag:
    case cache::GitDataType_Blob:
    default:
      state_ = &blob_state_;
      break;
    }
    state_->Reset(declaration);
  }

  //-----------------------------------------------------------------------------

  void ProcessData::OnCommit(CacheCommitRef commit)
  {
    more_to_process_ = true;
    state_ = &cat_file_state_;
    state_->Reset(declaration_);
    cache_thread_->OnCommit(commit);
  }

  //-----------------------------------------------------------------------------

  void ProcessData::OnTree(CacheTreeRef tree)
  {
    more_to_process_ = true;
    state_ = &cat_file_state_;
    state_->Reset(declaration_);
    cache_thread_->OnTree(tree);
  }

  //-----------------------------------------------------------------------------

  void ProcessData::OnBlob(CacheBlobRef blob)
  {
    more_to_process_ = true;
    state_ = &cat_file_state_;
    state_->Reset(declaration_);
    cache_thread_->OnBlob(blob);
  }

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class RecursiveListing : public CacheThread::Task
{
public:
  void operator()(CacheThread::FinishCallback finish,
                  CacheThread::RequestCallback request)
  {
    finish_  = finish;
    request_ = request;
    count_   = 0;
    second_count_ = 0;
    count_++;
    second_count_++;
    request_(CacheId("7a1e8b5b31087018f993cfd39e104d33344fe86b"));
  }
private:
  virtual void OnTree(CacheTreeRef tree)
  {
    for(Tree::Children::const_iterator ii = tree->data_.children_.begin();
        ii != tree->data_.children_.end();
        ++ii)
    {
      if (ii->tree_ && !ii->tree_->valid_)
      {
        second_count_++;
        count_++;
        request_(ii->tree_->id_);
      }
      else if (ii->blob_ && !ii->blob_->valid_)
      {
        second_count_++;
        count_++;
        request_(ii->blob_->id_);
      }
    }
    if (--count_ == 0)
    {
      Cache::lease lease;
      LOG("test") << "finished";
      finish_();
    }
  }
  virtual void OnBlob(CacheBlobRef blob)
  {
    if (--count_ == 0)
    {
      Cache::lease lease;
      LOG("test") << "finished";
      finish_();
    }
  }
  int second_count_;
  int count_;
  CacheThread::FinishCallback  finish_;
  CacheThread::RequestCallback request_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CacheThread::CacheThread(LogContext log, const fs::path& path)
: log_(log),
  log_read_(log/"Read"),
  log_write_(log/"Write"),
  log_write_profile_(log_write_/"Profile"),
  git_("C:/Program Files/Git/bin/git.exe", path, "cat-file --batch"),
  git_stdin_(git_.stdin_fd()),
  read_thread_(boost::bind(&CacheThread::ReadThread,this)),
  write_thread_(boost::bind(&CacheThread::WriteThread,this)),
  cache_queues_signal_(CreateEvent(NULL,FALSE,FALSE,NULL)),
  task_signal_(CreateEvent(NULL,FALSE,FALSE,NULL))
{
  LOG(log_) << "Launching git";
  //git_stdin_ << "73d23d6bb61c8e41dba58dc2576910c3738f21ef\n" //head commit
             //<< "ed448d4618c266cfbbc2ae74daf85d98bed66976\n" //head tree
             //<< "3914f69f2daa00a492a570998e15477c82f228a0\n" //some random blob
             //<< std::flush;
  //AddTask(spnew<RecursiveListing>());
}

//-----------------------------------------------------------------------------

CacheThread::~CacheThread()
{
  CloseHandle(task_signal_);
  CloseHandle(cache_queues_signal_);
}

//-----------------------------------------------------------------------------

void CacheThread::AddTask(shared_ptr<Task> task)
{
  LOG(log_) << "Adding task";
  guard g(task_queue_lock_);
  bool start = task_queue_.empty();
  task_queue_.push(task);
  if (start)
    SetEvent(task_signal_);
}

//-----------------------------------------------------------------------------

void CacheThread::ReadThread()
{
  try
  {
    cache::ProcessData data(log_read_,this);
    data(git_.stdout_fd());
  }
  catch(std::exception& e)
  {
    FATAL(log_read_) << e.what();
  }
}

//-----------------------------------------------------------------------------

void CacheThread::WriteThread()
{
  try
  {
    for(;;)
    {
      HANDLE handles[2] = {cache_queues_signal_,task_signal_};
      if(WaitForMultipleObjects(2,handles,FALSE,INFINITE) > WAIT_OBJECT_0+1)
        WIN32_FATAL(log_write_);

      LOG(log_write_) << "Main write loop";
      if (!current_task_)
      {
        guard g(task_queue_lock_);
        if(!task_queue_.empty())
        {
          LOG(log_write_) << "Starting new task";
          current_task_ = task_queue_.front();
          task_queue_.pop();
          current_task_->operator()(boost::bind(&CacheThread::Finish,this),boost::bind(&CacheThread::RequestId,this,_1));
        }
      }

      if (current_task_)
      {
        QSMP_PROFILE(log_write_profile_,"Main loop");
        for(;;)
        {
          CacheCommitRef commit = NULL;
          CacheTreeRef tree = NULL;
          CacheBlobRef blob = NULL;
          {
            guard g(cache_queues_lock_);
            if (!commit_queue_.empty())
            {
              commit = commit_queue_.front();
              commit_queue_.pop_front();
            }
            if (!tree_queue_.empty())
            {
              tree = tree_queue_.front();
              tree_queue_.pop_front();
            }
            if (!blob_queue_.empty())
            {
              blob = blob_queue_.front();
              blob_queue_.pop_front();
            }
          }
          if (commit)
          {
            FLOG(log_write_, "Giving commit to task: Id %1%") % commit->id_;
            current_task_->OnCommit(commit);
          }
          if (tree)
          {
            FLOG(log_write_, "Giving tree to task: Id %1%") % tree->id_;
            current_task_->OnTree(tree);
          }
          if (blob)
          {
            FLOG(log_write_, "Giving blob to task: Id %1%") % blob->id_;
            current_task_->OnBlob(blob);
          }
          if (!commit && !tree && !blob)
            break;
        }
        git_stdin_ << std::flush;
      }
    }
  }
  catch(std::exception& e)
  {
    FATAL(log_write_) << e.what();
  }
}

//-----------------------------------------------------------------------------

void CacheThread::Finish()
{
  LOG(log_write_) << "Finish";
  current_task_.reset();
  SetEvent(task_signal_);
}

//-----------------------------------------------------------------------------

void CacheThread::RequestId(const CacheId& id)
{
  FLOG(log_write_, "Getting %1%") % id;
  git_stdin_ << id << "\n";
}

//-----------------------------------------------------------------------------

void CacheThread::OnCommit(CacheCommitRef commit)
{
  FLOG(log_read_, "Got commit: %1%") % commit->id_;
  guard g(cache_queues_lock_);
  commit_queue_.push_back(commit);
  SetEvent(cache_queues_signal_);
}

//-----------------------------------------------------------------------------

void CacheThread::OnTree(CacheTreeRef tree)
{
  FLOG(log_read_, "Got tree: %1%") % tree->id_;
  guard g(cache_queues_lock_);
  tree_queue_.push_back(tree); 
  SetEvent(cache_queues_signal_);
}

//-----------------------------------------------------------------------------

void CacheThread::OnBlob(CacheBlobRef blob)
{
  FLOG(log_read_, "Got blob: %1%") % blob->id_;
  guard g(cache_queues_lock_);
  blob_queue_.push_back(blob);
  SetEvent(cache_queues_signal_);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Cache::Cache(boost::restricted)
: log_("Cache",LogDefaults_Disable),
cache_thread_(log_/"Thread", "E:/SCM/qsmp/build/qsmp_indexer/debug/test/")
{
}

//-----------------------------------------------------------------------------

CacheBlobRef Cache::LookupCacheBlob(const CacheId& id)
{
  FLOG(log_, "Lookup cache blob: Id %1%") % id;
  lock_guard lock(cache_lock_);
  BlobCache::iterator ii = blob_cache_.find(id);
  if (ii == blob_cache_.end())
  {
    std::pair<CacheId,CacheBlob> blob(id,CacheBlob(id));
    ii = blob_cache_.insert(blob).first;
  }
  return &(ii->second);
}

//-----------------------------------------------------------------------------

CacheTreeRef Cache::LookupCacheTree(const CacheId& id)
{
  FLOG(log_, "Lookup cache tree: Id %1%") % id;
  lock_guard lock(cache_lock_);
  TreeCache::iterator ii = tree_cache_.find(id);
  if (ii == tree_cache_.end())
  {
    std::pair<CacheId,CacheTree> tree(id,CacheTree(id));
    ii = tree_cache_.insert(tree).first;
  }
  return &(ii->second);
}

//-----------------------------------------------------------------------------

CacheCommitRef Cache::LookupCacheCommit(const CacheId& id)
{
  FLOG(log_, "Lookup cache commit: Id %1%") % id;
  lock_guard lock(cache_lock_);
  CommitCache::iterator ii = commit_cache_.find(id);
  if (ii == commit_cache_.end())
  {
    std::pair<CacheId,CacheCommit> commit(id,CacheCommit(id));
    ii = commit_cache_.insert(commit).first;
  }
  return &(ii->second);
}

//-----------------------------------------------------------------------------

CacheBlobRef Cache::LookupBlob(const CacheId& id)
{
  FLOG(log_, "Lookup blob: Id %1%") % id;
  lock_guard lock(cache_lock_);
  CacheBlobRef blob = &blob_cache_[id];

  {
    //Now we need to go out to git to get the blob data
  }

  return blob;
}

//-----------------------------------------------------------------------------

CacheBlobRef Cache::SetBlob(const CacheId& id, const Blob& blob)
{
  FLOG(log_, "Set blob: Id %1%") % id;
  lock_guard lock(cache_lock_);
  BlobCache::iterator ii = blob_cache_.find(id);
  if (ii == blob_cache_.end())
  {
    std::pair<CacheId,CacheBlob> to_insert;
    to_insert.first        = id;
    to_insert.second.id_   = id;
    to_insert.second.data_ = blob;
    ii = blob_cache_.insert(to_insert).first;
  }
  else if (!ii->second.valid_)
  {
    ii->second.valid_ = true;
    ii->second.data_ = blob;
  }
  return &ii->second;
}

//-----------------------------------------------------------------------------

CacheTreeRef Cache::SetTree(const CacheId& id, const Tree& tree)
{
  FLOG(log_, "Set tree: Id %1%") % id;
  lock_guard lock(cache_lock_);
  TreeCache::iterator ii = tree_cache_.find(id);
  if (ii == tree_cache_.end())
  {
    std::pair<CacheId,CacheTree> to_insert;
    to_insert.first         = id;
    to_insert.second.id_    = id;
    to_insert.second.data_  = tree;
    to_insert.second.valid_ = true;
    ii = tree_cache_.insert(to_insert).first;
  }
  else if (!ii->second.valid_)
  {
    ii->second.data_ = tree;
    ii->second.valid_ = true;
  }
  return &ii->second;
}

//-----------------------------------------------------------------------------

CacheCommitRef Cache::SetCommit(const CacheId& id, const Commit& commit)
{
  FLOG(log_, "Set commit: Id %1%") % id;
  lock_guard lock(cache_lock_);
  CommitCache::iterator ii = commit_cache_.find(id);
  if (ii == commit_cache_.end())
  {
    std::pair<CacheId,CacheCommit> to_insert;
    to_insert.first        = id;
    to_insert.second.id_   = id;
    to_insert.second.data_ = commit;
    ii = commit_cache_.insert(to_insert).first;
  }
  else if (!ii->second.valid_)
  {
    ii->second.valid_ = true;
    ii->second.data_  = commit;
  }
  return &ii->second;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END