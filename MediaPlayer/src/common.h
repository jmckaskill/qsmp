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

//Boost Includes
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

//TBB Includes


//Lua Includes
#include <lua.h>

#ifdef _WIN32
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "phonon4.lib")
#endif

#endif
