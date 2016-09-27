
#ifndef __THINGSERVER_EVENTTASK_H__
#define __THINGSERVER_EVENTTASK_H__

#include "ITask.h"
#include "IEventServer.h"

class TaskPart;
struct IActor;

class EventTask : public ITask,public IEventListener
{
public:
	EventTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);
	virtual ~EventTask();

public:
	virtual void OnEvent(XEventData & EventData);

public:

	virtual bool Create();

	virtual void Release();

	//获取任务ID
	virtual TTaskID  GetTaskID();

	//是否已完成
	virtual bool  IsFinished();

	virtual const STaskCnfg * GetTaskCnfg();

	//获得DB数据
	virtual bool  GetDBContext(void * pBuffer,INT32 & len);

	virtual bool  SetDBContext(const void * pBuffer,INT32  len); 

	//任务复置
	virtual void Reset() ;

	//任务信息放到buffer中,发送给客户端
	virtual bool AddToBuffer(IActor * pActor, OBuffer4k & ob);

	//领取奖励
	virtual bool TakeAward();

	//任务状态有更新
	void NotifyClientUpdateTaskState();

	//任务数据信息放到buffer中
	virtual bool AddDBToBuffer(OBuffer4k & ob);

	//GM命令让任务结束
	virtual bool GMFinishTask();

	//设置是否需要更新到数据库
	virtual void SetNeedUpdate(bool bNeedUpdate);

	//获取是否需要更新到数据库
	virtual bool GetNeedUpdate();

protected:
	//任务奖励得到物品
	void TakeGoods(TGoodsID GoodsID, INT32 nNum, bool bBind = true);

	//完成任务
	void FinishTask();

protected:
	TaskPart *         m_pTaskPart; //该任务所属的TaskPart
	const STaskCnfg *  m_pTaskCnfg;
	INT32              m_nCurCount; //当前计数
	UINT32             m_FinishTime;

	bool			   m_bNeedUpdate;	//是否需要更新到数据库

};





#endif
