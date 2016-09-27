
#include "IActor.h"
#include "GoodsNumTask.h"
#include "IConfigServer.h"
#include "TaskPart.h"
#include "IPacketPart.h"
#include "DMsgSubAction.h"
#include "IBasicService.h"
#include "ThingServer.h"
#include "IGoodsServer.h"


GoodsNumTask::GoodsNumTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg) : EventTask(pTaskPart, pTaskCnfg)
{
}

bool GoodsNumTask::Create()
{
	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
	if( 0 == pActor){
		return false;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return false;
	}

	//先检测下背包是否有,有则直接完成任务
	if( m_pTaskCnfg->m_vectParam.size() != 1){
		TRACE("<warming> %s : %d line 任务配置有错,参数个数(%d)不对!",__FUNCTION__,__LINE__, m_pTaskCnfg->m_vectParam.size());
		return false;
	} 

	if( !pPacketPart->HaveGoods(m_pTaskCnfg->m_vectParam[0], m_pTaskCnfg->m_AttainNum)){
		//还没完成
		EventTask::Create();
	}else{
		//任务完成了,背包里已有任务物品
		this->FinishTask();

		m_pTaskPart->NotifyClientUpdateTaskState(this);

		//任务完成后,发出事件,引发其它任务
		SS_TaskFinished TaskFinished;
		TaskFinished.m_TaskID	 = m_pTaskCnfg->m_TaskID;
		TaskFinished.m_TaskClass = (UINT8)m_pTaskCnfg->m_TaskClass;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);
		
		pActor->OnEvent(msgID,&TaskFinished,sizeof(SS_TaskFinished));

		m_pTaskPart->NoticeChengTaskStatus(enTaskStatus_FinishTask);

		//从玩家背包中删除这些物品
		pPacketPart->DestroyGoods(m_pTaskCnfg->m_vectParam[0], m_pTaskCnfg->m_AttainNum);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Task,m_pTaskCnfg->m_vectParam[0],UID(),m_pTaskCnfg->m_AttainNum,"完成物品任务,扣除任务物品");
	}

	return true;
}

void GoodsNumTask::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pTaskCnfg->m_EventID);

	if(EventData.m_MsgID == msgID)
	{
		if( this->IsFinished()){
			return;
		}

		if( m_pTaskCnfg->m_vectParam.size() != 1){
			TRACE("<warming> %s : %d line 任务配置有错,参数个数(%d)不对!",__FUNCTION__,__LINE__, m_pTaskCnfg->m_vectParam.size());
			return;
		}

		if( 0 == EventData.m_pContext){
			TRACE("<warming> %s : %d line 事件现场指针为空!",__FUNCTION__,__LINE__);
			return;
		}

		SS_GoodsNum * pGoodsNum = (SS_GoodsNum *)EventData.m_pContext;
		if( pGoodsNum->m_GoodsID != m_pTaskCnfg->m_vectParam[0]){
			return;
		}

		IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
		if( 0 == pActor){
			return;
		}

		IPacketPart * pPacketPart = pActor->GetPacketPart();
		if( 0 == pPacketPart){
			return;
		}
		
		if( pPacketPart->HaveGoods(m_pTaskCnfg->m_vectParam[0], m_pTaskCnfg->m_AttainNum)){
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

			//从玩家背包中删除这些物品
			pPacketPart->DestroyGoods(m_pTaskCnfg->m_vectParam[0], m_pTaskCnfg->m_AttainNum);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Task,m_pTaskCnfg->m_vectParam[0],UID(),m_pTaskCnfg->m_AttainNum,"完成物品任务,扣除任务物品");
		} 

		m_pTaskPart->NotifyClientUpdateTaskState(this);

		//设置成需要更新到数据库
		m_bNeedUpdate = true;
	}
}
