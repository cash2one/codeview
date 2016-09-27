
#include "IActor.h"
#include "CombatServer.h"
#include "IConfigServer.h"
#include "IBasicService.h"


#include "Magic.h"

#include "EffectChangeAttack.h"
#include "EffectPersistDamage.h"
#include "EffectChangeProp.h"
#include "EffectChangeTriggerProb.h"
#include "EffectForbidMagic.h"
#include "EffectForbidPhysics.h"
#include "Status.h"
#include "EffectAddStatus.h"
#include "EffectDirectDamage.h"
#include "IFuBenPart.h"
#include "EffectSuckBlood.h"
#include "EffectCure.h"
#include "EffectReturnBlood.h"
#include "EffectAvoidDie.h"

#include "ICombatPart.h"

IGameServer * g_pGameServer = 0;

ICombatServer * CreateCombatServer(IGameServer *pServerGlobal)
{
	g_pGameServer = pServerGlobal;
	CombatServer * pCombatServer = new CombatServer();
	if(pCombatServer->Create()==false)
	{
		delete pCombatServer;
		return 0;
	}

	return pCombatServer;
}

CombatServer::CombatServer()
{
}

CombatServer::~CombatServer()
{
}

bool  CombatServer::Create()
{
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_DelCombat,this,3*1000,"CombatServer::Create[enTimerID_DelCombat]");
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Combat,this);
}


//释放
void  CombatServer::Release(void)
{
	delete this;
}

void  CombatServer::Close()
{
	g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_DelCombat,this);
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Combat,this);
}

 void CombatServer::OnTimer(UINT32 timerID)
 {
	 if(enTimerID_DelCombat == timerID )
	 {
		 for(std::list<ICombat*>::iterator it = m_listDelCombat.begin(); it != m_listDelCombat.end();++it)
		 {
			 ICombat* pCombat = (*it);

			 pCombat->Release();

		 }
		 m_listDelCombat.clear();
	 }
 }

//创建法术
IMagic * CombatServer::CreateMagic(const SCreateMagicCnt & CreateMagicCnt)
{
	const SMagicLevelCnfg* pMagicConfig = g_pGameServer->GetConfigServer()->GetMagicLevelCnfg(CreateMagicCnt.m_MagicID,CreateMagicCnt.m_Level);

	if(pMagicConfig == 0)
	{
		TRACE("<error> %s : %d line 找不到法术等配置信息 MagicID = %d Level = %d",__FUNCTION__,__LINE__,CreateMagicCnt.m_MagicID,CreateMagicCnt.m_Level);
		return 0;
	}

	Magic * pMagic = new Magic();

	if(pMagic->Create(pMagicConfig)==false)
	{
		delete pMagic;
		pMagic = 0;
		return 0;
	}

	return pMagic;
}


//创建效果
IEffect *  CombatServer::CreateEffect(TEffectID EffectID)
{
	const SEffectCnfg * pEffectCnfg = g_pGameServer->GetConfigServer()->GetEffectCnfg(EffectID);

	if(pEffectCnfg==0)
	{
		TRACE("<error> %s : %d line 找不到效果配置信息,EffectID = % d", __FUNCTION__,__LINE__,EffectID);
		return 0;
	}

	Effect * pEffect = 0;

	switch(pEffectCnfg->m_EffectType)
	{
	case enEffectType_ChangeProp:   //增减属性
		{
			pEffect = new EffectChangeProp();
		}
		break;

	case enEffectType_PersistDamage:  //持续伤害
		{
			pEffect = new EffectPersistDamage();
		}
		break;

	case enEffectType_DirectDamage:  //直接伤害
		{
			pEffect = new EffectDirectDamage();
		}
		break;
	case enEffectType_ForbidMagic:  //禁止施放法术
		{
			pEffect = new EffectForbidMagic();
		}
		break;
	case enEffectType_ForbidPhysics:  //禁止施放物理攻击
		{
			pEffect = new EffectForbidPhysics();
		}
		break;

	case enEffectType_ChangeTriggerProb:  //改变触发几率
		{
			pEffect = new EffectChangeTriggerProb();
		}
		break;

	case enEffectType_ChangeAttack:   //改变攻击力
		{
			pEffect = new EffectChangeAttack();
		}
		break;
	case enEffectType_AddStatus:   //增加状态
		{
			pEffect = new EffectAddStatus();
		}
		break;
	case enEffectType_SuckBlood:   //吸血
	{
		pEffect = new EffectSuckBlood();
	}
	   break;
	case enEffectType_Cure:
		{
			pEffect = new EffectCure();
		}
		break;
	case enEffectType_ReturnBlood:
		{
			pEffect = new EffectReturnBlood();
		}
		break;
	case enEffectType_AvoidDie:
		{
			pEffect = new EffectAvoidDie();
		}
		break;

		
	default:
		{
			TRACE("<error> %s : %d line 错误的效果类型,EffectID = % d type = %d", __FUNCTION__,__LINE__,EffectID,pEffectCnfg->m_EffectType);
		}
		break;
	}

	if(pEffect)
	{
		if(pEffect->Create(pEffectCnfg)==false)
		{
			delete pEffect;
			pEffect = 0;
			TRACE("<error> %s : %d line 创建效果失败 EffectID = %d ", __FUNCTION__,__LINE__,EffectID);
		}
	}

	return pEffect;
}

//创建状态
IStatus *  CombatServer::CreateStatus(TStatusID  StatusID)
{
	const SStatusCnfg* pStatusInfo = g_pGameServer->GetConfigServer()->GetStatusCnfg(StatusID);

	if(pStatusInfo == 0)
	{
		TRACE("<error> %s : %d line 找不到状态配置信息,StatusID = % d", __FUNCTION__,__LINE__,StatusID);
		return 0;
	}

	Status * pStatus = new Status();
	
	if(pStatus->Create(pStatusInfo)==false)
	{
		delete pStatus;

		pStatus = 0;

		TRACE("<error> %s : %d line 状态创建不成功,StatusID = % d", __FUNCTION__,__LINE__,StatusID);


		return 0;
	}

	return pStatus;
}

//创建状态
IStatus * CombatServer::CreateStatus(const SStatusInfo & StatusInfo)
{
	const SStatusCnfg* pStatusInfo = g_pGameServer->GetConfigServer()->GetStatusCnfg(StatusInfo.m_StatusID);

	if( pStatusInfo == 0){
		TRACE("<error> %s : %d line 找不到状态配置信息,StatusID = % d", __FUNCTION__,__LINE__,StatusInfo.m_StatusID);
		return 0;
	}

	Status * pStatus = new Status();

	if( pStatus->Create(pStatusInfo, StatusInfo) == false){
		delete pStatus;

		pStatus = 0;

		TRACE("<error> %s : %d line 状态创建不成功,StatusID = % d", __FUNCTION__,__LINE__,StatusInfo.m_StatusID);

		return 0;
	}

	return pStatus;
}


//玩家pActor和生物uidCreature开始战斗
bool CombatServer::Combat(IActor * pActor,UID uidCreature)
{
	return false;
}


//收到MSG_ROOT消息
void CombatServer::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	 typedef  void (CombatServer::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enCombatCmd_Max]=
	 {
		 &CombatServer::OnCombat,
		 & CombatServer::OnCombatNpcInFuBen,
		 & CombatServer::OnLeaveFuBen,
	 };

	 if(nCmd>=enCombatCmd_Max || 0==s_funcProc[nCmd])
	 {
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}


//发起战斗
void CombatServer::OnCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{    
	SCombatCombat Req;
	
	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	switch(Req.m_SubCmd)
	{
	case enCombatSubCmd_CS_FireMagic:
		{
			ManualMagic(pActor,Req.m_CombatID,ib);
		}
		break;
	case enCombatSubCmd_CS_ActionAck:
		{
			AckAction(pActor,Req.m_CombatID,ib);
		}
		break;
	default:
		TRACE("<error> %s : %d Line 意外的命令字 SubCmd = %d !", __FUNCTION__,__LINE__,Req.m_SubCmd);
		break;
	}
	
}

		//增加战斗
void CombatServer::AddCombat(ICombat * pCombat)
{
	m_mapCombat[pCombat->GetCombatID()] = pCombat;
}

	//删除战斗
 void CombatServer::DeleteCombat(ICombat * pCombat)
 {
	 m_mapCombat.erase(pCombat->GetCombatID());

	 m_listDelCombat.push_back(pCombat); //需要延时删除
 }

 		//结束战斗
void CombatServer::EndCombat(UINT64 CommbatID,enCombatResult CombatResult)
{
	MAP_COMBAT::iterator it = m_mapCombat.find(CommbatID);

	if(it == m_mapCombat.end())
	{
		return;
	}

	ICombat * pCombat = (*it).second;

	pCombat->EndCombat(CombatResult);

}

void CombatServer::AckAction(IActor *pActor,UINT64 CombatID,IBuffer & ib)
{
	SCombatActionAck Req;


	ib >> Req;
	if(ib.Error())
	  {
		       TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		        return ;
	  }

	MAP_COMBAT::iterator it = m_mapCombat.find(CombatID);

	if(it == m_mapCombat.end())
	{
		//TRACE("<error> %s : %d line 战斗不存在！",__FUNCTION__,__LINE__,CombatID);
		return;
	}

	if(pActor->GetThingClass() != enThing_Class_Actor)
	{
		return ;
	}

	ICombat * pCombat = (*it).second;

	pCombat->AckAction(pActor,Req);
}

void CombatServer::ManualMagic(IActor *pActor,UINT64 CombatID,IBuffer & ib)
{
	CS_CombatFireMagic_Req FireMagicReq;


	ib >> FireMagicReq;
	if(ib.Error())
	  {
		       TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		        return ;
	  }

	MAP_COMBAT::iterator it = m_mapCombat.find(CombatID);

	if(it == m_mapCombat.end())
	{
		//TRACE("<error> %s : %d line 战斗不存在！",__FUNCTION__,__LINE__,CombatID);
		return;
	}

	if(pActor->GetThingClass() != enThing_Class_Actor)
	{
		return ;
	}

	ICombat * pCombat = (*it).second;

	pCombat->ManualMagic(pActor,FireMagicReq);
}


//挑战副本中的怪物
void  CombatServer::OnCombatNpcInFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_CombatCombatNpcInFuBen_Req Req;

	ib >> Req;

	//Req.m_CombatIndexMode = enCombatIndexMode_Team;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pFuBenPart->CombatNpc(Req.m_FuBenID,Req.m_uidNpc, Req.m_CombatIndexMode);
}

//离开副本
void CombatServer::OnLeaveFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_CombatLeaveFuBen_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pFuBenPart->LeaveFuBen(Req.m_FuBenID);
}
