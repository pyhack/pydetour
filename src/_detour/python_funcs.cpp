#include "stdafx.h"
#include "python_funcs.h"
#include "python_module_pydetour.h"
#include "python_type_registers.h"

using namespace CPPPython;

PDict pyLocals = NULL;
PDict pyGlobals = NULL;
//PyObject* myPyGlobals;
//PyObject* myPyLocals;

PYTHON_DETOUR_API PyObject* run_python_string(char* pycode) {
	P_GIL gil;

	PyObject* ret = PyRun_String(pycode, Py_single_input, pyGlobals, pyLocals);	
	if (PyErr_Occurred()) {
		OutputDebugString("error occured running pystring");
		PyErr_Print();
	}
	return ret;
}

/*
Error codes are 32-bit values (bit 31 is the most significant bit).
Bit 29 is reserved for application-defined error codes; no system error code has this bit set.
If you are defining an error code for your application, set this bit to one.

Source: http://msdn.microsoft.com/en-us/library/ms679360%28VS.85%29.aspx

*/
#define BIT_29 (1 << 29)

PYTHON_DETOUR_API int run_python_file_error_success = 0;
PYTHON_DETOUR_API int run_python_file_error_open = BIT_29 + 3;
PYTHON_DETOUR_API int run_python_file_error_exception = BIT_29 + 4;

PYTHON_DETOUR_API int run_python_file(char* filename, bool debugging) {
	//Return values:
	//0 - Success
	//Others, see above

	char fnbuf[1024];
	memset(&fnbuf, 0, sizeof(fnbuf));
	if (filename[1] != ':') {
		//assume that this is a relative path from this dll
		HMODULE h = GetModuleHandle("_detour.pyd");
		GetModuleFileName(h, fnbuf, 1022 - strlen(filename));
		int l = strlen(fnbuf);
		for(int i = l; i > 0; i--) {
			if (fnbuf[i] == '\\') {
				//strncpy(&fnbuf[i+1], filename, 1023 - i);
				strncpy_s(&fnbuf[i+1], 1023 - i, filename, 1023 - i);
				break;
			}
		}
	} else {
		//strncpy(fnbuf, filename, 1023);
		strncpy_s(fnbuf, 1023, filename, 1023);
	}

	P_GIL gil;

	PyObject* pyfile = PyFile_FromString(fnbuf,"r"); //new ref




	if (PyErr_Occurred()) { 
		PyErr_Print(); 
		OutputDebugString("pyerror occured reading file");
	}
	if(pyfile==NULL){
		OutputDebugString("pyfile is null");
		Py_XDECREF(pyfile);
		return run_python_file_error_open;
	}

	FILE* f = PyFile_AsFile(pyfile); //no ref

	if (PyErr_Occurred()) {
		//TODO: bail
		PyErr_Clear();
	}

	PyRun_File(f,fnbuf, Py_file_input, pyGlobals, pyLocals);

	if (PyErr_Occurred()) {
		if (debugging) {
			PyObject *a, *b, *c;
			PyErr_Fetch(&a, &b, &c); //we get refs
			Py_INCREF(a); Py_INCREF(b); //we now have 2 refs of a and b
			if (c) {
				Py_INCREF(c); //I now have 2 refs of c too. We have to special case this because (a) could be a SyntaxError, making this (c) NULL
			}
			PyErr_Restore(a, b, c); //steals, we now have 1 ref
			PyErr_Print();
			//PyErr_Restore(a, b, c);
			//PModule traceback = PModule::importModule("traceback", pyGlobals, pyLocals);
			//PObject traceback_print = traceback.getAttr("print_tb");
			//traceback_print.call(c, NULL);
			if (c) {
				printf("\n\nBreaking into debugger...\n");
				PModule dbg = PModule::importModule("pdb", pyGlobals, pyLocals);
				PObject dbg_run = dbg.getAttr("post_mortem");
				dbg_run.call(c, NULL);
			} else {
				printf("\n\nCan't debug exceptions with no traceback.");
			}
			Py_XDECREF(a); Py_XDECREF(b); Py_XDECREF(c); //and now we have 0 ref again
		} else {
			PyErr_Print();
		}
		//probably an exception
		OutputDebugString("pyerror occured running file");

		Py_XDECREF(pyfile);
		return run_python_file_error_exception;
	}


	Py_DECREF(pyfile);
	return run_python_file_error_success;
}

////////////////////////////////////////////////////////









//------------------------------------------------------------------------------

//PyThreadState* mainPythonThreadState = NULL;
#ifdef _DEBUG
static char* python_dll_name = "python26_d.dll";
#else
static char* python_dll_name = "python26.dll";
#endif

static char python_dll_path[MAX_PATH];
static HMODULE python_dll_handle = NULL;


void cstring_strip_trailing_dirs(char* str, int count=1) {
	int cut = 0;
	for(int i = strlen(str); i > 0; i--) {
		char* cur = (char*)(str + i);
		if (*cur == '\\') {
			cut++;
			if (cut == count) {
				return;
			}
		}
		*cur = '\0';
	}
}

void Python_Initialize() {

	python_dll_handle = GetModuleHandle(python_dll_name);
	ZeroMemory(python_dll_path, sizeof(python_dll_path));
	GetModuleFileName(python_dll_handle, python_dll_path, MAX_PATH-50);

#ifdef _DEBUG
	//Here we set up PythonHome to our debugging version a few directories up.
	//We expect the debug version to be straight form a build.

	char python_home[MAX_PATH]; //points to 
	char python_sys_path[MAX_PATH];
	ZeroMemory(python_home, sizeof(python_home));
	ZeroMemory(python_sys_path, sizeof(python_sys_path));
	strcpy_s(python_sys_path, "PYTHONPATH=");

	strcat_s(python_home, python_dll_path);			//home = dll path
	strcat_s(python_sys_path, python_dll_path);		//sys = dll path

	cstring_strip_trailing_dirs(python_home, 2); //home = up two dirs from dll path
	cstring_strip_trailing_dirs(python_sys_path, 1); //sys = up one dir from dll path
	python_home[strlen(python_home)-1] = 0; //kill trailing slash
	python_sys_path[strlen(python_sys_path)-1] = 0; //kill trailing slash

	strcat_s(python_sys_path, ";");
	strcat_s(python_sys_path, python_home);

	Py_SetPythonHome(python_home);
	_putenv(python_sys_path);

#else
	//Here we used the installed version of Python at C:\Python26 (we assume)
#endif
	Py_Initialize();
	if (!Py_IsInitialized()) {
		OutputDebugString("Python could not be initialized\n");
	}
	PyEval_InitThreads();
	


	PModule mainmod = PModule::getModule("__main__");
	PDict d = PyModule_GetDict(mainmod); //borrowed ref

	pyGlobals = d;
	pyLocals = d;

	//PyRun_SimpleString(tempbuf);

	//These three do import gdetour
	//initgdetour();
	//PModule gd = PModule::importModule("gdetour", pyGlobals, pyLocals);
	//mainmod.AddObject("gdetour", gd);

	PyThreadState* _save = PyEval_SaveThread();	//required to release the GIL we got for free by initializing Python before this thread vanishes
}
void Python_Unload() {
	P_GIL gil;



	pyGlobals = NULL;
	pyLocals = NULL;

	printf("python about to unload\n");

	Py_Finalize();

	printf("python unloaded\n");
}
