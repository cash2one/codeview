#include "Python.h"

int main()
{
	Py_Initialize();  

	// 检查初始化是否成功   
	if ( !Py_IsInitialized() ) {  
		return -1;  
	}

	PyObject * pModule = NULL;
	PyObject * pFunc = NULL;
	PyObject *pName = NULL;

	//PyRun_SimpleString("import test");

	//PyRun_SimpleString("test.show()");

	//PyRun_SimpleString("import test");

	//PyRun_SimpleString("test.name");

	//PyRun_SimpleString("import sys");

    //PyRun_SimpleString("sys.path.append('.\')");

	//pName = PyString_FromString("test");

	//pModule = PyImport_Import(pName);
	//PyRun_SimpleString("print('sdfsadf')");

	pModule = PyImport_ImportModule("testpy");

	//pModule = PyImport_Import(pName);

	//PyObject *pDict = PyModule_GetDict(pModule);

	//pFunc = PyDict_GetItemString(pDict,"show");

	pFunc = PyObject_GetAttrString(pModule, "show");


	PyEval_CallObject(pFunc, NULL);


	int i = 0;

	scanf("%d", &i);
	 
	// 关闭Python   
	Py_Finalize(); 
	return 0;
}
