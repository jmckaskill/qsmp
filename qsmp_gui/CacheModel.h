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

#ifndef QSMP_CACHEVIEW_H_
#define QSMP_CACHEVIEW_H_

#include <qsmp_gui/common.h>
#include <qsmp_gui/Cache.h>
#include <qsmp_gui/TreeModel.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CacheModel;

class CacheModelNode : public Model::TreeModelNode<CacheModelNode, CacheModel>
{
public:
  CacheModelNode(CacheModel* model)
    : TreeModelNode(model),
      tree_(NULL),blob_(NULL),commit_(NULL),more_children_(false),requested_more_children_(false)
  {}

  bool        canFetchMore()const{return more_children_;}
  int         columnCount()const{return 3;}
  QVariant    data(int column, int role)const;
  void        fetchMore();
  bool        hasChildren()const{return TreeModelNode::hasChildren() || more_children_;}

  void        AddChildrenFromCache(bool remove_loading);
  void        AddLoadingChild();
  void        Init(CacheTreeRef   tree, const QString& name);
  void        Init(CacheBlobRef   blob, const QString& name);
  void        Init(CacheCommitRef commit, const QString& name);

private:
  QString                    name_;
  QString                    id_;
  QString                    data_;
  CacheTreeRef               tree_;
  CacheBlobRef               blob_;
  CacheCommitRef             commit_;
  bool                       requested_more_children_;
  bool                       more_children_;
};

//-----------------------------------------------------------------------------

class CacheModel : public Model::TreeModel<CacheModelNode, CacheModel>
{
  Q_OBJECT
public:
  CacheModel(CacheTreeRef tree);

public Q_SLOTS:
  void IndexLoaded(const QModelIndex& index);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CacheView : public QTreeView
{
public:
  CacheView(const CacheId& id)
    : model_(Cache::lease()->LookupCacheTree(id))
  {
    setModel(&model_);
  }
private:
  CacheModel model_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END


#endif