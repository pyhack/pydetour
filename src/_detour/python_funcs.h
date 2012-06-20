#pragma once

#ifdef PYTHON_DETOUR_EXPORTS
#define PYTHON_DETOUR_API extern "C" __declspec(dllexport)
#else
#define PYTHON_DETOUR_API extern "C" __declspec(dllimport)
#endif

#include "CPPPython.h"

//extern PyThreadState* mainPythonThreadState;

//extern PyObject* myPyGlobals;
//extern PyObject* myPyLocals;
extern CPPPython::PDict pyLocals;
extern CPPPython::PDict pyGlobals;

PYTHON_DETOUR_API void run_test();

PYTHON_DETOUR_API PyObject* run_python_string(char* pycode);
PYTHON_DETOUR_API int run_python_file(char* filename, bool debugging=true);

void Python_Initialize();
void Python_Unload();

extern char python_dll_path[MAX_PATH];



