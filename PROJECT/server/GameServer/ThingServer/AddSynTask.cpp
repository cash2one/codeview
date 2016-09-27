#include "IActor.h"
#include "AddSynTask.h"
#include "TaskPart.h"
#include "DMsgSubAction.h"
#include "IConfigServer.h"

AddSynTask::AddSynTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg) : EventTask(pTaskPart, pTaskCnfg)
{
}

bool AddSynTask::Create()
{
	if ( m_pTaskPart == 0 || m_pTaskCnfg == 0){

		return false;
	}

	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();

	if ( 0 == pActor){

		return false;
	}

	if ( this->IsFinished()){
		
		return true;
	}

	//检测是否已经有帮派
	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 != pSyn){
		
		//已经有帮派了，完成任务
		this->FinishTask();

		m_pTaskPart->NotifyClientUpdateTaskState(this);

		//任务完成后,发出事件,引发其它任务
		SS_TaskFinished TaskFinished;
		TaskFinished.m_TaskID	 = m_pTaskCnfg->m_TaskID;
		TaskFinished.m_TaskClass = (UINT8)m_pTaskCnfg->m_TaskClass;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);
		
		pActor->OnEvent(msgID,&TaskFinished,sizeof(SS_TaskFinished));

		m_pTaskPart->NoticeChengTaskStatus(enTaskStatus_FinishTask);

		m_bNeedUpdate = true;
	}else{
		
		//还没完成
		EventTask::Create();
	}

	return true;
}
