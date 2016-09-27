
#ifndef __THINGSERVER_ITASKPART_H__
#define __THINGSERVER_ITASKPART_H__


#include "IThingPart.h"
#include "GameSrvProtocol.h"
struct ITask;

struct ITaskPart : public IThingPart
{
	//打开任务栏
	virtual void OpenTaskPanel(bool bChangeStatus = true) = 0;

	//领取奖励
	virtual void TakeAward(TTaskID  TaskID,enTaskClass  TaskClass) = 0;

	//刷新任务
	virtual void FlushTask() = 0;

	//GM命令创建任务
	virtual ITask * GMCreateTask(TTaskID TaskID) = 0;

	//GM命令完成任务
	virtual bool	GMFinishTask(TTaskID TaskID) = 0;

	//任务完成后,创建引发的任务
	virtual bool	TaskFinish_CreateTask(TTaskID TaskID) = 0;

	//玩家升级后,创建引发的任务
	virtual bool	UpLevel_CreateTask(UINT8 Level) = 0;

	//通知客户端改变主界面任务状态图标
	virtual void	NoticeChengTaskStatus(enTaskStatus TaskStatus, bool bInit = false) = 0;

	//是否在新手引导中
	virtual bool	IsInGuide() = 0;

	//获得日常任务总数
	virtual UINT8 GetDailyTaskNum() = 0;

	//获得已完成的日常任务数
	virtual UINT8 GetDailyTaskNotOkNum() = 0;

	
};




#endif

