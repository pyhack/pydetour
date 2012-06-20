#include "stdafx.h"
#include "python_type_detourconfig.h"

#include "structmember.h"

typedef struct detourconfig {
    PyObject_HEAD
	DWORD bytesToPop;
    PyObject* executeOriginal; //Python True or False
} detourconfig;

static void detourconfig_dealloc(detourconfig* self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject * detourconfig_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    detourconfig *self;
    self = (detourconfig *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->bytesToPop = 0;
		Py_INCREF(Py_False);
		self->executeOriginal = Py_False;
	}
    return (PyObject *)self;
}

static int detourconfig_init(detourconfig *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"bytesToPop", "executeOriginal", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ib", kwlist, 
									&self->bytesToPop,
									&self->executeOriginal
									)) {
        return -1; 
	}
    return 0;
}

static PyMemberDef detourconfig_members[] = {
    {"bytesToPop", T_ULONG, offsetof(detourconfig, bytesToPop), 0, "bytesToPop"},
	//executeOriginal is set via the get/setters below
    //{"executeOriginal", T_OBJECT_EX, offsetof(detourconfig, executeOriginal), 0, "executeOriginal"},
    {NULL}  /* Sentinel */
};

PyObject* detourconfig_get_executeOriginal(detourconfig* self, void* closure) {
	Py_INCREF(self->executeOriginal);
	return self->executeOriginal;
}
int detourconfig_set_executeOriginal(detourconfig* self, PyObject* newvalue, void* closure) {
	if (newvalue == Py_True || newvalue == Py_False) {
		Py_INCREF(newvalue);
		Py_DECREF(self->executeOriginal);
		self->executeOriginal = newvalue;
	} else {
		PyErr_Format(PyExc_ValueError, "%r is neither True or False.", newvalue);
		return -1;
	}
	return 0;


}


static PyObject* detourconfig_repr(detourconfig* self) {
    static PyObject *format = NULL;
    PyObject *args, *result;



    if (format == NULL) {
        format = PyString_FromString(
			"<detourConfig object (bytesToPop 0x%x,"
			" executeOriginal %s"
			")>"
			);
        if (format == NULL)
            return NULL;
    }
	
	PyObject* tmp_eO = PyObject_Repr(self->executeOriginal);

	args = Py_BuildValue("iO", 
									self->bytesToPop,
									tmp_eO
	);
    if (args == NULL)
        return NULL;

	Py_DECREF(tmp_eO);

    result = PyString_Format(format, args);
    Py_DECREF(args);
    
    return result;
}
static PyGetSetDef detourconfig_getseters[] = {
	{"executeOriginal", (getter)detourconfig_get_executeOriginal, (setter)detourconfig_set_executeOriginal, "executeOriginal boolean", NULL},
	//    {"last", (getter)Noddy_getlast, (setter)Noddy_setlast, "last name", NULL},
	{NULL}  /* Sentinel */
};
static PyMethodDef detourconfig_methods[] = {
//    {"name", (PyCFunction)Noddy_name, METH_NOARGS, "Return the name, combining the first and last name"},
    {NULL}  /* Sentinel */
};

static PyTypeObject detourconfigType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "gdetour.detourConfig",             /*tp_name*/
    sizeof(detourconfig),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)detourconfig_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    (reprfunc)detourconfig_repr,                         /*tp_repr*/
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
    "detourconfig type. A detourconfig object holds options that the detour will use to determine what to do.",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    detourconfig_methods,             /* tp_methods */
    detourconfig_members,             /* tp_members */
    detourconfig_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)detourconfig_init,      /* tp_init */
    0,                         /* tp_alloc */
    detourconfig_new,                 /* tp_new */
};

bool add_module_type_detourconfig(PyObject* m) {
	if (PyType_Ready(&detourconfigType) < 0) {
        return false;
	}

	if (m == NULL){ 
		return false;
	}

    Py_INCREF(&detourconfigType);
	PyModule_AddObject(m, "detourConfig", (PyObject *)&detourconfigType); //steals ref

	return true;
}