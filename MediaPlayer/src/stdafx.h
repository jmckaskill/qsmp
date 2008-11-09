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

#include "qsmp/common.h"

// All third party headers should go in here (even if they are used in only one file)
// This file is then precompiled

#include <afx.h>

//STL Includes
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

//Boost Includes
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/concepts.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/random.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility/singleton.hpp>

//TBB Includes
#include <tbb/atomic.h>
#include <tbb/parallel_sort.h>
#include <tbb/tbb_thread.h>
#include <tbb/task_scheduler_init.h>

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

