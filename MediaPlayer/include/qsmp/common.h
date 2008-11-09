#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32

//#ifndef _UNICODE
//#define _UNICODE
//#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef WINVER
#define WINVER 0x501
#endif

#define _AFXDLL

#include <afx.h>

#endif

#define QSMP_BEGIN namespace qsmp {
#define QSMP_END }

// All third party headers should go in here (even if they are used in only one file)
// This file is then precompiled

//STL Includes
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

QSMP_BEGIN
using std::vector;
using std::endl;
using std::string;
using std::wstring;
QSMP_END

//Boost Includes
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/concepts.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/random.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/spirit/home/phoenix/core.hpp>
//#include <boost/spirit/home/phoenix/function.hpp>
//#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/utility/singleton.hpp>
//#define BOOST_SIGNALS_NAMESPACE signalslib;
//#include <boost/signal.hpp>//this needs to be replaced by thread-safe signals

QSMP_BEGIN
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::bind;
using boost::ref;
using boost::cref;
using boost::begin;
using boost::end;
using boost::range_value;
using boost::iterator_range;
using boost::range_iterator;
using boost::function_requires;
using namespace boost_concepts;
using boost::RandomAccessRangeConcept;
using boost::array;
using boost::tuples::tuple;
using boost::tuples::make_tuple;
using boost::tuples::tie;
using boost::mutex;
using boost::lock_guard;
//using namespace boost::phoenix::arg_names; //brings in arg1 etc and _1 etc
//using boost::phoenix::function;
//using boost::phoenix::bind;
//using boost::phoenix::ref;
//using boost::phoenix::cref;
//using boost::signal;
typedef boost::filesystem::path Path;
typedef boost::filesystem::basic_directory_entry<Path> directory_entry;
typedef boost::filesystem::directory_iterator directory_iterator;
typedef boost::filesystem::recursive_directory_iterator recursive_directory_iterator;
QSMP_END

//TBB Includes
#include <tbb/atomic.h>
#include <tbb/parallel_sort.h>
#include <tbb/tbb_thread.h>
#include <tbb/task_scheduler_init.h>

QSMP_BEGIN
using tbb::atomic;
using tbb::parallel_sort;
typedef tbb::tbb_thread thread;
QSMP_END

//Lua Includes
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//Qt Includes
#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>
#include <phonon/videowidget.h>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>

//Id3lib Includes
#include <id3/tag.h>

#undef QT_BEGIN_MOC_NAMESPACE
#undef QT_END_MOC_NAMESPACE
#define QT_BEGIN_MOC_NAMESPACE QSMP_BEGIN
#define QT_END_MOC_NAMESPACE QSMP_END


#define QSMP_NON_COPYABLE(class_name) \
  private:  \
    class_name(const class_name&);  \
    class_name& operator=(const class_name&);


#endif
