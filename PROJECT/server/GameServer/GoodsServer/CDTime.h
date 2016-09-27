#ifndef __GOODSSERVER_CDTIME_H__
#define __GOODSSERVER_CDTIME_H__

#include "ICDTime.h"
#include <map>
#include "ITimeAxis.h"

struct SCDTimeInfo
{
	UINT32				m_EndTime;					//冷却的结束时间	
	ICDTimeEndHandler *	m_pCDTimeEndHandler;		//返回函数
};

class CDTimeMgr : public ICDTimeMgr, public ITimerSink
{
	//定时器ID
	enum enumCDTimeTimerID
	{
		enCDTimeTimerID,
	};
public:
	CDTimeMgr();
	virtual ~CDTimeMgr();
	bool Create();
	void Close();

public:
	//////////////////////////////////////////////////////////////////////////
	//登记冷却的信息(会检测配置表中是否有，没有则不登记)
	//func:CD时间到时的回调函数
	//////////////////////////////////////////////////////////////////////////
	virtual void RegistCDTime(IActor * pActor, TCDTimerID CDTime_ID, ICDTimeEndHandler * pCDTimeEndHandler = 0);

	//////////////////////////////////////////////////////////////////////////
	//检测指定的CD时间是否已到
	//func:CD时间到时的回调函数
	//CD时间已到返回true,CD时间未到返回false
	//////////////////////////////////////////////////////////////////////////
	virtual bool IsCDTimeOK(IActor * pActor, TCDTimerID CDTime_ID);

	//卸载这个玩家身上的所有冷却时间
	virtual void UnLoadCDTime(IActor * pActor);

	//卸载这个玩家身上指定的冷却时间
	virtual void UnLoadCDTime(IActor * pActor, TCDTimerID CDTime_ID);

	virtual void OnTimer(UINT32 timerID);

private:
	typedef std::map<TCDTimerID, SCDTimeInfo>	MAPCDTIME;

	std::map<UID, MAPCDTIME>							m_mapCDTime;
};

#endif
