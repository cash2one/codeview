
#include "IActor.h"
#include "EmployeeNumTask.h"
#include "IConfigServer.h"
#include "TaskPart.h"
#include "DMsgSubAction.h"
#include "IBasicService.h"
#include "ThingServer.h"



EmployeeNumTask::EmployeeNumTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg) : EventTask(pTaskPart, pTaskCnfg)
{
}

bool EmployeeNumTask::Create()
{
	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
	if( 0 == pActor){
		return false;
	}


	
	if( m_pTaskCnfg->m_vectParam.size() != 2){
		TRACE("<warming> %s : %d line 任务配置有错,参数个数(%d)不对!",__FUNCTION__,__LINE__, m_pTaskCnfg->m_vectParam.size());
		return false;
	} 

	//先检测下招募角色个数,足够则直接完成任务
	if( pActor->GetEmployeeNum() < m_pTaskCnfg->m_vectParam[1]){
		//还没完成
		EventTask::Create();
	}else{
		//任务完成了,招募角色个数满足任务
		this->FinishTask();

		m_pTaskPart->NotifyClientUpdateTaskState(this);

		//任务完成后,发出事件,引发其它任务
		SS_TaskFinished TaskFinished;
		TaskFinished.m_TaskID	 = m_pTaskCnfg->m_TaskID;
		TaskFinished.m_TaskClass = (UINT8)m_pTaskCnfg->m_TaskClass;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);
		
		pActor->OnEvent(msgID,&TaskFinished,sizeof(SS_TaskFinished));

		m_pTaskPart->NoticeChengTaskStatus(enTaskStatus_FinishTask);

	}

	return true;
}

void EmployeeNumTask::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pTaskCnfg->m_EventID);

	if(EventData.m_MsgID == msgID)
	{
		if( this->IsFinished()){
			return;
		}

		if( m_pTaskCnfg->m_vectParam.size() != 2){
			TRACE("<warming> %s : %d line 任务配置有错,参数个数(%d)不对!",__FUNCTION__,__LINE__, m_pTaskCnfg->m_vectParam.size());
			return;
		}

		if( 0 == EventData.m_pContext){
			TRACE("<warming> %s : %d line 事件现场指针为空!",__FUNCTION__,__LINE__);
			return;
		}

		IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
		if( 0 == pActor){
			return;
		}

		
		if( pActor->GetEmployeeNum() >= m_pTaskCnfg->m_vectParam[1]){
			//任务完成了
			this->FinishTask();	

			IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
			if( 0 == pActor){
				return;
			}
			//任务完成后,发出事件,引发其它任务
			SS_TaskFinished TaskFinished;
			TaskFinished.m_TaskID	 = m_pTaskCnfg->m_TaskID;
			TaskFinished.m_TaskClass = (UINT8)m_pTaskCnfg->m_TaskClass;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);
			
			pActor->OnEvent(msgID,&TaskFinished,sizeof(SS_TaskFinished));

			m_pTaskPart->NoticeChengTaskStatus(enTaskStatus_FinishTask);


		} 

		m_pTaskPart->NotifyClientUpdateTaskState(this);

		//设置成需要更新到数据库
		m_bNeedUpdate = true;
	}
}
