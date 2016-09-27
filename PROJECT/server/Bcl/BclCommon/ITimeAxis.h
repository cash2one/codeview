
#ifndef __BCL_ITIME_AXIS_H__
#define __BCL_ITIME_AXIS_H__

#include "BclHeader.h"

namespace boost
{
	namespace asio
	{
		class io_service;
	};
};



//定时器回调接口
struct BCL_API ITimerSink
{
	virtual void OnTimer(UINT32 timerID) = 0;

	virtual ~ITimerSink();
};


// 时间轴接口
struct ITimeAxis
{
		// 创建时间轴
	virtual bool Create(boost::asio::io_service * pIoService,UINT32 shortTimerSlotCount=1024, UINT32 dwGridElapse=20)=0;
	
	// 时间轴检查处理
	virtual void OnCheck(void) = 0;

	// 设置定时器, 注意szTimerDesc必须指向常量字符串
	virtual bool SetTimer(UINT32 timerID, ITimerSink *pTimerSink,UINT32 dwElapse, const char *szTimerDesc=0) = 0;

	// 销毁定时器
	virtual void KillTimer(UINT32 timerID, ITimerSink *pTimerSink) = 0;

	//销毁所有的定时器
	virtual void KillAllTimer(ITimerSink* pSink) = 0;

	//停止
	virtual void Stop() = 0;
};



#endif

