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
#include "qsmp_gui/CacheModel.h"
#include "qsmp_gui/CacheModel.moc"


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CacheModelFetchMore : public CacheThread::Task
{
public:
  CacheModelFetchMore(LogContext log, const CacheId& parent, boost::function<void ()> onfinish)
    : log_(log),
      model_finish_(onfinish),
      parent_(parent)
  {
  }

  void operator()(CacheThread::FinishCallback finish,
                  CacheThread::RequestCallback request)
  {
    task_finish_  = finish;
    request_      = request;
    count_        = 0;
    count_++;
    request_(parent_);
  }
private:
  virtual void OnTree(CacheTreeRef tree)
  {
    for(Tree::Children::const_iterator ii = tree->data_.children_.begin();
        ii != tree->data_.children_.end();
        ++ii)
    {
      if (ii->blob_ && !ii->blob_->valid_)
      {
        count_++;
        request_(ii->blob_->id_);
      }
    }

    if (--count_ == 0)
      Finish();
  }
  virtual void OnBlob(CacheBlobRef blob)
  {
    if (--count_ == 0)
      Finish();
  }
  void Finish()
  {
    FLOG(log_, "Finished: %1%") % parent_;
    model_finish_();
    task_finish_();
  }

  LogContext log_;
  int     count_;
  CacheId parent_;
  boost::function<void ()> model_finish_;
  CacheThread::FinishCallback task_finish_;
  CacheThread::RequestCallback request_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QVariant CacheModelNode::data(int column, int role)const
{
  if (role == Qt::DisplayRole)
  {
    switch(column)
    {
    case 0:
      return name_;
    case 1:
      return id_;
    case 2:
      return data_;
    }
  }
  return QVariant();
}

//-----------------------------------------------------------------------------

void CacheModelNode::fetchMore()
{
  if (!requested_more_children_)
  {
    //We only want to send off a cache request once
    requested_more_children_ = true;
    if (tree_->valid_)
    {
      AddChildrenFromCache(false);
    }
    else
    {
      Cache::lease()->AddThreadTask(
        spnew<CacheModelFetchMore>("Fetch",
                                   tree_->id_, 
                                   boost::bind(InvokeMethod(model(),"IndexLoaded"),"QModelIndex",index(0))));
      AddLoadingChild();
    }
  }
}

//-----------------------------------------------------------------------------

void CacheModelNode::AddChildrenFromCache(bool remove_loading)
{
  if (remove_loading)
    RemoveChildren(0,1);

  const Tree::Children& children = tree_->data_.children_;
  more_children_ = false;

  BeginAddChildren(children.size());
  CacheModel* m = model();

  CacheModelNode* child = AddChildren(children.size());
  for(Tree::Children::const_iterator ii = children.begin();
      ii != children.end();
      ++ii, ++child)
  {
    if (ii->blob_)
      child->Init(ii->blob_, ii->name_);
    else if (ii->tree_)
      child->Init(ii->tree_, ii->name_);
  }

  EndAddChildren(m);
}

//-----------------------------------------------------------------------------

void CacheModelNode::AddLoadingChild()
{
  BeginAddChildren(1);

  CacheModel* m = model();

  CacheModelNode* child = AddChildren(1);
  child->name_ = "Loading ...";

  EndAddChildren(m);
}

//-----------------------------------------------------------------------------

void CacheModelNode::Init(CacheTreeRef tree, const QString& name)
{
  name_ = name;
  id_   = tree->id_.string();
  tree_ = tree;
  more_children_ = !(tree->valid_ && tree->data_.children_.empty());
}

//-----------------------------------------------------------------------------

void CacheModelNode::Init(CacheBlobRef blob, const QString& name)
{
  name_ = name;
  id_   = blob->id_.string();
  blob_ = blob;
  data_ = blob->data_;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CacheModel::CacheModel(CacheTreeRef tree)
: TreeModel(this)
{
  FromIndex(QModelIndex())->Init(tree, "/");
}

//-----------------------------------------------------------------------------

void CacheModel::IndexLoaded(const QModelIndex& index)
{
  FromIndex(index)->AddChildrenFromCache(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

