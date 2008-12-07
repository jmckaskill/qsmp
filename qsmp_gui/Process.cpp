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

#include "stdafx.h"
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <qsmp_gui/common.h>
#include <qsmp_gui/Process.h>

#define foreach BOOST_FOREACH

QSMP_BEGIN

#ifdef WIN32

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Win32Process::Win32Process(LogContext log, const std::string& process, const fs::path& working_dir, const std::vector<std::string>& arguments);
: stdin_(INVALID_HANDLE_VALUE),
  stdout_(INVALID_HANDLE_VALUE),
  stderr_(INVALID_HANDLE_VALUE)
{
  Run(process,working_dir,arguments);
}

//-----------------------------------------------------------------------------

void Win32Process::Run(const std::string& process, const fs::path& working_dir, const std::vector<std::string>& arguments)
{
  process_     = process;
  working_dir_ = working_dir;
  arguments_   = arguments;
  Run();
}

//-----------------------------------------------------------------------------

void Win32Process::Run()
{
  DWORD error;
  proc_info_.hThread = INVALID_HANDLE_VALUE;
  proc_info_.hProcess = INVALID_HANDLE_VALUE;

  SECURITY_ATTRIBUTES security_attributes;
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  security_attributes.bInheritHandle = TRUE;
  security_attributes.lpSecurityDescriptor = NULL;

  ScopedHandle other_stdin, other_stdout, other_stderr;
  if (!::CreatePipe(&other_stdin.handle(),&stdin_.handle(),&security_attributes,0))
    error = ::GetLastError();
  if (!::CreatePipe(&stdout_.handle(),&other_stdout.handle(),&security_attributes,0))
    error = ::GetLastError();
  if (!::CreatePipe(&stderr_.handle(),&other_stderr.handle(),&security_attributes,0))
    error = ::GetLastError();
  if (!::SetHandleInformation(stdin_,HANDLE_FLAG_INHERIT,0))
    error = ::GetLastError();
  if (!::SetHandleInformation(stdout_,HANDLE_FLAG_INHERIT,0))
    error = ::GetLastError();
  if (!::SetHandleInformation(stderr_,HANDLE_FLAG_INHERIT,0))
    error = ::GetLastError();

  STARTUPINFOA startup_info;
  ZeroMemory(&startup_info,sizeof(STARTUPINFOA));
  startup_info.cb         = sizeof(STARTUPINFOA);
  startup_info.dwFlags    = STARTF_USESTDHANDLES;
  startup_info.hStdInput  = other_stdin;
  startup_info.hStdOutput = other_stdout;
  startup_info.hStdError  = other_stderr;

  std::string command_line = "\"" + process_ + "\"";
  foreach(std::string& str, arguments_)
    command_line += " \"" + str + "\"";

  BOOL success = ::CreateProcessA(
    process_.c_str(), //application name
    const_cast<char*>(command_line.c_str()), //command line
    NULL,                  //security attributes
    NULL,                  //primary thread security attributes
    TRUE,                  //handles are inherited (needed to be able to use pipe)
    0,                     //creation flags
    NULL,                  //use parent's environ
    working_dir_.string().c_str(),                  //working directory
    &startup_info,         //further setup through STARTUPINFO
    &proc_info_); //results are received through PROCESS_INFORMATION

  process_handle_ = proc_info_.hProcess;
  thread_handle_  = proc_info_.hThread;

  if (!success)
  {
    error = ::GetLastError();
    Terminate(-1);
  }
}

//-----------------------------------------------------------------------------

Win32Process::~Win32Process()
{
  Terminate(-1);
}

//-----------------------------------------------------------------------------

void Win32Process::Terminate(int exit_code)
{
  if (is_running())
    TerminateProcess(process_handle(), exit_code);
  process_handle_.reset();
  thread_handle_.reset();
  stdin_.reset();
  stderr_.reset();
  stdout_.reset();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif


#ifdef UNIX 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PosixProcess::PosixProcess(LogContext log)
: log_(log),
  log_errno_(log/"errno"),
  stdin_fd_(-1),
  stdout_fd_(-1),
  stderr_fd_(-1),
  pid_(-1)
{
  LOG(log_) << "Init";
}

//-----------------------------------------------------------------------------

PosixProcess::PosixProcess(LogContext log, const std::string& process, const fs::path& working_dir, const std::vector<std::string>& arguments)
: log_(log),
  log_errno_(log/"errno"),
  process_(process),
  working_dir_(working_dir),
  arguments_(arguments),
  stdin_fd_(-1),
  stdout_fd_(-1),
  stderr_fd_(-1),
  pid_(-1)
{
  LOG(log_) << "Init";
  Run();
}

//-----------------------------------------------------------------------------

PosixProcess::~PosixProcess()
{
  LOG(log_) << "Shutdown";
  if (stdin_fd_ != -1)
    close(stdin_fd_);
  if (stdout_fd_ != -1)
    close(stdout_fd_);
  if (stderr_fd_ != -1)
    close(stderr_fd_);
}

//-----------------------------------------------------------------------------

void PosixProcess::Run(const std::string& process, const fs::path& working_dir, const std::vector<std::string>& arguments)
{
  process_      = process;
  working_dir_  = working_dir;
  arguments_    = arguments;
  Run();
}

//-----------------------------------------------------------------------------

enum
{
  FD_STDIN    = 0,
  FD_STDOUT   = 1,
  FD_STDERR   = 2,

  FD_READ     = 0,
  FD_WRITE    = 1,
};

void PosixProcess::Run()
{
  LOG(log_) << "Run";
  using boost::array;
  array<array<int,2>, 3> fds;
  pipe(&fds[0][0]);
  pipe(&fds[1][0]);
  pipe(&fds[2][0]);
  std::string working_dir = working_dir_.string();
  std::string process     = process_;
  std::vector<char*> arguments;
  arguments.reserve(arguments_.size() + 1);

  FLOG(log_, "Process: %1%, Working Dir: %2%") % process % working_dir;
  {
    qsmp::Log logger(log_);
    logger << "Arguments: ";
    foreach(std::string& str, arguments_)
    {
      arguments.push_back(const_cast<char*>(str.c_str()));
      logger << " \"" << str << "\"";
    }
  }
  arguments.push_back(NULL);
  
  int pid = fork();
  switch(pid)
  {
  case 0:
    LOG(log_) << "Child context";
    //child context
    //setup our pipe fds as the local stdin, stdout, stderr
    if (dup2(fds[FD_STDIN][FD_READ], STDIN_FILENO) == -1)
      ERRNO_LOG(log_errno_);
    if (dup2(fds[FD_STDOUT][FD_WRITE], STDOUT_FILENO) == -1)
      ERRNO_LOG(log_errno_);
    if (dup2(fds[FD_STDERR][FD_WRITE], STDERR_FILENO) == -1)
      ERRNO_LOG(log_errno_);
    //close all of the fds as we no longer need them
    for (int i = 0; i < fds.size(); i++)
      for (int j = 0; j < fds[i].size(); j++)
        if(close(fds[i][j]))
          ERRNO_LOG(log_errno_);
    //change working dir
    if (chdir(working_dir.c_str()))
      ERRNO_LOG(log_errno_);
    //and finally execute the child process
    if (execvp(process_.c_str(), arguments.data()))
      ERRNO_LOG(log_errno_);
    break;
  case -1:
    WARNING(log_) << "Fork failed";
    ERRNO_LOG(log_errno_);
    pid_ = pid;
    //close all of the fds as we no longer need them
    for (int i = 0; i < fds.size(); i++)
      for (int j = 0; j < fds[i].size(); j++)
        if(close(fds[i][j]))
          ERRNO_LOG(log_errno_);
    break;
  default:
    //parent context - success
    pid_ = pid;
    //close read handle of stdin and write handles of stdout/stderr
    if (close(fds[FD_STDIN][FD_READ]))
      ERRNO_LOG(log_errno_);
    if (close(fds[FD_STDOUT][FD_WRITE]))
      ERRNO_LOG(log_errno_);
    if (close(fds[FD_STDERR][FD_WRITE]))
      ERRNO_LOG(log_errno_);
    stdin_fd_   = fds[FD_STDIN][FD_WRITE];
    stdout_fd_  = fds[FD_STDOUT][FD_READ];
    stderr_fd_  = fds[FD_STDERR][FD_READ];
    FLOG(log_, "Parent context: Child PID %1%, stdin %2%, stdout %3%, stderr %4%")
      % pid_
      % stdin_fd_
      % stdout_fd_
      % stderr_fd_;
    
    break;
  }
  
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif

QSMP_END
