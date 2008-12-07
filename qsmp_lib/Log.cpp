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

#include <qsmp_lib/Log.h>

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/range.hpp>
#include <boost/thread.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <locale>

#ifdef WIN32
#include <crtdbg.h>
#endif

namespace {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//TODO(james): Move this to someplace where it can be called from other files
boost::filesystem::path PersistantPath(const std::string& file_name)
{
  boost::filesystem::path path;
#ifdef _WIN32
  path = getenv("APPDATA");
  path /= "QSmp";
#else
  path = getenv("HOME");
  path /= ".qsmp";
#endif
  boost::filesystem::create_directories(path);
  path /= file_name;
  return path;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

boost::filesystem::path NewLogPath()
{
  using namespace boost::posix_time;
  typedef boost::date_time::time_facet<ptime,char> Facet;
  Facet* time_f = new Facet("%Y-%m-%d_%H-%M-%S");
  std::stringstream str_buf;
  str_buf.imbue(std::locale(str_buf.getloc(),time_f));
  ptime time(second_clock::local_time());

  str_buf << "Log_" << time << ".log";
  boost::filesystem::path path = PersistantPath(str_buf.str());
  while(boost::filesystem::exists(path))
  {
    str_buf.clear();
    str_buf << "Log_" << time << "_" << rand() << ".log";
    path = PersistantPath(str_buf.str());
  }
  return path;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

}



QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LoggerData* GetLoggerData()
{
  static boost::thread_specific_ptr<LoggerData> data;
  if (!data.get())
    data.reset(new LoggerData);
  return data.get();
}

//-----------------------------------------------------------------------------

void LoggerData::StartNewEntry(const LogContext& context)
{
  using namespace boost::posix_time;
  stream_ << microsec_clock::local_time()
    << ": [" 
    << context.full_key()
    << " - " 
    << boost::this_thread::get_id()
    << "] ";
}

//-----------------------------------------------------------------------------

void LoggerData::OutputData(LogBase* logger)
{
  if (!buffer_.empty())
  {
    manager_->LogOutput(&buffer_[0],
                        buffer_.size(),
                        logger->context_,
                        logger->severity_,
                        logger->file_name_,
                        logger->line_no_);
    buffer_.clear();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LogBase::LogBase(const LogContext& context, LogSeverity severity, const char* file_name, int line_no, bool raw)
: context_(context),
  severity_(severity),
  file_name_(file_name),
  line_no_(line_no),
  data_(NULL),
  raw_(raw)
{
  if (!context.mute(severity))
  {
    data_ = GetLoggerData();
    if (!raw_)
      data_->StartNewEntry(context_);
  }
}

//-----------------------------------------------------------------------------

LogBase::~LogBase()
{
  if (!mute())
  {
    if (!raw_)
      data_->stream_ << std::endl;
    data_->stream_ << std::flush;
    data_->OutputData(this);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Log::Log(const LogContext& context, LogSeverity severity, const char* file_name, int line_no, bool raw)
: LogBase(context,severity,file_name,line_no,raw)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FormatLog::FormatLog(const LogContext& context, const char* format_string, LogSeverity severity, const char* file_name, int line_no, bool raw)
: LogBase(context,severity,file_name,line_no,raw)
{
  if (!mute())
  {
    data_->formatter_.parse(format_string);
  }
}

//-----------------------------------------------------------------------------

FormatLog::~FormatLog()
{
  if (!mute())
    data_->stream_ << data_->formatter_;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LogManager::LogManager(boost::restricted)
: file_log_(NewLogPath().file_string().c_str()),
  //locale_(std::locale(),new boost::date_time::time_facet<boost::posix_time::ptime,char>("%Y/%m/%d %H:%M:%S"))
  locale_(std::locale(),new boost::date_time::time_facet<boost::posix_time::ptime,char>("%H:%M:%s"))
{
}

//-----------------------------------------------------------------------------

LogContextIter LogManager::GetContext(LogContextIter context, const char *sub_context, LogDefaults defaults)
{
  typedef boost::iterator_range<const char*> Range;
  static const Range seperator = boost::as_literal("/");

  Range tail = boost::as_literal(sub_context);
  Range sub_string, next_sep;
  while(boost::size(tail) > 0)
  {
    next_sep   = boost::find_first(tail, seperator);
    sub_string = Range(boost::begin(tail),boost::begin(next_sep));
    tail       = Range(boost::end(next_sep),boost::end(tail));
    if (!boost::empty(sub_string))
    {
      LogTree* node = (context.valid()) ? context.node() : &logs_;
      LogContextIter new_context = std::find(node->begin(),node->end(),sub_string);
      if (new_context == node->end())
      {
        std::string key(boost::begin(sub_string),boost::end(sub_string));
        LogContextData data(node->get(),key,defaults);
        new_context = node->insert(data);
      }
      context = new_context;
    }
  }
  return context;
}

//-----------------------------------------------------------------------------

void LogManager::LogOutput(const char* begin, size_t size,
                           LogContextIter iter, LogSeverity severity,
                           const char* file_name, int line_no)
{
  using boost::lock_guard;
  using boost::mutex;

#ifdef WIN32
  if (iter->log(LogOutput_Trace, severity))
  {
    int report_type;
    switch(severity)
    {
    case LogSeverity_Fatal:
      report_type = _CRT_ERROR;
      break;
    case LogSeverity_Normal:
    case LogSeverity_Warning:
    default:
      report_type = _CRT_WARN;
      break;
    }
    std::string str(begin,size);
    if(_CrtDbgReport(report_type,file_name,line_no,"","%s",str.c_str()))
      _CrtDbgBreak();
  }
#endif
  if (iter->log(LogOutput_Stderr, severity))
  {
    lock_guard<mutex> lock(stderr_lock_);
    //if (file_name)
      //std::cerr << file_name << "(" << line_no << "): ";
    std::cerr.write(begin, size);
  }
  if (iter->log(LogOutput_LogFile, severity))
  {
    lock_guard<mutex> lock(file_lock_);
    file_log_.write(begin, size);
    file_log_.flush();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END
