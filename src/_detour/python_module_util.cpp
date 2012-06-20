#include "stdafx.h"
#include "python_funcs.h"
#include "python_module_pydetour.h"
#include "python_module_util.h"

#include <wchar.h>

#include "CPPPython.h"
#include "guiconsole.h"
using namespace CPPPython;


PyObject* util_loadPythonFile(PyObject* self, PyObject* args) {
	char* filename;
	if (!PyArg_ParseTuple(args, "s", &filename)) {
		return NULL;
	}
	int ret = run_python_file(filename);
	if (ret == 0) {
		return PyErr_Format(PyExc_IOError, "%s is an invalid filename", filename);
	}
	return Py_BuildValue("i", ret);
}
PyObject* util_WriteMemory(PyObject* self, PyObject* args) {
	char* address;
	char* bytes;
	int bytenum;

	if (!PyArg_ParseTuple(args, "is#", &address, &bytes, &bytenum)) {
		return NULL;
	}
	DWORD oldProt;
	DWORD dummy;
	__try {
		VirtualProtect(address, bytenum, PAGE_EXECUTE_READWRITE, &oldProt);
		memcpy(address, bytes, bytenum);
		VirtualProtect(address, bytenum, oldProt, &dummy);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}

	return Py_BuildValue("i", true);
}
PyObject* util_WriteByte(PyObject* self, PyObject* args) {
	char* address;
	char bytes;
	if (!PyArg_ParseTuple(args, "ic", &address, &bytes)) {
		return NULL;
	}
	DWORD oldProt;
	DWORD dummy;
	__try {
		VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProt);
		memcpy(address, &bytes, 1);
		VirtualProtect(address, 1, oldProt, &dummy);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}

	return Py_BuildValue("i", true);
}
PyObject* util_WriteDWORD(PyObject* self, PyObject* args) {
	char* address;
	DWORD bytes;
	if (!PyArg_ParseTuple(args, "ii", &address, &bytes)) {
		return NULL;
	}
	DWORD oldProt;
	DWORD dummy;
	__try {
		VirtualProtect(address, 4, PAGE_EXECUTE_READWRITE, &oldProt);
		memcpy(address, &bytes, 4);
		VirtualProtect(address, 4, oldProt, &dummy);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}
	return Py_BuildValue("i", true);
}
PyObject* util_ReadASCIIZ(PyObject* self, PyObject* args) {
	char* address;

	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}
	__try {
		if (address == NULL) {
			int x = *(int*)0x0; //Python won't cause the access violation, it'll return None instead
		}
		return Py_BuildValue("s", address);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}
}
PyObject* util_ReadUnicodeZ(PyObject* self, PyObject* args) {
	wchar_t* address;

	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}
	__try {
		if (address == NULL) {
			int x = *(int*)0x0; //Python won't cause the access violation, it'll return None instead
		}
		Py_ssize_t size = wcslen(address);
		return PyUnicode_FromUnicode(address, size);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}
}
PyObject* util_ReadMemory(PyObject* self, PyObject* args) {
	char* address;
	int bytes;

	if (!PyArg_ParseTuple(args, "ii", &address, &bytes)) {
		return NULL;
	}
	__try {
		if (address == NULL) {
			int x = *(int*)0x0; //Python won't cause the access violation, it'll return None instead
		}
		return Py_BuildValue("s#", address, bytes);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}

}
PyObject* util_ReadByte(PyObject* self, PyObject* args) {
	char* address;

	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}
	__try {
		if (address == NULL) {
			int x = *(int*)0x0; //Python won't cause the access violation, it'll return None instead
		}
		return Py_BuildValue("s#", address, 1);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}
}
PyObject* util_ReadDWORD(PyObject* self, PyObject* args) {
	int* address;

	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}
	__try {
		return Py_BuildValue("i", *address);
	} __except(1) {
		if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
			return PyErr_Format(Detour_Exception_AccessViolation, "%p is an invalid memory address (Access Violation)", address);
		}
		return PyErr_Format(Detour_Exception_WindowsException, "%p is an invalid memory address (Windows Exception %d)", address, GetExceptionCode());
	}
}

PyObject* util_fix_console_io(PyObject* self) {
	AllocConsole();
	RedirectIOToConsole();
	Py_RETURN_TRUE;
}

static PyMethodDef util_funcs[] = {
	{"readASCIIZ", (PyCFunction)util_ReadASCIIZ, METH_VARARGS, "Reads memory, None on NULL pointer"},
	{"readUnicodeZ", (PyCFunction)util_ReadUnicodeZ, METH_VARARGS, "Reads memory, None on NULL pointer"},
	
	{"read", (PyCFunction)util_ReadMemory, METH_VARARGS, "Reads memory"},
	{"readByte", (PyCFunction)util_ReadByte, METH_VARARGS, "Reads memory"},
	{"readDWORD", (PyCFunction)util_ReadDWORD, METH_VARARGS, "Reads memory"},
	{"write", (PyCFunction)util_WriteMemory, METH_VARARGS, "Writes memory"},
	{"writeByte", (PyCFunction)util_WriteByte, METH_VARARGS, "Writes memory"},
	{"writeDWORD", (PyCFunction)util_WriteDWORD, METH_VARARGS, "Writes memory"},

	{"loadPythonFile", (PyCFunction)util_loadPythonFile, METH_VARARGS, "Loads and executes a python file"},
	{"fixconsole", (PyCFunction)util_fix_console_io, METH_NOARGS, "Fixes up some stdio stuff"},
};


PyMODINIT_FUNC initutil() {
	//this function is automagically called on import

    /* Create the module and add the functions */
    PyObject* m = Py_InitModule3("util", util_funcs, "Utilities");

	PModule pm = PModule(m);

}
