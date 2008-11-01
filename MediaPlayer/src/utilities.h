#ifndef UTILITIES_H
#define UTILITIES_H

#include "common.h"


QSMP_BEGIN

enum MetadataType
{
  MetadataType_FileName,
  MetadataType_Artist,

  MetadataType_Num,
};

enum SortingOrder
{
  SortingOrder_Ascending  = Qt::AscendingOrder,
  SortingOrder_Descending = Qt::DescendingOrder
};
inline SortingOrder sortingOrder(Qt::SortOrder order)
{return SortingOrder(order);}

template<class MediaEntry>
struct MediaOrdering;

template<class MediaEntry>
MediaOrdering<MediaEntry> mediaOrdering(MetadataType type, SortingOrder order)
{return MediaOrdering<MediaEntry>(type,order);}

class Entry
{
public:
  Entry(const directory_entry& dir)
    : path_(dir.path())
  {
    init();
  }
  Entry(const Path& path)
    : path_(path)
  {
    init();
  }
  void init()
  {
#if 1
    ID3_Tag tag(path_.file_string().c_str());
    ID3_Frame* frame = tag.Find(ID3FID_LEADARTIST);
    if (frame)
    {
      ID3_Field* field = frame->GetField(ID3FN_TEXT);
      if (field)
        artist_ = field->GetRawText();
    }
#endif
  }
  std::string artist_;
  Path path_;
  uint queue_index_;
  int  history_index_;
};
template<>
struct MediaOrdering<Entry> 
  : std::binary_function<const Entry&, const Entry&, bool>
{
  MediaOrdering(MetadataType type, SortingOrder order)
    : type_(type),order_(order)
  {}

  bool operator()(const Entry& l, const Entry& r)const
  {
    switch (type_)
    {
    case MetadataType_FileName:
      return (order_ == SortingOrder_Ascending) ? (l.path_ < r.path_) : (r.path_ < l.path_);
      break;
    case MetadataType_Artist:
      return (order_ == SortingOrder_Ascending) ? (l.artist_ < r.artist_) : (r.artist_ < l.artist_);
      break;
    default:
      return false;
    }
  }
  MetadataType type_;
  SortingOrder order_;
};

template<class Pred>
struct TestExtension 
{
  TestExtension(const Pred& pred)
    : pred_(pred)
  {}
  bool operator()(const Entry& entry)const
  {
    return pred_(entry.path_.extension());
  }

  template<class Path>
  bool operator()(const boost::filesystem::basic_directory_entry<Path>& dir)const
  {
    return pred_(dir.path().extension());
  }
  template<class String, class Traits>
  bool operator()(const boost::filesystem::basic_path<String,Traits>& path)const
  {
    return pred_(path.extension());
  }
  Pred     pred_;
};

template<class Pred>
TestExtension<Pred> testExtension(const Pred& pred)
{return TestExtension<Pred>(pred);}

template<class Range2T, class PredicateT>
struct Equals
{
  Equals(const Range2T& test, PredicateT comp)
    : test_(test),comp_(comp){}

  typedef bool result_type;

  template<typename Range1T>
  bool operator()(const Range1T& range)const
  {
    return boost::equals(range,test_,comp_);
  }
  Range2T test_;
  PredicateT comp_;
};

template<class Range2T,class PredicateT>
Equals<Range2T,PredicateT> equals(const Range2T& test, PredicateT comp = boost::is_equal())
{return Equals<Range2T,PredicateT>(test,comp);}

template<class PredicateT>
Equals<string,PredicateT> equals(const char* test, PredicateT comp = boost::is_equal())
{return Equals<string,PredicateT>(test,comp);}

template<class PredicateT>
Equals<wstring,PredicateT> equals(const wchar_t* test, PredicateT comp = boost::is_equal())
{return Equals<wstring,PredicateT>(test,comp);}

template<class Pred,
class OutputIterator,
class ValueType = typename boost::remove_reference<typename OutputIterator::reference>::type>
class OutputFilterIterator 
  : public std::iterator_traits<OutputIterator>
{
public:
  OutputFilterIterator(const Pred& pred,OutputIterator output)
    : output_(output),pred_(pred){}
  OutputFilterIterator&
    operator=(const ValueType& val)
  {
    if (pred_(val))
      *output_ = val;
    return *this;
  }

  OutputFilterIterator& operator*()
  {return *this;}

  OutputFilterIterator& operator++()
  {	
    ++output_;
    return *this;
  }

  OutputFilterIterator operator++(int)
  {	
    OutputIterator ret(*this);
    ++output_;
    return ret;
  }

protected:
  OutputIterator output_;
  Pred           pred_;
};

template<class Pred, class OutputIterator>
OutputFilterIterator<Pred,OutputIterator> outputFilterIterator(const Pred& pred, OutputIterator output)
{
  return OutputFilterIterator<Pred,OutputIterator>(pred,output);
}

template<class ValueType, class Pred, class OutputIterator>
OutputFilterIterator<Pred,OutputIterator,ValueType> valueOutputFilterIterator(const Pred& pred, OutputIterator output)
{
  return OutputFilterIterator<Pred,OutputIterator,ValueType>(pred,output);
}

template<class T>
void sort(T range, MetadataType type, SortingOrder order)
{
  BOOST_CONCEPT_ASSERT((RandomAccessRangeConcept<T>));
  BOOST_CONCEPT_ASSERT((LvalueIteratorConcept<typename range_iterator<T>::type>));
  std::sort(begin(range),end(range),mediaOrdering<range_value<T>::type>(type,order));
}

template<class T>
typename boost::range_value<T>::type* chooseRandom(T range)
{
  BOOST_CONCEPT_ASSERT((RandomAccessRangeConcept<T>));
  BOOST_CONCEPT_ASSERT((ReadableIteratorConcept<typename range_iterator<T>::type>));
  int r = rand();
  return &*(boost::begin(range) + (r % boost::size(range)));
}

QSMP_END

#endif