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

#ifndef QSMP_LOG_H_
#define QSMP_LOG_H_


#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/utility/singleton.hpp>
#include <iostream>
#include <fstream>
#include <qsmp/common.h>
#include <string>
#include <tcl/tree.h>
#include <utility>


#define QSMP_LOG_NEW_SCOPE(context) qsmp::LogEntryBuffer _qsmp_logger(context,__FILE__,__LINE__,false)
#define QSMP_LOG_SCOPE _qsmp_logger.StartLogEntry()
#define QSMP_LOG(context) qsmp::LogEntryBuffer(context,__FILE__,__LINE__,true)


QSMP_BEGIN

class LogEntryBuffer;

enum LogOutput
{
  LogOutput_Trace,
  LogOutput_Stderr,
  LogOutput_LogFile,

  LogOutput_Num,
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct LogContextData
{
  LogContextData()
  {
    std::fill(outputs_.begin(),outputs_.end(),true);
  }
  LogContextData(const LogContextData& parent, const std::string& key)
    : key_(key)
  {
    std::copy(parent.outputs_.begin(),parent.outputs_.end(),outputs_.begin());
    if (parent.full_key_.empty())
      full_key_ = key;
    else
      full_key_ = parent.full_key_ + "/" + key;
  }
  std::string full_key_;
  std::string key_;

  boost::array<bool, LogOutput_Num> outputs_;

  bool operator==(boost::iterator_range<const char*> str)const
  {
    return boost::equals(key_,str);
  }
  bool operator==(const std::string& str)const
  {
    return boost::equals(key_,str);
  }
  bool operator<(const LogContextData& r)const
  {return key_ < r.key_;}
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef tcl::tree<LogContextData>::iterator LogContext;
typedef std::pair<LogContext, std::locale> LogBufferData;

void QtMsgHandler(QtMsgType type, const char* buf);

//-----------------------------------------------------------------------------

class LogManager : public boost::singleton<LogManager>
{
public:
  LogManager(boost::restricted);

  LogContext  GetContext(LogContext context, const char* sub_context);
  LogContext  GetContext(LogContext context){return context;}
  LogContext  GetContext(const char* context){return GetContext(LogContext(),context);}

  void         LogOutput(LogEntryBuffer* buffer);

  const std::locale& locale()const{return locale_;}
private:
  boost::mutex     stderr_lock_;
  boost::mutex     file_lock_;
  std::ofstream    file_log_;
  std::locale      locale_;
  typedef tcl::tree<LogContextData> LogTree;
  LogTree logs_;
};

//-----------------------------------------------------------------------------

inline LogContext GetLogContext(const char* context)
{return LogManager::lease()->GetContext(context);}

inline LogContext GetLogContext(LogContext context, const char* sub_context)
{return LogManager::lease()->GetContext(context,sub_context);}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class LogEntryBuffer : boost::noncopyable
{
  QSMP_NON_COPYABLE(LogEntryBuffer)
public:
  LogEntryBuffer(const char* context, const char* file_name, int line_no, bool start);
  LogEntryBuffer(LogContext context, const char* file_name, int line_no, bool start);
  ~LogEntryBuffer();
  LogEntryBuffer& StartLogEntry();



  std::stringstream        string_buffer_;
private:
  friend class LogManager;
  LogContext          context_;
  const char*         file_name_;
  int                 line_no_;
  bool                have_started_;
  LogManager::lease   manager_;
};

//-----------------------------------------------------------------------------

//We need to wrap all calls to << to get ADL to work correctly
// ie we want last stage lookup of our type T to be with a type in std
template<class T>
LogEntryBuffer& operator<<(LogEntryBuffer& buffer, const T& a)
{
  buffer.string_buffer_ << a;
  return buffer;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif // QSMP_LOG_H_