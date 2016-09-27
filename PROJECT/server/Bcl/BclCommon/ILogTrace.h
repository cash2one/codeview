
#ifndef __BCL_ILOG_TRACE_H__
#define __BCL_ILOG_TRACE_H__

struct ILogTrace
{
	virtual void Release()=0;

	//创建
	virtual bool Create( const char * szFileName) = 0;

	// Trace普通信息(带回车换行功能)
	virtual bool TraceLn(const char * lpszFormat, ...)=0;
	
	virtual void Stop()=0;
};


#endif

