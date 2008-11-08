#ifndef QSMP_VIEWSELECTOR_H_
#define QSMP_VIEWSELECTOR_H_

#include "qsmp/common.h"
#include "tcl/sequential_tree.h"

namespace tree
{
  // undefined_range    child_range(iterator parent);

  template<class Iterator>
  boost::iterator_range<Iterator> child_range(Iterator it)
  {
    return tree::child_range(*it);
  }

  //Element iterator
  template<class stored_type>
  boost::iterator_range<typename tcl::sequential_tree<stored_type>::iterator>
  child_range(typename tcl::sequential_tree<stored_type>::iterator it)
  {
    return boost::iterator_range<typename tcl::sequential_tree<stored_type>::iterator>(it->begin(),it->end());
  }

  //Node iterators
  template<class stored_type>
  boost::iterator_range<typename tcl::sequential_tree<stored_type>::node_iterator>
  child_range(typename tcl::sequential_tree<stored_type>::node_iterator it)
  {
    return boost::iterator_range<typename tcl::sequential_tree<stored_type>::node_iterator>(it->node_begin(),it->node_end());
  }

  struct ChildRange
  {
    template<class Iterator>
    boost::iterator_range<Iterator> operator()(Iterator it)const
    {
      return tree::child_range(it);
    }
  };


  struct Data
  {
    template<class Iterator>
    int row_count(Iterator it)const
    {
      return tree::row_count(*it);
    }
    template<class Iterator>
    QVariant operator()(Iterator it, int role)const
    {
      return tree::data(*it,role);
    }
  };

}

QSMP_BEGIN

template<class T, class Data = tree::Data, class ChildRange = tree::ChildRange>
class TreeModel : public QAbstractItemModel
{
public:
  typedef typename boost::range_reference<T>::type       Reference;
  typedef typename boost::range_iterator<T>::type        Iterator;
  typedef typename boost::iterator_range<Iterator>::type Range;

  BOOST_CONCEPT_ASSERT((RandomAccessRangeConcept<T>));
  //We need to be able to store the iterators in a QModelIndex which only gives us a void*
  BOOST_STATIC_ASSERT((sizeof(Iterator) == sizeof(void*)));

  TreeModel(T           range,
            Iterator    root_node   = Iterator(),
            Data        data_mapper = Data(),
            ChildRange  child_range = ChildRange());

  void  set_selected(boost::function<void (Reference)> selected){selected_ = selected;}
  void  set_double_clicked(boost::function<void (Reference)> double_clicked){double_clicked_ = double_clicked;}

  virtual int         columnCount(const QModelIndex &parent = QModelIndex() )const;
  virtual QVariant    data(const QModelIndex &index, int role = Qt::DisplayRole)const;
  virtual bool        hasChildren(const QModelIndex &parent = QModelIndex() )const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent  = QModelIndex() )const;
  virtual QModelIndex parent(const QModelIndex& index) const;
  virtual int         rowCount(const QModelIndex &parent = QModelIndex() )const;
private:
  Range               GetRange(const QModelIndex& parent)const;
  Iterator            GetNode(const QModelIndex& index)const;

  Data                              data_;
  ChildRange                        child_range_;
  Iterator                          root_node_;
  T                                 range_;
  boost::function<void (Reference)> selected_;
  boost::function<void (Reference)> double_clicked_;
};

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
typename TreeModel<T,Data,ChildRange>::Iterator 
TreeModel<T,Data,ChildRange>::GetNode(const QModelIndex& index)const
{
  void* ptr = parent.internalPointer();
  if (!ptr)
    return boost::begin(range_);
  else
    return it = *reinterpret_cast<Iterator*>(ptr);
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
typename TreeModel<T,Data,ChildRange>::Range 
TreeModel<T,Data,ChildRange>::GetRange(const QModelIndex& parent)const
{
  void* ptr = parent.internalPointer();
  if (!ptr)
    return range_;
  else
    return tree::child_range(*reinterpret_cast<Iterator*>(ptr));
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
QModelIndex TreeModel<T,Data,ChildRange>::index(int row, int column, const QModelIndex &parent /* = QModelIndex */)const
{
  Range range = GetRange(parent);
  Iterator ii = boost::begin(range);
  int i = 0;
  while(ii != boost::end(range) && i < row)
  {
    ++ii;
    ++i;
  }
  if (i == row)
    return createIndex(row, column, reinterpret_cast<void*>(ii));
  else
    return QModelIndex();
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
int TreeModel<T,Data,ChildRange>::columnCount(const QModelIndex &parent /* = QModelIndex */)const
{
  return boost::size(GetRange(parent));
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
bool TreeModel<T,Data,ChildRange>::hasChildren(const QModelIndex &parent /* = QModelIndex */)const
{
  return !boost::empty(GetRange(parent));
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
int TreeModel<T,Data,ChildRange>::rowCount(const QModelIndex &parent /* = QModelIndex */)const
{
  return data_.row_count(GetNode(parent));
}

//-----------------------------------------------------------------------------

template<class T, class Data, class ChildRange>
QVariant TreeModel<T,Data,ChildRange>::data(const QModelIndex &index, int role /* = Qt::DisplayRole */)const
{
  return data_(GetNode(index),role);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif