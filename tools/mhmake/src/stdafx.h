/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2010 marha@sourceforge.net
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CDC9F92E_2B83_4EFC_92B5_44861521ED45__INCLUDED_)
#define AFX_STDAFX_H__CDC9F92E_2B83_4EFC_92B5_44861521ED45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning (disable:4786)
#pragma warning (disable:4503)
#pragma warning (disable:4530)
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <sstream>

#ifdef _MSC_VER
#include <io.h>
#define __CDECL __cdecl
#else
#define __CDECL
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define MAX_PATH 255
#include <sys/wait.h>
#include <popt.h>
#include <glob.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <string.h>
#ifdef _MSC_VER
#include <direct.h>
#define mkdir(name,mode) _mkdir(name)
#endif
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

using namespace std;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CDC9F92E_2B83_4EFC_92B5_44861521ED45__INCLUDED_)

