
#include "IActor.h"
#include "TaskPart.h"
#include "XDateTime.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IConfigServer.h"
#include "EventTask.h"
#include "DBProtocol.h"
#include <string>
#include <vector>
#include "DMsgSubAction.h"
#include "GoodsNumTask.h"
#include "ContributionTask.h"
#include "RandomService.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "AddSynTask.h"
#include "GetSynWarScoreTask.h"
#include "EmployeeNumTask.h"
#include "IActivityPart.h"

TaskPart::TaskPart()
{
	m_bClientOpen = false;
	m_LastUpdateTime = 0;
	m_TaskStatus  = enTaskStatus_Common;
	m_DailyTaskNum = 0;
	m_DailyTaskNotOkNum = 0;

}

TaskPart::~TaskPart()
{
}



//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool TaskPart::Create(IThing *pMaster, void *pContext, int nLen) 
{
	m_pActor = (IActor*)pMaster;

	if(pContext==0 || nLen < SIZE_OF(DB_TaskPartData()))
	{
		return false;
	}

	STaskPart * pTaskPart = (STaskPart *)pContext;

	DB_TaskPartData * pTaskPartData = (DB_TaskPartData*)pTaskPart->m_pData;

	//初始化m_TaskHistory
	m_TaskHistory.set((unsigned char*)pTaskPartData->m_TaskHistory,SIZE_OF(pTaskPartData->m_TaskHistory));

	m_LastUpdateTime = pTaskPartData->m_LastUpdateTime;

	m_TaskStatus	 = (enTaskStatus)pTaskPartData->m_TaskStatus;

	if(m_LastUpdateTime == 0){
		m_LastUpdateTime = CURRENT_TIME();
	}
	
	DB_NoSaveTask * pNoSaveTask = (DB_NoSaveTask *)&pTaskPartData->m_NoSaveTask;

	for( int i = 0; i < pTaskPartData->m_NoSaveNum; ++i,++pNoSaveTask)
	{
		ITask * pTask = CreateTask(pNoSaveTask->m_TaskID);
		if(pTask == 0){
			TRACE("<warning> %s : %d line 创建任务失败 taskid=%d",__FUNCTION__,__LINE__,pNoSaveTask->m_TaskID);
			continue;
		}		
	}

	if( (pTaskPart->m_Len - sizeof(DB_TaskPartData)) / sizeof(DB_TaskData) >= 1){

		DB_TaskData * pTaskData = (DB_TaskData *)(pTaskPartData + 1);

		//创建Task
		int count = (nLen-SIZE_OF(*pTaskPartData))/SIZE_OF(*pTaskData);

		for(int i=0; i<count; i++,pTaskData++)
		{
			if(pTaskData->m_TaskID == 0){
				break;
			}
			ITask * pTask = CreateTask(pTaskData);
			if(pTask == 0){
				TRACE("<warning> %s : %d line 创建任务失败 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
				continue;
			}
		}	
	}

	//判断当天是否已刷新过任务
	if( !XDateTime::GetInstance().IsSameDay(m_LastUpdateTime,CURRENT_TIME()))
	{
		this->FlushTask();
	}

	////注册升级事件,引发任务
	TMsgID msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_UpLevel);

	m_pActor->SubscribeEvent(msgID,this,"TaskPart::Create");

	//注册加入帮派事件，引发任务
	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_AddSyn);

	m_pActor->SubscribeEvent(msgID,this,"TaskPart::Create");

	//注册退帮事件，删除帮派任务
	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_QuitSyn);

	m_pActor->SubscribeEvent(msgID,this,"TaskPart::Create");

	

	//this->__Check_FlushTask_Daily();

	//this->__Check_FlushTask_Syn();

	if ( pTaskPart->m_bNewUser )
	{
		//新建角色创建第一个引导任务
		this->UpLevel_CreateTask(m_pActor->GetCrtProp(enCrtProp_Level));

		//this->SaveData();
	}
	this->__Check_FlushTask_Main();

	return true;
}

ITask * TaskPart::CreateTaskWithConfig(const STaskCnfg *  pTaskCnfg)
{
	ITask * pTask = 0;
	
	switch(pTaskCnfg->m_TaskType)
	{
	case enTaskType_Event:
		{
			pTask = new EventTask(this,pTaskCnfg);
		}
		break;
	case enTaskType_SynGoods:
		{
			pTask = new GoodsNumTask(this, pTaskCnfg);
		}
		break;
	case enTaskType_Contribution:
		{
			pTask = new ContributionTask(this, pTaskCnfg);
		}
		break;
	case enTaskType_AddSyn:
		{
			pTask = new AddSynTask(this, pTaskCnfg);
		}
		break;
	case enTaskType_GetSynWarScore:
		{
			pTask = new GetSynWarScoreTask(this, pTaskCnfg);
		}
		break;
	case enTaskType_Employee:
		{
			pTask = new EmployeeNumTask(this, pTaskCnfg);
		}
		break;
	default:
		TRACE("<warning> %s : %d line 创建任务失败 错误的任务类型 taskid=%d tasktype=%d",__FUNCTION__,__LINE__,pTaskCnfg->m_TaskID,pTaskCnfg->m_TaskType);
		break;
	}

	return pTask;
}
ITask * TaskPart::CreateTask(TTaskID TaskID)
{
	const STaskCnfg *  pTaskCnfg = g_pGameServer->GetConfigServer()->GetTaskCnfg(TaskID);
	if(pTaskCnfg == 0)
	{
		TRACE("<warning> %s : %d line 创建任务失败 找不到任务配置信息 taskid=%d",__FUNCTION__,__LINE__,TaskID);
		return 0;
	}

	ITask * pTask = CreateTaskWithConfig(pTaskCnfg);

	if ( 0 == pTask){
		
		TRACE("<error> %s : %d Line 配置文件创建任务失败！！任务ID = %d", __FUNCTION__, __LINE__, pTaskCnfg->m_TaskID);
		return 0;
	}

	if( !pTask->Create()){
		delete pTask;
		return 0;
	}

	if( 0 != pTask){
		this->NotifyClientAddTask(pTask);

		MAP_TASK & mapTask = m_TaskList[pTaskCnfg->m_TaskClass];
		mapTask[TaskID] = pTask;

		//通知有新任务
		this->NoticeChengTaskStatus(enTaskStatus_NewTask);

		SDB_UpdateTaskReq DBReq;

		DBReq.m_UserUID = m_pActor->GetUID().ToUint64();

		int nLen = sizeof(DBReq);

		pTask->GetDBContext(&DBReq, nLen);

		OBuffer1k ob;
		ob << DBReq;
		//g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTask,ob.TakeOsb(),0,0);
	}

	return pTask;
}

ITask * TaskPart::CreateTask(DB_TaskData * pTaskData)
{
	const STaskCnfg *  pTaskCnfg = g_pGameServer->GetConfigServer()->GetTaskCnfg(pTaskData->m_TaskID);
	if(pTaskCnfg == 0)
	{
		TRACE("<warning> %s : %d line 创建任务失败 找不到任务配置信息 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
		return 0;
	}

	ITask * pTask = CreateTaskWithConfig(pTaskCnfg);
	if(pTask == 0)
	{
		TRACE("<warning> %s : %d line 创建任务失败 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
		TRACE("<warning> %s : %d line 创建任务失败 找不到任务配置信息 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
		return 0;
	}

	if(pTask->SetDBContext(pTaskData,SIZE_OF(*pTaskData))==false)
	{
		TRACE("<warning> %s : %d line 设置数据失败 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
		delete pTask;
		pTask = 0;
		return 0;
	}

	//不需要保存到数据库
	pTask->SetNeedUpdate(false);

	if(pTask->Create()==false)
	{
		TRACE("<warning> %s : %d line 创建任务失败 taskid=%d",__FUNCTION__,__LINE__,pTaskData->m_TaskID);
		delete pTask;
		pTask = 0;
		return 0;
	}

	if( 0 != pTask){
		this->NotifyClientAddTask(pTask);

		MAP_TASK & mapTask = m_TaskList[pTaskCnfg->m_TaskClass];
		mapTask[pTaskData->m_TaskID] = pTask;
	}


	return pTask;
}

//释放
void TaskPart::Release(void)
{
	//删除任务内存
	for( int i = (int)enTaskClass_Mainline; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTask = m_TaskList[(enTaskClass)i];

		for(MAP_TASK::iterator iter = mapTask.begin(); iter != mapTask.end(); ++iter)
		{
			iter->second->Release();
		}
	}
	delete this;
}

//取得部件ID
enThingPart TaskPart::GetPartID(void)
{
	return enThingPart_Actor_Task;
}

//取得本身生物
IThing*		TaskPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool TaskPart::OnGetDBContext(void * buf, int &nLen)
{
	if( 0 == buf || nLen < sizeof(SDB_Update_TaskPartInfo_Req)){
		return false;
	}
	SDB_Update_TaskPartInfo_Req * pTaskPartData = (SDB_Update_TaskPartInfo_Req *)buf;

	pTaskPartData->m_LastUpdateTime = m_LastUpdateTime;

	memcpy(pTaskPartData->m_TaskHistory, &m_TaskHistory, sizeof(pTaskPartData->m_TaskHistory));

	pTaskPartData->m_UserUID = m_pActor->GetUID().ToUint64();

	pTaskPartData->m_NotSaveNum = 0;
//	DB_NoSaveTask * pNoSaveTask = (DB_NoSaveTask *)&pTaskPartData->m_NotSaveTaskID;

	OBuffer2k ob;
	for( int i = (int)enTaskClass_Mainline; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTaskMain = m_TaskList[(enTaskClass)i];
		MAP_TASK::iterator iter = mapTaskMain.begin();
		for(; iter != mapTaskMain.end(); ++iter)
		{
			ITask * pTask = (iter->second);
			if( 0 == pTask){
				continue;
			}

			const STaskCnfg * pTaskCnfg = pTask->GetTaskCnfg();
			if( 0 == pTaskCnfg){
				continue;
			}

			if( !pTaskCnfg->m_bSaveToDB){
				++pTaskPartData->m_NotSaveNum;
				ob.Push(&iter->first, sizeof(TTaskID));
//				pNoSaveTask->m_TaskID = iter->first;
			}
		}
	}

	nLen = sizeof(SDB_Update_TaskPartInfo_Req);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void TaskPart::InitPrivateClient()
{
	m_bClientOpen = false;
	SetDailyTaskNum();
	OpenTaskPanel(false);
	this->NoticeChengTaskStatus(m_TaskStatus,true);
}


//玩家下线了，需要关闭该ThingPart
void TaskPart::Close()
{
}

//保存数据
void TaskPart::SaveData()
{
	//保存PART数据
	SDB_Update_TaskPartInfo_Req TaskPartDataReq;
	
	TaskPartDataReq.m_LastUpdateTime = m_LastUpdateTime;

	memcpy(&TaskPartDataReq.m_TaskHistory, &m_TaskHistory, sizeof(TaskPartDataReq.m_TaskHistory));

	TaskPartDataReq.m_UserUID = m_pActor->GetUID().ToUint64();

	TaskPartDataReq.m_TaskStatus = (UINT8)m_TaskStatus;

	TaskPartDataReq.m_NotSaveNum = 0;

	OBuffer1k ob;
	for( int i = (int)enTaskClass_Mainline; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTaskMain = m_TaskList[(enTaskClass)i];
		MAP_TASK::iterator iter = mapTaskMain.begin();
		for(; iter != mapTaskMain.end(); ++iter)
		{
			ITask * pTask = (iter->second);
			if( 0 == pTask){
				continue;
			}

			const STaskCnfg * pTaskCnfg = pTask->GetTaskCnfg();
			if( 0 == pTaskCnfg){
				continue;
			}

			if( !pTaskCnfg->m_bSaveToDB){
				++TaskPartDataReq.m_NotSaveNum;
				ob.Push(&iter->first, sizeof(TTaskID));
			}
		}
	}

	TaskPartDataReq.m_VarBinary.m_size = ob.Size();

	OBuffer1k ob1;
	ob1 << TaskPartDataReq << ob;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTaskPartInfo,ob1.TakeOsb(),0,0);

	//保存所有任务数据
	//SDB_Update_TaskInfo_Req Update_TaskInfo_Req;
	//Update_TaskInfo_Req.m_TaskNum = 0;
	//Update_TaskInfo_Req.m_UserUID = m_pActor->GetUID().ToUint64();

	//OBuffer4k ob2;
	/*OBuffer1k ob2;

	for( int i = 0; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTask = m_TaskList[(enTaskClass)i];

		for( MAP_TASK::iterator iter = mapTask.begin(); iter != mapTask.end(); ++iter)
		{
			ITask * pTask = iter->second;
			if( 0 == pTask){
				continue;
			}

			SDB_UpdateTaskReq DBReq;

			DBReq.m_UserUID = m_pActor->GetUID().ToUint64();

			int nLen = sizeof(DBReq);

			pTask->GetDBContext(&DBReq, nLen);

			ob2.Reset();
			ob2 << DBReq;
			g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTask,ob2.TakeOsb(),0,0);
		}
	}*/

	//保存所有任务数据
	SDB_Update_TaskInfo_Req Update_TaskInfo_Req;
	Update_TaskInfo_Req.m_TaskNum = 0;
	Update_TaskInfo_Req.m_UserUID = m_pActor->GetUID().ToUint64();

	OBuffer4k ob2;

	for( int i = 0; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTask = m_TaskList[(enTaskClass)i];

		for( MAP_TASK::iterator iter = mapTask.begin(); iter != mapTask.end(); ++iter)
		{
			ITask * pTask = iter->second;
			if( 0 == pTask){
				continue;
			}

			pTask->AddDBToBuffer(ob2);

			++Update_TaskInfo_Req.m_TaskNum;
		}
	}

	Update_TaskInfo_Req.m_VarBinary.m_size = ob2.Size();

	OBuffer4k ob3;
	ob3 << Update_TaskInfo_Req << ob2;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTaskInfo,ob3.TakeOsb(),0,0);


	//Update_TaskInfo_Req.m_VarBinary.m_size = ob2.Size();

	//OBuffer4k ob3;
	//ob3 << Update_TaskInfo_Req << ob2;
	//g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTaskInfo,ob3.TakeOsb(),0,0);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//new
}

//打开任务栏
void TaskPart::OpenTaskPanel(bool bChangeStatus)
{
	if( m_TaskStatus != enTaskStatus_Common)
	{
		if (bChangeStatus)
			this->NoticeChengTaskStatus(enTaskStatus_Common);
		else
			this->NoticeChengTaskStatus(m_TaskStatus);
	}

	if( m_bClientOpen){
		return;
	}

	SC_TaskOpenTask_Rsp Rsp;
	Rsp.m_TaskNum = 0;

	OBuffer4k ob;

	for( int i = 0; i < (int)enTaskClass_Max; ++i)
	{
		MAP_TASK & mapTask = m_TaskList[(enTaskClass)i];

		for( MAP_TASK::iterator iter = mapTask.begin(); iter != mapTask.end(); ++iter)
		{
			ITask * pTask = iter->second;
			if( 0 == pTask){
				continue;
			}

			pTask->AddToBuffer(m_pActor, ob);
			if( ob.Error()){
				return;
			}

			++Rsp.m_TaskNum;
		}	
	}

	OBuffer4k ob2;
	ob2 << TaskHeader(enTaskCmd_CS_OpenTask, sizeof(Rsp) + ob.Size()) << Rsp << ob;
	m_pActor->SendData(ob2.TakeOsb());

	//打开过任务栏
	m_bClientOpen = true;
}


//领取奖励
void TaskPart::TakeAward(TTaskID  TaskID,enTaskClass  TaskClass)
{
	if( TaskClass >= enTaskClass_Max){
		return;
	}

	MAP_TASK & mapTask = m_TaskList[TaskClass];

	MAP_TASK::iterator iter = mapTask.find(TaskID);
	if( iter == mapTask.end()){
		return;
	}

	ITask * pTask = iter->second;
	if( 0 == pTask){
		return;
	}

	if( pTask->TakeAward()){
		this->DeleteTask(pTask,false);
	}
}

//刷新任务
void TaskPart::FlushTask()
{
	m_LastUpdateTime = CURRENT_TIME();

	//刷新日常任务
	this->__FlushTask_Daily();

	//刷新帮派任务
	this->__FlushTask_Syn();
}

//删除任务
bool TaskPart::DeleteTask(ITask * pTask,bool bNoticeClient)
{
	//从数据库中删除
	//SDB_DeleteTaskReq DBReq;

	//DBReq.m_uidUser = m_pActor->GetUID().ToUint64();
	//DBReq.m_TaskID = pTask->GetTaskID();

	//OBuffer1k ob;
	//ob << DBReq;
	//g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteTask,ob.TakeOsb(),0,0);



	MAP_TASK & mapTask = m_TaskList[pTask->GetTaskCnfg()->m_TaskClass];

	MAP_TASK::iterator iter = mapTask.find(pTask->GetTaskID());

	if( iter == mapTask.end()){
		return false;
	}

	if ( bNoticeClient )
	{
		this->NotifyClientDelTask(pTask);
	}

	mapTask.erase(iter);

	pTask->Release();
	return true;
}

//任务完成,设置任务完成记录
void TaskPart::RecordFinished(TTaskID TaskID)
{
	if( TaskID > MAX_TASK_ID){
		return;
	}

	m_TaskHistory.set(TaskID, true);

}

//GM命令创建任务
ITask * TaskPart::GMCreateTask(TTaskID TaskID)
{
	const STaskCnfg *  pTaskCnfg = g_pGameServer->GetConfigServer()->GetTaskCnfg(TaskID);
	if(pTaskCnfg == 0){
		TRACE("<warning> %s : %d line 创建任务失败 找不到任务配置信息 taskid=%d",__FUNCTION__,__LINE__,TaskID);
		return 0;
	}

	MAP_TASK & mapTask = m_TaskList[pTaskCnfg->m_TaskClass];
	
	MAP_TASK::iterator iter = mapTask.find(TaskID);
	if( iter != mapTask.end()){
		return 0;
	}

	this->NoticeChengTaskStatus(enTaskStatus_NewTask);

	return this->CreateTask(TaskID);
}

//GM命令完成任务
bool	TaskPart::GMFinishTask(TTaskID TaskID)
{
	const STaskCnfg *  pTaskCnfg = g_pGameServer->GetConfigServer()->GetTaskCnfg(TaskID);
	if(pTaskCnfg == 0){
		TRACE("<warning> %s : %d line 创建任务失败 找不到任务配置信息 taskid=%d",__FUNCTION__,__LINE__,TaskID);
		return false;
	}

	MAP_TASK & mapTask = m_TaskList[pTaskCnfg->m_TaskClass];
	
	MAP_TASK::iterator iter = mapTask.find(TaskID);
	if( iter == mapTask.end()){
		return false;
	}

	ITask * pTask = iter->second;
	if( 0 == pTask){
		return false;
	}
	
	return pTask->GMFinishTask();
}

//任务状态有更新
void TaskPart::NotifyClientUpdateTaskState(ITask * pTask)
{
	if(m_bClientOpen == false){
		return;
	}

	pTask->NotifyClientUpdateTaskState();
}


//增加了任务
void TaskPart::NotifyClientAddTask(ITask * pTask)
{
	if(m_bClientOpen == false){
		return;
	}

	OBuffer4k ob;

	pTask->AddToBuffer(m_pActor, ob);

	OBuffer4k ob2;
	ob2 << TaskHeader(enTaskCmd_SC_AddTask, ob.Size()) << ob;
	m_pActor->SendData(ob2.TakeOsb());
}

//删除任务
void TaskPart::NotifyClientDelTask(ITask * pTask)
{
	if(m_bClientOpen == false){
		return;
	}

	SC_TaskDeleteTask DeleteTask;
	DeleteTask.m_TaskID = pTask->GetTaskID();

	OBuffer1k ob;
	ob << TaskHeader(enTaskCmd_SC_DelTask, sizeof(DeleteTask)) << DeleteTask;
	m_pActor->SendData(ob.TakeOsb());
}

void TaskPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_UpLevel);
	if( EventData.m_MsgID == msgID){
		//升级事件
		SS_UpLevel * pUpLevel = (SS_UpLevel *)EventData.m_pContext;
		if( 0 == pUpLevel){
			return;
		}

		//创建引发任务
		this->UpLevel_CreateTask(pUpLevel->m_Level);
		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_AddSyn);
	if( EventData.m_MsgID == msgID){
		//加帮派事件
		this->__Check_FlushTask_Syn();
		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_QuitSyn);
	if( EventData.m_MsgID == msgID){
		//退帮派 事件
		this->__ClearTask_Syn();
		return;
	}
}

//任务完成后,创建引发的任务
bool	TaskPart::TaskFinish_CreateTask(TTaskID TaskID)
{
	for( int i = (int)enTaskClass_Mainline; i < enTaskClass_Max; ++i)
	{
		const std::hash_map<TTaskID,STaskCnfg> * pmapTask = g_pGameServer->GetConfigServer()->GetTaskList((enTaskClass)i);
		if( 0 == pmapTask){
			return false;
		}	

		for(std::hash_map<TTaskID,STaskCnfg>::const_iterator iter = pmapTask->begin(); iter != pmapTask->end(); ++iter)
		{
			const STaskCnfg & TaskCnfg = iter->second;

			if( TaskCnfg.m_PreTaskID != TaskID){
				continue;
			}

			MAP_TASK & mapTask = m_TaskList[i];

			MAP_TASK::iterator it = mapTask.find(TaskCnfg.m_TaskID);
			if( it != mapTask.end()){
				continue;
			}

			switch((enTaskClass)i)
			{
			case enTaskClass_Mainline:
				{	
					//创建下个主线任务
					this->CreateTask(TaskCnfg.m_TaskID);

				}
				break;
			case enTaskClass_Daily:
				{
					//日常任务和帮派任务如果已触发过刚不再触发,每日刷新既可
					if( m_TaskHistory.get(TaskCnfg.m_TaskID)){
						continue;
					}
					m_DailyTaskNum ++ ;
					this->CreateTask(TaskCnfg.m_TaskID);

				}
				break;
			case enTaskClass_Gang:
				{
					if ( TaskCnfg.m_TaskType == enTaskType_SynGoods )
						break;

					//日常任务和帮派任务如果已触发过刚不再触发,每日刷新既可
					if( m_TaskHistory.get(TaskCnfg.m_TaskID)){
						continue;
					}

					this->CreateTask(TaskCnfg.m_TaskID);
				}
				break;
			default:
				{
					TRACE("<warning> %s : %d 行, 任务分类错误, %d", __FUNCTION__, __LINE__, i);
				}
				break;
			}
		}
	}

	return true;
}

//玩家升级后,创建引发的任务
bool	TaskPart::UpLevel_CreateTask(UINT8 Level)
{
	for( int i = (int)enTaskClass_Mainline; i < enTaskClass_Max; ++i)
	{
		const std::hash_map<TTaskID,STaskCnfg> * pmapTask = g_pGameServer->GetConfigServer()->GetTaskList((enTaskClass)i);
		if( 0 == pmapTask){
			return false;
		}

		MAP_TASK & mapTask = m_TaskList[i];

		for(std::hash_map<TTaskID,STaskCnfg>::const_iterator iter = pmapTask->begin(); iter != pmapTask->end(); ++iter)
		{
			const STaskCnfg & TaskCnfg = iter->second;
			if( TaskCnfg.m_OpenLevel == 0 || TaskCnfg.m_OpenLevel > Level){
				continue;
			}

			if( i >= (int)enTaskClass_Max){
				TRACE("<warning> %s : %d 行, 任务分类错误, %d", __FUNCTION__, __LINE__, i);	
				continue;
			}

			MAP_TASK::iterator it = mapTask.find(TaskCnfg.m_TaskID);
			if (it != mapTask.end()){
				continue;
			}

			if (m_TaskHistory.get(TaskCnfg.m_TaskID)){
				continue;
			}

			if (i == enTaskClass_Gang){
				
				if (TaskCnfg.m_TaskType == enTaskType_SynGoods){
					continue;
				}

				if (m_pActor->GetSyndicate() == 0){
					continue;
				}
			}

			if (i == enTaskClass_Daily){
				m_DailyTaskNum ++ ;
			}
			this->CreateTask(TaskCnfg.m_TaskID);
		}
	}

	return true;
}

//刷新日常任务
void	TaskPart::__FlushTask_Daily()
{
	MAP_TASK & mapTaskDaily = m_TaskList[enTaskClass_Daily];

	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);

	//日常任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskDaily = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Daily);


	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskDaily->begin(); it != pmapTaskDaily->end(); ++it)
	{
		const STaskCnfg & TaskCnfg = it->second;

		//设置成未完成
		m_TaskHistory.set(TaskCnfg.m_TaskID, false);

		bool bCreate = false;
		if( TaskCnfg.m_PreTaskID != 0){
			//前提任务触发的,只触发前提任务完成过的
			if( m_TaskHistory.get(TaskCnfg.m_PreTaskID)){
				bCreate = true;
			}		
		}

		if( TaskCnfg.m_OpenLevel != 0 && TaskCnfg.m_OpenLevel <= Level){
			//等级触发的,比触发等级低的都触发
			bCreate = true;			
		}

		if( bCreate){
			MAP_TASK::iterator iter = mapTaskDaily.find(it->first);
			if( iter == mapTaskDaily.end()){
				this->CreateTask(it->first);
				
				continue;
			}

			ITask * pTask = iter->second;
			if( 0 == pTask){
				continue;
			}

			if( !pTask->IsFinished()){
				pTask->Reset();
			}	
		}
	}
}

//刷新帮派任务
void	TaskPart::__FlushTask_Syn()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	if( pSynMgr->GetSyndicate(m_pActor->GetUID()) == 0){
		return;
	}

	MAP_TASK & mapTaskSyn = m_TaskList[enTaskClass_Gang];

	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);

	int nCountSynGoods = 0; //帮派物品任务个数

	std::vector<TTaskID> vectSynGoods;	//可抽取的帮派物品任务

	//帮派任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskDaily = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Gang);


	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskDaily->begin(); it != pmapTaskDaily->end(); ++it)
	{
		const STaskCnfg & TaskCnfg = it->second;

		//设置成未完成
		m_TaskHistory.set(TaskCnfg.m_TaskID, false);

		bool bCreate = false;
		if( TaskCnfg.m_PreTaskID != 0){
			//前提任务触发的,只触发前提任务完成过的
			if( m_TaskHistory.get(TaskCnfg.m_PreTaskID)){
				bCreate = true;
			}		
		}

		if( TaskCnfg.m_OpenLevel <= Level){
			//等级触发的,比触发等级低的都触发
			bCreate = true;			
		}

		if( bCreate){
			MAP_TASK::iterator iter = mapTaskSyn.find(it->first);
			if( iter == mapTaskSyn.end()){
				if( (it->second).m_TaskType != enTaskType_SynGoods){
					this->CreateTask(it->first);

				}else{
					vectSynGoods.push_back(TaskCnfg.m_TaskID);
				}
				continue;
			}

			ITask * pTask = iter->second;
			if( 0 == pTask){
				continue;
			}

			if( !pTask->IsFinished()){
				if( (it->second).m_TaskType == enTaskType_SynGoods){
					this->DeleteTask(pTask);
					vectSynGoods.push_back(TaskCnfg.m_TaskID);
				}else{
					pTask->Reset();

				}
				continue;
			}else{
				if( (it->second).m_TaskType == enTaskType_SynGoods){
					++nCountSynGoods;
				}
			}
		}
	}

	int nTotalRandom = 0;
	for( int i = 0; i < vectSynGoods.size(); ++i)
	{
		const SSynGoodsTask * pSynGoodsTask = g_pGameServer->GetConfigServer()->GetSynGoodsTask(vectSynGoods[i]);
		if( 0 == pSynGoodsTask){
			TRACE("<error> %s : %d 行, 帮派物品任务配置文件获取失败, 任务ID = %d", __FUNCTION__, __LINE__, vectSynGoods[i]);
			continue;
		}

		nTotalRandom += pSynGoodsTask->m_RandomNum;
	}

	//要抽取的帮派物品任务数量
	UINT8 SynGoodTaskNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SynGoodsTaskNum;

	for( int i = 0; i < vectSynGoods.size() && nCountSynGoods < SynGoodTaskNum; ++i)
	{
		INT32 nRandom = RandomService::GetRandom() % nTotalRandom;

		INT32 nTempRandom = 0;
		for( int k = 0; k < vectSynGoods.size(); ++k)
		{
			const SSynGoodsTask * pSynGoodsTask = g_pGameServer->GetConfigServer()->GetSynGoodsTask(vectSynGoods[k]);
			if( 0 == pSynGoodsTask){
				continue;
			}

			nTempRandom += pSynGoodsTask->m_RandomNum;

			if( nTempRandom >= nRandom){
				MAP_TASK::iterator iter = mapTaskSyn.find(vectSynGoods[k]);
				if( iter != mapTaskSyn.end()){
					continue;
				}

				this->CreateTask(vectSynGoods[k]);
				++nCountSynGoods;
				nTotalRandom -= pSynGoodsTask->m_RandomNum;

				break;
			}
		}
	}
}

//通知客户端改变主界面任务状态图标
void	TaskPart::NoticeChengTaskStatus(enTaskStatus TaskStatus, bool bInit)
{
	if( m_TaskStatus == TaskStatus && !bInit){
		//已经是此状态的话,则不发消息给客户端
		return;
	}

	if( TaskStatus >= enTaskStatus_Max){
		return;
	}

	m_TaskStatus = TaskStatus;

	SC_ChangeTaskStatus ChangeTaskStatus;
	ChangeTaskStatus.m_TaskStatus = TaskStatus;

	OBuffer1k ob;
	ob << TaskHeader(enTaskCmd_SC_UpdateIcon, sizeof(ChangeTaskStatus)) << ChangeTaskStatus;
	m_pActor->SendData(ob.TakeOsb());
}


//刷新帮派主线任务
void    TaskPart::__Check_FlushTask_Main()
{
	MAP_TASK & mapTask = m_TaskList[enTaskClass_Mainline];

	if ( mapTask.size() > 0 )
	{
		//有任务的不进行推导
		return;		
	}

	TRACE("对玩家%s(等级:%d)进行任务推导!!", m_pActor->GetName(), m_pActor->GetCrtProp(enCrtProp_Level));

	//完成过的新手引导任务
	std::vector<TTaskID> vectGuide;

	//完成过的主线非新手引导任务
	std::set<TTaskID>	setAllMainTask;

	for ( int i = 0; i < MAX_TASK_ID; ++i )
	{
		if ( m_TaskHistory.get(i) )
		{
			const STaskCnfg * pTaskCnfg = g_pGameServer->GetConfigServer()->GetTaskCnfg(i);

			if ( 0 == pTaskCnfg )
				continue;

			if ( pTaskCnfg->m_TaskClass != enTaskClass_Mainline )
				continue;

			setAllMainTask.insert(i);
			
			if ( pTaskCnfg->m_bGuide )
			{
				vectGuide.push_back(i);
			}
		}
	}

	//推导下个新手引导任务
	TTaskID NextTaskID = g_pThingServer->GetTaskMgr().GetNextGuideTask(vectGuide);
	
	if ( INVALID_TASK_ID != NextTaskID )
	{
		this->CreateTask(NextTaskID);
	}

	//推导主线任务
	if ( setAllMainTask.size() <= 0 )
		return;

	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskMail = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Mainline);

	if( 0 == pmapTaskMail )
	{
		return;
	}

	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);

	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskMail->begin(); it != pmapTaskMail->end(); ++it )
	{
		const STaskCnfg & TaskCnfg = it->second;

		if ( TaskCnfg.m_bGuide )
			continue;

		std::set<TTaskID>::iterator iter = setAllMainTask.find(TaskCnfg.m_TaskID);

		if ( iter != setAllMainTask.end() )
			continue;

		iter = setAllMainTask.find(TaskCnfg.m_PreTaskID);

		if ( iter != setAllMainTask.end() )
		{
			//创建任务
			this->CreateTask(TaskCnfg.m_TaskID);			
		}

		if ( TaskCnfg.m_OpenLevel <= Level && TaskCnfg.m_OpenLevel > 0 )
		{
			//创建任务
			this->CreateTask(TaskCnfg.m_TaskID);			
		}
	}
}

//上线时检测刷新日常任务
void	TaskPart::__Check_FlushTask_Daily()
{
	MAP_TASK & mapTaskDaily = m_TaskList[enTaskClass_Daily];

	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);

	//所有日常任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskDaily = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Daily);
	if( 0 == pmapTaskDaily){
		return;
	}

	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskDaily->begin(); it != pmapTaskDaily->end(); ++it)
	{
		const STaskCnfg & TaskCnfg = it->second;

		//等级触发的
		if( TaskCnfg.m_OpenLevel != 0 && TaskCnfg.m_OpenLevel <= Level){
			//没完成，并且已有任务中没有的则创建
			MAP_TASK::iterator iter = mapTaskDaily.find(TaskCnfg.m_TaskID);

			if( iter == mapTaskDaily.end() && !m_TaskHistory.get(TaskCnfg.m_TaskID))
			{
				this->CreateTask(TaskCnfg.m_TaskID);
			}

			continue;
		}

		//前提任务触发的
		if( TaskCnfg.m_PreTaskID == INVALID_TASK_ID){
			continue;
		}

		if( !m_TaskHistory.get(TaskCnfg.m_PreTaskID)){
			continue;
		}

		//没完成，并且已有任务中没有的则创建
		MAP_TASK::iterator iter = mapTaskDaily.find(TaskCnfg.m_TaskID);
		if( iter == mapTaskDaily.end() && !m_TaskHistory.get(TaskCnfg.m_TaskID))
		{
			this->CreateTask(TaskCnfg.m_TaskID);
		}
	}
}

//上线时检测刷新帮派任务
void	TaskPart::__Check_FlushTask_Syn()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	if( pSynMgr->GetSyndicate(m_pActor->GetUID()) == 0){
		//清除下帮派任务，防止离线时被帮主踢出去，但没删除帮派任务
		this->__ClearTask_Syn();
		return;
	}

	MAP_TASK & mapTaskSyn = m_TaskList[enTaskClass_Gang];

	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);

	//所有日常任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskSyn = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Gang);
	if( 0 == pmapTaskSyn){
		return;
	}

	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskSyn->begin(); it != pmapTaskSyn->end(); ++it)
	{
		const STaskCnfg & TaskCnfg = it->second;

		if (TaskCnfg.m_TaskType == enTaskType_SynGoods){
			continue;
		}

		if (m_TaskHistory.get(TaskCnfg.m_TaskID)){
			continue;
		}

		//等级触发的
		if (TaskCnfg.m_OpenLevel != 0 && TaskCnfg.m_OpenLevel <= Level){
			//没完成，并且已有任务中没有的则创建
			MAP_TASK::iterator iter = mapTaskSyn.find(TaskCnfg.m_TaskID);

			if( iter == mapTaskSyn.end())
			{
				this->CreateTask(TaskCnfg.m_TaskID);
			}
			continue;
		}

		//前提任务触发的
		if( TaskCnfg.m_PreTaskID == INVALID_TASK_ID){
			continue;
		}

		if( !m_TaskHistory.get(TaskCnfg.m_PreTaskID)){
			continue;
		}

		//没完成，并且已有任务中没有的则创建
		MAP_TASK::iterator iter = mapTaskSyn.find(TaskCnfg.m_TaskID);
		if( iter == mapTaskSyn.end())
		{
			this->CreateTask(TaskCnfg.m_TaskID);
		}
	}
}

//清除帮派任务
void	TaskPart::__ClearTask_Syn()
{
	MAP_TASK & mapTaskSyn = m_TaskList[enTaskClass_Gang];

	MAP_TASK::iterator iter = mapTaskSyn.begin();

	for( ; iter != mapTaskSyn.end(); ++iter)
	{
		ITask* pTask = iter->second;
		if( 0 == pTask){
			continue;
		}

		pTask->Release();
	}

	mapTaskSyn.clear();
}

//是否在新手引导中
bool	TaskPart::IsInGuide()
{
	MAP_TASK & mapTaskSyn = m_TaskList[enTaskClass_Mainline];

	MAP_TASK::iterator iter = mapTaskSyn.begin();

	for ( ; iter != mapTaskSyn.end(); ++iter )
	{
		ITask * pTask = iter->second;

		if ( 0 == pTask )
			continue;

		const STaskCnfg * pTaskCnfg = pTask->GetTaskCnfg();

		if ( 0 == pTaskCnfg )
			continue;

		if ( pTaskCnfg->m_bGuide )
			return true;
	}

	return false;
}

//设置日常任务总数
void TaskPart::SetDailyTaskNum()
{
	UINT8	Level = m_pActor->GetCrtProp(enCrtProp_Level);
	//日常任务
	const std::hash_map<TTaskID,STaskCnfg> * pmapTaskDaily = g_pGameServer->GetConfigServer()->GetTaskList((UINT8)enTaskClass_Daily);

	m_DailyTaskNum = pmapTaskDaily->size();
	for( std::hash_map<TTaskID,STaskCnfg>::const_iterator it = pmapTaskDaily->begin(); it != pmapTaskDaily->end(); ++it)
	{
		const STaskCnfg & TaskCnfg = it->second;

		if( TaskCnfg.m_PreTaskID != 0){
			//前提任务触发的,只触发前提任务完成过的
			if( !m_TaskHistory.get(TaskCnfg.m_PreTaskID)){
				m_DailyTaskNum -- ;
			}		
		}

		if( TaskCnfg.m_OpenLevel != 0 && TaskCnfg.m_OpenLevel > Level){
			//等级触发的,比触发等级低的都触发
			m_DailyTaskNum -- ;			
		}
	}

	
}

//获得日常任务总数
UINT8 TaskPart::GetDailyTaskNum()
{
	return m_DailyTaskNum;
}

//获得未完成日常任务数
UINT8 TaskPart::GetDailyTaskNotOkNum()
{
	m_DailyTaskNotOkNum = m_TaskList[enTaskClass_Daily].size();
	return m_DailyTaskNotOkNum;
}

//结束新手引导要做的事
void  TaskPart::GuideOverDoSomething()
{
	//如果是最后一个新手引导任务，要发送活动数据给客户端
	IActivityPart * pActivityPart = m_pActor->GetActivityPart();

	if ( 0 == pActivityPart )
		return;

	pActivityPart->OpenActivity();
}
