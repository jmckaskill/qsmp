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

//#include <windows.h>
//#include <tchar.h>
#include <iostream>
//#include <boost/shared_ptr.hpp>

#ifdef _WIN32
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "phonon4.lib")
#endif

#endif
