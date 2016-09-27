#ifndef __TEST_DLL_TEST_DLL_H__
#define __TEST_DLL_TEST_DLL_H__

#include "Itest_dll.h"

class test_dll : public Itest_dll
{
public:
	virtual void printA();

	virtual void printB();
};


#endif
