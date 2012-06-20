#include <map>

#include <gdetour.h>

void CallPythonDetour(GDetour &d, DETOUR_LIVE_SETTINGS &stack_live_settings);

PyMODINIT_FUNC initpydetour();

extern PyObject* Detour_Exception;
extern PyObject* Detour_Exception_AlreadyInitilized;
extern PyObject* Detour_Exception_WindowsException;
extern PyObject* Detour_Exception_AccessViolation;