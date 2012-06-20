#pragma once
#include <string>

namespace CPPPython {
	class CPPPythonException {
		public:
			char* errmsg;
			CPPPythonException(char* err="");
			~CPPPythonException();
			operator const char*();
	};
	class NULLPyObjectException: CPPPythonException {
		public: NULLPyObjectException();
	};
	class IndexOutOfRangeException: CPPPythonException {
		public: IndexOutOfRangeException();
	};
	
	class P_GIL {
		//P_GIL is designed to make management of the GIL even easier.
		//Just declare a type of it in the block you want to use Python code in
		//and it will take care of everything else:
		//
		//P_GIL gil;
		PyGILState_STATE state;
	public:
		P_GIL();
		~P_GIL();
	};

	class PObject {
		protected:
			PyObject* myObject;
		public:
			/*
				When you create a PObject, the default action is to INCREF the reference, 
				and DECREF it when the object dies. This is suitable for passing in BORROWED references
				from Python's API. If you don't want an an auto-INCREF, pass 1 as the second arg (stealReferance).

				If you don't want the auto-DECREF, set del_on_destruct to false.

			*/
			PObject();
			void incRef();
			void decRef();
			PObject(PyObject* obj, bool stealReferance=false);
			PObject(const PObject& old); //copy constructor
			~PObject();
			bool del_on_destruct;

			PObject& operator =(const PObject& p); //assignment operator
			operator PyObject*() const; //autocast to PyObject*
			operator bool() const; //is this not pointing to NULL

			PyObject* getObject() const;
			//Technically speaking, almost all of these methods could be const, as the 
			//mutation occurs on the PyObject*, not on this class. However, I've only
			//defined const where it makes logical sense (ie, on get instead of set)

//----------------

			std::string getRepr() const;
			std::string getStr() const;

			Py_ssize_t getLength() const;

			bool hasAttr(char* attr) const;
			bool hasAttr(PyObject* attr) const;

			PObject getAttr(char* attr) const;
			PObject getAttr(PyObject* attr) const;

			bool setAttr(const char* attr, PyObject* value);
			bool setAttr(PyObject* attr, PyObject* value);
    
			bool delAttr(const char* attr);
			bool delAttr(PyObject* attr);


			bool isCallable() const;
			PObject call();
			PObject call(PyObject* arg0); //A NULL argument MUST BE PASSED at the end!
			PObject call(PyObject* arg0,PyObject* arg1); //A NULL argument MUST BE PASSED at the end!
			PObject call(PyObject* arg0,PyObject* arg1,PyObject* arg2); //A NULL argument MUST BE PASSED at the end!
			PObject call(PyObject* arg0,PyObject* arg1,PyObject* arg2,PyObject* arg3); //A NULL argument MUST BE PASSED at the end!
	};
//--------------------
//--------------------
//--------------------
	class PNumber: public PObject {
		public:
			PNumber(PyObject* obj, bool stealReferance=false);
			PNumber(long num);
	};
	class PBool: public PObject {
		public:
			PBool(PyObject* obj, bool stealReferance = false);
			static PBool getTrue();
			static PBool getFalse();
	};
	class PString: public PObject {
		public:
			PString(PyObject* obj, bool stealReferance = false);
			PString(const char*);
			operator const char*();
	};
	class PMapping: public PObject {
		public:
			PMapping(PyObject* obj, bool stealReferance = false);

			bool HasKey(char* key) const;
			bool HasKey(PyObject* key) const;

			bool DelItem(char* key);
			bool DelItem(PyObject* key);

			PObject GetItem(char* key) const;
			bool SetItem(char* key, PyObject* value);
	};
	class PDict: public PObject {
		public:
			PDict(PyObject* obj, bool stealReferance = false);

			static PDict getNewDict();
			bool Contains(char* key) const;
			bool Contains(PyObject* key) const;

			bool DelItem(char* key);
			bool DelItem(PyObject* key);

			PObject GetItem(char* key) const;
			bool SetItem(char* key, PyObject* value);
	};
	class PModule: public PObject {
		public:
			PModule(PyObject* obj, bool stealReferance = false);

			static PModule getModule(const char *name); //Gets a referance to an already imported module
			PDict getDict();

			static PModule getNewModule(const char* name); //Creates a new module
			static PModule importModule(char* name, PyObject* globals, PyObject* locals);
			bool AddObject(const char* name, PyObject* value); //Add an object to module as name. This is a convenience function which can be used from the module's initialization function. This steals a reference to value.
			bool AddConstant(const char* name, long value); //Add an integer constant to module as name. This convenience function can be used from the module's initialization function.
			bool AddConstant(const char *name, const char *value); //Add a string constant to module as name. This convenience function can be used from the module's initialization function. The string value must be null-terminated. Return -1 on error, 0 on success. New in version 2.0. 
	};
	class PSequence: public PObject {
		public:
			PSequence(PyObject* obj, bool stealReferance = false);

			bool Contains(PyObject* key) const;

			bool DelItem(Py_ssize_t i);

			PObject GetItem(Py_ssize_t i) const;
			bool SetItem(Py_ssize_t i, PyObject* value);
	};
};