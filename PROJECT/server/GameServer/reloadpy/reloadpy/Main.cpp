#include "python.h"


PyObject * pModule1 = NULL;
PyObject * pModule2 = NULL;

PyObject * pFun1 = NULL;
PyObject * pFun2 = NULL;

bool Init_Py()
{
	Py_Initialize();

	return Py_IsInitialized();
}

void Decler_Py()
{
	Py_DECREF(pModule1);
	Py_DECREF(pModule2);
	Py_DECREF(pFun1);
	Py_DECREF(pFun2);

	pModule1 = NULL;
	pModule2 = NULL;
	pFun1    = NULL;
	pFun2	 = NULL;
}

void Del_Py()
{
	Decler_Py();
	Py_Finalize();
}

bool Reload()
{
	Del_Py();

	if ( !Init_Py() )
		return false;

	pModule1 = PyImport_ImportModule("testpy1");

	pModule2 = PyImport_ImportModule("testpy2");

	if ( pModule1 == NULL || pModule2 == NULL )
		return false;

	pFun1 = PyObject_GetAttrString(pModule1, "show");
	pFun2 = PyObject_GetAttrString(pModule2, "show");

	if ( pFun1 == NULL || pFun2 == NULL )
		return false;

	return true;
}


int main()
{
	if ( !Init_Py() )
		return 1;

	if ( !Reload() )
		return 1;

	printf("1:UseFunc	2:Reload	3:return\n");

	while(1)
	{
		int i = 0;

		scanf("%d", &i);

		switch(i)
		{
		case 1:
			{
				PyEval_CallObject(pFun1, NULL);
				PyEval_CallObject(pFun2, NULL);
			}
			break;
		case 2:
			{
				if ( !Reload() )
					return 1;
			}
			break;
		case 3:
			{
				Del_Py();
				return 0;
			}
			break;
		}
	}

	Del_Py();
	return 0;
}
