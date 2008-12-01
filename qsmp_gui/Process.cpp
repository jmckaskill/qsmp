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
#include <qsmp_gui/common.h>
#include <qsmp_gui/Process.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Win32Process::Win32Process(const std::string& process, const fs::path& working_dir, const std::string& arguments)
: stdin_(INVALID_HANDLE_VALUE),
  stdout_(INVALID_HANDLE_VALUE),
  stderr_(INVALID_HANDLE_VALUE)
{
  Run(process,working_dir,arguments);
}

//-----------------------------------------------------------------------------

void Win32Process::Run(const std::string& process, const fs::path& working_dir, const std::string& arguments)
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

  std::string command_line = "\"" + process_ + "\" " + arguments_;

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

QSMP_END