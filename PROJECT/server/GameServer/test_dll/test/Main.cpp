#include "Itest_dll.h"
#include "stdio.h"

#  pragma comment(lib,"test_dll.lib")

Itest_dll * g_pTest_dll = 0;

int main()
{
	//Itest_dll * pTest_dll = GetTest_dll();
	Itest_dll * pTest_dll = g_pTest_dll;

	pTest_dll->printA();

	pTest_dll->printB();

	int i = 0;

	scanf("%d", &i);
}
