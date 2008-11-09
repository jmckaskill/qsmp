#include "qsmp/Log.h"
#include <afx.h>


QSMP_BEGIN

namespace {

//TODO(james): Move this to someplace where it can be called from other files
Path PersistantPath(const std::string& file_name)
{
  Path path;
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

Path NewLogPath()
{
  using namespace boost::posix_time;
  typedef boost::date_time::time_facet<ptime,char> Facet;
  Facet* time_f = new Facet("%Y-%m-%d_%H-%M-%S");
  std::stringstream str_buf;
  str_buf.imbue(std::locale(str_buf.getloc(),time_f));
  ptime time(second_clock::local_time());

  str_buf << "Log_" << time << ".log";
  Path path = PersistantPath(str_buf.str());
  while(boost::filesystem::exists(path))
  {
    str_buf.clear();
    str_buf << "Log_" << time << "_" << rand() << ".log";
    path = PersistantPath(str_buf.str());
  }
  return path;
}
}


LogEntryBuffer::LogEntryBuffer(const char* context, const char* file_name, int line_no, bool start)
: file_name_(file_name),
  line_no_(line_no),
  have_started_(false)
{
  context_ = manager_->GetContext(context);
  string_buffer_.imbue(manager_->locale());
  if (start)
    StartLogEntry();
}

LogEntryBuffer::LogEntryBuffer(LogContext context, const char* file_name, int line_no, bool start)
: file_name_(file_name),
  line_no_(line_no),
  have_started_(false)
{
  context_ = manager_->GetContext(context);
  string_buffer_.imbue(manager_->locale());
  if (start)
    StartLogEntry();
}

LogEntryBuffer::~LogEntryBuffer()
{
  manager_->LogOutput(this);
}

LogEntryBuffer& LogEntryBuffer::StartLogEntry()
{
  if (have_started_)
    string_buffer_ << std::endl;

  have_started_ = true;

  using namespace boost::posix_time;
  string_buffer_ << second_clock::local_time()
                 << ": [" 
                 << context_->full_key_ 
                 << " - " 
                 << ::GetCurrentThreadId() //TODO(james): Replace with something more portable
                 << "]\t";

  return *this;
}

void QtMsgHandler(QtMsgType type, const char* buf)
{
  static LogContext debug    = LOG_CONTEXT("Qt/Debug");
  static LogContext warning  = LOG_CONTEXT("Qt/Warning");
  static LogContext critical = LOG_CONTEXT("Qt/Critical");
  static LogContext fatal    = LOG_CONTEXT("Qt/Fatal");
  switch(type)
  {
  case QtDebugMsg:
    LOG(debug) << buf;
    break;
  case QtWarningMsg:
    LOG(warning) << buf;
    break;
  case QtCriticalMsg:
    LOG(critical) << buf;
    break;
  case QtFatalMsg:
    LOG(fatal) << buf;
    break;
  }
}

LogManager::LogManager(boost::restricted)
: file_log_(NewLogPath().file_string().c_str()),
  locale_(std::locale(),new boost::date_time::time_facet<boost::posix_time::ptime,char>("%Y/%m/%d %H:%M:%S"))
{
}

LogContext LogManager::GetContext(LogContext context, const char *sub_context)
{
  typedef iterator_range<const char*> Range;
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
      LogContext new_context = std::find(node->begin(),node->end(),sub_string);
      if (new_context == node->end())
      {
        std::string key(boost::begin(sub_string),boost::end(sub_string));
        LogContextData data(node->get(),key);
        new_context = node->insert(data);
      }
      context = new_context;
    }
  }
  return context;
}

void LogManager::LogOutput(LogEntryBuffer* buffer)
{
  LogContext context = buffer->context_;
  //TODO(james): we could probably pull the data straight out of the stringstream buffer
  std::string str = buffer->string_buffer_.str();

  if (context->outputs_[LogOutput_Trace])
  {
    //Not sure whether this needs to be locked or not, but I don't believe it does
    ATL::CTraceFileAndLineInfo(buffer->file_name_,buffer->line_no_)("%s\n",str.c_str());
  }
  if (context->outputs_[LogOutput_Stderr])
  {
    lock_guard<mutex> lock(stderr_lock_);
    std::cerr.write(str.data(),str.size());
    std::cerr.put('\n');
  }
  if (context->outputs_[LogOutput_LogFile])
  {
    lock_guard<mutex> lock(file_lock_);
    file_log_.write(str.data(),str.size());
    file_log_.put('\n');
    file_log_.flush();
  }
}


QSMP_END