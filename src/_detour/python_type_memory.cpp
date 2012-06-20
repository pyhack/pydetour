#include "stdafx.h"
#include "python_type_memory.h"

#include "structmember.h"

#include <windows.h>
#include "CPPPython.h"

using namespace CPPPython;

typedef struct memory {
    PyObject_HEAD
	PyObject* ctypes;

	BYTE* cursor;
	PyObject* autoInc;


} memory;


static PyObject * memory_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    memory *self;
    self = (memory *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->ctypes = PyImport_ImportModule("ctypes"); //new ref, cleared later in memory_dealloc
		if (self->ctypes == NULL) {
			Py_DECREF(self);
			return PyErr_Format(PyExc_NameError, "Error creating <memory> type: Required module ctypes not found.");
		}
		self->cursor = 0;
		Py_INCREF(Py_False);
		self->autoInc = Py_False;
	}
    return (PyObject *)self;
}
static void memory_dealloc(memory* self) {
	Py_CLEAR(self->ctypes);
	Py_CLEAR(self->autoInc);
	self->ob_type->tp_free((PyObject*)self);
}
static int memory_init(memory *self, PyObject *args, PyObject *kwds) {
	PyArg_ParseTuple(args, "|l", &self->cursor);
    return 0;
}

static PyObject* memory_repr(memory* self) {
	PyObject* s0 = PyString_FromFormat("<memory object with cursor at %p, autoInc ", self->cursor);
	PyObject* s1 = PyObject_Repr(self->autoInc);
	PyObject* s2 = PyString_FromString(">");

	PyString_ConcatAndDel(&s0, s1);
	PyString_ConcatAndDel(&s0, s2);
	return s0;
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//------Reading Methods

#pragma region memory Reading Methods

//static PyObject* memory_byte(memory* self, PyObject* args) {
//	long count = 1;
//	if (!PyArg_ParseTuple(args, "|l", &count)) {
//		return NULL;
//	}
//	if (IsBadReadPtr((LPVOID)self->cursor, count)) {
//		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
//	}
//	PyObject* ret;
//	ret = PyString_FromStringAndSize((char*)self->cursor, count);
//	if (self->autoInc == Py_True) {
//		self->cursor += count;
//	}
//	return ret;
//}
static PyObject* memory_get_dword(memory* self, void* closure) {
	assert(sizeof(unsigned long) == 4);
	if (IsBadReadPtr((LPVOID)self->cursor, 4)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
	}
	PyObject* ret = PyLong_FromLong(*(long*)self->cursor);
	if (self->autoInc == Py_True) {
		self->cursor += (4);
	}
	return ret;
}
int memory_set_dword(memory* self, PyObject* newvalue, void* closure) {
	assert(sizeof(unsigned long) == 4);
	if (newvalue == NULL) {
		PyErr_Format(PyExc_ValueError, "You can't delete memory!");
	}
	if (IsBadWritePtr((LPVOID)self->cursor, 4)) {
		PyErr_Format(PyExc_IndexError, "%p is an invalid (nonwriteable) memory address", self->cursor);
		return -1;
	}
	if (!PyInt_Check(newvalue)) {
		PyErr_Format(PyExc_ValueError, "You can only assign an integral value as a dword.");
		return -1;
	}
	unsigned long in_int = PyInt_AsLong(newvalue);
	assert(sizeof(long) == 4);
	memcpy(self->cursor, &in_int, 4);
	return 0;
}
static PyObject* memory_get_word(memory* self, void* closure) {
	assert(sizeof(unsigned short int) == 2);
	if (IsBadReadPtr((LPVOID)self->cursor, 2)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
	}
	unsigned long l_ret = *(unsigned short int*)self->cursor;
	PyObject* ret = PyLong_FromLong(l_ret);
	if (self->autoInc == Py_True) {
		self->cursor += (2);
	}
	return ret;
}
int memory_set_word(memory* self, PyObject* newvalue, void* closure) {
	assert(sizeof(unsigned short int) == 2);
	if (newvalue == NULL) {
		PyErr_Format(PyExc_ValueError, "You can't delete memory!");
	}
	if (IsBadWritePtr((LPVOID)self->cursor, 2)) {
		PyErr_Format(PyExc_IndexError, "%p is an invalid (nonwriteable) memory address", self->cursor);
		return -1;
	}
	if (!PyInt_Check(newvalue)) {
		PyErr_Format(PyExc_ValueError, "You can only assign an integral value as a dword.");
		return -1;
	}
	unsigned long in_int = PyInt_AsLong(newvalue);
	if (in_int >= (1 << 16)) {
		PyErr_Format(PyExc_ValueError, "A word's value must less than 65536.");
		return -1;
	}
	assert(sizeof(short) == 2);
	memcpy(self->cursor, &in_int, 2);
	return 0;
}
static PyObject* memory_get_byte(memory* self, void* closure) {
	assert(sizeof(BYTE) == 1);
	if (IsBadReadPtr((LPVOID)self->cursor, 1)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
	}
	unsigned long l_ret = *(BYTE*)self->cursor;
	PyObject* ret = PyLong_FromLong(l_ret);
	if (self->autoInc == Py_True) {
		self->cursor += (1);
	}
	return ret;
}
int memory_set_byte(memory* self, PyObject* newvalue, void* closure) {
	assert(sizeof(BYTE) == 1);
	if (newvalue == NULL) {
		PyErr_Format(PyExc_ValueError, "You can't delete memory!");
	}
	if (IsBadWritePtr((LPVOID)self->cursor, 1)) {
		PyErr_Format(PyExc_IndexError, "%p is an invalid (nonwriteable) memory address", self->cursor);
		return -1;
	}
	if (!PyInt_Check(newvalue)) {
		PyErr_Format(PyExc_ValueError, "You can only assign an integral value as a dword.");
		return -1;
	}
	unsigned long in_int = PyInt_AsLong(newvalue);
	if (in_int >=  (1 << 8)) {
		PyErr_Format(PyExc_ValueError, "A byte's value must less than 256.");
		return -1;
	}
	memcpy(self->cursor, &in_int, 1);
	return 0;
}

//static PyObject* memory_dword(memory* self, PyObject* args) {
//	long count = 1;
//	PyObject* in_bool = NULL;
//	if (!PyArg_ParseTuple(args, "|lO", &count, &in_bool)) {
//		return NULL;
//	}
//	if (IsBadReadPtr((LPVOID)self->cursor, count*4)) {
//		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
//	}
//	PyObject* ret;
//	if (count == 1) {
//		if (in_bool == Py_True) {
//			PyObject* obj = PyObject_GetAttrString(self->ctypes, "c_long");
//			ret = PyObject_CallMethod(obj,"from_address", "l", (long*)self->cursor);
//			Py_DECREF(obj);
//		} else {
//			ret = PyLong_FromLong(*(long*)self->cursor);
//		}
//	} else {
//		ret = PyTuple_New(count);
//		for (int i = 0; i < count; i++) {
//			PyTuple_SetItem(ret, i, PyLong_FromLong(*((long*)self->cursor+i))); //hooray pointer math and ref stealing tuple funcs
//		}
//	}ret = PyLong_FromLong(*(long*)self->cursor);
//	if (self->autoInc == Py_True) {
//		self->cursor += (count * 4);
//	}
//	return ret;
//}
//static PyObject* memory_qword(memory* self, PyObject* args) {
//	long count = 1;
//	if (!PyArg_ParseTuple(args, "|l", &count)) {
//		return NULL;
//	}
//	if (IsBadReadPtr((LPVOID)self->cursor, count*8)) {
//		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
//	}
//	PyObject* ret;
//	if (count == 1) {
//		ret = PyLong_FromLongLong(*(PY_LONG_LONG*)self->cursor);
//	} else {
//		ret = PyTuple_New(count);
//		for (int i = 0; i < count; i++) {
//			PyTuple_SetItem(ret, i, PyLong_FromLong(*((PY_LONG_LONG*)self->cursor+i))); //hooray pointer math and ref stealing tuple funcs
//		}
//	}
//	if (self->autoInc == Py_True) {
//		self->cursor += (count * 8);
//	}
//	return ret;
//}

#pragma endregion //These methods are those like "memory_byte", "memory_dword", etc

static PyObject* memory_rshift(memory* self, PyObject* other) {
	long amt;
	if (self->ob_type != &memoryType) {
		return PyErr_Format(PyExc_IndexError, "left operand must be memory");
	}
	if (!PyInt_Check(other)) {
		return PyErr_Format(PyExc_IndexError, "right operand must be int");
	}
	amt = PyInt_AsLong(other);
	if (IsBadReadPtr((LPVOID)self->cursor, 4)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
	}
	if (IsBadReadPtr((const void*) (self->cursor + amt), 4)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", *(long*)self->cursor + amt);
	}
	PObject mT((PyObject*)&memoryType);
	PObject tmp = PyInt_FromLong(*(long*)self->cursor + amt);
	PObject newObj = mT.call(tmp, NULL);
	newObj.incRef(); //live beyond return

	return newObj;

}

static PyObject* memory_pointer(memory* self, PyObject* args) {
	PObject mT((PyObject*)&memoryType);
	int count = 1;
	if (!PyArg_ParseTuple(args, "")) {
		return NULL;
	}
	if (IsBadReadPtr((LPVOID)self->cursor, count*4)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", self->cursor);
	}


	PObject tmp = PyLong_FromLong(*(long*)self->cursor);
	PObject newObj = mT.call(tmp, NULL);
	newObj.incRef(); //live beyond return

	return newObj;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//------Sequence Methods

static PyObject* memory_getItem(memory* self, Py_ssize_t i) {
	PObject mT((PyObject*)&memoryType);

	i += (Py_ssize_t) self->cursor;
	if (IsBadReadPtr((LPVOID)i, 1)) {
		return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", i);
	}

	PyObject* tmp = PyInt_FromSsize_t(i);
	PObject newObj = mT.call(tmp, NULL);
	newObj.incRef(); //live beyond return
	Py_DECREF(tmp);

	return newObj;
}



static PyObject* memory_getSlice(memory* self, Py_ssize_t i, Py_ssize_t len) {
	i += (long)self->cursor;
	if (IsBadReadPtr((LPVOID)i, len)) {
		goto err_ptr;
	}
	return PyString_FromStringAndSize((char*)((long)i), len);
err_ptr:
	return PyErr_Format(PyExc_IndexError, "%p is an invalid (nonreadable) memory address", i);
}

static int memory_setItem(memory* self, Py_ssize_t i, PyObject *v) {
	unsigned long objsize = 1;
	unsigned long objval = 0;

	if (PyString_Check(v) == FALSE) {
		PyErr_Format(PyExc_TypeError, "Item must be a sequence of bytes");
		return -1;
	}
	PString s = PString(v);

	objsize = s.getLength();
	char* st = PyString_AsString(v);

	i += (long)self->cursor;

	if (IsBadWritePtr((LPVOID)i, objsize)) {
		goto err_ptr;
	}
	memcpy((void*)i, st, objsize);
	return 0;
err_ptr:
	PyErr_Format(PyExc_IndexError, "%p is an invalid (nonwriteable) memory address", i);
	return -1;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//----------memory get/setters
//-------------------------------------------------------------------------
#pragma region memory getters and setters

PyObject* memory_get_autoInc(memory* self, void* closure) {
	Py_INCREF(self->autoInc);
	return self->autoInc;
}
int memory_set_autoInc(memory* self, PyObject* newvalue, void* closure) {
	if (newvalue == NULL) {
		PyErr_Format(PyExc_ValueError, "autoInc cannot be deleted.");
	}
	if (newvalue == Py_True || newvalue == Py_False) {
		Py_INCREF(newvalue);
		Py_DECREF(self->autoInc);
		self->autoInc = newvalue;
	} else {
		PyErr_Format(PyExc_ValueError, "%r is neither True nor False.", newvalue);
		return -1;
	}
	return 0;
}

#pragma endregion
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//----------memory Python Type Information
//-------------------------------------------------------------------------

static PyMemberDef memory_members[] = {
	//Comment to hide implementation
	{"cursor", T_ULONG, offsetof(memory, cursor), 0, "Memory offset from which reading methods begin"},
	//{"autoInc", T_ULONG, offsetof(memory, base), 0, "If True, base pointer is incremented after reading memory"},
	{NULL}  /* Sentinel */
};
static PyGetSetDef memory_getseters[] = {
	{"autoInc", (getter)memory_get_autoInc, (setter)memory_set_autoInc, "If True, cursor is incremented after reading memory", NULL},
	{"dword", (getter)memory_get_dword, (setter)memory_set_dword, "Read / Write this location in memory as a dword", NULL},
	{"word", (getter)memory_get_word, (setter)memory_set_word, "Read / Write this location in memory as a word", NULL},
	{"byte", (getter)memory_get_byte, (setter)memory_set_byte, "Read / Write this location in memory as a byte", NULL},
	{NULL}  /* Sentinel */
};
static PyMethodDef memory_methods[] = {
	{"pointer", (PyCFunction)memory_pointer, METH_VARARGS, "Returns another memory instance (like indexing) by following the pointer at the current address. This function never increases the autoInc cursor."},
	//{"byte", (PyCFunction)memory_byte, METH_VARARGS, "Returns memory as a sequence of bytes."},
	//{"dword", (PyCFunction)memory_dword, METH_VARARGS, "Returns memory as a sequence of dwords."},
	//{"qword", (PyCFunction)memory_qword, METH_VARARGS, "Returns memory as a sequence of qwords."},
	{NULL}  /* Sentinel */
};
static PySequenceMethods memory_seq_methods[] = {
	0, /*lenfunc sq_length;*/
	0, //(binaryfunc) memory_concat,
	0, /*ssizeargfunc sq_repeat;*/
	(ssizeargfunc) memory_getItem, //ssizeargfunc /*sq_item;*/
	(ssizessizeargfunc) memory_getSlice, //ssizessizeargfunc /*sq_slice;*/
	(ssizeobjargproc) memory_setItem, //ssizeobjargproc /*sq_ass_item;*/
	0, /*ssizessizeobjargproc sq_ass_slice;*/
	0, /*objobjproc sq_contains;*/
	/* Added in release 2.0 */
	0, /*binaryfunc sq_inplace_concat;*/
	0, /*ssizeargfunc sq_inplace_repeat;*/
};
static PyNumberMethods memory_num_methods[] = {
	0, //binaryfunc nb_add;
	0, //binaryfunc nb_subtract;
	0, //binaryfunc nb_multiply;
	0, //binaryfunc nb_divide;
	0, //binaryfunc nb_remainder;
	0, //binaryfunc nb_divmod;
	0, //ternaryfunc nb_power;
	0, //unaryfunc nb_negative;
	0, //unaryfunc nb_positive;
	0, //unaryfunc nb_absolute;
	0, //inquiry nb_nonzero;
	0, //unaryfunc nb_invert;
	0, //nb_lshift;
	(binaryfunc) memory_rshift, //nb_rshift;
	0, //binaryfunc nb_and;
	0, //binaryfunc nb_xor;
	0, //binaryfunc nb_or;
	0, //coercion nb_coerce;
	0, //unaryfunc nb_int;
	0, //unaryfunc nb_long;
	0, //unaryfunc nb_float;
	0, //unaryfunc nb_oct;
	0, //unaryfunc nb_hex;
	/* Added in release 2.0 */
	0, //binaryfunc nb_inplace_add;
	0, //binaryfunc nb_inplace_subtract;
	0, //binaryfunc nb_inplace_multiply;
	0, //binaryfunc nb_inplace_divide;
	0, //binaryfunc nb_inplace_remainder;
	0, //ternaryfunc nb_inplace_power;
	0, //binaryfunc nb_inplace_lshift;
	0, //binaryfunc nb_inplace_rshift;
	0, //binaryfunc nb_inplace_and;
	0, //binaryfunc nb_inplace_xor;
	0, //binaryfunc nb_inplace_or;

	/* Added in release 2.2 */
	/* The following require the Py_TPFLAGS_HAVE_CLASS flag */
	0, //binaryfunc nb_floor_divide;
	0, //binaryfunc nb_true_divide;
	0, //binaryfunc nb_inplace_floor_divide;
	0, //binaryfunc nb_inplace_true_divide;

	/* Added in release 2.5 */
	0, //unaryfunc nb_index;
};
static PyTypeObject memoryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "gdetour.memoryType",             /*tp_name*/
    sizeof(memory),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)memory_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    (reprfunc)memory_repr,                         /*tp_repr*/
    memory_num_methods,                         /*tp_as_number*/
    memory_seq_methods,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
    "Instances of memory allow indexing into the memory of the current process. Use memory[0x1234] to return the single byte at that location. Use memory[0x1234] = 23 to set the single byte. Alternatively, to set larger sized values, use memory[0x1234] = (4, 23456) (That is, set it to a tuple of (bytesize, value). Note that bytesize can be at most 4.",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    memory_methods,             /* tp_methods */
    memory_members,             /* tp_members */
    memory_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)memory_init,      /* tp_init */
    0,                         /* tp_alloc */
    memory_new,                 /* tp_new */
};


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

bool add_module_type_memory(PyObject* m) {
	if (PyType_Ready(&memoryType) < 0) {
        return false;
	}

	if (m == NULL){ 
		return false;
	}

    Py_INCREF(&memoryType);
	PyModule_AddObject(m, "memoryType", (PyObject *)&memoryType); //steals ref

	PObject mT((PyObject*)&memoryType);
	PObject myMem = mT.call(NULL);
	myMem.incRef();
	PyModule_AddObject(m, "memory", myMem); //steals ref!!

	return true;
}