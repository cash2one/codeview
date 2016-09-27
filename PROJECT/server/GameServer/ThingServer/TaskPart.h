
#ifndef __THINGSERVER_TASKPART_H__
#define __THINGSERVER_TASKPART_H__

#include "ITaskPart.h"
#include <hash_map>
#include <bitset>
#include "DBProtocol.h"
#include "IEventServer.h"

struct IActor;
struct ITask;
struct STaskCnfg;

class TaskPart : public ITaskPart, public IEventListener
{
public:
	TaskPart();
	virtual ~TaskPart();

public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen) ;

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

	//任务状态有更新
	void NotifyClientUpdateTaskState(ITask * pTask);

	//增加了任务
	void NotifyClientAddTask(ITask * pTask);

	//删除任务
	void NotifyClientDelTask(ITask * pTask);

	virtual void OnEvent(XEventData & EventData);

public:
	//打开任务栏
	virtual void OpenTaskPanel(bool bChangeStatus = true);

	//领取奖励
	virtual void TakeAward(TTaskID  TaskID,enTaskClass  TaskClass);
	
	//刷新任务
	virtual void FlushTask();

	//删除任务
	virtual bool DeleteTask(ITask * pTask,bool bNoticeClient = true);

	//任务完成,设置任务完成记录
	virtual void RecordFinished(TTaskID TaskID);

	//GM命令创建任务
	virtual ITask * GMCreateTask(TTaskID TaskID);

	//GM命令完成任务
	virtual bool	GMFinishTask(TTaskID TaskID);

	//任务完成后,创建引发的任务
	virtual bool	TaskFinish_CreateTask(TTaskID TaskID);

	//玩家升级后,创建引发的任务
	virtual bool	UpLevel_CreateTask(UINT8 Level);

	//通知客户端改变主界面任务状态图标
	virtual void	NoticeChengTaskStatus(enTaskStatus TaskStatus, bool bInit = false);

	//是否在新手引导中
	virtual bool	IsInGuide();

	//获得日常任务总数
	virtual UINT8 GetDailyTaskNum();

	//获得已完成的日常任务数
	virtual UINT8 GetDailyTaskNotOkNum();

	//结束新手引导要做的事
	virtual void  GuideOverDoSomething();

private:
	ITask * CreateTask(DB_TaskData * pTaskData);

	ITask * CreateTask(TTaskID TaskID);

	//刷新日常任务
	void	__FlushTask_Daily();

	//刷新帮派任务
	void	__FlushTask_Syn();

	//上线时检测刷新主线任务
	void    __Check_FlushTask_Main();

	//上线时检测刷新日常任务
	void	__Check_FlushTask_Daily();

	//上线时检测刷新帮派任务
	void	__Check_FlushTask_Syn();

	ITask * CreateTaskWithConfig(const STaskCnfg *  pTaskCnfg);

	//清除帮派任务
	void	__ClearTask_Syn();

	//设置日常任务总数
	void SetDailyTaskNum();

private:
    IActor *				 m_pActor;

	TBitArray<MAX_TASK_ID>   m_TaskHistory;	//完成过的任务记录

	typedef std::hash_map<TTaskID,ITask*>  MAP_TASK;

	MAP_TASK				 m_TaskList[enTaskClass_Max];

	UINT32					 m_LastUpdateTime; //最后更新任务时间

	bool					 m_bClientOpen;  //客户端是否打开过任务栏

	enTaskStatus			 m_TaskStatus;	 //任务主界面显示状态

	typedef std::hash_map<TTaskID, UINT8/*TaskClass*/> MAP_DELETETASK;

	MAP_DELETETASK			 m_mapDeleteTask;	//保存数据时,需要删除的任务

	int						 m_DailyTaskNum;	//日常任务总数

	int						 m_DailyTaskNotOkNum;  //未完成的日常任务数

};






#endif
