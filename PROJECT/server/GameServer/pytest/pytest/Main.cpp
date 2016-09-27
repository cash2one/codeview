#include "python.h"

void printDict(PyObject* obj)
{   
	if ( !PyDict_Check(obj) )
		return;   

	PyObject *k, *keys;

	keys = PyDict_Keys(obj);

	for (int i = 0; i < PyList_GET_SIZE(keys); i++)
	{   
		k = PyList_GET_ITEM(keys, i);
		char* c_name = PyString_AsString(k);
		printf("%s\n", c_name);
	}
} 


void main()
{
	Py_Initialize();

	if ( !Py_IsInitialized() )
		return;

	//PyRun_SimpleString("import sys");

	//PyRun_SimpleString("sys.path.append('./')");

	PyObject * pModule = PyImport_ImportModule("test1");

	if ( 0 == pModule )
		return;

	//PyObject * pDict = PyModule_GetDict(pModule);

	//if ( 0 == pDict )
	//	return;

	//printDict(pDict);

	PyObject * pFunHi = PyObject_GetAttrString(pModule, "sayhi");
	//PyObject * pFunHi = PyDict_GetItemString(pDict, "sayhi");

	if ( 0 == pFunHi )
		return;

	//PyEval_CallObject(pFunHi, NULL);

	PyObject_CallFunction(pFunHi, "s", "lhb");

	//Py_DECREF(pFunHi);
	//PyObject * pClassSecond = PyDict_GetItemString(pDict, "Second");
	PyObject * pClassSecond = PyObject_GetAttrString(pModule, "Second");
	if ( 0 == pClassSecond )
		return;

	//PyObject * pClassPerson = PyDict_GetItemString(pDict, "Person");

	PyObject * pClassPerson = PyObject_GetAttrString(pModule, "Person");

	if ( 0 == pClassPerson )
		return;

	PyObject * pInstanceSecond = PyInstance_New(pClassSecond, NULL, NULL);

	if ( 0 == pInstanceSecond )
		return;

	PyObject * pInstancePerson = PyInstance_New(pClassPerson, NULL, NULL);

	if ( 0 == pInstancePerson )
		return;

	PyObject_CallMethod(pInstanceSecond, "invoke", "O", pInstancePerson);

	Py_Finalize();

	int i = 0;

	scanf("%d", &i);
}
