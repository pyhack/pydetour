#include "stdafx.h"
#include "python_module_pydetour.h"
#include "python_module_util.h"
#include "python_funcs.h"
#include "python_type_registers.h"
#include "python_type_detourconfig.h"
#include "python_type_memory.h"
#include "structmember.h"

using namespace CPPPython;

PyObject* Detour_Exception;
PyObject* Detour_Exception_AlreadyInitilized;
PyObject* Detour_Exception_WindowsException;
PyObject* Detour_Exception_AccessViolation;

PyObject* detour_break(PyObject* self, PyObject* args) {
	if (!PyArg_ParseTuple(args, "")) {
		return NULL;
	}
	__asm {
		NOP
		NOP
		NOP
		NOP
		INT 3
		NOP
		NOP
		NOP
		NOP

	}
	Py_RETURN_TRUE;
}



PyObject* detour_callback(PyObject* self, PyObject* args) {

	char* address;
	int detoured_addr;
	int registers[8];
	int flags;
	int caller;
	if (!PyArg_ParseTuple(args, "i(iiiiiiii)ii", &address, &detoured_addr, &registers[0], &registers[1], &registers[2], &registers[3], &registers[4], &registers[5], &registers[6], &registers[7], &flags, &caller)) {
		return NULL;
	}
	OutputDebugString("Default Python -> C++ callback called\n");
	
	return Py_BuildValue("s", 0);
}
PyObject* detour_setRegisters(PyObject* self, PyObject* args) {

	BYTE* address;
	REGISTERS registers;
	int flags;
	int caller;
	if (!PyArg_ParseTuple(args, "i(iiiiiiii)ii", 
			&address, 
			&registers.eax, 
			&registers.ecx, 
			&registers.edx, 
			&registers.ebx,
			&registers.esp, 
			&registers.ebp, 
			&registers.esi, 
			&registers.edi, 
			&flags, 
			&caller)) {
		return NULL;
	}
	GDetour* dt = gdetour_get(address);
	if (dt == NULL) {
		return PyErr_Format(PyExc_LookupError, "%p is an invalid detoured address", address);
	}

	GDetour &d = *dt;

	d.live_settings.registers.eax = registers.eax;
	d.live_settings.registers.ecx = registers.ecx;
	d.live_settings.registers.edx = registers.edx;
	d.live_settings.registers.ebx = registers.ebx;
	d.live_settings.registers.esp = registers.esp;
	d.live_settings.registers.ebp = registers.ebp;
	d.live_settings.registers.esi = registers.esi;
	d.live_settings.registers.edi = registers.edi;
	d.live_settings.flags = flags;
	//d.live_settings.caller_ret = caller;


	Py_RETURN_TRUE;
}
PyObject* detour_createDetour(PyObject* self, PyObject* args) {
	BYTE* address;
	int overwrite_length;
	int bytes_to_pop;
	int type = 0;

	if (!PyArg_ParseTuple(args, "iiii", &address, &overwrite_length, &bytes_to_pop, &type)) {
		return NULL;
	}

	GDetour* ret = gdetour_create(address, overwrite_length, bytes_to_pop, CallPythonDetour, type);
	
	if (ret) {
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;

}
PyObject* detour_removeDetour(PyObject* self, PyObject* args) {
	BYTE* address;
	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}
	GDetour* d = gdetour_get(address);
	if (!d) {
		return PyErr_Format(PyExc_LookupError, "Detour not found at %p", address);
	}
	gdetour_destroy(d);

	Py_RETURN_TRUE;
}
PyObject* detour_getDetourSettings(PyObject* self, PyObject* args) {
	
	
	BYTE* address;
	if (!PyArg_ParseTuple(args, "i", &address)) {
		return NULL;
	}

	GDetour* d = gdetour_get(address);
	if (d == NULL) {
		return PyErr_Format(PyExc_LookupError, "%p is an invalid detoured address", address);
	}
	return Py_BuildValue("(iiii)",
		d->gateway_opt.bytes_to_pop_on_ret,
		d->gateway_opt.call_original_on_return,
		d->gateway_opt.original_code,
		d->gateway_opt.int3_after_call
	);
}
PyObject* detour_setDetourSettings(PyObject* self, PyObject* args) {
	DETOUR_GATEWAY_OPTIONS n;
	BYTE* address;
	if (!PyArg_ParseTuple(args, "i(iii)", &address, 
		&n.bytes_to_pop_on_ret, //TODO: FIXME: This is broken because we don't restore the RETN xx number
			&n.call_original_on_return,
			&n.int3_after_call
			)
			) {
		return NULL;
	}
	GDetour* d = gdetour_get(address);
	if (d == NULL) {
		return PyErr_Format(PyExc_LookupError, "%p is an invalid detoured address", address);
	}
	if (n.int3_after_call > 0) {
		n.int3_after_call = 1;
	}
	d->gateway_opt.bytes_to_pop_on_ret = n.bytes_to_pop_on_ret;
	d->gateway_opt.call_original_on_return = n.call_original_on_return;
	d->gateway_opt.int3_after_call = n.int3_after_call;

	return Py_BuildValue("i", true);
}


////////////////////////////////////////////////////////////////////
static PyMethodDef detour_funcs[] = {

	{"break_into_debugger", (PyCFunction)detour_break, METH_VARARGS, "Execute an INT3 instruction"},

	{"callback", (PyCFunction)detour_callback, METH_VARARGS, "Default callback function"},
	{"setRegisters", (PyCFunction)detour_setRegisters, METH_VARARGS, "Sets registers"},
	
	{"createDetour", (PyCFunction)detour_createDetour, METH_VARARGS, "Creates a detour"},
	{"removeDetour", (PyCFunction)detour_removeDetour, METH_VARARGS, "Removes a detour"},

	{"getDetourSettings", (PyCFunction)detour_getDetourSettings, METH_VARARGS, "Retreives a detour's settings"},
	{"setDetourSettings", (PyCFunction)detour_setDetourSettings, METH_VARARGS, "Modifies a detour's settings"},
	{NULL, NULL, 0, NULL}
};



PyMODINIT_FUNC init_detour() {
	//this function is automagically called on import

    /* Create the module and add the functions */
    PyObject* m = Py_InitModule3("_detour", detour_funcs, "Generic Process Detour");

	Detour_Exception = PyErr_NewException("gdetour.DetourException", NULL, NULL);
	Detour_Exception_AlreadyInitilized = PyErr_NewException("gdetour.DetourExceptionAlreadyInitilized", Detour_Exception, NULL);
	Detour_Exception_WindowsException = PyErr_NewException("gdetour.DetourWindowsException", Detour_Exception, NULL);
	Detour_Exception_AccessViolation = PyErr_NewException("gdetour.DetourAccessViolationException", Detour_Exception_WindowsException, NULL);

	PModule pm = PModule(m);
	pm.AddObject("DetourException", Detour_Exception);
	pm.AddObject("DetourAlreadyInitilizedException", Detour_Exception_AlreadyInitilized);
	pm.AddObject("DetourWindowsException", Detour_Exception_WindowsException);
	pm.AddObject("DetourAccessViolationException", Detour_Exception_AccessViolation);

	add_module_type_registers(m);
	add_module_type_detourconfig(m);
	add_module_type_memory(m);

	initutil();

	PyObject* util = PyImport_ImportModule("util");

	//PyModule_AddObject(m, "util", util); //steals ref
	pm.AddObject("util", util);
}














void CallPythonDetour(GDetour &d, DETOUR_LIVE_SETTINGS &stack_live_settings) {
		/* ensure we hold the lock */
	P_GIL gil;


	PyObject* m = PyImport_AddModule("_detour");

	if (m == NULL) {
		OutputDebugString("Can't call detour! module not in locals!\n");
		return;
	}

	PyObject* detour_pyfunc = PyObject_GetAttrString(m, "callback");

	if (detour_pyfunc == NULL) {
		PyErr_Clear();
		OutputDebugString("Can't call detour! detour_pyfunc is null!\n");
		return;
	}

	PyObject* ret = PyEval_CallFunction(detour_pyfunc, 
		"i(iiiiiiii)ii", 
		d.live_settings.ret_addr-5,
		d.live_settings.registers.eax,
		d.live_settings.registers.ecx,
		d.live_settings.registers.edx,
		d.live_settings.registers.ebx,
		d.live_settings.registers.esp,
		d.live_settings.registers.ebp,
		d.live_settings.registers.esi,
		d.live_settings.registers.edi,
		d.live_settings.flags,
		d.live_settings.caller_ret - 5
	);
	if (PyErr_Occurred()) { PyErr_Print(); }


	Py_XDECREF(detour_pyfunc);

}