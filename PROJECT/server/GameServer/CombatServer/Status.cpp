#include "IThing.h"
#include "Status.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IEffect.h"
#include "IStatusPart.h"

#include "DMsgSubAction.h"
#include "IStatusPart.h"
#include "XDateTime.h"
#include "CombatServer.h"
#include "IActor.h"
#include "IDBProxyClient.h"

Status::Status()
{
	m_bAlreadyStart		= false;				//状态是否已启动
	m_uidCreator		= UID();				//源角色UID,即是谁发起或者创建的这个状态
	m_MagicID			= INVALID_MAGIC_ID;		//引发这个状态的法术

	m_pStatusPart		= 0;					//本状态所属管理器
	m_pStatusInfo		= 0;					//本状态的配置,指向

	m_LeftRoundNum		= 0;

	m_EndStatusTime		= 0;

	m_DynamicStatusData = 0;



	m_LeftEffectCount = 0;
}

Status::~Status()
{
	for(int i=0; i<m_Effects.size(); i++)
	{
		m_Effects[i]->End();
		m_Effects[i]->Release();
	}

	m_Effects.clear();
}

//结束该状态
void Status::OverStatus()
{
	for(int i=0; i<m_Effects.size(); i++)
	{
		m_Effects[i]->End();
		m_Effects[i]->Release();
	}

	m_Effects.clear();
}

//获取状态剩余轮数
UINT32 Status::GetLeftRoundNum()
{
	return m_LeftRoundNum;
}

void Status::SetLeftRoundNum(INT32 num)
{
	m_LeftRoundNum = num;
}

void Status::SetLeftEffectCount(INT32 num)
{
	this->m_LeftEffectCount = num;
}

//减少轮数
void Status::DecreaseRoundNum()
{
	m_LeftRoundNum--;
}

//状态配置数据
const SStatusCnfg* Status::GetStatusCnfg() 
{
	return m_pStatusInfo;
}

bool Status::Create(const SStatusCnfg* pStatusInfo)						// 创建
{
	if(pStatusInfo == NULL)
	{
		return false;
	}

	//配置信息
	m_pStatusInfo   = pStatusInfo;

	this->m_LeftEffectCount = m_pStatusInfo->m_EffectCount;

	m_EndStatusTime = m_pStatusInfo->m_TimeNum + CURRENT_TIME();

	if( enStatusRocordType_Interval == pStatusInfo->m_RecordType){
		m_LeftRoundNum  = m_pStatusInfo->m_RoundNum;
	}



	//添加效果
	for(int i = 0; i < m_pStatusInfo->m_vectEffect.size(); i++)
	{
		IEffect *pEffect = g_pGameServer->GetCombatServer()->CreateEffect(m_pStatusInfo->m_vectEffect[i]);
		if(pEffect == NULL)
		{
			TRACE("创建效果失败 Effect = %d, status = %d", m_pStatusInfo->m_vectEffect[i], m_pStatusInfo->m_StatusID);
			return  false;
		}
		m_Effects.push_back(pEffect);
	}


	return true;
}

bool Status::Create(const SStatusCnfg* pStatusInfo, const SStatusInfo & StatusInfo)	//创建
{
	if( 0 == pStatusInfo){
		return false;
	}

	//配置信息
	m_pStatusInfo = pStatusInfo;

	this->m_LeftEffectCount = m_pStatusInfo->m_EffectCount;

	m_EndStatusTime = StatusInfo.m_EndStatusTime;

	if( enStatusRocordType_Interval == pStatusInfo->m_RecordType){
		m_LeftRoundNum  = StatusInfo.m_LeftRoundNum;
	}

	m_uidCreator = StatusInfo.m_uidCreator;

	//添加效果
	for(int i = 0; i < m_pStatusInfo->m_vectEffect.size(); i++)
	{
		IEffect *pEffect = g_pGameServer->GetCombatServer()->CreateEffect(m_pStatusInfo->m_vectEffect[i]);
		if(pEffect == NULL)
		{
			TRACE("创建效果失败 Effect = %d, status = %d", m_pStatusInfo->m_vectEffect[i], m_pStatusInfo->m_StatusID);
			return  false;
		}
		m_Effects.push_back(pEffect);
	}

	return true;
}

void Status::OnEvent(XEventData & EventData)
{
	bool bRemove = false;

	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_CombatOver))
	{
		bRemove = true;
	}
	else if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_MyCampAttacked))
	{
	
		SMyCampAttackedContext * pMyCampAttackedCnt = (SMyCampAttackedContext *)EventData.m_pContext;

		for(int i=0;i<pMyCampAttackedCnt->m_uidTarget.size();i++)
		{
			if(pMyCampAttackedCnt->m_uidTarget[i]==m_pStatusPart->GetMaster()->GetUID())
			{
				this->m_LeftEffectCount--;
				break;
			}
		}

		if( m_LeftEffectCount<=0)
		{
			pMyCampAttackedCnt->m_vectRemoveStatus.push_back(m_pStatusInfo->m_StatusType);
			bRemove = true;
		}
	}

	//移除
	if(bRemove && m_pStatusPart)
	{
		m_pStatusPart->RemoveStatus(m_pStatusInfo->m_StatusID,m_uidCreator);
	}
}

void Status::OnTimer(UINT32 timerID)
{
	if( enStatusTimerID_TimeNumStatus == timerID){
		//状态持续时间到，删除
		if( m_pStatusPart){
			m_pStatusPart->RemoveStatus(m_pStatusInfo->m_StatusID, m_uidCreator);
		}
	}
	else if(enStatusTimerID_TimeIntervalStatus == timerID)
	{
		StartEffect();

		//发事件
		if(m_pStatusPart)
		{
		   SS_TimerStatus TimerStatus;
		   TimerStatus.m_StatusID = m_pStatusInfo->m_StatusID;
		   TimerStatus.m_uidCreator = m_uidCreator;
		   TimerStatus.m_uidActor = m_pStatusPart->GetMaster()->GetUID();

		   UINT32 MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TimerStatus);


			m_pStatusPart->GetMaster()->OnAction(MsgID,&TimerStatus,sizeof(TimerStatus));
		}

		StopEffect();

		DecreaseRoundNum();
		if(GetLeftRoundNum()==0)
		{
			//移除
	      if(m_pStatusPart)
	      {
		     m_pStatusPart->RemoveStatus(m_pStatusInfo->m_StatusID,m_uidCreator);
	      }			
		}		
	}
}

bool Status::CanStart(IStatusPart *pStatusPart, UID uidCreator)			// 是否可以开始
{
	return true;
}

//启动效果
void Status::StartEffect()
{
	for(int i=0; i<m_Effects.size(); i++)
	{
		if(m_Effects[i]->Start((ICreature *)m_pStatusPart->GetMaster(),m_uidCreator,this)==false)
		{
			TRACE("<error> %s : %d line 启动效果失败,EffectID = %d !",__FUNCTION__,__LINE__,m_Effects[i]->GetEffectID());
		}
	}
}

//暂停效果
void Status::StopEffect()
{
	for(int i=0; i<m_Effects.size(); i++)
	{
		if(m_Effects[i]->End()==false)
		{
			TRACE("<error> %s : %d line 启动效果失败,EffectID = %d !",__FUNCTION__,__LINE__,m_Effects[i]->GetEffectID());
		}
	}
}

bool Status::Start(IStatusPart *pStatusPart, UID uidCreator, TMagicID MagicID )
{
	if( 0 == pStatusPart){
		return false;
	}
	
	m_pStatusPart = pStatusPart;

	if( enStatusRocordType_Interval == (enStatusRocordType)m_pStatusInfo->m_RecordType && m_LeftRoundNum <= 0){
		return true;
	}

	if( enStatusRocordType_Time == (enStatusRocordType)m_pStatusInfo->m_RecordType && m_EndStatusTime <= CURRENT_TIME()){
		return true;
	}

	m_MagicID = MagicID;

	
	if( enStatusRocordType_Time == m_pStatusInfo->m_RecordType){
		//如果是以时间为结束，则设置定时器
		g_pGameServer->GetTimeAxis()->SetTimer(enStatusTimerID_TimeNumStatus, this, (this->GetEndTimeNum() - CURRENT_TIME()) * 1000, "Status::Start");

	}else if( enStatusRocordType_Interval == m_pStatusInfo->m_RecordType){
		//注册事件
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_CombatOver);  //战斗结束

		pStatusPart->GetMaster()->SubscribeEvent(msgID,this,"Status::Start [enMsgID_CombatOver]");

		INT32 timeInterval = m_pStatusInfo->m_TimeNum/(m_pStatusInfo->m_RoundNum < 1 ? 1 : m_pStatusInfo->m_RoundNum);
		if(timeInterval<1)
		{
			timeInterval = 1;
		}

		g_pGameServer->GetTimeAxis()->SetTimer(enStatusTimerID_TimeIntervalStatus, this, timeInterval * 1000, "Status::Start");

		//暂时不启动效果
		return true;
		
	}
	else if(enStatusRocordType_Count == m_pStatusInfo->m_RecordType)
	{
			//注册事件
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_MyCampAttacked);

		pStatusPart->GetMaster()->SubscribeEvent(msgID,this,"Status::Start [enMsgID_RoundStart]");

		msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_CombatOver);  //战斗结束

		pStatusPart->GetMaster()->SubscribeEvent(msgID,this,"Status::Start [enMsgID_CombatOver]");

		g_pGameServer->GetTimeAxis()->SetTimer(enStatusTimerID_TimeNumStatus, this, (this->GetEndTimeNum() - CURRENT_TIME()) * 1000, "Status::Start");
	}

	StartEffect();

	return true;
}



Status* Status::Clone()													// clone一个本状态
{
	return 0;
}

void Status::Release()												// 释放
{
	delete this;
}

//获取状态的记录类型
enStatusRocordType Status::GetStatusRecordType()
{
	if( enStatusRocordType_Max <= m_pStatusInfo->m_RecordType){
		return enStatusRocordType_Max;
	}

	return (enStatusRocordType)m_pStatusInfo->m_RecordType;
}


//获取状态的结束时间
UINT32	Status::GetEndTimeNum()
{
	return m_EndStatusTime;
}

//设置状态的结束时间
void	Status::SetEndTimeNum(UINT32 EndTime)
{
	INT32 nCurTime = CURRENT_TIME();

	if( m_EndStatusTime - nCurTime < 0){
		return;
	}

	m_EndStatusTime = EndTime;

	//删除旧定时器
	g_pGameServer->GetTimeAxis()->KillTimer(enStatusTimerID_TimeNumStatus, this);

	//设置新定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enStatusTimerID_TimeNumStatus, this, (m_EndStatusTime - nCurTime) * 1000, "Status::SetEndTimeNum");

}

//获得引发这个状态的法术ID
TMagicID Status::GetMagicID()
{
	return m_MagicID;
}

//设置状态动态数据
 void Status::SetStatusData(INT32 value)
 {
	 m_DynamicStatusData = value;
 }

//获得状态动态数据
INT32  Status::GetStatusData()
{
	return m_DynamicStatusData;
}


//保存
void Status::SaveData()
{
	if( 0 == m_pStatusPart || 0 == m_pStatusInfo){
		return;
	}

	if( m_pStatusInfo->m_RecordType == enStatusRocordType_Interval){
		return;
	}

	IThing* pThing = m_pStatusPart->GetMaster();
	if( 0 == pThing){
		return;
	}

	if( pThing->GetThingClass() != enThing_Class_Actor){
		return;
	}

	IActor * pActor = (IActor *)pThing;

	SDB_Update_Status_Req Req;

	Req.m_uidUser  = pActor->GetUID().ToUint64();
	Req.m_StatusID = m_pStatusInfo->m_StatusID;
	Req.m_EndStatusTime = m_EndStatusTime;
	Req.m_UidCreator = m_uidCreator.ToUint64();

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateStatusInfo, ob.TakeOsb(), 0, 0);
}
