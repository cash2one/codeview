#ifndef __THINGSERVER_CDTIMERPART_H__
#define __THINGSERVER_CDTIMERPART_H__

#include "ICDTimerPart.h"
#include "ITimeAxis.h"
#include "DBProtocol.h"
#include <map>
#include <set>

struct IThing;

struct SCDTimeData
{
	UINT32					m_EndTime;					//冷却的结束时间	
	ICDTimerEndHandler *	m_pCDTimeEndHandler;		//返回函数
};

class CDTimerPart : public ICDTimerPart, public ITimerSink
{
public:
	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	virtual void OnTimer(UINT32 timerID);

public:
	//////////////////////////////////////////////////////////////////////////
	//登记冷却的信息(会检测配置表中是否有，没有则不登记)
	//func:CD时间到时的回调函数
	//bMaster:是否是所有角色共用的
	//////////////////////////////////////////////////////////////////////////
	virtual void RegistCDTime(TCDTimerID CDTime_ID, bool bMaster = false, ICDTimerEndHandler * pCDTimeEndHandler = 0);

	//////////////////////////////////////////////////////////////////////////
	//检测指定的CD时间是否已到
	//bMaster:是否要获取主角的冷却时间(有些冷却是所有角色共用的)
	//CD时间已到返回true,CD时间未到返回false
	//////////////////////////////////////////////////////////////////////////
	virtual bool IsCDTimeOK(TCDTimerID CDTime_ID, bool bMaster = false);

	//卸载指定的冷却时间
	virtual void UnLoadCDTime(TCDTimerID CDTime_ID);

	//卸载所有冷却时间
	virtual void UnLoadAllCDTime();

	//得到CD剩余时间
	virtual UINT32 GetRemainCDTime(TCDTimerID CDTime_ID);

private:
	//CD时间是否已到
	bool	__IsCDTimeOK(TCDTimerID CDTime_ID);

	//创建定时器
	bool	__StartTimer(UINT32 TimerID, UINT32 TimeLong);

	//销毁定时器
	void	__StopTimer(UINT32 TimerID);

	//登记数据库获得的冷却
	void	__RegistDBCDTime(const SDB_CDTimerData * pCDTimerData);

private:
	IActor *					m_pActor;

	typedef std::map<TCDTimerID, SCDTimeData>	MAPCDTIMER;

	MAPCDTIMER					m_mapCDTimer;
};


#endif

