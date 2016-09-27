
#ifndef __BCL_ICRASHDUMP_H__
#define __BCL_ICRASHDUMP_H__

#include "BclHeader.h"

#include <string>

#include <windows.h>
#include <dbghelp.h>

#ifndef BUILD_CRASHDUMP_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"CrashDump.lib")
#endif
#endif





enum
{
	enDumpLog,				//Log------如果此项为FALSE，则一下几项除了MiniDump都无效
	enDumpBasicInfo,		//基本信息
	enDumpModuleList,		//模块列表
	enDumpWindowList,		//窗口列表
	enDumpProcessList,		//进程列表
	enDumpRegister,			//寄存器
	enDumpCallStack,		//调用栈
	enDumpMiniDump,			//输出dmp文件
	enDumpCount
};

struct ICrashDump;

using namespace std;

struct ICrashDumpSink
{
	//开始dump信息会调用这个函数----实现这个函数，设置错误报告内容
	virtual void OnDumpStart(ICrashDump*) = 0;
	//可以用ICrashDump的DumpLn Dump想输出到LogFile的信息
	virtual void OnDump(ICrashDump*,PEXCEPTION_POINTERS) = 0;
	//Dump结束会调用此函数，此函数返回值会被当作异常捕获的返回值
	virtual LONG OnDumpFinish(ICrashDump*,PEXCEPTION_POINTERS) = 0;
};
struct ICrashDump
{
	virtual void CrtSetDbgFlag()=0;

    virtual void CrtSetBreakAlloc(int BreakNum)=0;

	virtual void CrtSetReportMode(int mode) = 0; 


	//挂一个ICrashDumpSink的回调
	virtual bool AttachCrashDumpSink(ICrashDumpSink *) = 0;
	virtual bool DetachCrashDumpSink(ICrashDumpSink *) = 0;
	//设置Log文件名
	virtual void SetLogFileName(const char * szLogFileName) = 0;
	//设置Dmp文件名
	virtual void SetDumpFileName(const char * szDumpFileName) = 0;
	//对上面的枚举的开关
	virtual void SetDumpOn(int iEnum,bool bOn) = 0;

	/*
	设置Dump类型，lDumpType可以为一下这几种类型
	注意:MiniDumpWithIndirectlyReferencedMemory在win98下不能用

    MiniDumpNormal                         = 0x0000,
    MiniDumpWithDataSegs                   = 0x0001,
    MiniDumpWithFullMemory                 = 0x0002,
    MiniDumpWithHandleData                 = 0x0004,
    MiniDumpFilterMemory                   = 0x0008,
    MiniDumpScanMemory                     = 0x0010,
    MiniDumpWithUnloadedModules            = 0x0020,
    MiniDumpWithIndirectlyReferencedMemory = 0x0040,
    MiniDumpFilterModulePaths              = 0x0080,
    MiniDumpWithProcessThreadData          = 0x0100,
    MiniDumpWithPrivateReadWriteMemory     = 0x0200,
    MiniDumpWithoutOptionalData            = 0x0400,

  //
  // A normal minidump contains just the information
  // necessary to capture stack traces for all of the
  // existing threads in a process.
  //
  // A minidump with data segments includes all of the data
  // sections from loaded modules in order to capture
  // global variable contents.  This can make the dump much
  // larger if many modules have global data.
  //
  // A minidump with full memory includes all of the accessible
  // memory in the process and can be very large.  A minidump
  // with full memory always has the raw memory data at the end
  // of the dump so that the initial structures in the dump can
  // be mapped directly without having to include the raw
  // memory information.
  //
  // Stack and backing store memory can be filtered to remove
  // data unnecessary for stack walking.  This can improve
  // compression of stacks and also deletes data that may
  // be private and should not be stored in a dump.
  // Memory can also be scanned to see what modules are
  // referenced by stack and backing store memory to allow
  // omission of other modules to reduce dump size.
  // In either of these modes the ModuleReferencedByMemory flag
  // is set for all modules referenced before the base
  // module callbacks occur.
  //
  // On some operating systems a list of modules that were
  // recently unloaded is kept in addition to the currently
  // loaded module list.  This information can be saved in
  // the dump if desired.
  //
  // Stack and backing store memory can be scanned for referenced
  // pages in order to pick up data referenced by locals or other
  // stack memory.  This can increase the size of a dump significantly.
  //
  // Module paths may contain undesired information such as user names
  // or other important directory names so they can be stripped.  This
  // option reduces the ability to locate the proper image later
  // and should only be used in certain situations.
  //
  // Complete operating system per-process and per-thread information can
  // be gathered and stored in the dump.
  //
  // The virtual address space can be scanned for various types
  // of memory to be included in the dump.
  //
  // Code which is concerned with potentially private information
  // getting into the minidump can set a flag that automatically
  // modifies all existing and future flags to avoid placing
  // unnecessary data in the dump.  Basic data, such as stack
  // information, will still be included but optional data, such
  // as indirect memory, will not.
  //
	*/
	virtual void SetDumpType(INT32 lDumpType) = 0;
	//得到异常码
	virtual UINT32 GetExceptionCode(PEXCEPTION_POINTERS pExceptionInfo) = 0;
	//得到异常描述
	virtual LPCTSTR	GetExceptionDesc(PEXCEPTION_POINTERS pExceptionInfo) = 0;
	//得到崩溃的模块文件偏移
	virtual UINT32	GetCrashFileOffset(PEXCEPTION_POINTERS pExceptionInfo) = 0;
	//得到崩溃的模块文件名
	virtual LPCTSTR	GetCrashModuleName(PEXCEPTION_POINTERS pExceptionInfo) = 0;
	//输出一些Dump信息到Log文件
	virtual void DumpLn(LPCTSTR szMsg) = 0;
	//取得崩溃模块的产品版本号
	virtual void GetCrashModuleProductVer(PEXCEPTION_POINTERS pExceptionInfo,UINT16 pwVer[4]) = 0;
	//取得当前调用栈
	virtual string GetCallStack(CONTEXT& Context,HANDLE hThread) = 0;
};



BCL_API ICrashDump * CreateCrashDump();








#endif

