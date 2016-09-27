#include "IActor.h"

#include "GetSynWarScoreTask.h"

#include "DMsgSubAction.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "TaskPart.h"

GetSynWarScoreTask::GetSynWarScoreTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg) : EventTask(pTaskPart, pTaskCnfg)
{

}

void GetSynWarScoreTask::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pTaskCnfg->m_EventID);

	if(EventData.m_MsgID == msgID)
	{
		if( this->IsFinished()){
			return;
		}

		if( 0 == EventData.m_pContext){
			TRACE("<warming> %s : %d line 事件现场指针为空!",__FUNCTION__,__LINE__);
			return;
		}

		SS_GetSynWarScore * pGetSynWarScore = (SS_GetSynWarScore *)EventData.m_pContext;

		m_nCurCount += pGetSynWarScore->GetNum;

		if( m_pTaskCnfg->m_AttainNum <= m_nCurCount){
			//任务完成
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
