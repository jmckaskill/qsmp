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

#ifndef QSMP_GUI_PROCESS_H_
#define QSMP_GUI_PROCESS_H_

#include <qsmp_gui/common.h>


QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef WIN32

class ScopedHandle
{
  QSMP_NON_COPYABLE(ScopedHandle)
public:
  ScopedHandle()
    : handle_(INVALID_HANDLE_VALUE){}
  ScopedHandle(HANDLE h)
    : handle_(h){}
  void operator=(HANDLE h)
  {
    reset();
    handle_ = h;
  }

  ~ScopedHandle()
  {
    reset();
  }

  void reset()
  {
    if (valid())
      ::CloseHandle(handle_);
  }

  bool valid()const{return handle_ != INVALID_HANDLE_VALUE;}

  HANDLE& handle(){return handle_;}

  operator HANDLE()const{return handle_;}
private:
  HANDLE handle_;
};

class Win32Process
{
  QSMP_NON_COPYABLE(Win32Process);
public:
  explicit Win32Process(const std::string& process, const std::string& arguments);
  explicit Win32Process(const std::string& process, const fs::path& working_dir, const std::string& arguments);

  //template<class Range1T>
  //explicit Win32Process(const Range1T& arguments);

  ~Win32Process();

  void Run();
  void Run(const std::string& process, const fs::path& working_dir, const std::string& arguments);

  bool is_running()const{return process_handle_.valid();}

  HANDLE stdin_fd()const{return stdin_;}
  HANDLE stdout_fd()const{return stdout_;}
  HANDLE stderr_fd()const{return stderr_;}

  HANDLE process_handle()const{return process_handle_;}
  HANDLE thread_handle()const{return thread_handle_;}

  void PostMessageToThread(UINT Msg, WPARAM wparam, LPARAM lparam);

  void WaitForStartup(DWORD timeout = INFINITE);
  int  WaitForFinish(DWORD timeout = INFINITE);

  void Terminate(int exit_code);

private:
  std::string         process_;
  fs::path            working_dir_;
  std::string         arguments_;
  PROCESS_INFORMATION proc_info_;
  ScopedHandle        stdin_;
  ScopedHandle        stdout_;
  ScopedHandle        stderr_;
  ScopedHandle        process_handle_;
  ScopedHandle        thread_handle_;
};

typedef Win32Process Process;
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef POSIX
class PosixProcess
{
public:
  explicit PosixProcess(const std::string& command_line);

  template<class Range1T>
  explicit PosixProcess(const Range1T& arguments);

  ~PosixProcess();

  bool is_running()const;

  int stdin_fd()const;
  int stdout_fd()const;
  int stderr_fd()const;

  pid_t pid()const;

  void Signal(int signal);

  void WaitForStartup(int timeout);
  int WaitForFinish(int timeout);
};

typedef PosixProcess Process;
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif