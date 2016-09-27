#include "IActor.h"
#include "EventTask.h"
#include "TaskPart.h"
#include "IConfigServer.h"

#include "DMsgSubAction.h"
#include "IBasicService.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "IEquipment.h"
#include "ISyndicateMgr.h"
#include "ISyndicateMember.h"
#include "ThingServer.h"
#include "ICombatPart.h"
#include "ISyndicate.h"
#include "IGoodsServer.h"
#include "XDateTime.h"

EventTask::EventTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg)
{
	 m_pTaskPart = pTaskPart; //该任务所属的TaskPart
	 m_pTaskCnfg = pTaskCnfg;
	 m_nCurCount = 0; //当前计数
	 m_FinishTime = 0;

	 m_bNeedUpdate = true;
}

EventTask::~EventTask()
{
}

bool EventTask::Create()
{
	if(m_pTaskPart == 0 || m_pTaskCnfg==0)
	{
		return false;
	}

	IActor * pActor =(IActor *) m_pTaskPart->GetMaster();

	//注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pTaskCnfg->m_EventID);

	pActor->SubscribeEvent(msgID,this,"EventTask::Create");


	return true;
}

void EventTask::Release()
{
	delete this;
}

//获取任务ID
TTaskID  EventTask::GetTaskID()
{
	return m_pTaskCnfg->m_TaskID;
}

//是否已完成
bool  EventTask::IsFinished()
{
	return m_FinishTime > 0;
}

const STaskCnfg * EventTask::GetTaskCnfg()
{
	return m_pTaskCnfg;
}

//获得DB数据
bool  EventTask::GetDBContext(void * pBuffer,INT32 & len)
{
	//SDB_UpdateTaskReq * pTaskData = (SDB_UpdateTaskReq *)pBuffer;

	//pTaskData->m_FinishTime = m_FinishTime;
	//pTaskData->m_CurCount = m_nCurCount;
	//pTaskData->m_TaskID	  = m_pTaskCnfg->m_TaskID;

	DB_TaskData * pTaskData = (DB_TaskData *)pBuffer;
	pTaskData->m_FinishTime = m_FinishTime;
	pTaskData->m_TaskCount = m_nCurCount;
	pTaskData->m_TaskID	  = m_pTaskCnfg->m_TaskID;

	return true;
}

bool  EventTask::SetDBContext(const void * pBuffer,INT32  len) 
{
	if( pBuffer == 0 || len < sizeof(DB_TaskData)){
		return false;
	}

	DB_TaskData * pTaskData = (DB_TaskData *)pBuffer;

	m_FinishTime = pTaskData->m_FinishTime;
	m_nCurCount = pTaskData->m_TaskCount;

	return true;
}

//任务复置
void EventTask::Reset() 
{
	m_nCurCount = 0;
	m_FinishTime = 0;
	m_pTaskPart->NotifyClientUpdateTaskState(this);

	//设置成需要更新到数据库
	m_bNeedUpdate = true;
}

//任务信息放到buffer中,发送给客户端
bool EventTask::AddToBuffer(IActor * pActor, OBuffer4k & ob)
{
	STaskData TaskData;
	TaskData.m_TaskID	   = m_pTaskCnfg->m_TaskID;
	TaskData.m_TaskClass   = (enTaskClass)m_pTaskCnfg->m_TaskClass;
	TaskData.m_FinishTime   = m_FinishTime;
	TaskData.m_bGuide	   = (m_pTaskCnfg->m_bGuide==1);
	TaskData.m_CurCount	   = m_nCurCount;
	TaskData.m_TargetCount = m_pTaskCnfg->m_AttainNum;
	TaskData.m_TaskAward.m_Experience  = m_pTaskCnfg->m_Experience;
	TaskData.m_TaskAward.m_GiftTicket  = m_pTaskCnfg->m_GiftTicket;
	TaskData.m_TaskAward.m_GodSwordNimbus = m_pTaskCnfg->m_GodSwordNimbus;
	TaskData.m_TaskAward.m_Nimbus	   = m_pTaskCnfg->m_Nimbus;
	TaskData.m_TaskAward.m_NimbusStone = m_pTaskCnfg->m_NimbusStone;
	TaskData.m_TaskAward.m_SynContribution = m_pTaskCnfg->m_SynContribution;
	TaskData.m_TaskAward.m_SynExp	   = m_pTaskCnfg->m_SynExp;
	TaskData.m_TaskAward.m_Honor	   = m_pTaskCnfg->m_Honor;
	TaskData.m_TaskAward.m_Credit	   = m_pTaskCnfg->m_Credit;
	TaskData.m_TaskAward.m_GhostSoul   = m_pTaskCnfg->m_GhostSoul;
	TaskData.m_TaskAward.m_PolyNimbus  = m_pTaskCnfg->m_PolyNimbus;
	
	for( int i = 0; i + 1 < m_pTaskCnfg->m_vectGoods.size() && (i / 2 < MAX_TASK_AWARD_GOODS_NUM); i += 2)
	{
		TaskData.m_TaskAward.m_Goods[i / 2].m_GoodsID = m_pTaskCnfg->m_vectGoods[i];
		TaskData.m_TaskAward.m_Goods[i / 2].m_Number  = m_pTaskCnfg->m_vectGoods[i + 1];

		pActor->SendGoodsCnfg(TaskData.m_TaskAward.m_Goods[i / 2].m_GoodsID);
	}

	ob << TaskData;

	// fly add 20121106

	ob.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskNameLangID), strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskNameLangID)) + 1);
	ob.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskTargetLangID), strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskTargetLangID)) + 1);
	ob.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskDescLangID), strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(m_pTaskCnfg->m_TaskDescLangID)) + 1);
	//ob.Push(m_pTaskCnfg->m_strName.c_str(), m_pTaskCnfg->m_strName.length() + 1);
	//ob.Push(m_pTaskCnfg->m_strTaskTarget.c_str(), m_pTaskCnfg->m_strTaskTarget.length() + 1);
	//ob.Push(m_pTaskCnfg->m_strTaskDesc.c_str(), m_pTaskCnfg->m_strTaskDesc.length() + 1);
	ob.Push(m_pTaskCnfg->m_strTaskAward.c_str(), m_pTaskCnfg->m_strTaskAward.length() + 1);
	return true;
}

//领取奖励
bool EventTask::TakeAward()
{
	if ( !this->IsFinished()){
		return false;
	}

	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
	if( 0 == pActor){
		return false;
	}

	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return false;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return false;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return false;
	}

	ICombatPart * pCombatPart = pActor->GetCombatPart();
	if( 0 == pCombatPart){
		return false;
	}

	SC_TaskTakeAward_Rsp Rsp;
	Rsp.m_Result = enTaskRetCode_Ok;
	Rsp.m_TaskID = m_pTaskCnfg->m_TaskID;
	OBuffer1k ob;

	if( m_pTaskCnfg->m_vectGoods.size() != 0 && !pPacketPart->CanAddGoods(m_pTaskCnfg->m_vectGoods)){
		//背包已满
		Rsp.m_Result = enTaskRetCode_ErrPacketFull;
	}else{
		//物品及数量
		SAwardGoods AwardGoods;
		for( int i = 0,n = 0; i + 1 < m_pTaskCnfg->m_vectGoods.size() && n < MAX_TASK_AWARD_GOODS_NUM; i += 2, ++n)
		{
			this->TakeGoods(m_pTaskCnfg->m_vectGoods[i], m_pTaskCnfg->m_vectGoods[i + 1]);

			AwardGoods.m_GoodsID = m_pTaskCnfg->m_vectGoods[i];
			AwardGoods.m_Number	 = m_pTaskCnfg->m_vectGoods[i + 1];
			Rsp.m_TaskAward.m_Goods[n] = AwardGoods;
		}

		//灵石
		pActor->AddCrtPropNum(enCrtProp_ActorStone, m_pTaskCnfg->m_NimbusStone);
		Rsp.m_TaskAward.m_NimbusStone = m_pTaskCnfg->m_NimbusStone;

		//礼券
		pActor->AddCrtPropNum(enCrtProp_ActorTicket, m_pTaskCnfg->m_GiftTicket);
		Rsp.m_TaskAward.m_GiftTicket = m_pTaskCnfg->m_GiftTicket;

		//聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, m_pTaskCnfg->m_PolyNimbus);
		Rsp.m_TaskAward.m_PolyNimbus = m_pTaskCnfg->m_PolyNimbus;

		//声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, m_pTaskCnfg->m_Credit);
		if(m_pTaskCnfg->m_Credit == 0){
			Rsp.m_TaskAward.m_Credit = m_pTaskCnfg->m_Credit;
		}else{
			Rsp.m_TaskAward.m_Credit = m_pTaskCnfg->m_Credit + pActor->GetVipValue(enVipType_AddGetCredit);
		}
		

		//灵魄
		pActor->AddCrtPropNum(enCrtProp_GhostSoul, m_pTaskCnfg->m_GhostSoul);
		Rsp.m_TaskAward.m_GhostSoul = m_pTaskCnfg->m_GhostSoul;

		//出战角色获得经验(不享受多倍经验)
		pActor->CombatActorAddExp(m_pTaskCnfg->m_Experience, false);

		Rsp.m_TaskAward.m_Experience = m_pTaskCnfg->m_Experience;

		//灵气
		std::vector<SCreatureLineupInfo> vectActor = pCombatPart->GetJoinBattleActor();
		for( int i = 0; i < vectActor.size(); ++i)
		{
			if( 0 == vectActor[i].m_pCreature){
				continue;
			}

			vectActor[i].m_pCreature->AddCrtPropNum(enCrtProp_ActorNimbus,m_pTaskCnfg->m_Nimbus);
		}

		Rsp.m_TaskAward.m_Nimbus = m_pTaskCnfg->m_Nimbus;

		//仙剑灵气m_pTaskCnfg->m_GodSwordNimbus
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, m_pTaskCnfg->m_GodSwordNimbus);

		Rsp.m_TaskAward.m_GodSwordNimbus = m_pTaskCnfg->m_GodSwordNimbus;

		//帮派方面的奖励
		ISyndicateMember * pSyndicateMember = pSynMgr->GetSyndicateMember(pActor->GetUID());
		if( 0 != pSyndicateMember){
			//获得的帮贡
			pSyndicateMember->AddContribution(m_pTaskCnfg->m_SynContribution);
			Rsp.m_TaskAward.m_SynContribution = m_pTaskCnfg->m_SynContribution;
			
			//获得的帮派经验
			ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pSyndicateMember->GetSynID());
			if( 0 != pSyndicate){
				pSyndicate->AddSynExp(m_pTaskCnfg->m_SynExp);
				Rsp.m_TaskAward.m_SynExp = m_pTaskCnfg->m_SynExp;
			}
		}

		//荣誉
		pActor->AddCrtPropNum(enCrtProp_ActorHonor, m_pTaskCnfg->m_Honor);
		Rsp.m_TaskAward.m_Honor = m_pTaskCnfg->m_Honor;

		//领取奖励完后记录到已完成任务中
		m_pTaskPart->RecordFinished(m_pTaskCnfg->m_TaskID);

		//客户端要求的顺序，先发领取返回码,再发删除，然后再创建别的任务
		ob << TaskHeader(enTaskCmd_TakeAward, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());

		m_pTaskPart->NotifyClientDelTask(this);

		//引发其它任务
		m_pTaskPart->TaskFinish_CreateTask(m_pTaskCnfg->m_TaskID);

		//新手引导任务结束，要做的事
		if ( g_pThingServer->GetTaskMgr().IsLastGuideTask(m_pTaskCnfg->m_TaskID) )
		{
			m_pTaskPart->GuideOverDoSomething();
		}
	
		return true;
	}

	ob << TaskHeader(enTaskCmd_TakeAward, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	if( Rsp.m_Result == enTaskRetCode_Ok){
		return true;
	}

	return false;
}


//任务状态有更新
void EventTask::NotifyClientUpdateTaskState()
{
	SC_TaskUpdateTask UpdateTask;
	UpdateTask.m_bFinished = this->IsFinished();
	UpdateTask.m_nCurCount = m_nCurCount;
	UpdateTask.m_TaskID	   = m_pTaskCnfg->m_TaskID;

	OBuffer1k ob;
	ob << TaskHeader(enTaskCmd_SC_Update, sizeof(UpdateTask)) << UpdateTask;

	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
	if( 0 == pActor){
		return;
	}
	pActor->SendData(ob.TakeOsb());
}




void EventTask::OnEvent(XEventData & EventData)
{
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pTaskCnfg->m_EventID);

		if(EventData.m_MsgID == msgID)
		{
			if( this->IsFinished())
			{
				return ;
			}

								
			if(g_pThingServer->IsEqual(EventData,m_pTaskCnfg->m_EventID,m_pTaskCnfg->m_vectParam)==false)
			{
				return ;
			}

			//满足条件
			m_nCurCount++;

			if(m_nCurCount>=m_pTaskCnfg->m_AttainNum)
			{
				//任务完成了
				this->FinishTask();	

				//发布事件
				SS_TaskFinished	FinishTask;
				FinishTask.m_TaskClass = m_pTaskCnfg->m_TaskClass;
				FinishTask.m_TaskID	   = m_pTaskCnfg->m_TaskID;

				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);

				IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
				if( 0 == pActor){
					return;
				}

				pActor->OnEvent(msgID,&FinishTask,sizeof(FinishTask));

				m_pTaskPart->NoticeChengTaskStatus(enTaskStatus_FinishTask);
			}

			m_pTaskPart->NotifyClientUpdateTaskState(this);

			//设置成需要更新到数据库
			m_bNeedUpdate = true;
		}
}


//任务数据信息放到buffer中
bool EventTask::AddDBToBuffer(OBuffer4k & ob)
{
	if( !m_pTaskCnfg->m_bSaveToDB){
		//不需要保存
		return true;
	}

	DB_TaskData TaskData;
	int nLen = sizeof(TaskData);
	if( !this->GetDBContext(&TaskData, nLen)){
		return false;
	}

	ob << TaskData;

	return true;
}

//GM命令让任务结束
bool EventTask::GMFinishTask()
{
	m_nCurCount = m_pTaskCnfg->m_AttainNum;
	m_FinishTime = CURRENT_TIME();

	IActor * pActor = (IActor *)m_pTaskPart->GetMaster();
	if( 0 == pActor){
		return false;
	}

	//任务完成后,发出事件,引发其它任务
	SS_TaskFinished TaskFinished;
	TaskFinished.m_TaskID	 = m_pTaskCnfg->m_TaskID;
	TaskFinished.m_TaskClass = m_pTaskCnfg->m_TaskClass;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TaskFinished);
	
	pActor->OnEvent(msgID,&TaskFinished,sizeof(SS_TaskFinished));

	m_pTaskPart->NotifyClientUpdateTaskState(this);

	//设置成需要更新到数据库
	m_bNeedUpdate = true;

	return true;
}

//设置是否需要更新到数据库
void EventTask::SetNeedUpdate(bool bNeedUpdate)
{
	m_bNeedUpdate = bNeedUpdate;
}

//获取是否需要更新到数据库
bool EventTask::GetNeedUpdate()
{
	return m_bNeedUpdate;
}

//任务奖励得到物品
void EventTask::TakeGoods(TGoodsID GoodsID, INT32 nNum, bool bBind)
{
	std::vector<IGoods *> vectGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsID, nNum, bBind);

	if( vectGoods.size() == 0){
		TRACE("<error> %s : %d 行 任务获取物品失败！！物品ID = %d，数量 = %d", __FUNCTION__, __LINE__, GoodsID, nNum);
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
	
	//加入背包
	for( int i = 0; i < vectGoods.size(); ++i)
	{
		IGoods * pGoods = vectGoods[i];
		if( 0 == pGoods){
			continue;
		}

		if( false == pPacketPart->AddGoods(pGoods->GetUID())){
			TRACE("<error> %s : %d 行 任务获得物品加入背包失败！！", __FUNCTION__, __LINE__);
			continue;
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Task,GoodsID,pGoods->GetUID(),pGoods->GetNumber(),"完成任务,获得奖励");
	}
}

//完成任务
void EventTask::FinishTask()
{
	m_FinishTime = CURRENT_TIME();
}

