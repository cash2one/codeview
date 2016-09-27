
#include "IGameWorld.h"
#include "ActorBasicPart.h"
#include "IActor.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IGoodsServer.h"
#include "IGodSword.h"
#include "XDateTime.h"
#include "IBasicService.h"

#include "IGameScene.h"
#include "IMonster.h"
#include "ICombatServer.h"
#include "ICombatPart.h"
#include "ISystemMsg.h"
#include "IResOutputPart.h"
#include "IEquipPart.h"

ActorBasicPart::ActorBasicPart() 
{
	m_pActor = 0;

	m_bFirstTimer = true;
}

ActorBasicPart::~ActorBasicPart()
{
}



//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool ActorBasicPart::Create(IThing *pMaster, void *pContext, int nLen) 
{
	m_pActor = (IActor*)pMaster;

	if( m_pActor->GetMaster() == 0)
	{
		UINT8 WallowSwitch = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_WallowSwitch ; ///防沉迷开关,0关闭,1打开

		if( 0 == WallowSwitch){
			return true;
		}

		INT32 LoginTime = m_pActor->GetCrtProp(enCrtProp_ActorLogin);

		time_t Currtime = CURRENT_TIME();

		INT32 OnlineTime = m_pActor->GetCrtProp(enCrtProp_ActorOnlineTimes);

		if(LoginTime != 0)
		{
			//if( (Currtime - LoginTime) > g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ClearWallow_OfflineTimeNum){
			//	//下线满指定时间，防沉迷清0
			//	m_pActor->SetCrtProp(enCrtProp_ActorOnlineTimes, 0, &OnlineTime);
			//}else{
				m_pActor->AddCrtPropNum(enCrtProp_ActorOnlineTimes,Currtime-LoginTime,&OnlineTime);
			//}
		}

		m_pActor->SetCrtProp(enCrtProp_ActorLogin,Currtime);

		//少于18岁的有防沉迷提示
		if( enIdentityStatus_LessEighteen == m_pActor->GetCrtProp(enCrtProp_ActorIdentity)/* || enIdentityStatus_Non == m_pActor->GetCrtProp(enCrtProp_ActorIdentity)先屏蔽*/){
			//定时器时长
			UINT32 nTimeNum = 0;

			if( OnlineTime < 3600 * 3){
				//3小时内，每小时提示一次
				nTimeNum = 3600 - OnlineTime % 3600;
			}else{
				//大于等于3小时，每半小时提示一次
				nTimeNum = 1800 - OnlineTime % 1800;
			}
			

			this->StartTimer(enActorBasicTimerID_OnLineTime, nTimeNum);	
		}	
	}

	return true;
}

//释放
void ActorBasicPart::Release(void) 
{
	delete this;
}

//取得部件ID
enThingPart ActorBasicPart::GetPartID(void)
{
	return enThingPart_Crt_Basic;
}

//取得本身生物
IThing*		ActorBasicPart::GetMaster(void) 
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool ActorBasicPart::OnGetDBContext(void * buf, int &nLen) 
{
	nLen = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void ActorBasicPart::InitPrivateClient()
{
	if ( m_pActor->GetMaster() != 0){

		return;
	}

	//发送给客户端玩家的设置信息
	SC_UserSet UserSet;	

	IResOutputPart * pResPart = m_pActor->GetResOutputPart();

	if ( 0 == pResPart){
		
		return;
	}

	UserSet.m_bOpenAutoTakeRes = pResPart->GetbOpenAutoTakeRes();

	OBuffer1k ob;
    ob << SMainUIHeader(enMainUICmd_SC_UserSet,SIZE_OF(UserSet)) << UserSet;
    m_pActor->SendData(ob.TakeOsb());

	//////
	UINT8 WallowSwitch = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_WallowSwitch ; ///防沉迷开关,0关闭,1打开

	if( 0 == WallowSwitch){
		return;
	}

	//在线时长
	UINT32 onLineTime = m_pActor->GetCrtProp(enCrtProp_ActorOnlineTimes) + CURRENT_TIME() - m_pActor->GetCrtProp(enCrtProp_ActorLogin);

	SC_OnlineTime_Sync Rsp;
	Rsp.m_OnlineTime = onLineTime;
	Rsp.m_IdentityStatus = (enIdentityStatus)m_pActor->GetCrtProp(enCrtProp_ActorIdentity);

	ob.Reset();
    ob << SMainUIHeader(enMainUICmd_OnlineTime,SIZE_OF(Rsp)) << Rsp;
    m_pActor->SendData(ob.TakeOsb());
}


//玩家下线了，需要关闭该ThingPart
void ActorBasicPart::Close()
{
	g_pGameServer->GetTimeAxis()->KillTimer(enActorBasicTimerID_OnLineTime, this);
}

//保存数据
void ActorBasicPart::SaveData() 
{
}

void ActorBasicPart::OnTimer(UINT32 timerID)
{
	if( timerID == enActorBasicTimerID_OnLineTime)
	{
		//在线时长
		UINT32 onLineTime = m_pActor->GetCrtProp(enCrtProp_ActorOnlineTimes) + CURRENT_TIME() - m_pActor->GetCrtProp(enCrtProp_ActorLogin);

		g_pGameServer->GetTimeAxis()->KillTimer(enActorBasicTimerID_OnLineTime, this);

		UINT32 nTimeNum = 0;

		if( onLineTime < 3600 * 3){
			//3小时内，每小时提示一次
			nTimeNum = 3600;
		}else{
			//大于等于3小时，每半小时提示一次
			nTimeNum = 1800;
		}

		this->StartTimer(enActorBasicTimerID_OnLineTime, nTimeNum);	

		//同步	
		SC_OnlineTime_Sync Rsp;
		Rsp.m_OnlineTime = onLineTime;
		Rsp.m_IdentityStatus = (enIdentityStatus)m_pActor->GetCrtProp(enCrtProp_ActorIdentity);

		 OBuffer1k ob;
		ob << SMainUIHeader(enMainUICmd_OnlineTime,SIZE_OF(Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());

	}
}

//属性变动
void ActorBasicPart::OnActorPropChange(IActor* pActor,enCrtProp PropID,INT32 nNewValue,INT32 nOldValue) 
{
	switch(PropID)
	{
	case enCrtProp_ActorExp:
		{
			if(pActor == m_pActor)
			{
				OnExpChange(PropID,nNewValue);
			}
		}
		break;
	case enCrtProp_Level: //升级了
		{
			if(pActor == m_pActor)
			{
				OnLevelUp(PropID,nNewValue);
			}
		}
		break;
	case enCrtProp_Blood:  //血量不能超出上限
		{
			if(pActor == m_pActor)
			{
				INT32 Boold = pActor->GetCrtProp(enCrtProp_Blood);
				INT32 BooldUp =  pActor->GetCrtProp(enCrtProp_ActorBloodUp);
				if(Boold>BooldUp)
				{
					pActor->SetCrtProp(enCrtProp_Blood,BooldUp);
				}
			}			
		}
		break;
	case enCrtProp_ActorAptitude:
		{
			if(pActor == m_pActor)
			{
				OnAptitudeChange(enCrtProp_ActorAptitude,nNewValue);
			}
		}
		break;
	case enCrtProp_ForeverAptitude:
		{
			if(pActor == m_pActor)
			{
				pActor->AddCrtPropNum(enCrtProp_ActorAptitude, nNewValue - nOldValue);
			}
		}
		break;
	case enCrtProp_ActorLayer:
		{
			//这几个属性改变需要重新计算内属性
			if(pActor == m_pActor)
			{			   
				pActor->RecalculateProp();
			}	

		}
		break;
	case enCrtProp_ForeverSpirit:
	case enCrtProp_ActorSpiritBasic:
	case enCrtProp_ActorSpiritEqup:
	case enCrtProp_Spirit:
		{
			if(pActor == m_pActor)
			{
				//灵力类属性变化
				this->OnSpiritChange(PropID, nNewValue, nOldValue);
			}
		}
		break;
	case enCrtProp_ForeverShield:
	case enCrtProp_ActorShieldBasic:
	case enCrtProp_ActorShieldEqup:
	case enCrtProp_Shield:
		{
			if(pActor == m_pActor)
			{
				//护盾类属性变化
				this->OnShieldChange(PropID, nNewValue, nOldValue);		
			}
		}
		break;
	case enCrtProp_ForeverAvoid:
	case enCrtProp_ActorAvoidBasic:
	case enCrtProp_ActorAvoidEqup:
	case enCrtProp_Avoid:
		{
			if(pActor == m_pActor)
			{
				//身法类属性变化
				this->OnAvoidChange(PropID, nNewValue, nOldValue);
			}
		}
		break;
	case enCrtProp_ForeverBloodUp:
	case enCrtProp_ActorBloodUpBasic:
	case enCrtProp_ActorBloodEqup:
		{
			if(pActor == m_pActor)
			{
				//气血上限类属性变化
				this->OnBloodUpChange(PropID, nNewValue, nOldValue);
			}
		}
		break;
	case enCrtProp_SynMagicPhysicsParam:
	case enCrtProp_SynMagicMagicParam:
	case enCrtProp_SynMagicDefendParam:
	case enCrtProp_SynMagicAloneXLParam:
		{
			//只有主角身上有的但又会影响招募角色的属性
			if( pActor == m_pActor)
			{
				pActor->RecalculateProp();

				for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
				{
					IActor * pEmployee = pActor->GetEmployee(i);
					if( 0 == pEmployee){
						continue;
					}

					pEmployee->RecalculateProp();
				}
			}
		}
		break;
	case enCrtProp_ForeverActorNimbusSpeed:
		{
			if(pActor == m_pActor)
			{
				//灵气速率属性变化
				pActor->AddCrtPropNum(enCrtProp_ActorNimbusSpeed, nNewValue - nOldValue);
			}
		}
		break;
	case enCrtProp_ActorCrit:
	case enCrtProp_ActorTenacity:
	case enCrtProp_ActorHit:
	case enCrtProp_ActorDodge:
		{
			if(pActor == m_pActor)
			{

				pActor->RecalculateCombatAbility();

			}
		}
		break;
	case enCrtProp_VipLevel:
		{
			if(pActor == m_pActor)
			{
				pActor->AddCrtPropNum(enCrtProp_TotalVipLevel, nNewValue - nOldValue);
			}			
		}
		break;
	//case enCrtProp_ActorCredit:
	//	{
	//		if(pActor == m_pActor)
	//		{
	//			if ( nNewValue > nOldValue )
	//			{
	//				INT32 VipAdd = pActor->GetVipValue(enVipType_AddGetCredit);

	//				if ( VipAdd > 0 )
	//				{
	//					pActor->AddCrtPropNum(enCrtProp_ActorCredit, VipAdd);
	//				}
	//			}
	//		}
	//	}
	//	break;
	default:
		break;
	}

			//得到对应于私有属性的排序编号
		INT32 ActorPropID = 0;
		
		if(PropID>enCrtProp_Actor)
		{
			ActorPropID = ActorPropMapID[PropID - enCrtProp_Actor - 1];
		}
		else
		{
			ActorPropID = CreaturePropMapID[PropID - 1];

		}

		if( -1 == ActorPropID){
			return ;
		}

		m_pActor->NoticClientUpdateThing(m_pActor->GetUID(),ActorPropID,m_pActor->GetCrtProp(PropID));
}

//资质变动
void ActorBasicPart::OnAptitudeChange(enCrtProp PropID,INT32 nNewValue)
{
		m_pActor->SetCrtProp(enCrtProp_ActorNenLi, (int)m_pActor->GetNenLi());

		//资质和等级变化时,都得重新计算部分基本属性
		m_pActor->LevelOrAptitudeChange();

		const std::vector<INT16> & vectAptitude = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_vectAptitudeFacade;

		//默认外观
		UINT16 DefaultFacadeID = m_pActor->GetEquipPart()->GetDefaultFacade();

		for(int i=0;i+1<vectAptitude.size();i += 2)
		{
			if(nNewValue >= vectAptitude[i])
			{
			
				INT16 SuitID = vectAptitude[i+1];
				UINT16 FacadeID = g_pGameServer->GetConfigServer()->GetUpgradeFacadeID(SuitID,DefaultFacadeID);
				if(FacadeID != 0)
				{
					m_pActor->GetEquipPart()->AddFacade(FacadeID);

				}
			}
			else
			{
				break;
			}
		}
}

//升级
void ActorBasicPart::OnLevelUp(enCrtProp PropID,INT32 nNewValue)
{
	//等级和资质变化时,都得重新计算部分基本属性
	m_pActor->LevelOrAptitudeChange();

	if( m_pActor->GetMaster() == 0){

		char szTip[DESCRIPT_LEN_50] = "\0";

		//发送升级事件
		SS_UpLevel UpLevel;
		UpLevel.m_Level = nNewValue;
		UpLevel.m_uidUser = m_pActor->GetUID().ToUint64();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_UpLevel);

		m_pActor->OnEvent(msgID, &UpLevel, sizeof(SS_UpLevel));

		//聊天框提示升级
		sprintf_s(szTip,sizeof(szTip),g_pGameServer->GetGameWorld()->GetLanguageStr(10088),m_pActor->GetName(),nNewValue);
		g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip,m_pActor,enTalkMsgType_UpLevel);
	}
}

//经验变动
void ActorBasicPart::OnExpChange(enCrtProp PropID,INT32 nNewValue)
{
	//等级
	UINT8 Level = m_pActor->GetCrtProp(enCrtProp_Level);

	//升到下一级
	const SActorLevelCnfg * pActorLevelCnfg = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(Level+1);

	if(pActorLevelCnfg==0)
	{
		return ;
	}

	if(nNewValue >= pActorLevelCnfg->m_NeedExp)
	{
		m_pActor->AddCrtPropNum(enCrtProp_Level,1);						//需要先加等级再扣经验，防止当经验可升多级时,获取的SActorLevelCnfg始终是相同的
		m_pActor->AddCrtPropNum(PropID,-pActorLevelCnfg->m_NeedExp);
	}

}


//获得当天进入该仙剑次数
INT32 ActorBasicPart::GetEnterGodSwordFuBenNumOfDay(IGodSword * pGodSword)
{
	time_t nCurTime = CURRENT_TIME();
	//最后进入时间
	INT32 LastEnterTime = 0;
	pGodSword->GetPropNum(enGoodsProp_LastEnterTime,LastEnterTime);

	if(LastEnterTime>0 && XDateTime::GetInstance().IsSameDay(nCurTime,nCurTime)==false)
	{
		//重置
		pGodSword->SetPropNum(m_pActor,enGoodsProp_FuBenLevel,0);
		pGodSword->SetPropNum(m_pActor,enGoodsProp_KillNpcNum,0);
		pGodSword->SetPropNum(m_pActor,enGoodsProp_LastEnterTime,0);
		return 0;
	}

	return 1;
}

//掉线，下线要做的一些事
void	ActorBasicPart::LeaveDoSomeThing()
{
//	g_pGameServer->GetTimeAxis()->KillTimer(enActorBasicTimerID_OnLineTime, this);
}

//灵力变动
void ActorBasicPart::OnSpiritChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue)
{
	switch(PropID)
	{
	case enCrtProp_ForeverSpirit:
		{
			//永久灵力改变,会影响基础灵力值
			m_pActor->AddCrtPropNum(enCrtProp_ActorSpiritBasic, nNewValue - nOldValue);
		}
		break;
	case enCrtProp_ActorSpiritEqup:
	case enCrtProp_ActorSpiritBasic:
		{
			//基础灵力和装备灵力改变,会影响总灵力值
			m_pActor->AddCrtPropNum(enCrtProp_Spirit, nNewValue - nOldValue);
		}
		break;
	case enCrtProp_Spirit:
		{
			//这个属性改变需要重新计算内属性
			m_pActor->RecalculateProp();
		}
		break;
	}
}

//护盾变动
void ActorBasicPart::OnShieldChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue)
{
	switch(PropID)
	{
	case enCrtProp_ForeverShield:
		{
			//永久护盾改变,会影响基础护盾值
			m_pActor->AddCrtPropNum(enCrtProp_ActorShieldBasic, nNewValue - nOldValue);
		}
		break;
	case enCrtProp_ActorShieldEqup:
	case enCrtProp_ActorShieldBasic:
		{
			//基础护盾和装备护盾改变,会影响总护盾值
			m_pActor->AddCrtPropNum(enCrtProp_Shield, nNewValue - nOldValue);
		}
		break;
	case enCrtProp_Shield:
		{
			//这个属性改变需要重新计算内属性
			m_pActor->RecalculateProp();
		}
		break;
	}
}

//身法变动
void ActorBasicPart::OnAvoidChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue)
{
	switch(PropID)
	{
	case enCrtProp_ForeverAvoid:
		{
			//永久身法改变,会影响基础身法值
			m_pActor->AddCrtPropNum(enCrtProp_ActorAvoidBasic, nNewValue - nOldValue);		
		}
		break;
	case enCrtProp_ActorAvoidBasic:
	case enCrtProp_ActorAvoidEqup:
		{
			//基础身法和装备身法改变,会影响总身法值
			m_pActor->AddCrtPropNum(enCrtProp_Avoid, nNewValue - nOldValue);
		}
		break;
	case enCrtProp_Avoid:
		{
			//这个属性改变需要重新计算内属性
			m_pActor->RecalculateProp();
		}
		break;
	}
}

//气血上限变动
void ActorBasicPart::OnBloodUpChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue)
{
	switch(PropID)
	{
	case enCrtProp_ForeverBloodUp:
		{
			//永久气血上限改变,会影响基础气血上限值
			m_pActor->AddCrtPropNum(enCrtProp_ActorBloodUpBasic, nNewValue - nOldValue);			
		}
		break;
	case enCrtProp_ActorBloodUpBasic:
	case enCrtProp_ActorBloodEqup:
		{
			//基础气血上限和装备气血上限改变,会影响总气血上限值
			m_pActor->AddCrtPropNum(enCrtProp_ActorBloodUp, nNewValue - nOldValue);
		}
		break;
	}
}

//定时器开始
void ActorBasicPart::StartTimer(UINT32 nTimerID, UINT32 nTimeNum)
{
	g_pGameServer->GetTimeAxis()->SetTimer(nTimerID, this, nTimeNum * 1000, "ActorBasicPart::StartTimer");
}

//定时器结束
void ActorBasicPart::StopTimer(UINT32 nTimerID)
{
	g_pGameServer->GetTimeAxis()->KillTimer(nTimerID, this);
}

//少于18岁的防沉迷提示
void ActorBasicPart::WallowTip(UINT32 nOnLineTime)
{
	//在线小时数
	UINT8 nHour = nOnLineTime / 3600;

	ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
	if( 0 == pSystemMsg){
		return;
	}

	SInsertMsg WallowMsg;

	// fly add	20121106
	switch(nHour)
	{
	case 0:
		break;
	case 1:
		
		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10002), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您累计在线时间已满1小时", sizeof(WallowMsg.szMsgBody));
		break;
	case 2:
		
		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10003), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您累计在线时间已满2小时", sizeof(WallowMsg.szMsgBody));
		break;
	case 3:
		
		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10004), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您已经进入疲劳游戏时间，您的游戏收益将降为正常值的50％，为了您的健康，请尽快下线休息，做适当身体活动，合理安排学习生活。", sizeof(WallowMsg.szMsgBody));
		break;
	case 4:

		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10004), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您已经进入疲劳游戏时间，您的游戏收益将降为正常值的50％，为了您的健康，请尽快下线休息，做适当身体活动，合理安排学习生活", sizeof(WallowMsg.szMsgBody));
		break;
	case 5:
		
		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10005), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害，您的收益已降为零，直到您的累计下线时间满5小时后，才能恢复正常", sizeof(WallowMsg.szMsgBody));
		break;
	default:

		strncpy(WallowMsg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10005), sizeof(WallowMsg.szMsgBody));
		//strncpy(WallowMsg.szMsgBody, "您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害，您的收益已降为零，直到您的累计下线时间满5小时后，才能恢复正常", sizeof(WallowMsg.szMsgBody));
		break;
	}

	pSystemMsg->ViewMsg(m_pActor, enMsgType_Insert, &WallowMsg);
}
