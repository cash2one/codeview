
#ifndef __THINGSERVER_TASKMGR_H__
#define __THINGSERVER_TASKMGR_H__


#include "IMessageDispatch.h"
#include "ITimeAxis.h"
#include <map>
#include <vector>
#include "DSystem.h"

class TaskMgr : public IMsgRootDispatchSink, public ITimerSink
{
	enum enTimerID
	{
		enTimerID_Flush,		//刷新任务定时器
	};

public:
    TaskMgr();
	~TaskMgr();

	
	bool Create();

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	virtual void OnTimer(UINT32 timerID);

	//得到下一个新手引导任务
	TTaskID GetNextGuideTask(std::vector<TTaskID> &vecTask);

	//是否是最后一个新手引导任务
	bool IsLastGuideTask(TTaskID TaskID);

private:
	//刷新所有在线玩家的任务
	void FlushTaskOnlineUser();

private:
	//打开任务栏
	void OpenTaskPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//领取奖励
	void TakeAward(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//对新手引导任务做顺序定义
	void TaskOrderByIndex();
	
private:
	std::map<TTaskID, int> m_mapTaskIndex;

	TTaskID					 m_FirstTaskID;
	TTaskID					 m_LastGuideTaskID;	//最后一个新手引导任务
};


#endif
