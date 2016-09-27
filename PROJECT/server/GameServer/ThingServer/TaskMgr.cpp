
#include "TaskMgr.h"
#include "ITaskPart.h"
#include "ThingServer.h"
#include "IActor.h"
#include "IBasicService.h"
#include "XDateTime.h"
#include "FlushOnlineUserTask.h"

TaskMgr::TaskMgr()
{
	m_FirstTaskID = INVALID_TASK_ID;
	m_LastGuideTaskID = INVALID_TASK_ID;
}

TaskMgr::~TaskMgr()
{
}

	
bool TaskMgr::Create()
{
	//设置定时器,晚上0点0分0秒更新任务
	this->TaskOrderByIndex();

	time_t nCurTime = CURRENT_TIME();

	tm * pTm = localtime(&nCurTime);

	UINT32 LeftTime = XDateTime::SECOND_OF_DAY - (pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec);

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Flush,this,LeftTime * 1000,"TaskMgr::Create");

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Task,this);
}

void TaskMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Task,this);
}


		//收到MSG_ROOT消息
void TaskMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (TaskMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enTaskCmd_Max] = 
	{

		&TaskMgr::OpenTaskPanel,
		NULL,
		NULL,
		NULL,
		&TaskMgr::TakeAward,

	};

	if(nCmd >= ARRAY_SIZE(s_funcProc) || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

void TaskMgr::OnTimer(UINT32 timerID)
{
	if( enTimerID_Flush == timerID){
		//刷新在线玩家任务
		this->FlushTaskOnlineUser();

		static bool bFirst = true;

		if( true == bFirst){
			//第一次要删除旧定时器,重新设置定时器,因为第一次设置的定时器时间是从开服时间到0点剩余时间
			g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Flush, this);

			g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Flush, this, XDateTime::SECOND_OF_DAY * 1000, "TaskMgr::OnTimer");

			bFirst = false;
		}
	}
}

//得到下一个新手引导任务
TTaskID TaskMgr::GetNextGuideTask(std::vector<TTaskID> &vecTask)
{
	if ( vecTask.size() == m_mapTaskIndex.size() )
		//新手引导已全部完成,不做推导
		return INVALID_TASK_ID;

	TTaskID TaskID = INVALID_TASK_ID;
	int index = 0;

	for ( int i = 0; i < vecTask.size(); ++i )
	{
		std::map<TTaskID, int>::iterator iter = m_mapTaskIndex.find(vecTask[i]);

		if ( iter == m_mapTaskIndex.end() )
			continue;

		int TmpIndex = iter->second;

		if ( TmpIndex > index )
		{
			index = TmpIndex;
			TaskID = vecTask[i];
		}
	}

	if ( INVALID_TASK_ID == TaskID )
		return m_FirstTaskID;

	//创建出以TaskID为前提的任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskMail = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Mainline);

	if ( 0 == pmapTaskMail )
		return INVALID_TASK_ID;

	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskMail->begin(); it != pmapTaskMail->end(); ++it )
	{
		const STaskCnfg & TaskCnfg = it->second;

		if ( TaskCnfg.m_bGuide && TaskCnfg.m_PreTaskID == TaskID )
		{
			return TaskCnfg.m_TaskID;
		}
	}

	return INVALID_TASK_ID;
}

//是否是最后一个新手引导任务
bool TaskMgr::IsLastGuideTask(TTaskID TaskID)
{
	if ( TaskID == m_LastGuideTaskID )
		return true;

	return false;
}

//刷新所有在线玩家的任务
void TaskMgr::FlushTaskOnlineUser()
{
	FlushOnlineUserTask FlushTalk;

	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, FlushTalk);
}

	//打开任务栏
void TaskMgr::OpenTaskPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITaskPart * pTaskPart = pActor->GetTaskPart();
	if(pTaskPart == 0){
		return ;
	}

	pTaskPart->OpenTaskPanel();
}


	//领取奖励
void TaskMgr::TakeAward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITaskPart * pTaskPart = pActor->GetTaskPart();
	if(pTaskPart == 0)
	{
		return ;
	}

	CS_TaskTakeAward_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTaskPart->TakeAward(Req.m_TaskID,Req.m_TaskClass);

	//如果是主线任务的话
}

//对新手引导任务做顺序定义
void TaskMgr::TaskOrderByIndex()
{
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskMail = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Mainline);

	if ( 0 == pmapTaskMail )
		return;

	//先得到第一个任务
	TTaskID TaskID = INVALID_TASK_ID;

	const int firstlv = 1;

	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskMail->begin(); it != pmapTaskMail->end(); ++it )
	{
		const STaskCnfg & TaskCnfg = it->second;

		if ( TaskCnfg.m_bGuide && TaskCnfg.m_OpenLevel == firstlv )
		{
			TaskID = TaskCnfg.m_TaskID;
			m_FirstTaskID = TaskCnfg.m_TaskID;
			TRACE("FirstTask:%d", m_FirstTaskID);
			break;
		}
	}

	if ( TaskID == INVALID_TASK_ID )
		return;

	int index = 1;

	m_mapTaskIndex[TaskID] = index++;

	int num = pmapTaskMail->size();

	//最后一个新手引导任务
	m_LastGuideTaskID = TaskID;

	for ( int i = 0; i < num; ++i )
	{
		bool bHave = false;

		for( std::hash_map<TTaskID,STaskCnfg>::const_iterator iter = pmapTaskMail->begin(); iter != pmapTaskMail->end(); ++iter )
		{
			const STaskCnfg & TaskCnfg = iter->second;

			if ( TaskCnfg.m_bGuide && TaskCnfg.m_PreTaskID == TaskID )
			{
				m_mapTaskIndex[TaskCnfg.m_TaskID] = index++;

				TRACE("TaskID:%d,Index:%d", TaskCnfg.m_TaskID,index);

				TaskID = TaskCnfg.m_TaskID;

				m_LastGuideTaskID = TaskCnfg.m_TaskID;

				bHave = true;
				break;
			}
		}

		if ( !bHave )
			break;
	}

	TRACE("LastGuideTaskID:%d",m_LastGuideTaskID);
}
