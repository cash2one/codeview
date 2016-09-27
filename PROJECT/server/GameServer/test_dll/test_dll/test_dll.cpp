// test_dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "test_dll.h"


extern "C" __declspec(dllexport) void SayHello()
{
    ::MessageBoxW(NULL, L"Hello", L"fangyukuan", MB_OK);
}

#include "test_dll.h"
#include "stdio.h"



Itest_dll * GetTest_dll()
{
	return new test_dll();
}


void test_dll::printA()
{
	printf("This is printA\n");
}

void test_dll::printB()
{
	printf("This is printB\n");
}
