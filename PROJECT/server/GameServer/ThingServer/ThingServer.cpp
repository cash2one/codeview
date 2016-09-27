
#include "IActor.h"
#include "ThingServer.h"
#include "GameWorld.h"
#include "IConfigServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "EquipPart.h"
#include "IResOutputPart.h"
#include "XDateTime.h"
#include "IMagicPart.h"
#include "IMagic.h"
#include "ITaskPart.h"
#include "IFuMoDongPart.h"
#include "UniqueIDGenerator.h"
#include "ICDTime.h"
#include "ISyndicateMember.h"
#include "ICDTimerPart.h"
#include "ITrainingHallPart.h"
#include "IFuBenPart.h"
#include "IDouFaPart.h"
#include "ITalismanPart.h"

IGameServer * g_pGameServer = 0;
ThingServer * g_pThingServer = 0;

 IThingServer * CreateThingServer(IGameServer *pGameServer)
{
	g_pGameServer = pGameServer;

	ThingServer * pThingServer = new ThingServer();
	if(pThingServer->Create()==false)
	{
		return 0;
	}

	g_pThingServer = pThingServer;
	return pThingServer;
}

 ThingServer::ThingServer()
 {
	 m_pGameWorld = 0;
 }

ThingServer::~ThingServer()
{
}

bool  ThingServer::Create()
{
	m_pGameWorld = new GameWorld();
	if(m_pGameWorld->Create()==false)
	{
		m_pGameWorld->Release();
		return false;
	}

	if(m_BuildingMgr.Create()==false)
	{
		return false;
	}

	if(m_PacketMgr.Create()==false)
	{
		return false;
	}
	
	if(m_EquipMgr.Create()==false)
	{
		return false;
	}

	if(m_XiuLianMgr.Create()==false)
	{
		return false;
	}

	if(m_FriendMgr.Create() == false)
	{
		return false;
	}

	if(m_MailMgr.Create() == false)
	{
		return false;
	}

	if(m_MainUIMgr.Create() == false)
	{
		return false;
	}

	if(m_TalismanMgr.Create(m_pGameWorld)==false)
	{
		return false;
	}

	if(m_TaskMgr.Create()==false)
	{
		return false;
	}

	if( m_DouFaMgr.Create() == false)
	{
		return false;
	}

	if(m_ChengJiuMgr.Create()==false)
	{
		return false;
	}

	if(m_ActivityMgr.Create()==false)
	{
		return false;
	}

	if( m_WebInterface.Create() == false)
	{
		return false;
	}

	if ( m_TeamMgr.Create() == false)
	{
		return false;
	}

	if ( m_MiJingMgr.Create() == false)
	{
		return false;
	}

	//GM命令，设置等级
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_SetLv,this)==false)
	{
	}

	//GM命令，设置部分属性
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Att,this)==false)
	{
	}

	//GM命令，设置当前帮派等级
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_SynProp,this)==false)
	{
	}

	//GM命令，设置仙剑灵气
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Sk,this)==false)
	{
	}

	//GM命令，资源快速成熟
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Cmprs,this)==false)
	{
	}

	//GM命令，玩家学会法术
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_StudyMagic,this)==false)
	{
	}

	//GM命令，玩家法术设置等级
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_MagicLvUp,this)==false)
	{
	}
	//GM命令，玩家得到任务
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_GetTask,this)==false)
	{
	}
	//GM命令，玩家完成任务
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_FinishTask,this)==false)
	{
	}
	//GM命令，清除今天挂机加速使用次数
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ClearAccellNum,this)==false)
	{
	}
	//GM命令，设置物品属性
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ChangeGoodsProp,this) == false)
	{
	}
	//GM命令,清除玩家所有CD时间
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ClearCDTime,this) == false)
	{
	}

	//GM命令,设置服务器时间
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_SetTime,this) == false)
	{
	}

	//GM命令,获取服务器时间
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_GetTime,this) == false)
	{
	}

	//GM命令，更新练功堂次数限制
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ClearTrainNum, this) == false)
	{
	}

	//GM命令，清除副本次数限制
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ClearFuBenNum, this) == false)
	{
	}

	//清除挑战次数
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_ChallengeNum, this) == false)
	{
	}

	//设置VIP等级
	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_SetVipLevel, this) == false)
	{
	}
	return true;
}

void ThingServer::Close()
{
	m_BuildingMgr.Close();

	m_PacketMgr.Close();
	m_EquipMgr.Close();
	m_XiuLianMgr.Close();
	m_FriendMgr.Close();
	m_MailMgr.Close();
	m_MainUIMgr.Close();
	m_TalismanMgr.Close();
	m_ChengJiuMgr.Close();
	m_ActivityMgr.Close();

	if(m_pGameWorld)
	{
		m_pGameWorld->Release();
		m_pGameWorld = 0;
	}

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_SetLv,this);

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_Att,this);

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_SynProp,this);
}

		//释放
void  ThingServer::Release(void)
{	
	delete this;
}

IGameWorld *  ThingServer::GetGameWorld()
{
	return m_pGameWorld;
}


//获得装备具有的属性值，为零表示不具有该属性
INT32 ThingServer::GetEquipProp(TGoodsID GoodsID,enEquipProp PropID)
{
	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);
	if(0==pGoodsCnfg)
	{
		return 0;
	}

	switch(PropID)
	{
	case enEquipProp_Spirit:   //灵力
		return pGoodsCnfg->m_SpiritOrMagic;
		break;
	case enEquipProp_Shield:  //护盾
		return pGoodsCnfg->m_ShieldOrWuXing;
		break;
	case enEquipProp_BloodUp:  //气血
		return pGoodsCnfg->m_BloodOrSwordkee;
		break;
	case enEquipProp_Avoid:   //身法
		return pGoodsCnfg->m_AvoidOrSwordLvMax;
		break;
	default:
		break;
	}

	return 0;

}

XiuLianMgr & ThingServer::GetXiuLianMgr()
{
	return m_XiuLianMgr;
}

BuildingMgr & ThingServer::GetBuildingMgr()
{
	return m_BuildingMgr;
}

//装载玩家入内存
bool  ThingServer::LoadActor(UID uidActor)
{
	return true;
}

TalismanMgr & ThingServer::GetTalismanMgr()
{
	return m_TalismanMgr;
}

TaskMgr & ThingServer::GetTaskMgr()
{
	return m_TaskMgr;
}

FriendMgr & ThingServer::GetFriendMgr()
{
	return m_FriendMgr;
}

MailMgr &	ThingServer::GetMailMgr()
{
	return m_MailMgr;
}

DouFaMgr &  ThingServer::GetDouFaMgr()
{
	return m_DouFaMgr;
}

TeamMgr &   ThingServer::GetTeamMgr()
{
	return m_TeamMgr;
}

MainUIMgr &   ThingServer::GetMainUIMgr()
{
	return m_MainUIMgr;
}

ActivityMgr & ThingServer::GetActivityMgr()
{
	return m_ActivityMgr;
}

void ThingServer::OnHandleGMCmd(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam)
{
	switch(Cmd)
	{
	case enGMCmd_SetLv:
		{
			this->OnHandleGMCmdSetLv(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_Att:
		{
			this->OnHandleGMCmdSetAtt(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_SynProp:
		{
			this->OnHandleGMCmdSetSynProp(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_Sk:
		{
			this->OnHandleGMCmdSetSK(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_Cmprs:
		{
			this->OnHandleGMCmdSetCmprs(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_StudyMagic:
		{
			this->OnHandleGMCmdStudyMagic(pActor, Cmd, vectParam); 
		}
		break;
	case enGMCmd_MagicLvUp:
		{
			this->OnHandleGMCmdMagicLvUp(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_GetTask:
		{
			this->OnHandleGMCmdGetTask(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_FinishTask:
		{
			this->OnHandleGMCmdFinishTask(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ClearAccellNum:
		{
			this->OnHandleGMCmdClearAccellNum(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ChangeGoodsProp:
		{
			this->OnHandleGMCmdChangeGoodsProp(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ClearCDTime:
		{
			this->OnHandleGMCmdClearCDTime(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_SetTime:
		{
			this->OnHandleGMCmdSetTime(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_GetTime:
		{
			this->OnHandleGMCmdGetTime(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ClearTrainNum:
		{
			this->OnHandleGMClearTrainNum(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ClearFuBenNum:
		{
			this->OnHandleGMClearFuBenNum(pActor, Cmd, vectParam);
		}
		break;
	case enGMCmd_ChallengeNum:
		{
		}
		break;
	case enGMCmd_SetVipLevel:
		{
			this->OnHandleGMCmdVipLevel(pActor, Cmd, vectParam);
		}
		break;
	default:
		break;
	}
}

void ThingServer::OnHandleGMCmdSetLv(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//设置成的等级
	INT32 Level = 0;

	//设置的对象名字
	char szName[THING_NAME_LEN] = {'\0'};

	//参数数量
	INT32 ParamNum = vectParam.size();

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( ParamNum > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],Level)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	if( ParamNum > 1){
		strncpy(szName, vectParam[1].c_str(), THING_NAME_LEN);
	}

	if( Level > 60){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("不成设成高于60级的！！", pActor);
		return;
	}

	IActor * pTargetActor = 0;

	if( szName[0] == '\0'){
		//默认对玩家设置等级
		pTargetActor = pActor;
		pTargetActor->SetCrtProp(enCrtProp_Level, Level);
	}else{
		//找出对哪个角色进行等级设置
		if( 0 == strcmp(pActor->GetName(), szName)){
			pActor->SetCrtProp(enCrtProp_Level, Level);
		}else{
			for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
			{
				pTargetActor = pActor->GetEmployee(i);
				if( 0 == pTargetActor){
					continue;
				}

				if( 0 == strcmp(pTargetActor->GetName(), szName)){
					pTargetActor->SetCrtProp(enCrtProp_Level, Level);
					break;
				}
			}
		}
	}

	if(pTargetActor == 0)
	{
		TRACE("<error> %s : %d line 不存在的目标[%s]!",__FUNCTION__,__LINE__,szName);

		sprintf_s(szTip, sizeof(szTip), "不存在的目标[%s]", szName);

		g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
		return ;
	}

	sprintf_s(szTip, sizeof(szTip), "%s成功设置成%d级", pTargetActor->GetName(), Level);
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

void ThingServer::OnHandleGMCmdSetAtt(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要设置的属性类型
	UINT8 AttType = 0;
	
	//要设置的属性值
	INT32  AttNum	  = 0;

	//设置的对象名字
	char szName[THING_NAME_LEN] = {'\0'};

	//参数数量
	INT32 ParamNum = vectParam.size();

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( ParamNum > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],AttType)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}

	if( ParamNum > 1){
		if( false == StringUtil::StrToNumber(vectParam[1],AttNum)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}

	//if( enAttType_Max <= AttType ||  0 >= AttType || 0 > AttNum){
	//	g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
	//	return;
	//}

	if( ParamNum > 2){
		strncpy(szName, vectParam[2].c_str(), THING_NAME_LEN);
	}

	IActor * pTargetActor = 0;
	if( szName[0] == '\0'){
		pTargetActor = pActor;
	}else{
		//找出对哪个角色进行属性设置
		if( 0 == strcmp(pActor->GetName(), szName)){
			pTargetActor = pActor;
		}else{
			IActor * pTmpActor = 0;
			for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
			{
				pTmpActor = pActor->GetEmployee(i);
				if( 0 == pTmpActor){
					continue;
				}

				if( 0 == strcmp(pTmpActor->GetName(), szName)){
					pTargetActor = pTmpActor;
					break;
				}
			}
		}
	}
	
	if( 0 == pTargetActor){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("获取不到该角色，检查输入的名字是否正确！！", pActor);
		return;
	}

	switch( (enAttType)AttType)
	{
	case enAttType_Spirit:
		{
			pTargetActor->SetCrtProp(enCrtProp_ForeverSpirit, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置基本灵力为%d", pTargetActor->GetName(), AttNum);
		}
		break;
	case enAttType_Shield:
		{
			pTargetActor->SetCrtProp(enCrtProp_ForeverShield, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置基本护盾为%d", pTargetActor->GetName(), AttNum);		
		}
		break;
	case enAttType_Blood:
		{
			pTargetActor->SetCrtProp(enCrtProp_ForeverBloodUp, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置基本气血上限为%d", pTargetActor->GetName(), AttNum);		
		}
		break;
	case enAttType_Avoid:
		{
			pTargetActor->SetCrtProp(enCrtProp_ForeverAvoid, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置基本身法为%d", pTargetActor->GetName(), AttNum);		
		}
		break;
	case enAttType_ActorNimbus:
		{
			INT32 NimbusUp = pTargetActor->GetCrtProp(enCrtProp_ActorNimbusUp);
			AttNum = ( AttNum > NimbusUp ? NimbusUp : AttNum);
			pTargetActor->SetCrtProp(enCrtProp_ActorNimbus, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置灵气为%d", pTargetActor->GetName(), AttNum);
		}
		break;
	case enAttType_Exp:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorExp, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置经验为%d", pTargetActor->GetName(), AttNum);		
		}
		break;
	case enAttType_Honor:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorHonor, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置荣誉为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_Aptitude:
		{
			if( AttNum < 0){
				sprintf_s(szTip, sizeof(szTip), "资质必须大于0");
			}else if( AttNum > 2500){
				sprintf_s(szTip, sizeof(szTip), "超出资质上限");
			}else{
				pTargetActor->SetCrtProp(enCrtProp_ForeverAptitude, AttNum);
				sprintf_s(szTip, sizeof(szTip), "%s成功设置资质为%0.01f", pTargetActor->GetName(), AttNum / 1000.0f);
			}	
		}
		break;
	case enAttType_Credit:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorCredit, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置声望为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_PolyNimbus:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorPolyNimbus, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置聚灵气为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_GhostSoul:
		{
			pTargetActor->SetCrtProp(enCrtProp_GhostSoul, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置灵魄为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_ActorCrit:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorCrit, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置爆击为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_ActorTenacity:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorTenacity, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置坚韧为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_ActorHit:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorHit, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置命中为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_ActorDodge:
		{
			pTargetActor->SetCrtProp(enCrtProp_ActorDodge, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置回避为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_MagicCD:
		{
			pTargetActor->SetCrtProp(enCrtProp_MagicCD, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置法术回复为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_GoldDamage:
		{
			pTargetActor->SetCrtProp(enCrtProp_GoldDamage, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置金剑诀伤害为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_WoodDamage:
		{
			pTargetActor->SetCrtProp(enCrtProp_GoldDamage, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置木剑诀伤害为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_WaterDamage:
		{
			pTargetActor->SetCrtProp(enCrtProp_GoldDamage, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置水剑诀伤害为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_FireDamage:
		{
			pTargetActor->SetCrtProp(enCrtProp_GoldDamage, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置火剑诀伤害为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	case enAttType_SoilDamage:
		{
			pTargetActor->SetCrtProp(enCrtProp_GoldDamage, AttNum);
			sprintf_s(szTip, sizeof(szTip), "%s成功设置土剑诀伤害为%d", pTargetActor->GetName(), AttNum);	
		}
		break;
	default:
		{
			if ( AttType <= enCrtProp_Actor || AttType > enCrtProp_Actor_End )
			{
				sprintf_s(szTip, sizeof(szTip), "GM命令输入的属性类型有错,类型为%d", AttType);
			}
			else
			{
				pTargetActor->SetCrtProp((enCrtProp)AttType, AttNum);
			}
		}
		break;
	}

	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

void ThingServer::OnHandleGMCmdSetSynProp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//设置成的帮派等级
	/*PropID: 1,等级，2为贡献，3为经验*/
	UINT8 PropID = 0;

	INT32 Value = 0;

	if( vectParam.size() > 1){
		if( false == StringUtil::StrToNumber(vectParam[0],PropID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}

		if( false == StringUtil::StrToNumber(vectParam[1],Value)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	if( 1 > PropID){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误,输入的帮派等级不能小于1！！", pActor);
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	ISyndicate * pSyn = pSynMgr->GetSyndicate(pActor->GetUID());
	if( 0 == pSyn){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("您目前没有帮派！！", pActor);
		return;
	}

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( PropID == 1){
		//设置帮派等级
		if( g_pGameServer->GetConfigServer()->GetSyndicateCnfg(Value) == 0){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("设置的帮派等级超过上限", pActor);
			return;
		}

		if( pSyn->SetSynLevel(Value)){
			sprintf_s(szTip, sizeof(szTip), "成功设置帮派等级为%d", Value);
		}
	}else if( PropID == 2){
		//贡献
		ISyndicateMember * pSynMember = pSynMgr->GetSyndicateMember(pActor->GetUID());
		if( 0 == pSynMember){
			return;
		}

		pSynMember->AddContribution(-pSynMember->GetContribution());
		pSynMember->AddContribution(Value);
		sprintf_s(szTip, sizeof(szTip), "成功设置帮派贡献为%d", Value);
	}else if( PropID == 3){
		//经验
		pSyn->AddSynExp(-pSyn->GetSynExp());
		pSyn->AddSynExp(Value);

		sprintf_s(szTip, sizeof(szTip), "成功设置帮派经验为%d", Value);
	}

	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

void ThingServer::OnHandleGMCmdSetSK(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要给仙剑设置的灵气值
	INT32 nNimbus = 0;

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],nNimbus)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	if( 0 > nNimbus){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误,输入的灵气值不能小于0！！", pActor);
		return;
	}

	INT32 nMaxGodSwordNimbus = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;

	if( nNimbus > nMaxGodSwordNimbus){
		pActor->SetCrtProp(enCrtProp_ActorGodSwordNimbus, nMaxGodSwordNimbus);
		sprintf_s(szTip, sizeof(szTip), "GM命令增加仙剑灵气%d！！", nMaxGodSwordNimbus);
	}else{
		pActor->SetCrtProp(enCrtProp_ActorGodSwordNimbus, nNimbus);
		sprintf_s(szTip, sizeof(szTip), "GM命令增加仙剑灵气%d！！", nNimbus);
	}

	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

void ThingServer::OnHandleGMCmdSetCmprs(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	enBuildingType  BuildType = enBuildingType_Max;
	if( vectParam.size() > 0){
		if( 0 == strcmp("灵兽园", vectParam[0].c_str())){
			BuildType = enBuildingType_Beast;
		}else if( 0 == strcmp("百草园", vectParam[0].c_str())){
			BuildType = enBuildingType_Grass;
		}else if( 0 == strcmp("灵石山", vectParam[0].c_str())){
			BuildType = enBuildingType_Stone;
		}
	}

	if( enBuildingType_Max == BuildType){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if( 0 == pResOutputPart){
		return;
	}

	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//把上次获取时间设置成现在时间减去间隔时间
	pResOutputPart->SetLastTakeResTime(BuildType, CURRENT_TIME() - ConfigParam.m_BuildingResOutputInterval + 60);

	sprintf_s(szTip, sizeof(szTip), "%s的资源已熟！！", vectParam[0].c_str());
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

//GM命令，学会法术
void ThingServer::OnHandleGMCmdStudyMagic(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要学的法术ID
	TMagicID nMagicID = 0;

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],nMagicID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	IMagicPart * pMagicPart = pActor->GetMagicPart();
	if( 0 == pMagicPart){
		return;
	}

	if( 0 !=  pMagicPart->GetMagic(nMagicID)){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("已学会该法术，不可再学！！", pActor);
		return;
	}

	IMagic * pMagic = pMagicPart->StudyMagic(nMagicID);

	if( 0 == pMagic){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("法术学习失败！！", pActor);
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("法术学习成功！！", pActor);
	}
}
//GM命令，设置法术等级
void ThingServer::OnHandleGMCmdMagicLvUp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要设置的法术ID
	TMagicID nMagicID = 0;

	//要给法术设置的等级
	UINT8 level = 0;

	if( vectParam.size() > 1){
		if( false == StringUtil::StrToNumber(vectParam[0],nMagicID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}

		if( false == StringUtil::StrToNumber(vectParam[1],level)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	IMagicPart * pMagicPart = pActor->GetMagicPart();
	if( 0 == pMagicPart){
		return;
	}

	if( false == pMagicPart->SetMagicLevel(nMagicID, level)){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("法术设置等级失败！！", pActor);
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("法术设置等级成功！！", pActor);
	}
}

//GM命令，得到任务
void ThingServer::OnHandleGMCmdGetTask(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要得到的任务ID
	TTaskID TaskID = 0;

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],TaskID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}
	
	ITaskPart * pTaskPart = pActor->GetTaskPart();
	if( 0 == pTaskPart){
		return;
	}

	if( 0 == pTaskPart->GMCreateTask(TaskID)){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("获取任务失败！！", pActor);	
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("获取任务成功！！", pActor);	
	}
}

//GM命令，完成任务
void ThingServer::OnHandleGMCmdFinishTask(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要完成的任务ID
	TTaskID TaskID = 0;

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],TaskID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}
	
	ITaskPart * pTaskPart = pActor->GetTaskPart();
	if( 0 == pTaskPart){
		return;
	}

	if( false == pTaskPart->GMFinishTask(TaskID)){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("完成任务失败！！", pActor);	
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("完成任务成功！！", pActor);	
	}	
}

//GM命令，清除今天挂机加速使用次数
void ThingServer::OnHandleGMCmdClearAccellNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if( 0 == pFuMoDongPart){
		return;
	}

	pFuMoDongPart->ClearAccellNum();

	g_pGameServer->GetGameWorld()->WorldSystemMsg("成功清除挂机加速使用次数！！", pActor);
}

//GM命令，改变物品属性
void ThingServer::OnHandleGMCmdChangeGoodsProp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//要设置哪个位置的装备
	UINT8 Pos	= 0;

	//要设置的物品属性ID
	UINT8 PropID = 0;

	//要设置的值
	INT32 Value = 0;

	if( vectParam.size() > 2){
		if( false == StringUtil::StrToNumber(vectParam[0],Pos)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}

		if( false == StringUtil::StrToNumber(vectParam[1],PropID)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}

		if( false == StringUtil::StrToNumber(vectParam[2],Value)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}else{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	IEquipment * pEquipment = pEquipPart->GetEquipByPos(Pos);
	if( 0 == pEquipment){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	switch(pEquipment->GetGoodsClass())
	{
	case enGoodsCategory_Equip:
		{
			if( PropID <= enGoodsProp_Equip || PropID >= enGoodsProp_EquipEnd){
				g_pGameServer->GetGameWorld()->WorldSystemMsg("装备没此属性ID！！", pActor);
				return;
			}

			pEquipment->SetPropNum(pActor, (enGoodsProp)PropID, Value);
		}
		break;
	case enGoodsCategory_GodSword:
		{
			if( PropID <= enGoodsProp_GodSword || PropID >= enGoodsProp_GodSwordEnd){
				g_pGameServer->GetGameWorld()->WorldSystemMsg("仙剑没此属性ID！！", pActor);
				return;
			}

			pEquipment->SetPropNum(pActor, (enGoodsProp)PropID, Value);
		}
		break;
	case enGoodsCategory_Talisman:
		{
			if( PropID <= enGoodsProp_Talisman || PropID >= enGoodsProp_TalismanEnd){
				g_pGameServer->GetGameWorld()->WorldSystemMsg("法宝没此属性ID！！", pActor);
				return;
			}

			pEquipment->SetPropNum(pActor, (enGoodsProp)PropID, Value);
		}
		break;
	default:
		g_pGameServer->GetGameWorld()->WorldSystemMsg("失败！！", pActor);
		break;
	}

	g_pGameServer->GetGameWorld()->WorldSystemMsg("成功！！", pActor);
}

//清除玩家所在CD时间
void ThingServer::OnHandleGMCmdClearCDTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	ICDTimerPart * pCDTimePart = pActor->GetCDTimerPart();
	if( 0 == pCDTimePart){
		return;
	}

	pCDTimePart->UnLoadAllCDTime();
}

//清除练功堂次数限制
void ThingServer::OnHandleGMClearTrainNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	ITrainingHallPart * pTrainPart = pActor->GetTrainingHallPart();
	if( 0 == pTrainPart){
		return;
	}

	pTrainPart->ClearTrainNum();
}

//清除副本次数限制
void ThingServer::OnHandleGMClearFuBenNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if( 0 == pFuBenPart){
		return;
	}

	pFuBenPart->GMCmdClearFuBenNumLimit();

	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	pTalismanPart->ClearTWEnterNum();
}

//设置服务器时间
void ThingServer::OnHandleGMCmdSetTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	time_t time = ::time(0);
	tm Tm = {0};

	::localtime_s(&Tm,&time);

	Tm.tm_mday = 1;
	Tm.tm_mon = 0;
	Tm.tm_hour = 0;
	Tm.tm_min = 0;
	Tm.tm_sec = 0;

	INT32 Value = 0;

	if(vectParam.size()==0)
	{
		XDateTime::GetInstance().ResetVirtualTime();
	}
	else
	{
		if(vectParam.size()>0)
	   {
		 StringUtil::StrToNumber(vectParam[0],Value);

		Tm.tm_mon = Value-1;
	   }

		if(vectParam.size()>1)
	   {
		 StringUtil::StrToNumber(vectParam[1],Value);

		Tm.tm_mday = Value;
	   }

		if(vectParam.size()>2)
	   {
		 StringUtil::StrToNumber(vectParam[2],Value);

		Tm.tm_hour = Value;
	   }

		if(vectParam.size()>3)
	   {
		 StringUtil::StrToNumber(vectParam[3],Value);

		Tm.tm_min = Value;
	   }

		time = ::mktime(&Tm);

		if(time==0)
		{
			g_pGameServer->GetGameWorld()->WorldSystemMsg("无效的时间值!", pActor);
			return;
		}

		XDateTime::GetInstance().SetVirtualTime(time);
	}

	SendServerTimeToClient(pActor);
}

//发送服务器时间给客户端
void ThingServer::SendServerTimeToClient(IActor * pActor)
{
	time_t CurrTime = CURRENT_TIME();

	tm Tm = {0};
	::localtime_s(&Tm,&CurrTime);

	char szTemp[100]={0};

	sprintf_s(szTemp,ARRAY_SIZE(szTemp),"服务器时间： %d 月 %d 日 %.2d : %.2d : %.2d ",Tm.tm_mon+1,Tm.tm_mday,Tm.tm_hour,Tm.tm_min,Tm.tm_sec);

	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTemp, pActor);	
}

		//获得服务器时间
void ThingServer::OnHandleGMCmdGetTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	SendServerTimeToClient(pActor);	                  
}

//清除挑战次数
void ThingServer::OnHandleGMCmdChallengeNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	pDouFaPart->ClearChallengeNum();
}

//判断是否满足条件
struct JudgeCondition
{
	template <int index,typename type>
	bool operator ()(Int2Type<index>,typename type & Value,const std::vector<INT32> & vectParam)const
	{
		if((index > vectParam.size()) 
			|| (vectParam[index-1] == INVALID_TASK_PARAM) 
			|| (vectParam[index-1] == (int)Value.GetValue(INT2TYPE(index))))
		{
			return true;
		}

		return false;
	}
};

//判断指定事件现场是否与向量一一毕配
bool ThingServer::IsEqual(XEventData & EventData,UINT16 EeventID,const std::vector<INT32> & vectParam)
{

	if(EventData.m_pContext==0)
	{
				TRACE("<warming> %s : %d line 事件现场指针为空!",__FUNCTION__,__LINE__);
				return false ;
	}

            #define YASK_CMD_TO_TYPE(cmd,type)  \
                    case cmd :                  \
			        {                            \
			            if(EventData.m_len<SIZE_OF(type()))         \
			            {                                            \
				             TRACE("<warming> %s : %d line 事件现场长度有误!",__FUNCTION__,__LINE__);     \
				             return false;                                                                       \
			            }                                                                                   \
						if(ForEach(*(type *)EventData.m_pContext,JudgeCondition(),vectParam)==false) { return false ;} \
			        }        \
			        break;    \

			
			switch(EeventID)
			{
              TASK_ALL_CMD_TO_TYPE
			default:
				return false;
			}

			return true;

}

//接收后台发来的消息
void ThingServer::WebOnRecv(UINT8 nCmd, IBuffer & ib)
{
	m_WebInterface.OnRecv(nCmd, ib);
}

//设置VIP等级
void ThingServer::OnHandleGMCmdVipLevel(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam)
{
	//Vip等级
	UINT8 VipLevel	= 0;

	if ( vectParam.size() <= 0){
		
		g_pGameServer->GetGameWorld()->WorldSystemMsg("请填写要设置的VIP等级！！", pActor);
		return;
	}

	if ( false == StringUtil::StrToNumber(vectParam[0],VipLevel)){

		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	const SVipConfig * pVipConfig = g_pGameServer->GetConfigServer()->GetVipConfig(VipLevel);

	if ( VipLevel != 0 && 0 == pVipConfig){
		
		g_pGameServer->GetGameWorld()->WorldSystemMsg("设置的VIP等级有错！！", pActor);
		return;
	}

	if ( pActor->SetCrtProp(enCrtProp_VipLevel, VipLevel)){
		
		g_pGameServer->GetGameWorld()->WorldSystemMsg("VIP等级设置成功！！", pActor);
	}
}
