// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#undef UNICODE

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here

#ifdef _DEBUG
//#undef _DEBUG //We can't compile a debug version of python
//#pragma message("Warning: Mixing CRT libraries between debug and nondebug isn't good! (Don't have python debug library)")
#include "Python.h"
//#define _DEBUG
#else
#include "Python.h"
#endif


#include <map>




