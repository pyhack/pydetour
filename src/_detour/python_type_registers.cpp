#include "stdafx.h"
#include "python_type_registers.h"

#include "structmember.h"

typedef struct registers {
    PyObject_HEAD
    DWORD edi;
	DWORD esi;
	DWORD ebp;
	DWORD esp;
	DWORD ebx;
	DWORD edx;
	DWORD ecx;
	DWORD eax;
	DWORD flags;
} registers;

static void registers_dealloc(registers* self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject * registers_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    registers *self;
    self = (registers *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->eax = 0;
		self->ecx = 0;
		self->edx = 0;
		self->ebx = 0;
		self->esp = 0;
		self->ebp = 0;
		self->esi = 0;
		self->edi = 0;
		self->flags = 0;
	}
    return (PyObject *)self;
}

static int registers_init(registers *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iiiiiiiii", kwlist, 
									&self->eax,
									&self->ecx,
									&self->edx,
									&self->ebx,
									&self->esp,
									&self->ebp,
									&self->esi,
									&self->edi,
									&self->flags
									)) {
        return -1; 
	}
    return 0;
}

static PyMemberDef registers_members[] = {
    {"eax", T_ULONG, offsetof(registers, eax), 0, "eax"},
    {"ecx", T_ULONG, offsetof(registers, ecx), 0, "ecx"},
	{"edx", T_ULONG, offsetof(registers, edx), 0, "edx"},
	{"ebx", T_ULONG, offsetof(registers, ebx), 0, "ebx"},
	{"esp", T_ULONG, offsetof(registers, esp), 0, "esp"},
	{"ebp", T_ULONG, offsetof(registers, ebp), 0, "ebp"},
	{"esi", T_ULONG, offsetof(registers, esi), 0, "esi"},
	{"edi", T_ULONG, offsetof(registers, edi), 0, "edi"},
	{"flags", T_ULONG, offsetof(registers, flags), 0, "flags"},
    {NULL}  /* Sentinel */
};


static PyGetSetDef registers_getseters[] = {
//    {"first", (getter)Noddy_getfirst, (setter)Noddy_setfirst, "first name", NULL},
//    {"last", (getter)Noddy_getlast, (setter)Noddy_setlast, "last name", NULL},
    {NULL}  /* Sentinel */
};

static PyObject * registers_repr(registers* self) {
    static PyObject *format = NULL;
    PyObject *args, *result;

    if (format == NULL) {
        format = PyString_FromString(
			"<registers object (eax 0x%08x,"
			" ecx 0x%08x,"
			" edx 0x%08x,"
			" ebx 0x%08x,"
			" esp 0x%08x,"
			" ebp 0x%08x,"
			" esi 0x%08x,"
			" edi 0x%08x,"
			" flags 0x%08x)>"
			);
        if (format == NULL)
            return NULL;
    }

    args = Py_BuildValue("iiiiiiiii", 
									self->eax,
									self->ecx,
									self->edx,
									self->ebx,
									self->esp,
									self->ebp,
									self->esi,
									self->edi,
									self->flags
		);
    if (args == NULL)
        return NULL;

    result = PyString_Format(format, args);
    Py_DECREF(args);
    
    return result;
}

static PyMethodDef registers_methods[] = {
//    {"name", (PyCFunction)Noddy_name, METH_NOARGS, "Return the name, combining the first and last name"},
    {NULL}  /* Sentinel */
};

static PyTypeObject registersType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "gdetour.registers",             /*tp_name*/
    sizeof(registers),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)registers_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    (reprfunc)registers_repr,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "registers type. A registers object holds the 8 standard registers and the flags register.",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    registers_methods,             /* tp_methods */
    registers_members,             /* tp_members */
    registers_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)registers_init,      /* tp_init */
    0,                         /* tp_alloc */
    registers_new,                 /* tp_new */
};

bool add_module_type_registers(PyObject* m) {
	if (PyType_Ready(&registersType) < 0) {
        return false;
	}

	if (m == NULL){ 
		return false;
	}

    Py_INCREF(&registersType);
	PyModule_AddObject(m, "registers", (PyObject *)&registersType); //steals ref

	return true;
}