
#include "CrashReport.h"
#include "io.h"
#include "stdio.h"



#define	LOG_PATH	"Log"		//dmp输出到这个目录

CCrashReport g_CrashReport;


//开始dump信息会调用这个函数----实现这个函数，设置错误报告内容
void CCrashReport::OnDumpStart(ICrashDump *pDump)
{
	if (pDump == NULL)
	{
		return;
	}


	if ( (_access(LOG_PATH, 0)) == -1 )
	{
		CreateDirectory(LOG_PATH, 0);
	}

	bool bLogDirExist = ( (_access(LOG_PATH, 0)) != -1 );

	//这里设置生成的log文件和dmp文件的文件名
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szMsg[256];
	memset(szMsg, 0, sizeof(szMsg));

	//嵌入世界ID，服务器ID,进程ID
	//sprintf(szMsg, "%s\\%d_%d_pid%d_%02d-%02d-%02d_%02d-%02d-%02d_T%2d.txt", bLogDirExist ? LOG_PATH : ".",	g_serverCfg.m_worldID,g_serverCfg.m_svrID,GetCurrentProcessId(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, g_serverCfg.m_dwDumpType);
	sprintf(szMsg, "%s\\%d_%d_pid%d_%02d-%02d-%02d_%02d-%02d-%02d_T%2d.txt", bLogDirExist ? LOG_PATH : ".",	1,1,GetCurrentProcessId(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 1);

	pDump->SetLogFileName(szMsg);

	//sprintf(szMsg, "%s\\%d_%d_pid%d_%02d-%02d-%02d_%02d-%02d-%02d_T%2d.dmp", bLogDirExist ? LOG_PATH : ".", g_serverCfg.m_worldID,g_serverCfg.m_svrID,GetCurrentProcessId(),st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, g_serverCfg.m_dwDumpType);
	sprintf(szMsg, "%s\\%d_%d_pid%d_%02d-%02d-%02d_%02d-%02d-%02d_T%2d.dmp", bLogDirExist ? LOG_PATH : ".", 1,1,GetCurrentProcessId(),st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 1);
       pDump->SetDumpFileName(szMsg);
}

//可以用ICrashDump的DumpLn Dump想输出到LogFile的信息
void CCrashReport::OnDump(ICrashDump *pDump, PEXCEPTION_POINTERS pExceptionInfo)
{
	//这里可以用pDum->DumpLn输出一些信息，诸如服务器名之类的
}

//Dump结束会调用此函数，此函数返回值会被当作异常捕获的返回值
LONG CCrashReport::OnDumpFinish(ICrashDump * pDump, PEXCEPTION_POINTERS pExceptionInfo)
{
	return EXCEPTION_CONTINUE_EXECUTION;
}
