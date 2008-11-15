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

#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32

//#ifndef _UNICODE
//#define _UNICODE
//#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif
#ifndef WINVER
#define WINVER 0x501
#endif

#define _AFXDLL

#endif

#define QSMP_BEGIN namespace qsmp {
#define QSMP_END }
#define QSMP_NON_COPYABLE(class_name) \
  private:  \
    class_name(const class_name&);  \
    class_name& operator=(const class_name&);

#endif
