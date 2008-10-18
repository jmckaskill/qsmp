#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef WINVER
#define WINVER 0x501
#endif

#endif

// All third party headers should go in here (even if they are used in only one file)
// This file is then precompiled

//STL Includes
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using std::vector;
using std::endl;
using std::string;

//Boost Includes
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
//#define BOOST_SIGNALS_NAMESPACE signalslib;
//#include <boost/signal.hpp>//this needs to be replaced by thread-safe signals
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::bind;
using boost::begin;
using boost::end;
using boost::iterator_range;
using boost::array;
//using boost::signal;
typedef boost::filesystem::path Path;
typedef boost::filesystem::basic_directory_entry<Path> directory_entry;
typedef boost::filesystem::directory_iterator directory_iterator;
typedef boost::filesystem::recursive_directory_iterator recursive_directory_iterator;

//TBB Includes
#include <tbb/atomic.h>
#include <tbb/parallel_sort.h>
#include <tbb/tbb_thread.h>
using tbb::atomic;
using tbb::parallel_sort;
typedef tbb::tbb_thread thread;

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

//Id3lib Includes
#include <id3/tag.h>

#endif
