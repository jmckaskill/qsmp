#ifndef UTILITIES_H
#define UTILITIES_H

#include "qsmp/common.h"


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

class Metadata
{
public:
  Metadata(const directory_entry& dir)
    : path_(dir.path()),
      queue_index_(-1)
  {
    init();
  }
  Metadata(const Path& path)
    : path_(path),
      queue_index_(-1)
  {
    init();
  }
  void init()
  {
#if 0
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
  int  queue_index_;
};

class Media
{
public:
  Media()
  {}

  Media(const Path& path)
    : metadata_(new Metadata(path))
  {}
  Media(const directory_entry& path)
    : metadata_(new Metadata(path))
  {}

  bool  valid()const{return metadata_.get() != NULL;}
  const std::string& artist()const{return metadata_->artist_;}
  const Path&        path()const{return metadata_->path_;}
  uint               queue_index()const{return metadata_->queue_index_;}
  void               set_queue_index(uint index){metadata_->queue_index_ = index;}
  bool               current()const{return metadata_->queue_index_ == 0;}
private:
  shared_ptr<Metadata> metadata_;
};

template<class CharT, class traits>
std::basic_ostream<CharT,traits>& operator<<(std::basic_ostream<CharT,traits>& stream, const Media& entry)
{
  stream << entry.path().file_string();
  return stream;
}

template<>
struct MediaOrdering<Media> 
  : std::binary_function<const Media&, const Media&, bool>
{
  MediaOrdering(MetadataType type, SortingOrder order)
    : type_(type),order_(order)
  {}

  bool operator()(const Media& l, const Media& r)const
  {
    switch (type_)
    {
    case MetadataType_FileName:
      return (order_ == SortingOrder_Ascending) ? (l.path() < r.path()) : (r.path() < l.path());
      break;
    case MetadataType_Artist:
      return (order_ == SortingOrder_Ascending) ? (l.artist() < r.artist()) : (r.artist() < l.artist());
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
  bool operator()(const Media& entry)const
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
typename boost::range_reference<T>::type chooseRandom(T range)
{
  BOOST_CONCEPT_ASSERT((RandomAccessRangeConcept<T>));
  BOOST_CONCEPT_ASSERT((ReadableIteratorConcept<typename range_iterator<T>::type>));
  int r = rand();
  return *(boost::begin(range) + (r % boost::size(range)));
}

template<class T>
struct construct
{
  typedef T result_type;

  T operator()()const
  {return T();}

  template<class T1>
  T operator()(const T1& a1)const
  {return T(a1);}

  template<class T1,class T2>
  T operator()(const T1& a1, const T2& a2)const
  {return T(a1,a2);}
};


QSMP_END

#endif