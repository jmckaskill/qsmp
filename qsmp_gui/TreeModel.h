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

#ifndef QSMP_TREEMODEL_H_
#define QSMP_TREEMODEL_H_

#include <qsmp_gui/common.h>
#include <QtCore/QAbstractItemModel>
#include <vector>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Node, class Model>
class TreeModel;

template<class Node, class Model>
class TreeModelNode
{
public:
  //Constructors in Node are expected to take these as their first arguments and toss on down
  TreeModelNode(Model* model);

  //Should be overridden in Node
  bool             canFetchMore()const{return false;}
  int              columnCount()const{return 0;}
  QVariant         data(int column, int role)const{return QVariant();}
  void             fetchMore(){}

  //Can be overridden in Node .. but should call as well
  bool             hasChildren()const{return !children_.empty();}

  //Shouldn't be overridden
  int              rowCount()const{return children_.size();}
  QModelIndex      index(int col)const;

protected:
  void             BeginAddChildren(size_t count);
  void             BeginInsertChildren(int row, size_t count);
  //Since this is normally called after calling AddChildren we can't use local variables
  static void      EndAddChildren(Model* model){EndInsertChildren(model);}
  static void      EndInsertChildren(Model* model);

  void             RemoveChildren(int row, size_t count);

  //Create a certain number of children and set them up
  //WARNING: after a call to either of these two all node pointers may be invalidated (including "this")
  //         as these will call a resize on the vector where this node is stored
  //         thus copy out all data to the stack before calling these
  Node*            AddChildren(size_t count);
  Node*            InsertChildren(int row, size_t count);

  void             ChildrenUpdated(int row, size_t count)const;

  Node*            child(int row);
  const Node*      child(int row)const;

  Model*           model()const{return model_;}

private:
  friend class TreeModel<Node, Model>;
  template<class StreamNode, class StreamModel>
  friend std::ostream& operator<<(std::ostream& stream, const TreeModelNode<StreamNode, StreamModel>& node);
  Model*           model_;
  int              id_;
  int              row_;
  int              parent_;
  std::vector<int> children_;
};

//-----------------------------------------------------------------------------

template<class Node, class Model>
std::ostream& operator<<(std::ostream& stream, const TreeModelNode<Node, Model>& node)
{
  for (int i = 0; i < node.rowCount(); i++)
  {
    stream << i << ":";

    const Node* child = node.child(i);
    for (int j = 0; j < child->columnCount(); j++)
      stream << "\t" << child->data(j, Qt::DisplayRole).toString();

    stream << std::endl;
  }
  return stream;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Node, class Model>
class TreeModel : public QAbstractItemModel
{
public:
  TreeModel(Model* model);

  //In general these shouldn't be further overridden but rather logic put in the Node class instead
  virtual bool        canFetchMore(const QModelIndex &parent)const
  {return FromIndex(parent)->canFetchMore();}
  virtual int         columnCount(const QModelIndex &parent = QModelIndex() )const
  {return FromIndex(parent)->columnCount();}
  virtual QVariant    data(const QModelIndex &index, int role = Qt::DisplayRole)const
  {return FromIndex(index)->data(index.column(), role);}
  virtual void        fetchMore(const QModelIndex &parent)
  {return FromIndex(parent)->fetchMore();}
  virtual bool        hasChildren(const QModelIndex &parent = QModelIndex() )const
  {return FromIndex(parent)->hasChildren();}
  virtual int         rowCount(const QModelIndex &parent = QModelIndex() )const
  {return FromIndex(parent)->rowCount();}

  virtual QModelIndex index(int row, int column, const QModelIndex &parent  = QModelIndex() )const;
  virtual QModelIndex parent(const QModelIndex& index) const;

protected:
  Node*               FromIndex(const QModelIndex& index);
  const Node*         FromIndex(const QModelIndex& index)const;
  QModelIndex         ToIndex(const Node* node, int col)const;

  Node*               root(){return FromIndex(QModelIndex());}

private:
  friend class TreeModelNode<Node, Model>;
  std::pair<int,Node*>        AddNodes(size_t count);

  std::vector<Node>   data_;
  Model*              model_;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif

#include <qsmp_gui/TreeModel.inl>
