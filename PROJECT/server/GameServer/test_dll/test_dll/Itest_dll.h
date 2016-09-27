#ifndef __TEST_DLL_ITEST_DLL_H__
#define __TEST_DLL_ITEST_DLL_H__


class Itest_dll
{
public:
	virtual void printA() = 0;

	virtual void printB() = 0;
};

__declspec(dllexport) Itest_dll * GetTest_dll();

#endif
