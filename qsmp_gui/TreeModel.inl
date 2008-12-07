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

#ifndef QSMP_TREEMODEL_INL_
#define QSMP_TREEMODEL_INL_

#include <boost/tuple/tuple.hpp>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Node, class Model> inline
TreeModelNode<Node, Model>::TreeModelNode(Model* model)
: model_(model),
  id_(0),
  row_(-1),
  parent_(0)
{
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
QModelIndex TreeModelNode<Node, Model>::index(int col)const
{
  if (row_ == -1)
    return QModelIndex();
  else
    return model_->createIndex(row_, col, id_);
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
void TreeModelNode<Node, Model>::BeginAddChildren(size_t count)
{
  BeginInsertChildren(children_.size(), count);
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
void TreeModelNode<Node, Model>::BeginInsertChildren(int row, size_t count)
{
  model_->beginInsertRows(index(0), row, row + count - 1);
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
void TreeModelNode<Node, Model>::EndInsertChildren(Model* model)
{
  model->endInsertRows();
}

//-----------------------------------------------------------------------------

template<class Node, class Model>
void TreeModelNode<Node, Model>::RemoveChildren(int row, size_t count)
{
  model_->beginRemoveRows(index(0), row, row + count - 1);
  children_.erase(children_.begin() + row, children_.begin() + row + count);

  for(size_t i = row; i < children_.size(); i++)
  {
    child(i)->row_ -= count;
  }

  model_->endRemoveRows();
}

//-----------------------------------------------------------------------------

template<class Node, class Model>
Node* TreeModelNode<Node, Model>::AddChildren(size_t count)
{
  using boost::tie;
  size_t row = children_.size();
  children_.insert(children_.end(), count, 0);

  TreeModel<Node, Model>* model = model_;

  int child_id = model->data_.size();
  for(size_t i = row; i < row + count; i++)
  {
    children_[i] = child_id++;
  }

  int begin;
  Node* pbegin;
  int id = id_;
  
  //After this point 'this' may be invalid as AddNodes may cause a realloc of TreeModel::data_ where
  //we are stored
  tie(begin,pbegin) = model->AddNodes(count);


  child_id = begin;
  for(Node* i = pbegin; i < pbegin + count; i++)
  {
    TreeModelNode* node = static_cast<TreeModelNode*>(i);
    node->row_    = row++;
    node->parent_ = id;
    node->id_     = child_id++;
  }

  return pbegin;
}

//-----------------------------------------------------------------------------

template<class Node, class Model>
Node* TreeModelNode<Node, Model>::InsertChildren(int row, size_t count)
{
  using boost::tie;
  for(size_t i = row; i < children_.size(); i++)
  {
    child(i)->row_ += count;
  }

  children_.insert(children_.begin() + row, count, 0);

  TreeModel<Node, Model>* model = model_;

  int child_id = model->data_.size();
  for(size_t i = row; i < row + count; i++)
  {
    children_[i] = child_id++;
  }

  int begin;
  Node* pbegin;
  int id = id_;

  //After this point 'this' may be invalid as AddNodes may cause a realloc of TreeModel::data_ where
  //we are stored
  tie(begin,pbegin) = model->AddNodes(count);

  child_id = begin;
  for(Node* i = pbegin; i < pbegin + count; i++)
  {
    TreeModelNode* node = static_cast<TreeModelNode*>(i);
    node->row_    = row++;
    node->parent_ = id;
    node->id_     = child_id++;
  }

  return pbegin;
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
void TreeModelNode<Node, Model>::ChildrenUpdated(int row, size_t count)const
{
  const Node* child_begin = child(row);
  const Node* child_end   = child(row+count);
  model_->dataChanged(child_begin->index(0), child_end->index(child_end->rowCount()));
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
Node* TreeModelNode<Node, Model>::child(int row)
{
  return &model_->data_[children_[row]];
}

template<class Node, class Model> inline
const Node* TreeModelNode<Node, Model>::child(int row)const
{
  return &model_->data_[children_[row]];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Node, class Model> inline
TreeModel<Node, Model>::TreeModel(Model* model)
: model_(model)
{
  //Add the root node
  data_.push_back(Node(model));
}

//-----------------------------------------------------------------------------

template<class Node, class Model>
QModelIndex TreeModel<Node, Model>::index(int row, int column, const QModelIndex &parent /* = QModelIndex */)const
{
  const Node* node = FromIndex(parent);
  if (!(0 <= row && row < (int)node->children_.size()))
    return QModelIndex();

  return createIndex(row, column, node->children_[row]);
}

//-----------------------------------------------------------------------------

template<class Node, class Model>
QModelIndex TreeModel<Node, Model>::parent(const QModelIndex& index)const
{
  const Node* node   = FromIndex(index);
  const Node* parent = &data_[node->parent_];
  if (parent->row_ == -1)
    return QModelIndex();
  else
    return createIndex(parent->row_, 0, node->parent_);
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
Node* TreeModel<Node, Model>::FromIndex(const QModelIndex& index)
{
  return &data_[(size_t)index.internalPointer()];
}

template<class Node, class Model> inline
const Node* TreeModel<Node, Model>::FromIndex(const QModelIndex& index)const
{
  return &data_[(size_t)index.internalPointer()];
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
QModelIndex TreeModel<Node, Model>::ToIndex(const Node* node, int col)const
{
  if (node->row_ == -1)
    return QModelIndex();
  else
    return createIndex(node->row_, col, node - &data_[0]);
}

//-----------------------------------------------------------------------------

template<class Node, class Model> inline
std::pair<int,Node*> TreeModel<Node, Model>::AddNodes(size_t count)
{
  int   begin  = data_.size();
  data_.insert(data_.end(), count, Node(model_));
  Node* pbegin = &data_[begin];
  return std::make_pair(begin,pbegin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif

