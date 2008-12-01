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


#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/utility/singleton.hpp>
#include <iostream>
#include <fstream>
#include <qsmp_gui/common.h>
#include <string>
#include <tcl/tree.h>
#include <utility>


#define LOG(context) qsmp::Log(context,LogSeverity_Normal,__FILE__,__LINE__)
#define WARNING(context) qsmp::Log(context,LogSeverity_Warning,__FILE__,__LINE__)
#define FATAL(context) qsmp::Log(context,LogSeverity_Fatal,__FILE__,__LINE__)
#define FLOG(context,format_string) qsmp::FormatLog(context,format_string,LogSeverity_Normal,__FILE__,__LINE__)
#define FWARNING(context,format_string) qsmp::FormatLog(context,format_string,LogSeverity_Warning,__FILE__,__LINE__)
#define FFATAL(context,format_string) qsmp::FormatLog(context,format_string,LogSeverity_Fatal,__FILE__,__LINE__)
#define ASSERTE(context, statement) \
  {\
    if (!(statement))  \
     FATAL(context) << #statement ; \
  }

#ifdef WIN32
#define WIN32_FATAL(context) qsmp::Win32Fatal(context,__FILE__,__LINE__)
#endif


QSMP_BEGIN

struct TestLog
{
};
template<class T>
TestLog& operator%(TestLog& log, const T&)
{
  return log;
}
template<class T>
TestLog& operator<<(TestLog& log, const T&)
{
  return log;
}

class Log;

enum LogOutput
{
  LogOutput_Any,
  LogOutput_Trace,
  LogOutput_Stderr,
  LogOutput_LogFile,

  LogOutput_Num,
};

enum LogSeverity
{
  LogSeverity_Normal,
  LogSeverity_Warning,
  LogSeverity_Fatal,

  LogSeverity_Num,
};

enum LogDefaults
{
  LogDefaults_Disable,
  LogDefaults_Enable,
  LogDefaults_Inherit,
  LogDefaults_Default = LogDefaults_Inherit,
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class LogContextData
{
public:
  LogContextData()
  {
    set_defaults(LogDefaults_Default);
  }
  LogContextData(const LogContextData& parent, const std::string& key, LogDefaults defaults)
    : key_(key)
  {
    if (defaults == LogDefaults_Inherit)
      std::copy(parent.outputs_.begin(),parent.outputs_.end(),outputs_.begin());
    else
      set_defaults(defaults);

    if (parent.full_key_.empty())
      full_key_ = key;
    else
      full_key_ = parent.full_key_ + "/" + key;
  }

  bool log(LogOutput output, LogSeverity severity)
  {
    return outputs_[severity*LogOutput_Num + output];
  }
  bool mute(LogSeverity severity)
  {
    return !log(LogOutput_Any,severity);
  }

  void set_defaults(LogDefaults defaults)
  {
    std::fill(outputs_.begin(),outputs_.end(),true);
    if (defaults == LogDefaults_Disable)
      set_log(LogOutput_Any, LogSeverity_Normal, false);
  }

  void set_log(LogOutput output, LogSeverity severity, bool log)
  {
    outputs_[severity*LogOutput_Num + output] = log;
  }

  const std::string& key()const{return key_;}
  const std::string& full_key()const{return full_key_;}

  bool operator<(const LogContextData& r)const
  {
    return key_ < r.key_;
  }
  bool operator==(boost::iterator_range<const char*> string)
  {
    return boost::equals(key_, string);
  }


private:
  boost::array<bool, LogSeverity_Num * LogOutput_Num> outputs_;
  std::string full_key_;
  std::string key_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef tcl::tree<LogContextData>::iterator LogContextIter;
struct FileLineInfo : public std::pair<const char*,int>
{FileLineInfo():pair(NULL,-1){}
 FileLineInfo(const char* file, int line):pair(file,line){}};

//void QtMsgHandler(QtMsgType type, const char* buf);

//-----------------------------------------------------------------------------

class LogManager : public boost::singleton<LogManager>
{
public:
  LogManager(boost::restricted);

  LogContextIter  GetContext(LogContextIter context, const char* sub_context, LogDefaults defaults);
  LogContextIter  GetContext(LogContextIter context){return context;}
  LogContextIter  GetContext(const char* context, LogDefaults defaults)
  {return GetContext(LogContextIter(),context,defaults);}

  void         LogOutput(const char* begin, size_t size,
                         LogContextIter iter, LogSeverity severity,
                         const char* file_name, int line_no);

  const std::locale& getloc()const{return locale_;}
  void imbue(const std::locale& loc){locale_ = loc;}
private:
  boost::mutex     stderr_lock_;
  boost::mutex     file_lock_;
  std::ofstream    file_log_;
  std::locale      locale_;
  typedef tcl::tree<LogContextData> LogTree;
  LogTree logs_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class LogContext
{
public:
  LogContext(const char* context, LogDefaults defaults = LogDefaults_Default)
    :iter_(LogManager::lease()->GetContext(context, defaults))
  {}
  LogContext(const LogContext& context, const char* sub_context, LogDefaults defaults = LogDefaults_Default)
    :iter_(LogManager::lease()->GetContext(context, sub_context, defaults))
  {}

  LogContext(const LogContext& r)
    :iter_(r.iter_)
  {}
  LogContext(const LogContextIter& iter)
    :iter_(iter)
  {}

  LogContext operator/(const char* sub_context)const
  {return LogManager::lease()->GetContext(iter_,sub_context,LogDefaults_Default);}

  bool               mute(LogSeverity severity)const{return iter_->mute(severity);}
  const std::string& full_key()const{return iter_->full_key();}

  operator LogContextIter()const{return iter_;}
private:  
  LogContextIter iter_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class LogBase;

class LoggerData
{
public:
  LoggerData()
    : stream_(buffer_)
  {
    stream_.imbue(manager_->getloc());
  }
  void StartNewEntry(const LogContext& context);
  void OutputData(LogBase* logger);
  LogManager::lease           manager_;
  std::vector<char>           buffer_;
  io::stream<io::back_insert_device<std::vector<char> > > stream_;
  boost::format               formatter_;
};

//-----------------------------------------------------------------------------

class LogBase : boost::noncopyable
{
public:
  LogBase(const LogContext& context,
          LogSeverity severity,
          const char* file_name,
          int line_no);

  bool mute()const{return data_ == NULL;}

protected:
  friend class LoggerData;
  const char*                 file_name_;
  int                         line_no_;
  LogContext                  context_;
  LogSeverity                 severity_;
  LoggerData*                 data_;
};

//-----------------------------------------------------------------------------

class Log : public LogBase
{
public:
  Log(const LogContext& context, LogSeverity severity = LogSeverity_Normal, const char* file_name = NULL, int line_no = -1);
  ~Log();

  template<class T>
  Log& operator<<(const T& a)
  {
    if (!mute())
    {
      data_->stream_ << a;
    }
    return *this;
  }

};

//-----------------------------------------------------------------------------

class FormatLog : public LogBase
{
public:
  FormatLog(const LogContext& context, const char* format_string, LogSeverity severity = LogSeverity_Normal, const char* file_name = NULL, int line_no = -1);
  ~FormatLog();

  bool mute()const{return data_ == NULL;}

  template<class T>
  FormatLog& operator%(const T& a)
  {
    if (!mute())
    {
      data_->formatter_ % a;
    }
    return *this;
  }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef WIN32
inline void Win32Fatal(LogContext context, const char* file_name, int line_no)
{
  LPSTR message;
  FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    ::GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&message, 0, NULL);
  Log(context,LogSeverity_Fatal,file_name,line_no) << message;
  LocalFree(message);
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif // QSMP_LOG_H_