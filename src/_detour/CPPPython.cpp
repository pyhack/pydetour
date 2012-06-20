#include "stdafx.h"
#include "CPPPython.h"

namespace CPPPython {
	CPPPythonException::CPPPythonException(char* err) {
		if (err == NULL) {
			this->errmsg = "";
			return;
		}
		size_t l = strlen(err) + 1;
		this->errmsg = new char[l];
		memset(this->errmsg, 0, l);
		strcpy_s(this->errmsg, l, err);
		OutputDebugString(this->errmsg);
		OutputDebugString("\n");
		//MessageBoxA(0, this->errmsg, "CPPPython Exception", 0);
		//__asm { INT 3 }
	}
	CPPPythonException::~CPPPythonException() {
		if (this->errmsg != NULL) {
			delete[] this->errmsg;
		}
	}
	CPPPythonException::operator const char *() {
		return this->errmsg;
	}
	NULLPyObjectException::NULLPyObjectException() {
		CPPPythonException::CPPPythonException("NULL PyObject pointer exception");
	}
	IndexOutOfRangeException::IndexOutOfRangeException() {
		CPPPythonException::CPPPythonException("Index out of range");
	}
//-------------------------------------------------------------------------
	P_GIL::P_GIL() {
		if (!Py_IsInitialized()) {
			OutputDebugString("Grabbing GIL from uninitilized Python?!");
		}
		this->state = PyGILState_Ensure();
	}
	P_GIL::~P_GIL() {
		if (::Py_IsInitialized()) {
			PyGILState_Release(state);
		}
	}
//-------------------------------------------------------------------------
	PObject::PObject() {
		this->del_on_destruct = true;
		this->myObject = NULL;
	}
	void PObject::incRef() {
		Py_XINCREF(this->myObject);
	}
	void PObject::decRef() {
		Py_XDECREF(this->myObject);
	}
	PObject::PObject(PyObject* obj, bool stealReferance) {
		this->del_on_destruct = true;
		if (!stealReferance && obj) {
			Py_INCREF(obj); //This is a borrowed ref, so we should take our own ref
		}
		this->myObject = obj;
	}
	PObject::PObject(const PObject& old) {
		//copy constructor
		this->del_on_destruct = true;
		Py_INCREF(old.myObject); //This new class always copies the ref
		this->myObject = old.myObject;
	}
	PObject::~PObject() {
		if (this->del_on_destruct) {
			Py_XDECREF(this->myObject);
		}
	}
	PObject& PObject::operator =(const PObject& p) {
		if (this == &p) {
			return *this; //Same object
		}
		this->del_on_destruct = true;
		this->myObject = p.myObject;
		if (p.del_on_destruct && p.myObject) {
			//take ownership here, the old object will die and drop it's ref
			Py_INCREF(p.myObject);
		} else {
			//don't addref it, the old is the lifetime manager
			//XXX: is this true? should we addref it?
		}
		return *this;
	}
	PObject::operator bool() const {
		return (this->myObject != NULL);
	}
	PObject::operator PyObject*() const {
		if (this->myObject == NULL) {
			//throw new CPPPythonException("Warning: Implicit conversion to a NULL PyObject*");
			OutputDebugString("Warning: Implicit conversion to a NULL PyObject*");
		}
		return this->myObject;
	}
	PyObject* PObject::getObject() const {
		return this->myObject;
	}

	//-----------
	std::string PObject::getRepr() const {
		PyObject* tmp = PyObject_Repr(this->myObject); //new ref
		if (!tmp) {
			throw CPPPythonException("Could not get repr() of this object");
		}
		std::string ret(::PyString_AsString(tmp));
		Py_DECREF(tmp);
		return ret;
	}
	std::string PObject::getStr() const {
		PyObject* tmp = PyObject_Str(this->myObject); //new ref
		if (!tmp) {
			throw CPPPythonException("Could not get str() of this object");
		}
		std::string ret(::PyString_AsString(tmp));
		Py_DECREF(tmp);
		return ret;
	}
	Py_ssize_t PObject::getLength() const {
		Py_ssize_t t = PyObject_Size(this->myObject);
		if (t == -1) {
			throw CPPPythonException("Trying to get len() of non-sequence or non-mapping");
		}
		return t;
	}
	bool PObject::hasAttr(PyObject* attr) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_HasAttr(this->myObject, attr) == 0) {
			return false;
		}
		return true;
	}
	bool PObject::hasAttr(char* attr) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_HasAttrString(this->myObject, attr) == 0) {
			return false;
		}
		return true;
	}





	PObject PObject::getAttr(char* attr) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PyObject* ret = ::PyObject_GetAttrString(this->myObject, attr);
		return PObject(ret, true);
	}
	PObject PObject::getAttr(PyObject* attr) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PyObject* ret = ::PyObject_GetAttr(this->myObject, attr);
		return PObject(ret, true);
	}

	bool PObject::setAttr(const char* attr, PyObject* value) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_SetAttrString(this->myObject, attr, value) == -1) {
			return false;
		}
		return true;
	}
	bool PObject::setAttr(PyObject* attr, PyObject* value) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_SetAttr(this->myObject, attr, value) == -1) {
			return false;
		}
		return true;
	}

	bool PObject::delAttr(const char* attr) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_DelAttrString(this->myObject, attr) == -1) {
			return false;
		}
		return true;
	}
	bool PObject::delAttr(PyObject* attr) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyObject_DelAttr(this->myObject, attr) == -1) {
			return false;
		}
		return true;
	}








	bool PObject::isCallable() const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyCallable_Check(this->myObject) == 0) {
			return false;
		}
		return true;
	}
	PObject PObject::call() {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		//A null argument MUST BE PASSED at the end!
		PyObject* ret = PyObject_CallFunctionObjArgs(this->myObject, NULL);
		return PObject(ret, true);
	}
	PObject PObject::call(PyObject* arg0) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		//A null argument MUST BE PASSED at the end!
		PyObject* ret = PyObject_CallFunctionObjArgs(this->myObject, arg0);
		return PObject(ret, true);
	}
	PObject PObject::call(PyObject* arg0, PyObject* arg1) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		//A null argument MUST BE PASSED at the end!
		PyObject* ret = PyObject_CallFunctionObjArgs(this->myObject, arg0, arg1);
		return PObject(ret, true);
	}
	PObject PObject::call(PyObject* arg0, PyObject* arg1, PyObject* arg2) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		//A null argument MUST BE PASSED at the end!
		PyObject* ret = PyObject_CallFunctionObjArgs(this->myObject, arg0, arg1, arg2);
		return PObject(ret, true);
	}
	PObject PObject::call(PyObject* arg0, PyObject* arg1, PyObject* arg2, PyObject* arg3) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		//A null argument MUST BE PASSED at the end!
		PyObject* ret = PyObject_CallFunctionObjArgs(this->myObject, arg0, arg1, arg2, arg3);
		return PObject(ret, true);
	}

//--------------------
//--------------------
//--------------------
	PNumber::PNumber(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PyNumber_Check(obj) == 0) {
			throw new CPPPythonException("PNumber objects must have valid numbers to wrap");
		}
		if (!stealReferance) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	PNumber::PNumber(long num) {
		this->myObject = PyInt_FromLong(num); //returns new ref
	}
//-------------------------------------------------------------------------
	PBool::PBool(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PyBool_Check(obj) == 0) {
			throw new CPPPythonException("PBool objects must wrap real Python Booleans.");
		}
		if (!stealReferance) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	PBool PBool::getTrue() {
		PBool x = (Py_True);
		return x;
	}
	PBool PBool::getFalse() {
		PBool x = (Py_True);
		return x;
	}
//-------------------------------------------------------------------------
	PString::PString(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PyString_Check(obj) == 0) {
			throw new CPPPythonException("PString objects must have valid strings to wrap");
		}
		if (!stealReferance && obj) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	PString::PString(const char* obj) {
		if (!obj) { this->myObject = NULL; return; }
		this->myObject = PyString_FromString(obj);
	}
//-------------------------------------------------------------------------
	PModule::PModule(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (!obj) {
			throw new CPPPythonException("PModule objects cannot wrap NULL objects");
		}
		if (PyModule_Check(obj) == 0) {
			throw new CPPPythonException("PModule objects must have valid modules to wrap");
		}
		if (!stealReferance && obj) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	PDict PModule::getDict() {
		PDict ret = PDict(::PyModule_GetDict(this->myObject));
		return ret;
	}
	PModule PModule::getModule(const char *name) {
		PModule ret = PModule(::PyImport_AddModule(name)); //borrowed
		return ret;
	}
	PModule PModule::getNewModule(const char *name) {
		PModule ret = PModule(::PyModule_New(name), true);
		return ret;
	}
	PModule PModule::importModule(char* name, PyObject* globals, PyObject* locals) {
		PyObject* ret;
		if (locals == globals == NULL) {
			ret = ::PyImport_ImportModule(name);
			return PModule(ret, true);
		}
		if (::PyMapping_Check(locals) == 0) {
			throw new CPPPythonException("PModule::importModule: locals must be a valid mapping");
		}
		if (::PyMapping_Check(globals) == 0) {
			throw new CPPPythonException("PModule::importModule: globals must be a valid mapping");
		}
		ret = ::PyImport_ImportModuleEx(name, globals, locals, NULL);
		return PModule(ret, true);
	}
	bool PModule::AddConstant(const char *name, long value) {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		if (::PyModule_AddIntConstant(this->myObject, name, value) == -1) {
			return false;
		}
		return true;
	}
	bool PModule::AddConstant(const char *name, const char* value) {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		if (::PyModule_AddStringConstant(this->myObject, name, value) == -1) {
			return false;
		}
		return true;
	}
	bool PModule::AddObject(const char *name, PyObject *value) {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		Py_INCREF(value); //will steal a ref
		if (::PyModule_AddObject(this->myObject, name, value) == -1) {
			return false;
		}
		return true;
	}
//----------------------------------------------------------------------
	PMapping::PMapping(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PyMapping_Check(obj) == 0) {
			throw new CPPPythonException("PMapping objects must have valid mappings to wrap");
		}
		if (!stealReferance && obj) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	bool PMapping::HasKey(char* key) const {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		if (::PyMapping_HasKeyString(this->myObject, key) == 0) {
			return false;
		}
		return true;
	}
	bool PMapping::HasKey(PyObject* key) const {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		if (::PyMapping_HasKey(this->myObject, key) == 0) {
			return false;
		}
		return true;
	}
	bool PMapping::DelItem(char* key) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (PyMapping_DelItemString(this->myObject, key) == -1) {
			return false;
		}
		return true;
	}
	bool PMapping::DelItem(PyObject* key) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (PyMapping_DelItem(this->myObject, key) == -1) {
			return false;
		}
		return true;
	}
	PObject PMapping::GetItem(char* key) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PyObject* ret = ::PyMapping_GetItemString(this->myObject, key);
		if (ret == NULL) {
			throw new IndexOutOfRangeException();
		}
		return PObject(ret, true);
	}
	bool PMapping::SetItem(char *key, PyObject *value) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyMapping_SetItemString(this->myObject, key, value) == -1) {
			return false;
		}
		return true;
	}
//-------------------------------------------------------------------------
	PDict PDict::getNewDict() {
		return PDict(PyDict_New(), true);
	}
	PDict::PDict(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PyDict_Check(obj) == 0) {
			throw new CPPPythonException("PDict objects must have valid mappings to wrap");
		}
		if (!stealReferance) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	bool PDict::Contains(PyObject* key) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyDict_Contains(this->myObject, key) == 1) {
			return true;
		}
		return false;
	}
	bool PDict::Contains(char* key) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PString str = PString(key);
		return this->Contains(str);
	}
	bool PDict::DelItem(char* key) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (PyDict_DelItemString(this->myObject, key) == -1) {
			return false;
		}
		return true;
	}
	bool PDict::DelItem(PyObject* key) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (PyDict_DelItem(this->myObject, key) == -1) {
			return false;
		}
		return true;
	}
	PObject PDict::GetItem(char* key) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PyObject* ret = ::PyDict_GetItemString(this->myObject, key);
		return PObject(ret, true);
	}
	bool PDict::SetItem(char *key, PyObject *value) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PyDict_SetItemString(this->myObject, key, value) == -1) {
			return false;
		}
		return true;
	}
//--------------------------------------------------------------------
	PSequence::PSequence(PyObject* obj, bool stealReferance) {
		if (!obj) { this->myObject = NULL; return; }
		if (PySequence_Check(obj) == 0) {
			throw new CPPPythonException("PSequence objects must have valid sequences to wrap");
		}
		if (!stealReferance && obj) {
			Py_INCREF(obj);
		}
		this->myObject = obj;
	}
	bool PSequence::Contains(PyObject* v) const {
		if (!this->myObject) {
			throw new NULLPyObjectException();
		}
		if (::PySequence_Contains(this->myObject, v) == 0) {
			return false;
		}
		return true;
	}
	bool PSequence::DelItem(Py_ssize_t i) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (PySequence_DelItem(this->myObject, i) == -1) {
			return false;
		}
		return true;
	}
	PObject PSequence::GetItem(Py_ssize_t i) const {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		PyObject* ret = ::PySequence_GetItem(this->myObject, i);
		if (ret == NULL) {
			if (PyErr_ExceptionMatches(PyExc_IndexError)) {
				PyErr_Clear();
				throw new IndexOutOfRangeException();
			}
		}
		return PObject(ret, true);
	}
	bool PSequence::SetItem(Py_ssize_t i, PyObject *value) {
		if (!this->myObject) { throw new NULLPyObjectException(); }
		if (::PySequence_SetItem(this->myObject, i, value) == -1) {
			return false;
		}
		return true;
	}

};