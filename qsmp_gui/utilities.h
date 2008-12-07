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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <boost/algorithm/string/predicate.hpp>
#include <boost/concept_check.hpp>
#include <boost/filesystem.hpp>
#include <boost/range.hpp>
#include <boost/range/concepts.hpp>
#include <boost/shared_ptr.hpp>
#include <iterator>
#include <ostream>
#include <qsmp_gui/common.h>
#include <qsmp_lib/Log.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtGui/qwidget.h>
#include <string>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum MetadataType
{
  MetadataType_FileName,
  MetadataType_Artist,

  MetadataType_Num,
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Metadata
{
public:
  template<class Path>
  Metadata(const boost::filesystem::basic_directory_entry<Path>& dir)
    : path_(QString::fromStdString(dir.path().file_string())),
      queue_index_(-1)
  {
    init();
  }
  Metadata(const boost::filesystem::path& path)
    : path_(QString::fromStdString(path.file_string())),
      queue_index_(-1)
  {
    init();
  }
  Metadata(const QString& path)
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
  QString artist_;
  QString path_;
  int  queue_index_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class Media
{
public:
  Media()
  {}

  template<class Path>
  Media(const boost::filesystem::basic_directory_entry<Path>& dir)
    : metadata_(new Metadata(dir))
  {}

  bool  valid()const{return metadata_.get() != NULL;}
  QString  artist()const{return metadata_->artist_;}
  QString  path()const{return metadata_->path_;}
  uint     queue_index()const{return metadata_->queue_index_;}
  void     set_queue_index(uint index){metadata_->queue_index_ = index;}
  bool     current()const{return metadata_->queue_index_ == 0;}

  bool     operator==(const Media& r)const
  {return metadata_ == r.metadata_;}
private:
  boost::shared_ptr<Metadata> metadata_;
};

//-----------------------------------------------------------------------------

template<class CharT, class traits>
std::basic_ostream<CharT,traits>& operator<<(std::basic_ostream<CharT,traits>& stream, const Media& entry)
{
  stream << entry.path();
  return stream;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum SortingOrder
{
  SortingOrder_Ascending  = Qt::AscendingOrder,
  SortingOrder_Descending = Qt::DescendingOrder
};

//-----------------------------------------------------------------------------

inline SortingOrder sortingOrder(Qt::SortOrder order)
{return SortingOrder(order);}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Media_t>
struct MediaOrdering;

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

template<class MediaEntry>
MediaOrdering<MediaEntry> mediaOrdering(MetadataType type, SortingOrder order)
{return MediaOrdering<MediaEntry>(type,order);}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Pred>
struct TestExtension 
{
  TestExtension(const Pred& pred)
    : pred_(pred)
  {}
  bool operator()(const Media& entry)const
  {
    return pred_(entry.path());
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

//-----------------------------------------------------------------------------

template<class Pred>
TestExtension<Pred> testExtension(const Pred& pred)
{return TestExtension<Pred>(pred);}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

template<class Range2T,class PredicateT>
Equals<Range2T,PredicateT> equals(const Range2T& test, PredicateT comp = boost::is_equal())
{return Equals<Range2T,PredicateT>(test,comp);}

//-----------------------------------------------------------------------------

template<class PredicateT>
Equals<std::string,PredicateT> equals(const char* test, PredicateT comp = boost::is_equal())
{return Equals<std::string,PredicateT>(test,comp);}

//-----------------------------------------------------------------------------

template<class PredicateT>
Equals<std::wstring,PredicateT> equals(const wchar_t* test, PredicateT comp = boost::is_equal())
{return Equals<std::wstring,PredicateT>(test,comp);}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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
      *output_ = ValueType(val);
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

//-----------------------------------------------------------------------------

template<class Pred, class OutputIterator>
OutputFilterIterator<Pred,OutputIterator> outputFilterIterator(const Pred& pred, OutputIterator output)
{
  return OutputFilterIterator<Pred,OutputIterator>(pred,output);
}

//-----------------------------------------------------------------------------

template<class ValueType, class Pred, class OutputIterator>
OutputFilterIterator<Pred,OutputIterator,ValueType> valueOutputFilterIterator(const Pred& pred, OutputIterator output)
{
  return OutputFilterIterator<Pred,OutputIterator,ValueType>(pred,output);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T>
void sort(T range, MetadataType type, SortingOrder order)
{
  BOOST_CONCEPT_ASSERT((boost::RandomAccessRangeConcept<T>));
  BOOST_CONCEPT_ASSERT((boost_concepts::LvalueIteratorConcept<typename boost::range_iterator<T>::type>));
  std::sort(boost::begin(range),boost::end(range),mediaOrdering<typename boost::range_value<T>::type>(type,order));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T>
typename boost::range_reference<T>::type chooseRandom(T range)
{
  BOOST_CONCEPT_ASSERT((boost::RandomAccessRangeConcept<T>));
  BOOST_CONCEPT_ASSERT((boost_concepts::ReadableIteratorConcept<typename boost::range_iterator<T>::type>));
  int r = rand();
  return *(boost::begin(range) + (r % boost::size(range)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T>
struct Construct
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T>
struct New
{
  typedef T* result_type;

  T* operator()()const
  {return new T();}

  template<class T1>
  T* operator()(const T1& a1)const
  {return new T(a1);}

  template<class T1,class T2>
  T* operator()(const T1& a1, const T2& a2)const
  {return new T(a1,a2);}
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class Layout>
struct NewLayout
{
  typedef Layout* result_type;

  Layout* operator()(QWidget* widget)const
  {
    Layout* layout = new Layout;
    layout->addWidget(widget);
    return layout;
  }

  Layout* operator()(QWidget* widget1, QWidget* widget2)const
  {
    Layout* layout = new Layout;
    layout->addWidget(widget1);
    layout->addWidget(widget2);
    return layout;
  }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class QtLayout>
class LayoutWidget : public QWidget
{
public:
  LayoutWidget(QWidget* widget)
  {
    QtLayout* layout = new QtLayout;
    setLayout(layout);
    layout->addWidget(widget);
  }

  LayoutWidget(QWidget* widget1, QWidget* widget2)
  {
    QtLayout* layout = new QtLayout;
    setLayout(layout);
    layout->addWidget(widget1);
    layout->addWidget(widget2);
  }

};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//TODO(james): These should be replaced with something more generic ... tuple/fusion?

template<class First>
struct Select1st
{
  typedef First& result_type;

  template<class Second>
  First& operator()(std::pair<First,Second>& pair)const
  {
    return pair.first;
  }
};

//-----------------------------------------------------------------------------

template<class First>
struct Select1stConst
{
  typedef const First& result_type;

  template<class Second>
  const First& operator()(const std::pair<First,Second>& pair)const
  {
    return pair.first;
  }
};

//-----------------------------------------------------------------------------

template<class Second>
struct Select2nd
{
  typedef Second& result_type;

  template<class First>
  Second& operator()(std::pair<First,Second>& pair)const
  {
    return pair.second;
  }
};

//-----------------------------------------------------------------------------

template<class Second>
struct Select2ndConst
{
  typedef const Second& result_type;

  template<class First>
  const Second& operator()(const std::pair<First,Second>& pair)const
  {
    return pair.second;
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef WIN32

struct ScopeProfile
{
  ScopeProfile(DWORD& ticks)
    : ticks_(ticks),
    tick_(::GetTickCount())
  {}
  ~ScopeProfile()
  {
    ticks_ += ::GetTickCount() - tick_;
  }
  DWORD& ticks_;
  DWORD  tick_;
};

//-----------------------------------------------------------------------------

#define QSMP_PROFILE(context, function) \
  static int i = 0;\
  static DWORD ticks = 0;\
  ScopeProfile _profile(ticks);\
  if (i++ > 10000)\
  {\
  LOG(context) << "PROFILE: " #function << " " << ticks;\
    ticks = 0;\
    i = 0;\
  }

#else
#define QSMP_PROFILE(context, function)
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class InvokeMethod
{
public:
  typedef void result_type;

  InvokeMethod(QObject* obj, const char* member,
               Qt::ConnectionType type = Qt::AutoConnection,
               QGenericReturnArgument ret = QGenericReturnArgument())
               : obj_(obj),member_(member),type_(type),ret_(ret)
  {
  }

  template<class T0>
  void operator()(const char* T0Name, const T0& a0)const
  {
    QMetaObject::invokeMethod(obj_,member_,type_,ret_,QArgument<T0>(T0Name,a0));
  }

private:
  QObject*               obj_;
  const char*            member_;
  Qt::ConnectionType     type_;
  QGenericReturnArgument ret_;
  
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T>
boost::shared_ptr<T> spnew()
{return boost::shared_ptr<T>(new T());}

//-----------------------------------------------------------------------------

template<class T, class T0>
boost::shared_ptr<T> spnew(const T0& a0)
{return boost::shared_ptr<T>(new T(a0));}

//-----------------------------------------------------------------------------

template<class T, class T0, class T1>
boost::shared_ptr<T> spnew(const T0& a0, const T1& a1)
{return boost::shared_ptr<T>(new T(a0, a1));}
//-----------------------------------------------------------------------------

template<class T, class T0, class T1, class T2>
boost::shared_ptr<T> spnew(const T0& a0, const T1& a1, const T2& a2)
{return boost::shared_ptr<T>(new T(a0, a1, a2));}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct NullDeleter
{
  template<class T>
  void operator()(T*)
  {
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class CharT, class traits>
std::basic_ostream<CharT,traits>& operator<<(std::basic_ostream<CharT,traits>& stream, const QString& string)
{
  QByteArray utf8 = string.toUtf8();
  stream << utf8.data();
  return stream;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


#endif
