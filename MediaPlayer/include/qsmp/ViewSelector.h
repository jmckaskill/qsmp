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

#ifndef QSMP_VIEWSELECTOR_H_
#define QSMP_VIEWSELECTOR_H_


#include <boost/concept_check.hpp>
#include <boost/function.hpp>
#include <boost/range.hpp>
#include <boost/static_assert.hpp>
#include <qsmp/common.h>
#include <qsmp/Log.h>
#include <qsmp/utilities.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qvariant.h>
#include <tcl/sequential_tree.h>



namespace tcl 
{
  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------

  //Element iterator
  template<class stored_type>
  boost::iterator_range<typename tcl::sequential_tree<stored_type>::iterator>
  child_range(typename tcl::sequential_tree<stored_type>::iterator it)
  {
    return boost::iterator_range<typename tcl::sequential_tree<stored_type>::iterator>(it->begin(),it->end());
  }

  //-----------------------------------------------------------------------------

  //Node iterators
  template<class stored_type>
  boost::iterator_range<typename tcl::sequential_tree<stored_type>::node_iterator>
  child_range(typename tcl::sequential_tree<stored_type>::node_iterator it)
  {
    return boost::iterator_range<typename tcl::sequential_tree<stored_type>::node_iterator>(it->node_begin(),it->node_end());
  }

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
}



QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<
  template<class ContainerStoredType, class ContainerAllocator> class Container,
  class StoredType,
  class Allocator = std::allocator<StoredType>
>
class SequenceTree 
{
public:
  typedef SequenceTree<Container,StoredType,Allocator> node_type;
  typedef Container<node_type,Allocator>               container_type;

  typedef typename container_type::value_type      value_type;
  typedef typename container_type::reference       reference;
  typedef typename container_type::const_reference const_reference;
  typedef typename container_type::pointer         pointer;
  typedef typename container_type::difference_type difference_type;
  typedef typename container_type::size_type       size_type;

  typedef typename container_type::iterator        iterator;
  typedef typename container_type::const_iterator  const_iterator;

  SequenceTree():parent_(NULL){}
  /*implicit*/ SequenceTree(const StoredType& element):element_(element),parent_(NULL){}
  SequenceTree(const SequenceTree& r)
    : element_(r.element_),
      children_(r.children_),
      parent_(NULL)
  {
    SetupChildren();
  }

  iterator          begin(){return children_.begin();}
  const_iterator    begin()const{return children_.begin();}
  iterator          end(){return children_.end();}
  const_iterator    end()const{return children_.end();}

  StoredType&       get(){return element_;}
  const StoredType& get()const{return element_;}

  node_type*        parent()const{return parent_;}
  void              set_parent(node_type* parent){parent_ = parent;}

  iterator          location()const{return location_;}
  void              set_location(iterator location){location_ = location;}

  void push_back(const node_type& child)
  {
    //We hack to determine if we need to resetup all the children or just the new one
    //Assumes that if the begin iterator has been invalidated then they probably all have been
    //In general this is sufficient since most containers either give no guarentee for iterator
    //validity after insert, or guarentee to only effect those after the insertion point
    node_type* first = children_.empty() ? NULL : &*begin();
    children_.push_back(child);
    if (&*begin() != first)
    {
      SetupChildren();
    }
    else
    {
      iterator i = end() - 1;
      i->set_parent(this);
      i->set_location(i);
    }
  }

private:
  void SetupChildren()
  {
    for(iterator i = begin(); i != end(); ++i)
    {
      i->set_parent(this);
      i->set_location(i);
    }
  }
  StoredType     element_;
  container_type children_;
  node_type*     parent_;
  iterator       location_;
};

//-----------------------------------------------------------------------------

template<class Tree, class DataMap>
class SequenceTreeMap
{
public:
  SequenceTreeMap(DataMap map = DataMap()):map_(map){}

  int column_count(Tree& t)const
  {
    return map_.column_count(t.get());
  }

  QVariant data(Tree& t, int role, int column)const
  {
    return map_.data(t.get(),role,column);
  }

  boost::iterator_range<typename Tree::iterator> child_range(Tree& t)const
  {
    return boost::iterator_range<typename Tree::iterator>(t.begin(),t.end());
  }

  Tree* parent(Tree& t)const
  {
    return t.parent();
  }

  typename Tree::iterator location(Tree& t)const
  {
    return t.location();
  }

private:
  DataMap map_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class TreeModelBase : public QAbstractItemModel
{
  Q_OBJECT
public:
  void SetView(QAbstractItemView* view)
  {
    connect(view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DoubleClicked(QModelIndex)));
    connect(view, SIGNAL(clicked(QModelIndex)), this, SLOT(Clicked(QModelIndex)));
  }
public Q_SLOTS:
  virtual void DoubleClicked(const QModelIndex& index)const=0;
  virtual void Clicked(const QModelIndex& index)const=0;
};

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
class TreeModel : public TreeModelBase
{
public:
  typedef typename boost::range_pointer<TreeRange>::type         Pointer;
  typedef typename boost::range_reference<TreeRange>::type       Reference;
  typedef typename boost::range_iterator<TreeRange>::type        Iterator;
  typedef TreeRange                                              Range;

  BOOST_CONCEPT_ASSERT((boost::RandomAccessRangeConcept<TreeRange>));
  //We need to be able to store the pointers in a QModelIndex which only gives us a void*
  BOOST_STATIC_ASSERT((sizeof(Pointer) == sizeof(void*)));

  TreeModel(boost::function<TreeRange ()>   get_range,
            TreeMap                         map = TreeMap())
            : get_range_(get_range),map_(map)
  {
    range_ = get_range_();
  }

  void  set_clicked(boost::function<void (Reference)> clicked){clicked_ = clicked;}
  void  set_double_clicked(boost::function<void (Reference)> double_clicked){double_clicked_ = double_clicked;}

  virtual int         columnCount(const QModelIndex &parent = QModelIndex() )const;
  virtual QVariant    data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  virtual bool        hasChildren(const QModelIndex &parent = QModelIndex() )const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent  = QModelIndex() )const;
  virtual QModelIndex parent(const QModelIndex& index) const;
  virtual void        reset();
  virtual int         rowCount(const QModelIndex &parent = QModelIndex() )const;
private:
  virtual void        DoubleClicked(const QModelIndex& index)const;
  virtual void        Clicked(const QModelIndex& index)const;
  Range               GetChildRange(const QModelIndex& parent)const;
  Pointer             GetNode(const QModelIndex& index)const;

  TreeRange                         range_;
  TreeMap                           map_;
  boost::function<void (Reference)> clicked_;
  boost::function<void (Reference)> double_clicked_;
  boost::function<TreeRange ()>     get_range_;
};

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
typename TreeModel<TreeRange,TreeMap>::Pointer
TreeModel<TreeRange,TreeMap>::GetNode(const QModelIndex& index)const
{
  QSMP_PROFILE(GetNode)
  void* ptr = index.internalPointer();
  return reinterpret_cast<Pointer>(ptr);
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
typename TreeModel<TreeRange,TreeMap>::Range 
TreeModel<TreeRange,TreeMap>::GetChildRange(const QModelIndex& parent)const
{
  QSMP_PROFILE(GetChildRange)
  void* ptr = parent.internalPointer();
  if (ptr)
    return map_.child_range(*reinterpret_cast<Pointer>(ptr));
  else
    return range_;
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
QModelIndex TreeModel<TreeRange,TreeMap>::index(int row, int column, const QModelIndex &parent /* = QModelIndex */)const
{
  QSMP_PROFILE(index)
  Range range = GetChildRange(parent);
  Iterator ii = boost::begin(range);
  if (row < boost::size(range))
  {
    ii += row;
    return createIndex(row, column, reinterpret_cast<void*>(&*ii));
  }
  else
  {
    return QModelIndex();
  }
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
QModelIndex TreeModel<TreeRange,TreeMap>::parent(const QModelIndex& index)const
{
  QSMP_PROFILE(parent)
  Pointer p = GetNode(index);
  if(p)
  {
    Pointer parent_p = map_.parent(*p);
    if(parent_p)
    {
      Range range = map_.child_range(*parent_p);
      if (boost::begin(range) == boost::begin(range_))
        return QModelIndex();
      Iterator location = map_.location(*p);
      return createIndex(std::distance(boost::begin(range),location),0,reinterpret_cast<void*>(p));
    }
    else
    {
      return QModelIndex();
    }
  }
  else
  {
    return QModelIndex();
  }
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
int TreeModel<TreeRange,TreeMap>::columnCount(const QModelIndex &parent /* = QModelIndex */)const
{
  QSMP_PROFILE(columnCount)
  return map_.column_count(*GetNode(parent));
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
bool TreeModel<TreeRange,TreeMap>::hasChildren(const QModelIndex &parent /* = QModelIndex */)const
{
  QSMP_PROFILE(hasChildren)
  return !boost::empty(GetChildRange(parent));
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
void TreeModel<TreeRange,TreeMap>::reset()
{
  QSMP_PROFILE(reset)
  range_ = get_range_();
  QAbstractItemModel::reset();
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
int TreeModel<TreeRange,TreeMap>::rowCount(const QModelIndex &parent /* = QModelIndex */)const
{
  QSMP_PROFILE(rowCount)
  return boost::size(GetChildRange(parent));
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
QVariant TreeModel<TreeRange,TreeMap>::data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const
{
  QSMP_PROFILE(data)
  return map_.data(*GetNode(index),role,index.column());
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
void TreeModel<TreeRange,TreeMap>::DoubleClicked(const QModelIndex& index)const
{
  QSMP_PROFILE(DoubleClicked)
  if (!double_clicked_.empty())
  {
    Pointer p = GetNode(index);
    if (p)
      double_clicked_(*p);
  }
}

//-----------------------------------------------------------------------------

template<class TreeRange, class TreeMap>
void TreeModel<TreeRange,TreeMap>::Clicked(const QModelIndex& index)const
{
  QSMP_PROFILE(Clicked)
  if (!clicked_.empty())
  {
    Pointer p = GetNode(index);
    if (p)
      clicked_(*p);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct ViewEntry
{
  boost::function<QLayout* ()> new_view_;
  QString                      text_;
};

//-----------------------------------------------------------------------------

struct ViewEntryMap
{
  int column_count(const ViewEntry& entry)const
  {
    return 1;
  }

  QVariant data(const ViewEntry& entry, int role, int row)const
  {
    if (role == Qt::DisplayRole && 
        row  == 0)
    {
      return entry.text_;
    }
    return QVariant();
  }
};

//-----------------------------------------------------------------------------

class ViewSelector : public QListView
{
public:
  ViewSelector(QWidget* parent_widget);

  void AddViewEntry(boost::function<QLayout* ()> new_view,
                    const std::string&           text);
private:
  typedef SequenceTree<std::vector,ViewEntry> Tree;
  typedef TreeModel<boost::iterator_range<Tree::iterator>,
                    SequenceTreeMap<Tree,ViewEntryMap> > Model;

  void  EntryDoubleClicked(const Tree& node);
  Tree& tree(){return tree_;}

  Tree      tree_;
  QWidget*  parent_widget_;
  Model*    model_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif