
#include "IActor.h"

#include "FuBenPart.h"

#include "DBProtocol.h"
#include "XDateTime.h"
#include "IGameServer.h"
#include "IConfigServer.h"
#include "TBuffer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IGameScene.h"
#include "IGameWorld.h"
#include "DMsgSubAction.h"
#include "IMonster.h"
#include "ICombatPart.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "ISyndicateMember.h"
#include "IActorBasicPart.h"
#include "IGoodsServer.h"
#include "IGodSword.h"
#include "DMsgSubAction.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "ITeamPart.h"
#include "ITaskPart.h"


FuBenPart::FuBenPart()
{
	m_pActor = 0;
	m_LastFreeEnterFuBenTime = 0;  //最后一次免费进入且记录副本进度时间
	m_FreeEnterFuBenNum = 0;       //今天免费进入次数
	m_SynWelfareEnterFuBenNum = 0; //帮派福利进入次数
	m_CostStoneEnterFuBenNum  = 0; //支付灵石进入次数

	m_SceneIDGodSward = INVALID_SCENE_ID;   //所进入的仙剑副本场景ID

	m_uidGodSword	  = UID();				//正在进入的仙剑	
	m_FuBenLevel	  = 0;
	m_EnterSynFuBenNum = 0;
	m_LastEnterSynFuBenTime = 0;
	m_EnterGodSwordWorldNum = 0;
	m_LastVipEnterFuBenTime = 0;
	m_VipEnterFuBenNum		= 0;
	m_LastSynWelfareFuBenTime = 0;
	m_FuBenMode = enFuBenMode_Single;

	m_EnterFuBenID = 0;

	//m_CommonCombatID = 0; //普通副本战斗ID
	m_GoldSwordCombatID = 0; //帮派保卫战
	m_SynGuardCombatID = 0;  //帮派保卫战
}

FuBenPart::~FuBenPart()
{
}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool FuBenPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster==0 || pContext==0 || nLen < sizeof(SDBFuBenData))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	SDBFuBenData * pDBFuBenData = (SDBFuBenData *)pContext;

	m_LastFreeEnterFuBenTime  = pDBFuBenData->m_LastFreeEnterFuBenTime;  //最后一次免费进入且记录副本进度时间
	m_LastStoneEnterFuBenTiem = pDBFuBenData->m_LastStoneEnterFuBenTime; //最后一次付灵石进入且记录副本进度时间
	m_LastSynWelfareFuBenTime = pDBFuBenData->m_LastSynWelfareEnterFuBenTime; //最后一次帮派福利进入且记录副本进度时间
	m_LastVipEnterFuBenTime	  = pDBFuBenData->m_LastVipEnterFuBenTime;		 //最后一次VIP进入副本时间
	m_FreeEnterFuBenNum       = pDBFuBenData->m_FreeEnterFuBenNum;           //今天免费进入次数
	m_SynWelfareEnterFuBenNum = pDBFuBenData->m_SynWelfareEnterFuBenNum;	//帮派福利进入次数
	m_CostStoneEnterFuBenNum  = pDBFuBenData->m_CostStoneEnterFuBenNum;		//支付灵石进入次数
	m_LastEnterFuBenGodSword  = pDBFuBenData->m_LastEnterFuBenGodSword;		//最后一次进入剑印世界时间
	m_LastEnterSynFuBenTime   = pDBFuBenData->m_LastEnterSynFuBenTime;
	m_EnterSynFuBenNum		  = pDBFuBenData->m_EnterSynFuBenNum;
	m_EnterGodSwordWorldNum	  = pDBFuBenData->m_EnterGodSwordWorldNum;
	m_VipEnterFuBenNum		  = pDBFuBenData->m_VipEnterFuBenNum;

	m_FinishedFuben.set(&pDBFuBenData->m_FinishedFuBen,SIZE_OF(pDBFuBenData->m_FinishedFuBen));
	m_HardFinishedFuben.set(&pDBFuBenData->m_HardFinishedFuBen, SIZE_OF(pDBFuBenData->m_HardFinishedFuBen));

	for(int i=0; i<pDBFuBenData->m_FuBenNum; ++i)
	{
		SFuBenProgress & FuBenProgress = pDBFuBenData->m_FuBenProgress[i];
		if(FuBenProgress.m_FuBenID == INVALID_FUBEN_ID)
		{
			continue;
		}

		SFuBenProgressInfo Info;		
		Info.m_FuBenID = FuBenProgress.m_FuBenID ;
		Info.m_KillMonsterNum = FuBenProgress.m_KillMonsterNum;
		Info.m_Level = FuBenProgress.m_Level;

		m_mapFuBen[FuBenProgress.m_FuBenID] = Info;

	}

	if( XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_LastEnterFuBenGodSword) == false)
	{
		//不是同一天,重置仙剑副本
		this->ResetGodSwordFuBen();
	}

	//注册事件,得到玩家在干什么
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);	

	m_pActor->SubscribeEvent(msgID,this,"FuBenPart::Create");

	//注册退队事件
	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitTeam);

	//m_pActor->SubscribeEvent(msgID,this,"FuBenPart::Create");

	return true;
}

//释放
void FuBenPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart FuBenPart::GetPartID(void)
{
	return enThingPart_Actor_FuBen;
}

//取得本身生物
IThing*		FuBenPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool FuBenPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SDBFuBenData))
	{
		return false;
	}

	SDBFuBenData * pDBFuBenData = (SDBFuBenData*)buf;

	pDBFuBenData->m_FreeEnterFuBenNum = GetFreeEnterFuBenNum();
	pDBFuBenData->m_CostStoneEnterFuBenNum  = GetCostStoneEnterFuBenNum();
	pDBFuBenData->m_SynWelfareEnterFuBenNum = GetSynWelfareEnterFuBenNum();
	pDBFuBenData->m_EnterSynFuBenNum  = m_EnterSynFuBenNum;
	pDBFuBenData->m_EnterGodSwordWorldNum = this->GetEnterGodSwordNum();

	pDBFuBenData->m_LastFreeEnterFuBenTime = m_LastFreeEnterFuBenTime;
	pDBFuBenData->m_LastStoneEnterFuBenTime = m_LastStoneEnterFuBenTiem;
	pDBFuBenData->m_LastSynWelfareEnterFuBenTime = m_LastSynWelfareFuBenTime;
	pDBFuBenData->m_LastEnterFuBenGodSword = m_LastEnterFuBenGodSword;
	pDBFuBenData->m_LastEnterSynFuBenTime  = m_LastEnterSynFuBenTime;
	pDBFuBenData->m_LastVipEnterFuBenTime  = m_LastVipEnterFuBenTime;
	pDBFuBenData->m_VipEnterFuBenNum	   = m_VipEnterFuBenNum;
	memcpy(&pDBFuBenData->m_FinishedFuBen, &m_FinishedFuben, sizeof(pDBFuBenData->m_FinishedFuBen));
	memcpy(&pDBFuBenData->m_HardFinishedFuBen, &m_HardFinishedFuben, sizeof(pDBFuBenData->m_HardFinishedFuBen));

	pDBFuBenData->m_FuBenNum = 0;

	for(MAP_FUBEN::iterator it = m_mapFuBen.begin(); it != m_mapFuBen.end(); it++)
	{
		SFuBenProgressInfo & Info = (*it).second;

		SFuBenProgress FuBenProgress;		
		FuBenProgress.m_FuBenID = Info.m_FuBenID;
		FuBenProgress.m_KillMonsterNum = Info.m_KillMonsterNum;
		FuBenProgress.m_Level = Info.m_Level;

		pDBFuBenData->m_FuBenProgress[pDBFuBenData->m_FuBenNum++] = FuBenProgress;
	}

	nLen = sizeof(SDBFuBenData);

	return true;
}

//进入副本次数
UINT16  FuBenPart::EnterFuBenNum()
{
	UINT16 num = GetFreeEnterFuBenNum();

	num += GetSynWelfareEnterFuBenNum();

	num += GetCostStoneEnterFuBenNum();

	num += this->GetVipEnterCommonFuBenNum();

	return num;
}

//副本最大进入次数
UINT16 FuBenPart::MaxEnterFuBenNum()
{
	const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();

	UINT16 num = ConfigParam.m_MaxFreeEnterFuBenNum;

	//判断是否有帮派福利
	ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(m_pActor->GetUID());

	if(pSyndicate)
	{		 
		num +=  pSyndicate->GetWelfareValue(enWelfare_EnterFuBenNum);
	}

	num += ConfigParam.m_MaxCostStoneEnterFuBenNum;

	num += m_pActor->GetVipValue(enVipType_AddFuBenNum);

	return num;
}

//进入仙剑副本
void FuBenPart::OnEnterSwordFuBen(UID uidGodSword,UINT8 Level)
{
	SC_MainUIChallenge_Rsp Rsp;
	Rsp.m_Result = enMainUICode_OK;
	Rsp.m_bOpenHardType = false;

	const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//首先确认玩家是否拥有该仙剑
	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,uidGodSword);

	IGodSword * pGodSword = (IGodSword*)pGoods;

	if(pGoods == 0)
	{
		Rsp.m_Result = enMainUICode_ErrNoGoods;
	}
	else if(pGoods->GetGoodsClass() != enGoodsCategory_GodSword)
	{
		Rsp.m_Result = enMainUICode_ErrNotGodSword;
	}
	else
	{
		const SGoodsCnfg * pGoodsCnfg =  pGodSword->GetGoodsCnfg();

		//剑诀等级
		INT32 SecretLevel = 0;

		pGodSword->GetPropNum(enGoodsProp_SecretLevel,SecretLevel);

		//已开启的副本级别
		INT32 FuBenLevel = 0;
		pGodSword->GetPropNum(enGoodsProp_FuBenLevel,FuBenLevel);

		//获得杀怪数量
		INT32 KillNpcNum = 0;
		pGodSword->GetPropNum(enGoodsProp_KillNpcNum,KillNpcNum);

		if ( m_pActor->HaveTeam() )
		{
			Rsp.m_Result = enMainUICode_ErrHaveTeam;
		}
		else if ( m_pActor->IsInDuoBao() )
		{
			Rsp.m_Result = enMainUICode_ErrInDuoBao;
		}
		else if ( m_pActor->IsInFastWaitTeam() )
		{
			Rsp.m_Result = enMainUICode_ErrWaitTeam;
		}
		else if( this->GetEnterGodSwordNum() >= this->GetMaxGodSwordEnterNum() && KillNpcNum <= 0)
		{
			Rsp.m_Result = enMainUICode_ErrEnterNumLimit;
		}
		else if(pGoodsCnfg == 0)
		{
			Rsp.m_Result = enMainUICode_ErrNoGoods;			
		}
		else if(pGoodsCnfg->m_SuitIDOrSwordSecretID == 0)
		{
			//没剑诀
			Rsp.m_Result = enMainUICode_ErrNotSecret;	
		}
		else if(Level > SecretLevel + 1)
		{
			Rsp.m_Result = enMainUICode_ErrLevelErr;
		}
		else
		{
			//获取仙剑世界配置信息
			const SGodSwordWorldCnfg * pSwordWorldCnfg = g_pGameServer->GetConfigServer()->GetGoldSwordWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);
			if(pSwordWorldCnfg == 0)
			{
				Rsp.m_Result = enMainUICode_ErrSwordWorldCnfg;
				TRACE("<error> %s : %d line 找不到剑印世界配置信息 SwordSecretID = %d",__FUNCTION__,__LINE__,pGoodsCnfg->m_SuitIDOrSwordSecretID);
			}
			else
			{
				//获取副本配置信息
				const SGodSwordFuBenCnfg * pSwordFuBenCnfg =  g_pGameServer->GetConfigServer()->GetGodSwordFuBenCnfg(pSwordWorldCnfg->m_FuBenID,Level);

				if(pSwordFuBenCnfg == 0)
				{
					Rsp.m_Result = enMainUICode_ErrFuBenCnfg;
					TRACE("<error> %s : %d line 找不到剑印副本配置信息 FuBenID = %d, Level  = %d",__FUNCTION__,__LINE__,pSwordWorldCnfg->m_FuBenID,Level);
				}
				else if(m_pActor->GetCrtProp(enCrtProp_Level)< pSwordFuBenCnfg->m_EnterLevel)
				{
					Rsp.m_Result = enMainUICode_ErrEnterLevelLimit;
				}
				else
				{
					Rsp.m_FuBenID = pSwordWorldCnfg->m_FuBenID;
					Rsp.m_Level   = Level;

					const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pSwordWorldCnfg->m_FuBenID);
					if(pFuBenCnfg == 0 ){
						TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pSwordWorldCnfg->m_FuBenID);
					}

					if( Level > pFuBenCnfg->m_MapID.size()){
						return;
					}

					if( m_SceneIDGodSward.IsValid() && m_uidGodSword == uidGodSword && m_SceneIDGodSward.m_mapid == pFuBenCnfg->m_MapID[Level - 1]){
						//已经有此场景，不用再创建新场景
						IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(m_SceneIDGodSward);
						if( pGameScene == 0){
							TRACE("<error> %s : %d line 创建场景失败FuBenID = %d ",__FUNCTION__,__LINE__,pSwordFuBenCnfg->m_FuBenID);
						}else{
							//进入副本
							pGameScene->EnterScene(m_pActor,true);
						}

					}else{
						if( m_SceneIDGodSward.IsValid()){
							//判断玩家是否就在该场景
							if( m_SceneIDGodSward.ToID() == m_pActor->GetCrtProp(enCrtProp_SceneID)){

								IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(m_SceneIDGodSward);
								if( 0 == pGameScene){
									TRACE("<error> %s : %d line获取场景失败!!",__FUNCTION__,__LINE__);
									//回到主场景
									m_pActor->ComeBackMainScene();
								}else{
									pGameScene->LeaveScene(m_pActor);
								}
							}

							//销毁旧场景
							g_pGameServer->GetGameWorld()->DeleteGameScene(m_SceneIDGodSward);
							m_SceneIDGodSward = INVALID_SCENE_ID;
						}

						//创建新场景
						IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(pFuBenCnfg->m_MapID[Level - 1] ,KillNpcNum);
						if(pGameScene==0)
						{
							TRACE("<error> %s : %d line 创建场景失败FuBenID = %d ",__FUNCTION__,__LINE__,pSwordFuBenCnfg->m_FuBenID);
						}
						else
						{	
							m_SceneIDGodSward = pGameScene->GetSceneID();	

							m_uidGodSword = uidGodSword;

							m_FuBenLevel = Level;

							//进入副本
							pGameScene->EnterScene(m_pActor,true);
						}
					}
				}
			}
		}
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//获得仙剑副本配置信息
const SGodSwordFuBenCnfg * FuBenPart::GetSwordFuBenCnfg(IGoods * pGoods,UINT8 FuBenLevel)
{
	const SGoodsCnfg * pGoodsCnfg = pGoods->GetGoodsCnfg();

	const SGodSwordWorldCnfg *	 pSwordWorldCnfg = g_pGameServer->GetConfigServer()->GetGoldSwordWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);
	if(pSwordWorldCnfg == 0)
	{		
		TRACE("<error> %s : %d line 找不到剑印世界配置信息 SwordSecretID = %d",__FUNCTION__,__LINE__,pGoodsCnfg->m_SuitIDOrSwordSecretID);
		return 0;
	}

	//获取副本配置信息
	const SGodSwordFuBenCnfg *	 pSwordFuBenCnfg =  g_pGameServer->GetConfigServer()->GetGodSwordFuBenCnfg(pSwordWorldCnfg->m_FuBenID,m_FuBenLevel);
	if(pSwordFuBenCnfg == 0)
	{			
		TRACE("<error> %s : %d line 找不到剑印副本配置信息 FuBenID = %d, Level  = %d",__FUNCTION__,__LINE__,pSwordWorldCnfg->m_FuBenID,m_FuBenLevel);
		return 0 ;
	}

	return pSwordFuBenCnfg;
}
//挑战仙剑怪物
void FuBenPart::CombatNpcGodSword(TFuBenID FuBenID, UID uidNpc)
{
	if(m_SceneIDGodSward==INVALID_SCENE_ID)
	{
		TRACE("<error> %s : %d line 没进入仙剑副本，不能挑战怪物",__FUNCTION__,__LINE__);
		return;
	}

	TMapID MapID = m_SceneIDGodSward.GetMapID();
	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);
	if(pVect==0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, MapID = %d ",__FUNCTION__,__LINE__ ,MapID);
		return;
	}

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(MapID);
	if( 0 == pMapConfig){
		TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, MapID);
		return;
	}


	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = FuBenID;

	m_EnterFuBenID = FuBenID;

	OBuffer4k ob;

	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidNpc);

	//首先确认玩家是否拥有该仙剑
	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,m_uidGodSword);

	IGodSword * pGodSword = (IGodSword*)pGoods;

	bool bWin = false;

	bool bUpgrade = false; //仙剑是否升级了


	if(pGoods == 0)
	{
		Rsp.m_Result = enCombatCode_ErrNoGoods;
	}
	else if(pGoods->GetGoodsClass() != enGoodsCategory_GodSword)
	{
		Rsp.m_Result = enCombatCode_ErrNotGodSword;
	}
	else if(pMonster ==0)
	{
		Rsp.m_Result = enCombatCode_ErrNotExistNpc; //该怪物不存在
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enCombatCode_ErrInDuoBao;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrWaitTeam;
	}
	else
	{		
		//获取副本配置信息
		const SGodSwordFuBenCnfg * pSwordFuBenCnfg =  this->GetSwordFuBenCnfg(pGoods,m_FuBenLevel);

		INT32 KillNpcNum = 0;
		pGodSword->GetPropNum(enGoodsProp_KillNpcNum,KillNpcNum);
		if(pMonster->GetCrtProp(enCrtProp_MonsterLineup) != KillNpcNum)
		{
			Rsp.m_Result = enCombatCode_ErrOrder;
		}		
		else if(pSwordFuBenCnfg == 0)
		{
			Rsp.m_Result = enCombatCode_ErrFuBenCnfg;
		}
		else
		{			
			if(pCombatPart->CombatWithNpc(enCombatType_GoldSword,uidNpc,m_GoldSwordCombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID))==false)
			{
				Rsp.m_Result = enCombatCode_ErrBusy;
			}	
			else
			{
				return;
			}			
		}
	}

	OBuffer4k ob2;

	ob2 << SCombatHeader(enCombatCmd_CombatNpcInFuBen, sizeof(Rsp)) << Rsp; 

	m_pActor->SendData(ob2.TakeOsb());

}

//免费进入次数
UINT8 FuBenPart::GetFreeEnterFuBenNum()
{
	time_t nCurTime = CURRENT_TIME();
	if(XDateTime::GetInstance().IsSameDay(nCurTime,m_LastFreeEnterFuBenTime)==false)
	{
		m_FreeEnterFuBenNum = 0;
	}

	return m_FreeEnterFuBenNum;
}

//帮派福利进入次数
UINT8 FuBenPart::GetSynWelfareEnterFuBenNum()
{
	time_t nCurTime = CURRENT_TIME();
	if(XDateTime::GetInstance().IsSameDay(nCurTime,m_LastSynWelfareFuBenTime)==false)
	{
		m_SynWelfareEnterFuBenNum = 0;
	}

	return m_SynWelfareEnterFuBenNum;
}

//支付灵石进入次数
UINT8 FuBenPart::GetCostStoneEnterFuBenNum()
{
	time_t nCurTime = CURRENT_TIME();
	if(XDateTime::GetInstance().IsSameDay(nCurTime,m_LastStoneEnterFuBenTiem)==false)
	{
		m_CostStoneEnterFuBenNum = 0;
	}

	return m_CostStoneEnterFuBenNum;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void FuBenPart::InitPrivateClient()
{
}


//玩家下线了，需要关闭该ThingPart
void FuBenPart::Close()
{
}

//保存数据
void FuBenPart::SaveData()
{
	SDBFuBenData FuBenData;

	int nLen = sizeof(SDBFuBenData);

	this->OnGetDBContext(&FuBenData, nLen);

	SDB_Update_BasicFuBenData_Req ReqBasicFuBen;

	ReqBasicFuBen.FreeEnterFuBenNum = FuBenData.m_FreeEnterFuBenNum;
	ReqBasicFuBen.CostStoneEnterFuBenNum = FuBenData.m_CostStoneEnterFuBenNum;
	ReqBasicFuBen.SynWelfareFuBenNum = FuBenData.m_SynWelfareEnterFuBenNum;
	ReqBasicFuBen.FuBenNum			= FuBenData.m_FuBenNum;
	ReqBasicFuBen.LastFreeEnterFuBenTime = FuBenData.m_LastFreeEnterFuBenTime;
	ReqBasicFuBen.LastStoneEnterFuBenTime = FuBenData.m_LastStoneEnterFuBenTime;
	ReqBasicFuBen.LastSynWelfareEnterFuBenTime = FuBenData.m_LastSynWelfareEnterFuBenTime;
	ReqBasicFuBen.Uid_User			= m_pActor->GetUID().ToUint64();
	ReqBasicFuBen.LastEnterFuBenGodSword  = FuBenData.m_LastEnterFuBenGodSword;
	ReqBasicFuBen.EnterSynFuBenNum = FuBenData.m_EnterSynFuBenNum;
	ReqBasicFuBen.LastEnterSynFuBenTime = FuBenData.m_LastEnterSynFuBenTime;
	ReqBasicFuBen.EnterGodSwordWorldNum = FuBenData.m_EnterGodSwordWorldNum;
	ReqBasicFuBen.LastVipEnterFuBenTime = FuBenData.m_LastVipEnterFuBenTime;
	ReqBasicFuBen.VipEnterFuBenNum	    = FuBenData.m_VipEnterFuBenNum;
	memcpy(&ReqBasicFuBen.m_FinishedFuBen, &m_FinishedFuben, sizeof(ReqBasicFuBen.m_FinishedFuBen));
	memcpy(&ReqBasicFuBen.m_HardFinishedFuBen, &m_HardFinishedFuben, sizeof(ReqBasicFuBen.m_HardFinishedFuBen));

	OBuffer1k ob;
	ob << ReqBasicFuBen;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateFuBenInfo,ob.TakeOsb(),0,0);

	for( int i = 0; i < FuBenData.m_FuBenNum; ++i){
		//更新副本进度，如果数据库上没有的则新建数据
		SDB_Update_FuBenProgressData_Req ReqFuBenProgress;
		ReqFuBenProgress.FuBenID		= FuBenData.m_FuBenProgress[i].m_FuBenID;
		ReqFuBenProgress.KillMonsterNum = FuBenData.m_FuBenProgress[i].m_KillMonsterNum;
		ReqFuBenProgress.Level			= FuBenData.m_FuBenProgress[i].m_Level;
		ReqFuBenProgress.Uid_User		= m_pActor->GetUID().ToUint64();

		ob.Reset();
		ob << ReqFuBenProgress;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateFuBenProgressInfo,ob.TakeOsb(),0,0);
	}

	//玩家在玄天的数据更新
	g_pThingServer->GetMainUIMgr().SaveXTData(m_pActor);
}	

void FuBenPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

	if( msgID == EventData.m_MsgID){
		SS_EnterScene * pEnterScene = (SS_EnterScene *)EventData.m_pContext;
		if( 0 == pEnterScene){
			return;
		}

		TSceneID SceneID = pEnterScene->m_NewSceneID;

		//取消监听
		m_pActor->UnsubscribeEvent(msgID, this);

		//普通副本
		MAP_FUBEN::iterator iter = m_mapFuBen.begin();
		for( ; iter != m_mapFuBen.end(); ++iter)
		{
			SFuBenProgressInfo & FuBenProgress  = iter->second;

			if( FuBenProgress.m_SceneID == SceneID){
                UINT8 Level = 0;
				TMapID MapID = this->GetFuBenMapID(&iter->second,Level);
				if(MapID != SceneID.GetMapID() && Level!=1)
				{
					EnterFuBen(&iter->second,enFuBenMode_Team == m_FuBenMode);
				}	
				else if(m_FuBenMode == enFuBenMode_Team)
				{
					IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
					if(pGameScene != 0)
					{
						ITeamPart * pTeamPart = m_pActor->GetTeamPart();

						if ( 0 != pTeamPart )
						{
							IActor * pMember = pTeamPart->GetTeamMember();

							if ( 0 != pMember && pMember->GetCrtProp(enCrtProp_SceneID) != SceneID.ToID() )
							{
								//如果玩家的上个场景在摆怪场景，则删除战斗场景的传送门
								if ( pMember->GetCrtProp(enCrtProp_ActorLastSceneID) == pGameScene->GetSceneID() )
								{
									IGameScene * pOldGameScene = g_pGameServer->GetGameWorld()->GetGameScene(pEnterScene->m_OldSceneID);
									if(pOldGameScene != 0)
									{
										//删除传送门
										pOldGameScene->DestroyThingClass(enThing_Class_Portal);
									}
								}

								//队友也进副本
								FriendEnterFuBen(pGameScene,&FuBenProgress);
							}
						}				
					}
				}

				break;
			}
		}

	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if (msgID == EventData.m_MsgID){

		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		if (0 == pActorDoing)
			return;

		TSceneID CurSceneID = m_pActor->GetCrtProp(enCrtProp_SceneID);

		if ( CurSceneID == m_SceneIDGodSward )
		{
			//在剑印世界副本
			pActorDoing->m_ActorDoing = enActorDoing_FuBen_Single;
			return;
		}

		if ( this->IsInTeamFuBen() )
		{
			//在组队副本
			pActorDoing->m_ActorDoing = enActorDoing_FuBen_Team;
			return;
		}

		if ( CurSceneID == m_SceneIDFuBen )
		{
			//在普通副本中
			pActorDoing->m_ActorDoing = enActorDoing_FuBen_Single;
			return;
		}
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitTeam);

	if ( msgID == EventData.m_MsgID )
	{
		//退队，如果在组队副本中的话，退回到主场景
		if ( enFuBenMode_Team == m_FuBenMode )
		{
			m_FuBenMode = enFuBenMode_Single;

			m_pActor->ComeBackMainScene();
		}
	}
}

//打开副本
void FuBenPart::OpenFuBen()
{
	const SGameConfigParam & ConfigParam =g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//返回副本信息
	SC_MainUIOpenFuBen_Rsp Rsp;

	//判断是否有帮派福利
	ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(m_pActor->GetUID());
	if(pSyndicate)
	{		 
		Rsp.m_MaxSynWelfareEnterNum =  pSyndicate->GetWelfareValue(enWelfare_EnterFuBenNum);
		Rsp.m_SynWelfareEnterNum = Rsp.m_MaxSynWelfareEnterNum - this->GetSynWelfareEnterFuBenNum();
	}


	Rsp.m_MaxVipEnterNum = m_pActor->GetVipValue(enVipType_AddFuBenNum);
	Rsp.m_FreeEnterNum = ConfigParam.m_MaxFreeEnterFuBenNum - this->GetFreeEnterFuBenNum();
	Rsp.m_VipEnterNum = Rsp.m_MaxVipEnterNum - this->GetVipEnterCommonFuBenNum();

	Rsp.m_CostStoneEnterNum  = ConfigParam.m_MaxCostStoneEnterFuBenNum - this->GetCostStoneEnterFuBenNum();

	UINT8  nProgressNum = 0;

	OBuffer4k ob2;

	UINT8 LvNumCommonAndHardFB = ConfigParam.m_LvNumCommonAndHardFB;

	for(MAP_FUBEN::iterator it = m_mapFuBen.begin(); it != m_mapFuBen.end(); ++it)
	{
		SFuBenProgress FuBenProgress;
		SFuBenProgressInfo & Info = (*it).second;
		FuBenProgress.m_FuBenID = Info.m_FuBenID;
		FuBenProgress.m_KillMonsterNum = Info.m_KillMonsterNum;
		FuBenProgress.m_Level = Info.m_Level;

		if( !Info.IsStart()){
			//杀怪数为0的同进度
			continue;
		}

		const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenProgress.m_FuBenID);

		if(pFuBenCnfg == 0 )
		{
			TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,FuBenProgress.m_FuBenID);
			return;
		}

		if ( pFuBenCnfg->m_Type != enFuBenType_Common )
			continue;

		//FuBenProgress.m_bCanOpenHardType = this->CanOpenHardType(FuBenProgress.m_FuBenID);
		FuBenProgress.m_bCanOpenHardType = this->CanOpenHardType(FuBenProgress.m_FuBenID);

		++nProgressNum;

		ob2 << FuBenProgress; 
	}

	Rsp.m_StartFuBenNum = nProgressNum;

	OBuffer4k ob;
	ob <<  SMainUIHeader(enMainUICmd_OpenFuBen,sizeof(Rsp)+ob2.Size()) << Rsp;

	if( ob2.Size() > 0){
		ob << ob2;
	}

	m_pActor->SendData(ob.TakeOsb());

}

//打开帮派副本面板
void FuBenPart::OpenSynFuBen()
{
	SC_OpenSynCombat_Rsp Rsp;

	Rsp.m_CombatNum = this->GetEnterSynFuBenNum();

	Rsp.m_MaxVipEnterNum = m_pActor->GetVipValue(enVipType_AddSynCombatNum);

	SFuBenProgressInfo * pFuBenProgress = this->GetSynFuBenProgressInfo();

	if ( 0 == pFuBenProgress){

		const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(m_pActor->GetCrtProp(enCrtProp_Level));

		if ( 0 == pSynCombatCnfg){

			Rsp.m_FuBenID = 0;
		}else{

			Rsp.m_FuBenID = pSynCombatCnfg->m_SynFuBenID;
		}

		Rsp.m_Level   = 0;
		Rsp.m_KillNum = 0;
	}else{
		Rsp.m_FuBenID = pFuBenProgress->m_FuBenID;
		Rsp.m_Level   = pFuBenProgress->m_Level;
		Rsp.m_KillNum = pFuBenProgress->m_KillMonsterNum;
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_OpenSynCombat, sizeof(Rsp)) << Rsp; 
	m_pActor->SendData(ob.TakeOsb());
}

//挑战普通副本
void FuBenPart::ChallengeFuBen(TFuBenID  FuBenID)
{
	SC_MainUIChallenge_Rsp Rsp;
	Rsp.m_Result  = 	enMainUICode_OK;
	Rsp.m_FuBenID =     FuBenID;
	Rsp.m_bOpenHardType = this->CanOpenHardType(FuBenID);

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	//第一步
	Rsp.m_Result = (enMainUICode)this->ChallengeComFuBenFirst(pFuBenProgress, FuBenID);

	if ( Rsp.m_Result == enMainUICode_OK){

		pFuBenProgress = GetFuBenProgressInfo(FuBenID);
	}

	if( Rsp.m_Result == enMainUICode_OK && pFuBenProgress)
	{
		//进入副本
		this->EnterComFuBen(pFuBenProgress, false);

		//Rsp.m_Level = pFuBenProgress->m_Level;
		return;
	}

	OBuffer1k ob;

	ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//挑战帮派副本
void FuBenPart::ChallengeSynFuBen()
{
	SC_MainUIChallenge_Rsp Rsp;
	Rsp.m_Result = enMainUICode_OK;
	Rsp.m_FuBenID = 0;
	Rsp.m_Level = 0;
	Rsp.m_bOpenHardType = false;

	SFuBenProgressInfo * pFuBenProgress = this->GetSynFuBenProgressInfo();

	Rsp.m_Result = (enMainUICode)this->ChallengeSynFuBenFirst(pFuBenProgress);

	if ( Rsp.m_Result == enMainUICode_OK){

		pFuBenProgress = this->GetSynFuBenProgressInfo();
	}

	if( Rsp.m_Result == enMainUICode_OK && pFuBenProgress)
	{
		if( !pFuBenProgress->IsStart() && this->GetEnterSynFuBenNum() >= this->GetMaxEnterSynFuBenNum())
		{
			Rsp.m_Result = enMainUICode_ErrNumLimit;
		}
		else
		{
			EnterFuBen(pFuBenProgress);

			return;
		}
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//重置副本
void FuBenPart::ResetFuBen(TFuBenID  FuBenID)
{
	SC_MainUIReset_Rsp Rsp;
	Rsp.m_Result = enMainUICode_OK;
	Rsp.m_FuBenID = FuBenID;

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	if(pFuBenProgress==0)
	{
		Rsp.m_Result = enMainUICode_ErrNoStartFuBen;
	}
	else
	{
		if ( this->IsInTeamFuBen() )
		{
			ITeamPart * pTeamPart = m_pActor->GetTeamPart();

			if ( 0 == pTeamPart )
			{
				Rsp.m_Result = enMainUICode_Err;
			}
			else if ( pTeamPart->GetTeamLeader() != m_pActor )
			{
				Rsp.m_Result = enMainUICode_ErrNoLeaderReset;
			}
		}

		if ( Rsp.m_Result == enMainUICode_OK )
		{
			IGameScene *pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(pFuBenProgress->m_SceneID);

			if(pGameScene != 0)
			{			
				g_pGameServer->GetGameWorld()->DeleteGameScene(pFuBenProgress->m_SceneID);
			}

			pFuBenProgress->m_SceneID = INVALID_SCENE_ID;

			pFuBenProgress->m_Level = 1;
			pFuBenProgress->m_KillMonsterNum = 0;		
		}
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_Reset,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//重置帮派保卫战
void FuBenPart::ResetSynFuBen()
{
	SC_ResetSynCombat_Rsp Rsp;
	Rsp.m_Result  = enSynRetCode_OK;

	SFuBenProgressInfo * pFuBenProgress = this->GetSynFuBenProgressInfo();
	if( 0 == pFuBenProgress){
		Rsp.m_Result = enSynRetCode_NoSynFuBen;
	}else{
		IGameScene *pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(pFuBenProgress->m_SceneID);

		if(pGameScene != 0)
		{			
			g_pGameServer->GetGameWorld()->DeleteGameScene(pFuBenProgress->m_SceneID);
		}

		pFuBenProgress->m_SceneID = INVALID_SCENE_ID;

		pFuBenProgress->m_Level = 1;
		pFuBenProgress->m_KillMonsterNum = 0;
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_ResetSynCombat, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//普通副本中挑战怪物, bOpenHardType为是否开启困难模式
void FuBenPart::CombatNpcCommon(TFuBenID  FuBenID,UID uidNpc, enCombatIndexMode CombatIndexMode)
{
	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = FuBenID;

	m_EnterFuBenID = FuBenID;

	OBuffer4k ob;

	IMonster * pMonster = 0;

	const std::vector<SMonsterOutput> * pVect = 0;

	bool bWin = false;

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	if ( 0 == pCombatPart || 0 == pTeamPart )
		return;

	if(pFuBenProgress == 0)
	{
		Rsp.m_Result = enCombatCode_ErrNoFuBen;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enCombatCode_ErrInDuoBao;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrWaitTeam;
	}
	else if ( m_pActor->HaveTeam() && enFuBenMode_Team != m_FuBenMode )
	{
		Rsp.m_Result = enCombatCode_ErrHaveTeam;
	}
	else if((pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidNpc))==0)
	{
		Rsp.m_Result = enCombatCode_ErrNoNpc;
	}
	else if( enFuBenMode_Team != m_FuBenMode &&  pMonster->GetCrtProp(enCrtProp_MonsterLineup) != pFuBenProgress->m_KillMonsterNum )
	{
		Rsp.m_Result = enCombatCode_ErrOrder;
	}
	else if( enCombatIndexMode_Hard == CombatIndexMode && !this->GetFuBenIsFinished(FuBenID) )
	{
		Rsp.m_Result = enCombatCode_ErrThrough;
	}
	else if ( enFuBenMode_Team == m_FuBenMode && !pTeamPart->IsHaveTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrBackMain;
	}
	else if ( enFuBenMode_Team == m_FuBenMode && m_pActor != pTeamPart->GetTeamLeader() )
	{
		Rsp.m_Result = enCombatCode_OnlyLeadCan;
	}
	else
	{
		TMapID mapID = pFuBenProgress->m_SceneID.GetMapID();

		if ( INVALID_MAP_ID == mapID && enFuBenMode_Team == m_FuBenMode )
		{
			//如果场景无效，则从队友场景中获取
			SFuBenProgressInfo * pMemberProgress = this->GetTeamMemberFuBenProgress(pFuBenProgress->m_FuBenID);

			if ( 0 == pMemberProgress )
				return;

			mapID = pMemberProgress->m_SceneID.GetMapID();
		}


		const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(mapID);
		if( 0 == pMapConfig){
			TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, mapID);
			return;
		}

		const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

		if(pFuBenCnfg == 0 )
		{
			TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
			return;
		}

		if ( enCombatIndexMode_Hard == CombatIndexMode && pFuBenCnfg->m_EnterLevel < g_pGameServer->GetConfigServer()->GetGameConfigParam().m_LvNumCommonAndHardFB)
		{
			Rsp.m_Result = enCombatCode_ErrFuBenLv;
		}
		else
		{
			if ( enFuBenMode_Team == m_FuBenMode ) 
			{
				IActor * pMember = pTeamPart->GetTeamMember();

				if ( 0 == pMember )
				{
					return;
				}

				enActorDoing enDoing = pMember->GetActorDoing();

				if ( enDoing != enActorDoing_None && enDoing != enActorDoing_FuBen_Team )
				{
					Rsp.m_Result = enCombatCode_ErrMemberNoFree;
				}
				else
				{
					//先检测进度是否相同，不同的先同步进度
					ITeamPart * pTeamPart = m_pActor->GetTeamPart();
					if ( 0 == pTeamPart )
						return;

					IActor * pMember = pTeamPart->GetTeamMember();

					if ( 0 == pMember )
						return;

					if ( !this->SynTeamProgress(pFuBenProgress, pMember) )
					{
						TRACE("<error> %s : %d Line 与队友同步进度失败！！！！", __FUNCTION__, __LINE__);
						return;
					}

					UINT64 CombatID = 0;

					if (pCombatPart->CombatWithNpc(enCombatType_BaoDe,uidNpc,CombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID),enCombatIndexMode_Team,enJoinPlayer_OneselfTeam, enCombatMode_OneselfTeam,(UINT32)enCombatIndexMode_Hard)==false)
					{
						Rsp.m_Result = enCombatCode_ErrBusy;
					}

					m_setCommonCombatID.insert(CombatID);			
				}
			}
			else
			{
				UINT64 CombatID = 0;

				if (pCombatPart->CombatWithNpc(enCombatType_BaoDe,uidNpc,CombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID),CombatIndexMode,(enJoinPlayer)(enJoinPlayer_Oneself|enJoinPlayer_OnlyEnemy),enCombatMode_Oneself, CombatIndexMode)==false)
				{
					Rsp.m_Result = enCombatCode_ErrBusy;
				}

				m_setCommonCombatID.insert(CombatID);
			}
		}
		
		if ( enCombatCode_OK == Rsp.m_Result)
		{
			m_CombatMode = CombatIndexMode;
		}
	}


	OBuffer4k ob1;

	ob1 <<  SCombatHeader(enCombatCmd_CombatNpcInFuBen,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob1.TakeOsb());	

}

//挑战帮派普通怪物
void FuBenPart::CombatNpcSyn(TFuBenID  FuBenID,UID uidNpc)
{
	ISyndicateMgr * pSyndicateMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSyndicateMgr){
		TRACE("<error> %s : %d line 获取不到SyndicateMgr",__FUNCTION__,__LINE__);
		return; 
	}

	//当前模式
	const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(m_pActor->GetCrtProp(enCrtProp_Level));

	if(pSynCombatCnfg==0)
	{
		TRACE("<error> %s : %d line 找不到帮战模式配置信息, 等级 = %d ",__FUNCTION__,__LINE__ ,m_pActor->GetCrtProp(enCrtProp_Level));
		return;
	}

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = FuBenID;

	this->m_EnterFuBenID = FuBenID;


	OBuffer4k ob;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);
	if( 0 == pFuBenCnfg){
		TRACE("<error> %s : %d 行 获取副本配置信息失败！！,副本ID = %d", __FUNCTION__, __LINE__, FuBenID);
		return;
	}

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	bool bWin = false;

	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidNpc);

	if(pFuBenProgress == 0)
	{
		Rsp.m_Result = enCombatCode_ErrNoFuBen;
	}
	if(pMonster ==0)
	{
		Rsp.m_Result = enCombatCode_ErrNoNpc; //该怪物不存在
	}
	else if(pMonster->GetCrtProp(enCrtProp_MonsterLineup) != pFuBenProgress->m_KillMonsterNum)
	{
		Rsp.m_Result = enCombatCode_ErrOrder;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enCombatCode_ErrInDuoBao;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enCombatCode_ErrWaitTeam;
	}
	else
	{
		TMapID mapID = pFuBenProgress->m_SceneID.GetMapID();

		const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(mapID);
		if(pVect==0)
		{
			TRACE("<error> %s : %d line 找不到地图怪物配置信息, MapID = %d ",__FUNCTION__,__LINE__ ,mapID);
			return;
		}


		const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(mapID);
		if( 0 == pMapConfig){
			TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, mapID);
			return;
		}

		if(pCombatPart->CombatWithNpc(enCombatType_SynGuard,uidNpc,m_SynGuardCombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID))==false)
		{
			Rsp.m_Result = enCombatCode_ErrBusy;
		}
		else 
		{
			return;
		}

		if ( enCombatCode_OK == Rsp.m_Result)
		{
			m_CombatMode = enCombatIndexMode_Com;
		}
	}

	OBuffer4k ob2;

	ob2 <<  SCombatHeader(enCombatCmd_CombatNpcInFuBen,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob2.TakeOsb());

}


//挑战怪物
void FuBenPart::CombatNpc(TFuBenID  FuBenID,UID uidNpc, enCombatIndexMode CombatIndexMode)
{
	//判断是否需要切换到下一场
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if(pFuBenCnfg == 0 )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息 FuBenID = %d ",__FUNCTION__,__LINE__,FuBenID);
		return ;
	}

	if(pFuBenCnfg->m_Type == enFuBenType_Common)
	{
		CombatNpcCommon(FuBenID,uidNpc, CombatIndexMode);
	}
	else if(pFuBenCnfg->m_Type == enFuBenType_Syndicate)
	{
		//帮派副本
		this->CombatNpcSyn(FuBenID,uidNpc);

	}else if(pFuBenCnfg->m_Type == enFuBenType_GoldSword)
	{
		//仙剑副本挑战怪物
		this->CombatNpcGodSword(FuBenID,uidNpc);
	}

}

//离开副本
void FuBenPart::LeaveFuBen(TFuBenID  FuBenID)
{
	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	SC_CombatLeaveFuBen_Rsp Rsp;
	if(pFuBenProgress==0)
	{
		return;
	}

	Rsp.m_Result = enCombatCode_OK;

	TSceneID SceneID;
	SceneID.From(m_pActor->GetCrtProp(enCrtProp_ActorMainSceneID));

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if(pGameScene==0)
	{
		TRACE("<error> %s : %d line 玩家[%s]主场景不存在",__FUNCTION__,__LINE__,m_pActor->GetName());
		return;
	}

	pGameScene->EnterScene(m_pActor);

	if(pFuBenProgress->IsStart()==false )
	{
		m_mapFuBen.erase(FuBenID);
	}
	else
	{
		pFuBenProgress->m_SceneID = INVALID_SCENE_ID;

	}

	OBuffer1k ob;
	ob <<  SCombatHeader(enCombatCmd_LeaveFuBen,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

SFuBenProgressInfo * FuBenPart::GetFuBenProgressInfo(TFuBenID  FuBenID)
{
	MAP_FUBEN::iterator it = m_mapFuBen.find(FuBenID);
	if(it == m_mapFuBen.end())
	{
		return 0;
	}

	return &(*it).second;
}

//重置所有仙剑副本进度
void FuBenPart::ResetGodSwordFuBen()
{
	IPacketPart * pPacketPart = m_pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	//重置背包中的仙剑副本
	pPacketPart->ResetGodSwordFuBen();

	//重置主角装备栏里的仙剑副本
	IEquipPart * pEquipPart = m_pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	pEquipPart->ResetGodSwordFuBen();

	//重置招募角色的装备栏的仙剑副本
	for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = m_pActor->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		IEquipPart * pTmpEquipPart = pEmployee->GetEquipPart();;
		if( 0 == pTmpEquipPart){
			continue;
		}

		pTmpEquipPart->ResetGodSwordFuBen();
	}
}

//获得当前进度对应的地图ID
TMapID FuBenPart::GetFuBenMapID(SFuBenProgressInfo * pFuBenProgress,UINT8 & Level)
{
	TMapID MapID = INVALID_MAP_ID;

	if( 0 == pFuBenProgress){
		return MapID;
	}

	TFuBenID FuBenID = pFuBenProgress->m_FuBenID;

	//进入副本
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if(pFuBenCnfg == 0 || pFuBenProgress->m_Level> pFuBenCnfg->m_MapID.size())
	{
		TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d",__FUNCTION__,__LINE__,FuBenID);
		return MapID;
	}

	TMapID CurMapID = pFuBenCnfg->m_MapID[pFuBenProgress->m_Level-1];

	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(CurMapID);

	if(pVect == 0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, FuBenID = %d MapID=%d",__FUNCTION__,__LINE__ ,pFuBenProgress->m_FuBenID,CurMapID);
		return MapID;
	}

	if(pFuBenProgress->m_KillMonsterNum >= pVect->size())
	{
		if(pFuBenProgress->m_Level < pFuBenCnfg->m_MapID.size())
		{
			pFuBenProgress->m_Level++;
			pFuBenProgress->m_KillMonsterNum = 0;

			return GetFuBenMapID(pFuBenProgress,Level);
		}
	}

	MapID = pFuBenCnfg->m_MapID[pFuBenProgress->m_Level-1];

	Level = pFuBenProgress->m_Level;

	return MapID;
}

//好友也进副本
void FuBenPart::FriendEnterFuBen(IGameScene * pGameScene,SFuBenProgressInfo * pFuBenProgress)
{
	ITeamPart * pTeamPart = m_pActor->GetTeamPart();

				if ( 0 == pTeamPart ){
					return ;
				}

				IActor * pMember = pTeamPart->GetTeamMember();

				if ( 0 == pMember ){
					TRACE("<warning> %s : %d Line 组队进副本，但找不到队友！！", __FUNCTION__, __LINE__);
					return ;
				}

				IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

				if ( 0 == pFuBenPart )
					return ;

				pFuBenPart->SetFuBenMode(enFuBenMode_Team);

				pFuBenPart->SetSceneIDFuBen(pGameScene->GetSceneID());

				//队友也进入副本
				SC_MainUIChallenge_Rsp Rsp;

				pGameScene->EnterScene(pMember,true);

				Rsp.m_bOpenHardType = this->CanOpenHardType(pFuBenProgress->m_FuBenID);
				Rsp.m_FuBenID = pFuBenProgress->m_FuBenID;
				Rsp.m_Level	= pFuBenProgress->m_Level;
				Rsp.m_Result = enMainUICode_OK;

				OBuffer1k ob;
				ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;
				pMember->SendData(ob.TakeOsb());

				////队友不保存场景，不然会出现杀敌数与场景怪物数不一致
				//SFuBenProgressInfo * pMemberFuBenProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

				//if ( pMemberFuBenProgress->m_SceneID != INVALID_SCENE_ID && pMemberFuBenProgress->m_SceneID != pGameScene->GetSceneID() )
				//{
				//	IGameScene *pMemberGameScene = g_pGameServer->GetGameWorld()->GetGameScene(pMemberFuBenProgress->m_SceneID);

				//	if ( 0 != pMemberGameScene )
				//	{
				//		if ( pMember->GetCrtProp(enCrtProp_SceneID) == pMemberFuBenProgress->m_SceneID )
				//		{
				//			//要先返回主场景
				//			pMember->ComeBackMainScene();
				//		}

				//		g_pGameServer->GetGameWorld()->DeleteGameScene(pMemberGameScene);
				//		pMemberGameScene = 0;
				//	}
				//}
}

//与队友同步下进度
bool	FuBenPart::SynTeamProgress(SFuBenProgressInfo * pFuBenProgress, IActor * pMember)
{
	if ( 0 == pFuBenProgress )
		return false;

	IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	if ( 0 == pFuBenPart )
		return false;

	SFuBenProgressInfo * pMemberFuBenProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

	if ( 0 == pMemberFuBenProgress )
		return false;

	//同步到进度大的
	if ( pFuBenProgress->m_Level > pMemberFuBenProgress->m_Level
		|| (pFuBenProgress->m_Level == pMemberFuBenProgress->m_Level && pFuBenProgress->m_KillMonsterNum >= pMemberFuBenProgress->m_KillMonsterNum) )
	{
		pMemberFuBenProgress->m_KillMonsterNum = pFuBenProgress->m_KillMonsterNum;
		pMemberFuBenProgress->m_Level		  = pFuBenProgress->m_Level;
		pMemberFuBenProgress->m_SceneID		  = INVALID_SCENE_ID;
	}
	else
	{
		pFuBenProgress->m_KillMonsterNum = pMemberFuBenProgress->m_KillMonsterNum;
		pFuBenProgress->m_Level			 = pMemberFuBenProgress->m_Level;
		pFuBenProgress->m_SceneID		 = pMemberFuBenProgress->m_SceneID;
		pMemberFuBenProgress->m_SceneID	 = INVALID_SCENE_ID;	
	}

	return true;
}

//同步成队长进度
bool	FuBenPart::SynTeamLeaderProgress(SFuBenProgressInfo * pFuBenProgress, IActor * pMember)
{
	if ( 0 == pFuBenProgress )
		return false;

	IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	if ( 0 == pFuBenPart )
		return false;

	SFuBenProgressInfo * pMemberFuBenProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

	if ( 0 == pMemberFuBenProgress )
		return false;

	pMemberFuBenProgress->m_KillMonsterNum = pFuBenProgress->m_KillMonsterNum;
	pMemberFuBenProgress->m_Level		  = pFuBenProgress->m_Level;

	return true;
}

//进入副本
void FuBenPart::EnterFuBen(SFuBenProgressInfo * pFuBenProgress,bool bTeam )
{
	if( 0 == pFuBenProgress){
		return;
	}
	SC_MainUIChallenge_Rsp Rsp;
	Rsp.m_Result  = 	enMainUICode_OK;
	Rsp.m_FuBenID =     pFuBenProgress->m_FuBenID;

	const SGameConfigParam & ConfigParam =g_pGameServer->GetConfigServer()->GetGameConfigParam();


	//进入副本
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);
	if(pFuBenCnfg == 0 || pFuBenProgress->m_Level> pFuBenCnfg->m_MapID.size())
	{
		TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d Level = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID,pFuBenProgress->m_Level);
		Rsp.m_Result = enMainUICode_ErrFuBenCnfg;
		m_mapFuBen.erase(pFuBenProgress->m_FuBenID);
	}
	else if(pFuBenCnfg->m_EnterLevel > m_pActor->GetCrtProp(enCrtProp_Level))
	{
		Rsp.m_Result = enMainUICode_ErrLevel;
		m_mapFuBen.erase(pFuBenProgress->m_FuBenID);
	}
	else
	{
		UINT8 Level = 0;

		TMapID MapID = GetFuBenMapID(pFuBenProgress,Level);

		if(MapID==INVALID_MAP_ID)
		{
			return;
		}


		IGameScene * pGameScene = 0;

		if(pFuBenProgress->m_SceneID.IsValid())
		{
			if(pFuBenProgress->m_SceneID.GetMapID() != MapID)
			{
				g_pGameServer->GetGameWorld()->DeleteGameScene(pFuBenProgress->m_SceneID);
				pFuBenProgress->m_SceneID = INVALID_SCENE_ID;
			}
			else
			{
				pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(pFuBenProgress->m_SceneID);					
			}
		}

		if(pGameScene == 0)
		{
			//进入副本
			pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID,pFuBenProgress->m_KillMonsterNum);
		}

		if(pGameScene==0)
		{
			TRACE("<error> %s : %d line 创建场景失败FuBenID = %d Level = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID,pFuBenProgress->m_Level);
			Rsp.m_Result = enMainUICode_ErrFuBenCnfg;
			m_mapFuBen.erase(pFuBenProgress->m_FuBenID);
		}
		else
		{
			pFuBenProgress->m_SceneID = pGameScene->GetSceneID();	

			//进入副本
			pGameScene->EnterScene(m_pActor,true);

			m_SceneIDFuBen = pFuBenProgress->m_SceneID;

			//如果是组队，队友也要进场景
			if ( bTeam )
			{
				m_FuBenMode = enFuBenMode_Team;
				FriendEnterFuBen(pGameScene,pFuBenProgress);
			}
			else 
			{
				m_FuBenMode = enFuBenMode_Single;
			}

		}	
	}

	Rsp.m_Level = pFuBenProgress->m_Level;

	Rsp.m_bOpenHardType = this->CanOpenHardType(pFuBenProgress->m_FuBenID);


	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}



//下线时，删除所有副本场景
void FuBenPart::ReleaseScene()
{
	//普通副本
	MAP_FUBEN::iterator iter = m_mapFuBen.begin();

	for( ; iter != m_mapFuBen.end(); ++iter)
	{
		SFuBenProgressInfo & ProgressInfo = iter->second;

		if( !ProgressInfo.m_SceneID.IsValid()){
			continue;
		}

		g_pGameServer->GetGameWorld()->DeleteGameScene(ProgressInfo.m_SceneID);

		ProgressInfo.m_SceneID.From(INVALID_SCENE_ID);
	}

	//剑仙副本
	if( m_SceneIDGodSward.IsValid()){
		g_pGameServer->GetGameWorld()->DeleteGameScene(m_SceneIDGodSward);

		m_SceneIDGodSward.From(INVALID_SCENE_ID);
	}
}

//进入帮派保卫战次数
UINT16 FuBenPart::GetEnterSynFuBenNum()
{
	UINT32 nCurTime = CURRENT_TIME();

	//先更新一下，当天进入次数
	if(XDateTime::GetInstance().IsSameDay(nCurTime, m_LastEnterSynFuBenTime) == false)
	{
		m_EnterSynFuBenNum = 0;
	}

	return m_EnterSynFuBenNum;
}

//得到帮派副本进度
SFuBenProgressInfo * FuBenPart::GetSynFuBenProgressInfo()
{
	const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(m_pActor->GetCrtProp(enCrtProp_Level));

	if ( 0 == pSynCombatCnfg){

		return 0;
	}

	MAP_FUBEN::iterator it = m_mapFuBen.find(pSynCombatCnfg->m_SynFuBenID);

	if ( it != m_mapFuBen.end()){

		return &(*it).second;
	}

	it = m_mapFuBen.begin();
	for( ; it != m_mapFuBen.end(); ++it)
	{
		SFuBenProgressInfo & FuBenProgree = it->second;

		if ( this->GetFuBenType(FuBenProgree.m_FuBenID) != enFuBenType_Syndicate){

			continue;
		}

		if (  FuBenProgree.m_SceneID.IsValid()){

			g_pGameServer->GetGameWorld()->DeleteGameScene(FuBenProgree.m_SceneID);

			FuBenProgree.m_SceneID = INVALID_SCENE_ID;
		}

		SFuBenProgressInfo NewFuBenProgree = FuBenProgree;

		NewFuBenProgree.m_FuBenID = pSynCombatCnfg->m_SynFuBenID;

		m_mapFuBen.erase(it);

		m_mapFuBen[NewFuBenProgree.m_FuBenID] = NewFuBenProgree;

		it = m_mapFuBen.find(NewFuBenProgree.m_FuBenID);

		if ( it == m_mapFuBen.end()){
			return 0;
		}

		return &(*it).second;	
	}

	return 0;
}

//清除副本的次数限制
void FuBenPart::GMCmdClearFuBenNumLimit()
{
	m_LastEnterFuBenGodSword = 0;

	m_FreeEnterFuBenNum = 0;       //今天免费进入次数
	m_SynWelfareEnterFuBenNum = 0; //帮派福利进入次数
	m_CostStoneEnterFuBenNum = 0;	 //支付灵石进入次数
	m_VipEnterFuBenNum  = 0;

	m_EnterSynFuBenNum = 0;
}

//得到副本类型
enFuBenType	FuBenPart::GetFuBenType(TFuBenID FuBenID)
{
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);
	if( 0 == pFuBenCnfg){
		return enFuBenType_Max;
	}

	return (enFuBenType)pFuBenCnfg->m_Type;
}

//获得进入仙剑副本次数
UINT16 FuBenPart::GetEnterGodSwordNum()
{
	UINT32 nCurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameDay(nCurTime, m_LastEnterFuBenGodSword))
	{
		m_EnterGodSwordWorldNum = 0;
	}

	return m_EnterGodSwordWorldNum;
}

//得到最大仙剑副本可进入次数
UINT16 FuBenPart::GetMaxGodSwordEnterNum()
{
	return m_pActor->GetVipValue(enVipType_AddGodSwordWorldNum) + g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxEnterGodSwordWorldNum;
}

//今天最大可进入帮派副本次数
UINT16 FuBenPart::GetMaxEnterSynFuBenNum()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxEnterSynCombatNum + m_pActor->GetVipValue(enVipType_AddSynCombatNum);
}

//得到VIP进入普通副本次数
UINT16 FuBenPart::GetVipEnterCommonFuBenNum()
{
	time_t nCurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameDay(nCurTime,m_LastVipEnterFuBenTime))
	{
		m_VipEnterFuBenNum = 0;
	}

	return m_VipEnterFuBenNum;
}

//设置副本完成记录
void FuBenPart::SetFuBenFinished(TFuBenID FuBenID, bool bFinish)
{
	m_FinishedFuben.set(FuBenID, bFinish);
}

//得到副本是否完成过
bool FuBenPart::GetFuBenIsFinished(TFuBenID FuBenID)
{
	return m_FinishedFuben.get(FuBenID);
}

//自动快速打普通副本, bOpenHardType为是否开启困难模式
void FuBenPart::CommonFuBenAutoCombat(TFuBenID FuBenID, bool bOpenHardType)
{
	IPacketPart * pPacketPart = m_pActor->GetPacketPart();

	if ( 0 == pPacketPart){

		return;
	}

	SC_CommonFuBenAutoCombat Rsp;

	OBuffer4k ob;

	if ( !m_pActor->GetVipValue(enVipType_bAutoKillFuBen)){

		Rsp.m_Result = enMainUICode_ErrVipLevel;

	}else if ( !bOpenHardType && !this->GetFuBenIsFinished(FuBenID)){

		Rsp.m_Result = enMainUICode_NotFinished;

	}else if ( bOpenHardType && !this->GetHardFuBenIsFinished(FuBenID) ){

		Rsp.m_Result = enMainUICode_NotFinished;

	}else if ( pPacketPart->GetSpace() < 10){

		Rsp.m_Result = enMainUICode_SpaceLess;
	}else if ( m_pActor->IsInDuoBao() ){

		Rsp.m_Result = enMainUICode_ErrInDuoBao;
	}else if ( m_pActor->HaveTeam() ){
	
		Rsp.m_Result = enMainUICode_ErrHaveTeam;
	}

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(FuBenID);

	if ( Rsp.m_Result == enMainUICode_OK){

		//第一步
		Rsp.m_Result = (enMainUICode)this->ChallengeComFuBenFirst(pFuBenProgress, FuBenID);

		if ( Rsp.m_Result == enMainUICode_OK){

			pFuBenProgress = GetFuBenProgressInfo(FuBenID);
		}
	}

	if ( Rsp.m_Result == enMainUICode_OK && pFuBenProgress){

		const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

		if ( 0 == pFuBenCnfg){

			TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d",__FUNCTION__,__LINE__,FuBenID);
			Rsp.m_Result = enMainUICode_ErrFuBenCnfg;

		}else if ( pFuBenProgress->m_Level > pFuBenCnfg->m_MapID.size()){

			TRACE("<error> %s : %d line 副本配置信息有错 = %d Level = %d",__FUNCTION__,__LINE__,FuBenID,pFuBenProgress->m_Level);
			Rsp.m_Result = enMainUICode_ErrFuBenCnfg;

		}else if ( pFuBenCnfg->m_EnterLevel > m_pActor->GetCrtProp(enCrtProp_Level)){

			Rsp.m_Result = enMainUICode_ErrLevel;
			m_mapFuBen.erase(FuBenID);

		}else if ( bOpenHardType && pFuBenCnfg->m_EnterLevel < g_pGameServer->GetConfigServer()->GetGameConfigParam().m_LvNumCommonAndHardFB){

			Rsp.m_Result = enMainUICode_ErrFuBenLv;

		}else{

			ICombatPart * pCombatPart = m_pActor->GetCombatPart();

			if ( 0 == pCombatPart){

				TRACE("<error> %s : %d Line 获取战斗PART出错！,userid = %d", __FUNCTION__, __LINE__, m_pActor->GetCrtProp(enCrtProp_ActorUserID));
				return;
			}

			this->AddCommonFuBenEnterNum(pFuBenProgress);

			for ( ; pFuBenProgress->m_Level <= pFuBenCnfg->m_MapID.size(); ++pFuBenProgress->m_Level)
			{
				//获得单个地图怪物分布信息
				const std::vector<SMonsterOutput> * pvecMonster = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(pFuBenCnfg->m_MapID[pFuBenProgress->m_Level - 1]);

				if ( 0 == pvecMonster){

					TRACE("<error> %s : %d Line 获取单个地图怪物分布配置信息出错！！，地图ID = %d", __FUNCTION__, __LINE__, pFuBenCnfg->m_MapID[pFuBenProgress->m_Level - 1]);
					return;
				}

				for ( int i = 0; i < (*pvecMonster).size(); ++i)
				{
					if ( i < pFuBenProgress->m_KillMonsterNum){

						continue;
					}

					AutoCombatNpc Combat_Rsp;

					const SMonsterOutput & MonsterOut = (*pvecMonster)[i];

					UINT32 CombatIndex = 0;

					if ( bOpenHardType){

						CombatIndex = MonsterOut.m_CombatIndexHard;
					}else{

						CombatIndex = MonsterOut.m_CombatIndex;
					}

					TMonsterID MainMonsterID = g_pGameServer->GetConfigServer()->GetMainMonsterID(CombatIndex);

					if ( MainMonsterID == INVALID_MONSTER_ID){

						TRACE("<error> %s : %d Line 获取主将失败！！战斗索引 = %d", __FUNCTION__, __LINE__, CombatIndex);
						continue;
					}

					const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MainMonsterID);

					if ( 0 == pMonsterCnfg){

						TRACE("<error> %s : %d Line 获取怪物配置信息出错！！,怪物ID = %d", __FUNCTION__, __LINE__, MainMonsterID);
						continue;
					}

					++Rsp.m_Num;

					strncpy(Combat_Rsp.m_NpcName, pMonsterCnfg->m_szName,sizeof(Combat_Rsp.m_NpcName));

					INT32 Exp = pCombatPart->CalculateExp(m_pActor->GetCrtProp(enCrtProp_Level), pMonsterCnfg->m_Level);

					if ( Exp > 0){

						INT32 OldExp = m_pActor->GetCrtProp(enCrtProp_ActorExp);

						INT32 NewExp = 0;

						//给出战角色经验
						((IActor *)m_pActor)->CombatActorAddExp(Exp, true, &NewExp);

						Combat_Rsp.m_Exp = NewExp - OldExp;
					}

					UINT16 DropID = MonsterOut.m_DropID;

					if (bOpenHardType){

						DropID = MonsterOut.m_DropIDHard;
					}

					//掉落
					std::vector<TGoodsID> vectGoods;

					if ( DropID != 0){

						g_pGameServer->GetGoodsServer()->GiveUserDropGoods((IActor*)m_pActor,DropID,vectGoods);
					}

					Combat_Rsp.m_DropNum = vectGoods.size();

					ob << Combat_Rsp;

					for ( int index = 0; index < vectGoods.size(); ++index)
					{
						DropGoodsInfo GoodsInfo;

						GoodsInfo.m_GoodsID = vectGoods[index];
						GoodsInfo.m_GoodsNum = 1;

						m_pActor->SendGoodsCnfg(GoodsInfo.m_GoodsID);

						ob << GoodsInfo;
					}

					//发布事件
					SS_KillMonster KillMonster;
					KillMonster.m_MonsterUID = 0;

					UINT32 msgIDKill = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_KillMonster);
					m_pActor->OnEvent(msgIDKill,&KillMonster,sizeof(KillMonster));
				}
			}

			Rsp.m_PolyNimbus = this->GetFuBenThroughPolyNimbus();

			//发布事件
			SS_ThroughFuBen ThroughFuBen;
			ThroughFuBen.m_FuBenID = pFuBenCnfg->m_FuBenID;
			ThroughFuBen.m_FuBenType = pFuBenCnfg->m_Type;
			ThroughFuBen.m_Level   = pFuBenProgress->m_Level;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThroughFuBen);
			m_pActor->OnEvent(msgID,&ThroughFuBen,sizeof(ThroughFuBen));

			pFuBenProgress->m_Level = 1;
			pFuBenProgress->m_KillMonsterNum = 0;
			pFuBenProgress->m_SceneID = INVALID_SCENE_ID;

			//获得聚灵气
		}
	}

	OBuffer4k ob2;
	ob2 <<  SMainUIHeader(enMainUICmd_AutoCommonFuBen,sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){

		ob2 << ob;
	}

	m_pActor->SendData(ob2.TakeOsb());
}

//打通副本可以获得的聚灵气
INT32 FuBenPart::GetFuBenThroughPolyNimbus()
{
	INT32 PolyNimbus = 0;
	const SPolyNimbusCnfg * pPolyNimbusCnfg = g_pGameServer->GetConfigServer()->GetPolyNimbusCnfg(enGetPolyNimbusID_FuBenThrough);

	if(pPolyNimbusCnfg)
	{
		PolyNimbus = pPolyNimbusCnfg->m_PolyNimbusNum;
	}

	return PolyNimbus;
}

//挑战普通副本第一步
UINT8	FuBenPart::ChallengeComFuBenFirst(SFuBenProgressInfo * pFuBenProgress, TFuBenID FuBenID)
{
	enMainUICode Result  = 	enMainUICode_OK;

	if ( m_pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		return enMainUICode_ErrInDuoBao;	
	}

	if ( m_pActor->HaveTeam() )
	{
		//组队中，无法进入
		return enMainUICode_ErrHaveTeam;
	}

	if ( m_pActor->IsInFastWaitTeam() )
	{
		//等待组队中，无法操作
		return enMainUICode_ErrWaitTeam;
	}

	//是否已开始
	if( pFuBenProgress == 0 || !pFuBenProgress->IsStart() )
	{
		//检测是否可进入副本
		Result = (enMainUICode)this->Check_CanEnterFuBen(FuBenID);

		if ( enMainUICode_OK != Result ){
			return Result;
		}

		if ( 0 == pFuBenProgress ){
			//创建进度
			pFuBenProgress = this->CreateFuBenProgress(FuBenID);
		}
	}

	return Result;
}

//快速挑战帮派副本第一步
UINT8	FuBenPart::FastChallengeSynFuBenFirst(SFuBenProgressInfo * pFuBenProgress)
{
	if( m_pActor->HaveTeam() )
	{
		//无法进入，正在组队中
		return enSynRetCode_ErrHaveTeam;
	}
	
	if ( m_pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		return enSynRetCode_ErrInDuoBao;
	}

	if ( m_pActor->IsInFastWaitTeam() )
	{
		//等待组队中，无法操作
		return enSynRetCode_ErrWaitTeam;
	}

	//是否已开始
	if ( pFuBenProgress == 0 || !pFuBenProgress->IsStart())
	{
		ISyndicate * pSyn = m_pActor->GetSyndicate();

		ISyndicateMember * pSynMember = 0;

		if ( 0 != pSyn){
			pSynMember = pSyn->GetSynMember(m_pActor->GetUID());
		}

		if ( 0 == pSyn || 0 == pSynMember){

			//没帮派
			return enSynRetCode_NoSyndicate;

		}else if( this->GetEnterSynFuBenNum() >= this->GetMaxEnterSynFuBenNum()){

			//次数上限
			return enSynRetCode_ErrNumLimit;

		}else{

			const time_t NowTime = time(0);

			tm * pTm = localtime(&NowTime);
			if ( 0 == pTm){
				return 100;
			}

			UINT16  nTime = pTm->tm_hour * 100 + pTm->tm_min;

			UINT8 Level = m_pActor->GetCrtProp(enCrtProp_Level);

			const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(Level);

			const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();

			if ( nTime < ConfigParam.m_SynCombatBeginTime || nTime > ConfigParam.m_SynCombatCloseTime){

				return enSynRetCode_NotOpenCombat;

			}else if ( pSynCombatCnfg == 0){

				return enSynRetCode_ErrLevelLimit;

			}else if ( pSynCombatCnfg->m_vectEnterLevel.size() < 2){

				TRACE("<error> %s : %d 行 帮派副本配置错误！！", __FUNCTION__, __LINE__);
				return 100;

			}else if(pSynCombatCnfg->m_vectEnterLevel[0] > Level || pSynCombatCnfg->m_vectEnterLevel[1] < Level){

				return enSynRetCode_ErrEnterLevelLimit ;

			}else{

				if( pFuBenProgress == 0){
					SFuBenProgressInfo Info;
					Info.m_FuBenID = pSynCombatCnfg->m_SynFuBenID;
					Info.m_Level = 1;
					m_mapFuBen[pSynCombatCnfg->m_SynFuBenID] = Info;
				}
			}
		}
	}

	return enMainUICode_OK;
}

//挑战帮派副本第一步
UINT8	FuBenPart::ChallengeSynFuBenFirst(SFuBenProgressInfo * pFuBenProgress)
{
	if( m_pActor->HaveTeam() )
	{
		//无法进入，正在组队中
		return enMainUICode_ErrHaveTeam;
	}
	
	if ( m_pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		return enMainUICode_ErrInDuoBao;
	}

	if ( m_pActor->IsInFastWaitTeam() )
	{
		//等待组队中，无法操作
		return enMainUICode_ErrWaitTeam;
	}

	//是否已开始
	if ( pFuBenProgress == 0 || !pFuBenProgress->IsStart())
	{
		ISyndicate * pSyn = m_pActor->GetSyndicate();

		ISyndicateMember * pSynMember = 0;

		if ( 0 != pSyn){
			pSynMember = pSyn->GetSynMember(m_pActor->GetUID());
		}

		if ( 0 == pSyn || 0 == pSynMember){

			//没帮派
			return enMainUICode_NoSyndicate;

		}else if( this->GetEnterSynFuBenNum() >= this->GetMaxEnterSynFuBenNum()){

			//次数上限
			return enMainUICode_ErrNumLimit;

		}else{

			const time_t NowTime = time(0);

			tm * pTm = localtime(&NowTime);
			if ( 0 == pTm){
				return 100;
			}

			UINT16  nTime = pTm->tm_hour * 100 + pTm->tm_min;

			UINT8 Level = m_pActor->GetCrtProp(enCrtProp_Level);

			const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(Level);

			const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();

			if ( nTime < ConfigParam.m_SynCombatBeginTime || nTime > ConfigParam.m_SynCombatCloseTime){

				return enMainUICode_NotOpenCombat;

			}else if ( pSynCombatCnfg == 0){

				return enMainUICode_ErrLevel;

			}else if ( pSynCombatCnfg->m_vectEnterLevel.size() < 2){

				TRACE("<error> %s : %d 行 帮派副本配置错误！！", __FUNCTION__, __LINE__);
				return 100;

			}else if(pSynCombatCnfg->m_vectEnterLevel[0] > Level || pSynCombatCnfg->m_vectEnterLevel[1] < Level){

				return enMainUICode_ErrLevelMode ;

			}else{

				if( pFuBenProgress == 0){
					SFuBenProgressInfo Info;
					Info.m_FuBenID = pSynCombatCnfg->m_SynFuBenID;
					Info.m_Level = 1;
					m_mapFuBen[pSynCombatCnfg->m_SynFuBenID] = Info;
				}
			}
		}
	}

	return enMainUICode_OK;
}

//自动快速打帮派副本
void FuBenPart::SynFuBenAutoCombat()
{
	IPacketPart * pPacketPart = m_pActor->GetPacketPart();

	if ( 0 == pPacketPart){

		return;
	}

	SC_SynFuBenAutoCombat Rsp;

	OBuffer4k ob;

	if ( pPacketPart->GetSpace() < 10){

		Rsp.m_Result = enSynRetCode_SpaceLess;
	}

	SFuBenProgressInfo * pFuBenProgress = this->GetSynFuBenProgressInfo();

	if ( Rsp.m_Result == enMainUICode_OK){

		//第一步
		Rsp.m_Result = (enSynRetCode)this->FastChallengeSynFuBenFirst(pFuBenProgress);

		if ( Rsp.m_Result == enMainUICode_OK){

			pFuBenProgress = this->GetSynFuBenProgressInfo();
		}
	}

	if ( Rsp.m_Result == enMainUICode_OK && pFuBenProgress){

		const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

		if ( 0 == pFuBenCnfg){

			TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
			Rsp.m_Result = enSynRetCode_Err;

		}else if( !m_pActor->GetVipValue(enVipType_AutoKillSynCombat)){

			Rsp.m_Result = enSynRetCode_ErrVipLevel;

		}else if( !this->GetFuBenIsFinished(pFuBenProgress->m_FuBenID)){

			Rsp.m_Result = enSynRetCode_NotFinished;
		}else if ( m_pActor->IsInDuoBao() ){

			Rsp.m_Result = enSynRetCode_ErrInDuoBao;
		}else if ( m_pActor->HaveTeam() ){
		
			Rsp.m_Result = enSynRetCode_ErrHaveTeam;
		}else{

			//获得单个地图怪物分布信息
			if ( pFuBenProgress->m_Level <= 0 || pFuBenProgress->m_Level > pFuBenCnfg->m_MapID.size()){

				pFuBenProgress->m_Level = 1;
			}

			ICombatPart * pCombatPart = m_pActor->GetCombatPart();

			if ( 0 == pCombatPart){

				TRACE("<error> %s : %d Line 获取战斗PART出错！,userid = %d", __FUNCTION__, __LINE__, m_pActor->GetCrtProp(enCrtProp_ActorUserID));
				return;
			}

			this->AddSynFuBenEnterNum(pFuBenProgress);

			for ( ; pFuBenProgress->m_Level <= pFuBenCnfg->m_MapID.size(); ++pFuBenProgress->m_Level)
			{

				const std::vector<SMonsterOutput> * pvecMonster = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(pFuBenCnfg->m_MapID[pFuBenProgress->m_Level - 1]);

				if ( 0 == pvecMonster){

					TRACE("<error> %s : %d Line 获取单个地图怪物分布配置信息出错！！，地图ID = %d", __FUNCTION__, __LINE__, pFuBenCnfg->m_MapID[pFuBenProgress->m_Level - 1]);
					return;
				}

				for ( int i = 0; i < (*pvecMonster).size(); ++i)
				{
					if ( i < pFuBenProgress->m_KillMonsterNum){

						continue;
					}

					AutoSynCombatNpc Combat_Rsp;

					const SMonsterOutput & MonsterOut = (*pvecMonster)[i];

					const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MonsterOut.m_MonsterID);

					if ( 0 == pMonsterCnfg){

						TRACE("<error> %s : %d Line 获取怪物配置信息出错！！,怪物ID = %d", __FUNCTION__, __LINE__, MonsterOut.m_MonsterID);
						continue;
					}

					++Rsp.m_Num;

					strncpy(Combat_Rsp.m_NpcName, pMonsterCnfg->m_szName, sizeof(Combat_Rsp.m_NpcName));

					INT32 Exp = pCombatPart->CalculateExp(m_pActor->GetCrtProp(enCrtProp_Level), pMonsterCnfg->m_Level);

					if ( Exp > 0){

						INT32 OldExp = m_pActor->GetCrtProp(enCrtProp_ActorExp);

						INT32 NewExp = 0;

						//给出战角色经验
						((IActor *)m_pActor)->CombatActorAddExp(Exp, true, &NewExp);

						Combat_Rsp.m_Exp = NewExp - OldExp;
					}

					//掉落
					std::vector<TGoodsID> vectGoods;

					if ( MonsterOut.m_DropID != 0){

						g_pGameServer->GetGoodsServer()->GiveUserDropGoods((IActor*)m_pActor,MonsterOut.m_DropID,vectGoods);
					}

					Combat_Rsp.m_DropNum = vectGoods.size();

					ob << Combat_Rsp;

					for ( int index = 0; index < vectGoods.size(); ++index)
					{
						DropSynGoodsInfo GoodsInfo;

						GoodsInfo.m_GoodsID = vectGoods[index];
						GoodsInfo.m_GoodsNum = 1;

						m_pActor->SendGoodsCnfg(GoodsInfo.m_GoodsID);

						ob << GoodsInfo;
					}

					//发布事件
					SS_KillMonster KillMonster;
					KillMonster.m_MonsterUID = 0;

					UINT32 msgIDKill = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_KillMonster);
					m_pActor->OnEvent(msgIDKill,&KillMonster,sizeof(KillMonster));
				}

				const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(m_pActor->GetCrtProp(enCrtProp_Level));

				if ( 0 == pSynCombatCnfg){

					continue;
				}

				ISyndicate * pSyn = m_pActor->GetSyndicate();
				if ( 0 == pSyn){

					continue;
				}

				ISyndicateMember * pSynMember = pSyn->GetSynMember(m_pActor->GetUID());
				if ( 0 == pSynMember){

					continue;
				}

				if ( pFuBenProgress->m_Level >= pFuBenCnfg->m_MapID.size()){

					//通了
					Rsp.m_SynContribution += pSynCombatCnfg->m_NeiGeContribution;
					Rsp.m_SynExp		  += pSynCombatCnfg->m_DaTongExp;	

					pSyn->AddSynExp(pSynCombatCnfg->m_DaTongExp);

					pSynMember->AddContribution(pSynCombatCnfg->m_NeiGeContribution);

					//发布事件
					SS_SynCombat SynCombat;
					SynCombat.m_Mode = pSynCombatCnfg->m_Mode;

					UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_SynCombat);
					m_pActor->OnEvent(msgID,&SynCombat,sizeof(SynCombat));
				}else{
					//获得打完大院的帮派贡献
					Rsp.m_SynContribution += pSynCombatCnfg->m_DaYuanContribution;

					pSynMember->AddContribution(pSynCombatCnfg->m_DaYuanContribution);
				}
			}

			pFuBenProgress->m_Level = 1;
			pFuBenProgress->m_KillMonsterNum = 0;
			pFuBenProgress->m_SceneID = INVALID_SCENE_ID;
		}	
	}

	OBuffer4k ob2;
	ob2 <<  SyndicateHeader(enSyndicateCmd_SynFuBenAutoCombat,sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){

		ob2 << ob;
	}

	m_pActor->SendData(ob2.TakeOsb());
}

//检测是否要增加副本进入次数
void	FuBenPart::AddCommonFuBenEnterNum(SFuBenProgressInfo * pFuBenProgress)
{
	if ( 0 == pFuBenProgress){

		return;
	}

	if ( !pFuBenProgress->IsStart()){

		const SGameConfigParam & ConfigParam =g_pGameServer->GetConfigServer()->GetGameConfigParam();

		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			TRACE("<error> %s : %d 行 获取帮派管理为空！！",__FUNCTION__,__LINE__);
			return;
		}

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());

		if( GetFreeEnterFuBenNum() < ConfigParam.m_MaxFreeEnterFuBenNum){
			//免费进入
			m_FreeEnterFuBenNum++;
			m_LastFreeEnterFuBenTime = CURRENT_TIME();
		}else if( this->GetVipEnterCommonFuBenNum() < m_pActor->GetVipValue(enVipType_AddFuBenNum)){
			//VIP进入
			++m_VipEnterFuBenNum;
			m_LastVipEnterFuBenTime = CURRENT_TIME();
		}else if( GetCostStoneEnterFuBenNum() < ConfigParam.m_MaxCostStoneEnterFuBenNum){
			//灵石进入
			m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -ConfigParam.m_EnterFuBenCostStone);
			++m_CostStoneEnterFuBenNum;
			m_LastStoneEnterFuBenTiem = CURRENT_TIME();
		}else if(  0 != pSyndicate && GetSynWelfareEnterFuBenNum() < pSyndicate->GetWelfareValue(enWelfare_EnterFuBenNum)){
			//福利进入
			++m_SynWelfareEnterFuBenNum;
			m_LastSynWelfareFuBenTime = CURRENT_TIME();
		}
	}
}

//设置所进入的普通副本场景ID
void	FuBenPart::SetSceneIDFuBen(TSceneID SceneID)
{
	m_SceneIDFuBen =  SceneID;
}

//设置副本模式
void	FuBenPart::SetFuBenMode(enFuBenMode FuBenMode)
{
	m_FuBenMode = FuBenMode;
}

//增加帮派副本进入次数
void	FuBenPart::AddSynFuBenEnterNum(SFuBenProgressInfo * pFuBenProgress)
{
	if ( 0 == pFuBenProgress){

		return;
	}


	if( !pFuBenProgress->IsStart()){

		//增加进入次数
		UINT32 nCurTime = CURRENT_TIME();

		//先更新一下，当天进入次数
		if(XDateTime::GetInstance().IsSameDay(nCurTime, m_LastEnterSynFuBenTime)==false)
		{
			m_EnterSynFuBenNum = 0;
		}

		++m_EnterSynFuBenNum;

		m_LastEnterSynFuBenTime = nCurTime;
	}
}

//是否能开启困难模式
bool	FuBenPart::CanOpenHardType(TFuBenID FuBenID)
{
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if ( pFuBenCnfg == 0 ){

		TRACE("<error> %s : %d line 找不到副本配置信息 FuBenID = %d",__FUNCTION__,__LINE__,FuBenID);
		return false;
	}

	if ( pFuBenCnfg->m_Type != enFuBenType_Common){

		return false;
	}

	UINT8 LvNumCommonAndHardFB = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_LvNumCommonAndHardFB;

	if ( this->GetFuBenIsFinished(FuBenID) && pFuBenCnfg->m_EnterLevel >= LvNumCommonAndHardFB){

		return true;
	}

	return false;
}

//副本还可进次数
void	FuBenPart::GetFuBenNum(SFuBenNum & FuBenNum)
{
	const SGameConfigParam & GameCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	FuBenNum.m_FreeNum = GameCnfg.m_MaxFreeEnterFuBenNum - this->GetFreeEnterFuBenNum();

	FuBenNum.m_StoneNum = GameCnfg.m_MaxCostStoneEnterFuBenNum - this->GetCostStoneEnterFuBenNum();

	FuBenNum.m_SynWelNum = 0;

	ISyndicate * pSyndicate = m_pActor->GetSyndicate();

	if ( 0 != pSyndicate ){

		FuBenNum.m_SynWelNum = pSyndicate->GetWelfareValue(enWelfare_EnterFuBenNum) - this->GetSynWelfareEnterFuBenNum();

		if ( FuBenNum.m_SynWelNum < 0 ){

			FuBenNum.m_SynWelNum = 0;
		}
	}

	this->GetSynWelfareEnterFuBenNum();

	FuBenNum.m_VipNum	 = m_pActor->GetVipValue(enVipType_AddFuBenNum) - this->GetVipEnterCommonFuBenNum();
}

//组队挑战副本
void	FuBenPart::TeamChallengeFuBen(TFuBenID FuBenID, bool bSycProgress)
{
	SC_MainUIChallenge_Rsp Rsp;

	Rsp.m_FuBenID = FuBenID;

	Rsp.m_Result = (enMainUICode)this->Check_TeamChallenge(FuBenID);

	if ( enMainUICode_OK == Rsp.m_Result ){
		//可以进入副本
		SFuBenProgressInfo * pFuBenProgree = this->GetFuBenProgressInfo(FuBenID);

		if ( 0 == pFuBenProgree ){
			pFuBenProgree = this->CreateFuBenProgress(FuBenID);

			if ( 0 == pFuBenProgree ){
				TRACE("<error> %s : %d Line 创建副本进度失败！！,副本ID = %d", __FUNCTION__, __LINE__, FuBenID);
				return;
			}
		}

		ITeamPart * pTeamPart = m_pActor->GetTeamPart();

		if ( 0 == pTeamPart ){
			return;
		}

		IActor * pMember = pTeamPart->GetTeamMember();

		if ( 0 == pMember ){
			return;
		}

		IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

		if ( 0 == pFuBenPart ){
			return;
		}

		SFuBenProgressInfo * pMemberFuBenProgree = pFuBenPart->GetFuBenProgressInfo(FuBenID);

		if ( 0 == pMemberFuBenProgree ){
			pMemberFuBenProgree = pFuBenPart->CreateFuBenProgress(FuBenID);

			if ( 0 == pMemberFuBenProgree ){
				TRACE("<error> %s : %d Line 创建副本进度失败！！,副本ID = %d", __FUNCTION__, __LINE__, FuBenID);
				return;
			}
		}

		if ( !bSycProgress
			&& (pFuBenProgree->m_KillMonsterNum != pMemberFuBenProgree->m_KillMonsterNum
			|| pFuBenProgree->m_Level != pMemberFuBenProgree->m_Level) )
		{
			//进度不同
			Rsp.m_Result = enMainUICode_WaitSycProcess;

			if ( pFuBenProgree->m_Level > pMemberFuBenProgree->m_Level
				|| ( pFuBenProgree->m_Level == pMemberFuBenProgree->m_Level && pFuBenProgree->m_KillMonsterNum >= pMemberFuBenProgree->m_KillMonsterNum ))
			{
				//我的进度远，询问队友是否要同步进度
				pFuBenPart->AskSynProgress(pMemberFuBenProgree, pFuBenProgree, m_pActor);
			} else {
				//队友的进度远，询问我是否要同步进度
				this->AskSynProgress(pFuBenProgree, pMemberFuBenProgree, pMember);
			}

		} else {
			//进度相同或已选择同步,直接进副本
			if (  pFuBenProgree->m_Level > pMemberFuBenProgree->m_Level
				|| ( pFuBenProgree->m_Level == pMemberFuBenProgree->m_Level && pFuBenProgree->m_KillMonsterNum >= pMemberFuBenProgree->m_KillMonsterNum ))
			{
				Rsp.m_Result = (enMainUICode)this->EnterComFuBen(pFuBenProgree, true);
			} else {
				Rsp.m_Result = (enMainUICode)this->EnterComFuBen(pMemberFuBenProgree, true);
			}
		}

		Rsp.m_Level = pFuBenProgree->m_Level;
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_Challenge,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//检测是否可以组队挑战副本
UINT8	FuBenPart::Check_TeamChallenge(TFuBenID FuBenID)
{
	const SGameServerConfigParam & ServerCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if( 0 == pFuBenCnfg )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d",__FUNCTION__,__LINE__,FuBenID);
		return enMainUICode_ErrFuBenCnfg;
	}

	if ( pFuBenCnfg->m_EnterLevel < ServerCnfg.m_OpenTeamFuBenUserLevel ){
		return enMainUICode_ErrNoTeamFuBen;
	}

	if ( !this->GetFuBenIsFinished(FuBenID) ){
		return enMainUICode_NoFinishSingle;
	}

	if ( enActorDoing_None != m_pActor->GetActorDoing() ){
		return enMainUICode_NoFreeMe;
	}

	ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		return enMainUICode_Err;
	}

	IActor * pMember = pTeamPart->GetTeamMember();

	if ( 0 == pMember ){
		//没队伍，进入快速组队
		if ( this->HaveProcessFuBen(FuBenID) )
		{
			//有进度不能进行快速组队
			return enMainUICode_ErrHaveProgress;
		}
		else
		{
			g_pThingServer->GetTeamMgr().FuBenFastCreateTeam(m_pActor, FuBenID);
		}

		return enMainUICode_FastCreateTeam;
	}

	if ( pTeamPart->GetTeamLeader() != m_pActor ){
		return enMainUICode_OnlyLeadCan;
	}

	IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	if ( 0 == pFuBenPart ){
		return enMainUICode_Err;
	}

	if ( !pFuBenPart->GetFuBenIsFinished(FuBenID) ){
		return enMainUICode_NoFinishSingle;
	}

	if ( enActorDoing_None != pMember->GetActorDoing() ){
		return enMainUICode_NoFree;
	}

	if ( !this->HaveProcessFuBen(FuBenID) ){
		//没进度，次数检测
		enMainUICode RetCode = (enMainUICode)this->Check_CanEnterFuBen(FuBenID);

		if ( enMainUICode_OK != RetCode ){
			return RetCode;
		}
	}

	if ( !pFuBenPart->HaveProcessFuBen(FuBenID) ){
		//队友没进度，次数检测
		enMainUICode RetCode = (enMainUICode)pFuBenPart->Check_CanEnterFuBen(FuBenID);

		if ( enMainUICode_OK != RetCode ){
			return RetCode;
		}
	}

	return enMainUICode_OK;
}

//副本次数检测
UINT8	FuBenPart::Check_EnterFuBenNum()
{
	//判断今天进入次数
	INT32 FreeEnterFuBenNum			= this->GetFreeEnterFuBenNum();
	INT32 SynWelfareEnterFuBenNum	= this->GetSynWelfareEnterFuBenNum();
	INT32 CostStoneEnterFuBenNum	= this->GetCostStoneEnterFuBenNum();
	INT32 VipEnterFuBenNum			= this->GetVipEnterCommonFuBenNum();

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( FreeEnterFuBenNum < ConfigParam.m_MaxFreeEnterFuBenNum ){
		//免费进入
		return enMainUICode_OK;
	}

	if ( VipEnterFuBenNum < m_pActor->GetVipValue(enVipType_AddFuBenNum) ){
		//VIP进入
		return enMainUICode_OK;
	}

	if ( CostStoneEnterFuBenNum < ConfigParam.m_MaxCostStoneEnterFuBenNum ){
		//花费灵石进入
		if ( m_pActor->GetCrtProp(enCrtProp_ActorStone) <  ConfigParam.m_EnterFuBenCostStone ){

			return enMainUICode_ErrNoStone;
		}

		return enMainUICode_OK;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();

	if ( 0 == pSynMgr ){

		TRACE("<error> %s : %d 行 获取帮派管理为空！！",__FUNCTION__,__LINE__);
		return enMainUICode_ErrEnterNumLimit;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());

	if ( 0 != pSyndicate && pSyndicate->GetWelfareValue(enWelfare_EnterFuBenNum) > SynWelfareEnterFuBenNum ){
		//帮派福利进入
		return enMainUICode_OK;		
	}

	//进入次数限制
	return enMainUICode_ErrEnterNumLimit;
}

//得到副本是否有进度
bool	FuBenPart::HaveProcessFuBen(TFuBenID FuBenID)
{
	SFuBenProgressInfo * pFuBenProgree = this->GetFuBenProgressInfo(FuBenID);

	if ( 0 == pFuBenProgree || !pFuBenProgree->IsStart() )
		return false;

	return true;
}

//进入副本
UINT8	FuBenPart::EnterComFuBen(SFuBenProgressInfo * pFuBenProgress, bool bTeam)
{
	if ( 0 == pFuBenProgress ){
		return enMainUICode_Err;
	}

	//进入副本
	TFuBenID FuBenID = pFuBenProgress->m_FuBenID;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if( 0 == pFuBenCnfg || pFuBenProgress->m_Level > pFuBenCnfg->m_MapID.size() )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息FuBenID = %d Level = %d",__FUNCTION__,__LINE__,FuBenID,pFuBenProgress->m_Level);
		return enMainUICode_ErrFuBenCnfg;
		m_mapFuBen.erase(FuBenID);
	}
	else
	{
		EnterFuBen(pFuBenProgress,bTeam);
	}

	return enMainUICode_OK;
}

//创建副本进度
SFuBenProgressInfo * FuBenPart::CreateFuBenProgress(TFuBenID FuBenID)
{
	SFuBenProgressInfo Info;

	Info.m_FuBenID = FuBenID;
	Info.m_Level = 1;
	m_mapFuBen[FuBenID] = Info;

	return this->GetFuBenProgressInfo(FuBenID);
}

//检测是否可进入副本
UINT8	FuBenPart::Check_CanEnterFuBen(TFuBenID FuBenID)
{
	//次数检测
	enMainUICode RetCode = (enMainUICode)this->Check_EnterFuBenNum();

	if ( enMainUICode_OK != RetCode ){
		return RetCode;
	}

	//进入等级检测
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(FuBenID);

	if ( 0 == pFuBenCnfg ){
		TRACE("<error> %s : %d Line 获取副本配置信息出错！！副本ID = %d", __FUNCTION__, __LINE__, FuBenID);
		return enMainUICode_Err;
	}

	if ( m_pActor->GetCrtProp(enCrtProp_Level) < pFuBenCnfg->m_EnterLevel ){
		return enMainUICode_ErrLevel;
	}

	//是否正在组队副本中
	if ( this->IsInTeamFuBen() ){
		return enMainUICode_InTeamFuBen;
	}

	return enMainUICode_OK;
}

//询问是否同步进度
void	FuBenPart::AskSynProgress(SFuBenProgressInfo * pMeFuBenProgress, SFuBenProgressInfo * pMemberFuBenProgress, IActor * pMember)
{
	if ( 0 == pMemberFuBenProgress || 0 == pMeFuBenProgress ){
		return;
	}

	SC_SycProcess Rsp;

	Rsp.m_FuBenID = pMeFuBenProgress->m_FuBenID;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pMeFuBenProgress->m_FuBenID);

	if ( 0 == pFuBenCnfg ){
		TRACE("<error> %s : %d Line 获取副本配置信息出错！！副本ID = %d", __FUNCTION__, __LINE__, pMeFuBenProgress->m_FuBenID);
		return;
	}

	Rsp.m_MonsterNum = 0;

	for ( int i = 0; i < pFuBenCnfg->m_MapID.size(); ++i )
	{
		const std::vector<SMonsterOutput> * pMonstOutput = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(pFuBenCnfg->m_MapID[i]);

		if ( 0 == pMonstOutput )
			continue;

		Rsp.m_MonsterNum += (*pMonstOutput).size();

		if ( pMeFuBenProgress->m_Level >= i + 2 )
		{
			Rsp.m_KillNum += (*pMonstOutput).size();
		}

		if ( pMemberFuBenProgress->m_Level >= i + 2 )
		{
			Rsp.m_MemKillNum += (*pMonstOutput).size();
		}
	}

	Rsp.m_KillNum += pMeFuBenProgress->m_KillMonsterNum;
	
	Rsp.m_KillNum = Rsp.m_MonsterNum - Rsp.m_KillNum;			//显示剩余数

	Rsp.m_MemKillNum += pMemberFuBenProgress->m_KillMonsterNum;

	Rsp.m_MemKillNum = Rsp.m_MonsterNum - Rsp.m_MemKillNum;

	strncpy(Rsp.m_MemberName, pMember->GetName(), sizeof(Rsp.m_MemberName));

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_SC_SycProcess,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	//通知队友等待同步
	SC_PopWaitSyc RspPop;

	RspPop.m_bOpen = true;

	ob.Reset();
	ob << SMainUIHeader(enMainUICmd_PopWaitSyc,sizeof(Rsp)) << Rsp;
	pMember->SendData(ob.TakeOsb());
}

//是否正在组队副本中
bool	FuBenPart::IsInTeamFuBen()
{
	if ( m_FuBenMode != enFuBenMode_Team )
		return false;

	ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	if ( 0 == pTeamPart )
		return false;

	IActor * pMember = pTeamPart->GetTeamMember();

	if ( 0 == pMember )
		return false;

	if ( m_pActor->GetCrtProp(enCrtProp_SceneID) == m_SceneIDFuBen || pMember->GetCrtProp(enCrtProp_SceneID) == m_SceneIDFuBen )
	{
		return true;
	}

	return false;
}

//得到队友的进度
SFuBenProgressInfo * FuBenPart::GetTeamMemberFuBenProgress(TFuBenID FuBenID)
{
	ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		return 0;
	}

	IActor * pMember = pTeamPart->GetTeamMember();

	if ( 0 == pMember ){
		return 0;
	}

	IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	if ( 0 == pFuBenPart ){
		return 0;
	}

	return pFuBenPart->GetFuBenProgressInfo(FuBenID);
}

//战斗结束了
void  FuBenPart::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	//if(pCombatCnt->CombatID == this->m_CommonCombatID)
	if ( m_setCommonCombatID.find(pCombatCnt->CombatID) != m_setCommonCombatID.end() )
	{
		OnCommonCombatOver(pCombatCnt,pCombatResult,vectGoods);
	}
	else if(pCombatCnt->CombatID == this->m_GoldSwordCombatID)
	{
		OnGoldSwordCombatOver(pCombatCnt,pCombatResult,vectGoods);
	}
	else if(pCombatCnt->CombatID == this->m_SynGuardCombatID)
	{
		OnSynGuardCombatOver(pCombatCnt,pCombatResult,vectGoods);
	}

	if(pCombatCnt->DropID != 0 && !pCombatCnt->bIsAutoFighte && pCombatResult->m_bWin)
	{
		//掉落
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);
		if(pActor != 0)
		{
			g_pGameServer->GetGoodsServer()->GiveUserDropGoods(pActor,pCombatCnt->DropID,vectGoods);	
		}
	}

}

void FuBenPart::OnCommonCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	pCombatResult->m_bTeam = pCombatCnt->bIsTeam;

	SS_OnCombat EventCombat;

	EventCombat.CombatPlace = enCombatPlace_FuBen;
	EventCombat.bWin = pCombatResult->m_bWin;
	EventCombat.bIsTeam = pCombatCnt->bIsTeam;
	EventCombat.bHard = (pCombatCnt->Param == enCombatIndexMode_Hard);

	IActor * pBacker = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( 0 == pBacker )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnCombat);
	pBacker->OnEvent(msgID,&EventCombat,sizeof(EventCombat));

	if ( pCombatCnt->uidUser != pCombatCnt->uidSource )
		return;

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = m_EnterFuBenID;

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(m_EnterFuBenID);

	if(pFuBenProgress == 0)
	{
		return;
	}

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

	if(pFuBenCnfg == 0 )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
		return;
	}

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	SS_ThroughFuBen ThroughFuBen;

	if(pCombatResult->m_bWin)
	{
		//if ( enFuBenMode_Team == m_FuBenMode )
		if ( pCombatCnt->bIsTeam )
		{
			this->CommonTeamCombatWin(pCombatCnt, pFuBenProgress, CombatData.m_bOver);
		}
		else
		{
			this->CommonSingleCombatWin(pFuBenProgress, CombatData.m_bOver);
		}

		if ( CombatData.m_bOver )
		{
			ThroughFuBen.m_FuBenID = pFuBenProgress->m_FuBenID;
			ThroughFuBen.m_FuBenType = enFuBenType_Common;
			ThroughFuBen.m_Level   = pFuBenProgress->m_Level;
			pCombatResult->m_PolyNimbus = GetFuBenThroughPolyNimbus();

			//增加
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidSource);
			if(pActor != 0)
			{
				pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus,pCombatResult->m_PolyNimbus);
			}


		}
	}
	else if ( pCombatCnt->bIsTeam && !pCombatCnt->bIsAutoFighte )
	{
		//组队失败时，监听回场景事件
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

		m_pActor->SubscribeEvent(msgID,this,"FuBenPart::CommonTeamCombatWin");

	}

	OBuffer4k ob1;

	if( Rsp.m_Result == enCombatCode_OK){
		ob1 <<  SCombatHeader(enCombatCmd_CombatNpcInFuBen,sizeof(Rsp) + sizeof(CombatData) ) << Rsp << CombatData;
	}else{
		ob1 <<  SCombatHeader(enCombatCmd_CombatNpcInFuBen,sizeof(Rsp)) << Rsp;
	}

	m_pActor->SendData(ob1.TakeOsb());

	if(pCombatResult->m_bWin)
	{
		pCombatPart->OnKillMonster(pCombatCnt->uidEnemy);

		if(CombatData.m_bOver)
		{
			//发布事件
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThroughFuBen);
			m_pActor->OnEvent(msgID,&ThroughFuBen,sizeof(ThroughFuBen));
		}
	}
}

void FuBenPart::OnGoldSwordCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{

	IGodSword * pGodSword = (IGodSword*)g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,m_uidGodSword);

	if(pGodSword==0)
	{
		return;
	}

	//剑诀等级
	INT32 SwordLevel = 0;

	bool bUpgrade = false; //仙剑是否升级了

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();


	TMapID MapID = m_SceneIDGodSward.GetMapID();
	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);
	if(pVect==0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, MapID = %d ",__FUNCTION__,__LINE__ ,MapID);
		return;
	}

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = m_EnterFuBenID;

	INT32 KillNpcNum = 0;

	//获取副本配置信息
	const SGodSwordFuBenCnfg * pSwordFuBenCnfg =  this->GetSwordFuBenCnfg(pGodSword,m_FuBenLevel);

	if( pCombatResult->m_bWin){
		//增加杀怪数量		        
		pGodSword->AddPropNum(m_pActor,enGoodsProp_KillNpcNum,1,&KillNpcNum);

		if(KillNpcNum == 1) //第一次击杀，增加进入次数
		{
			//先更新一下，当天进入时间及级别
			pGodSword->SetPropNum(m_pActor,enGoodsProp_FuBenLevel,m_FuBenLevel);
			pGodSword->SetPropNum(m_pActor,enGoodsProp_LastEnterTime,CURRENT_TIME());
			++m_EnterGodSwordWorldNum;
		}

		m_LastEnterFuBenGodSword = CURRENT_TIME();

		if(KillNpcNum>=pVect->size())
		{
			//结束了
			CombatData.m_bOver = true;

			this->SetFuBenFinished(m_EnterFuBenID, true);

			pGodSword->SetPropNum(m_pActor,enGoodsProp_FuBenLevel,0);
			pGodSword->SetPropNum(m_pActor,enGoodsProp_KillNpcNum,0);

			//剑诀等级
			pGodSword->GetPropNum(enGoodsProp_SecretLevel,SwordLevel);	

			if(SwordLevel<pSwordFuBenCnfg->m_SwordSecretLevel)
			{	
				SwordLevel = pSwordFuBenCnfg->m_SwordSecretLevel;
				bUpgrade = true;
				pGodSword->SetPropNum(m_pActor,enGoodsProp_SecretLevel,pSwordFuBenCnfg->m_SwordSecretLevel);	
				CombatData.m_SecretLevel = pSwordFuBenCnfg->m_SwordSecretLevel;
			}
		}
	}

	OBuffer4k ob2;

	ob2 << SCombatHeader(enCombatCmd_CombatNpcInFuBen, sizeof(Rsp) + sizeof(CombatData)) << Rsp << CombatData ; 

	m_pActor->SendData(ob2.TakeOsb());

	//需要先把战斗结果发给客户端，才可以触发事件

	if(pCombatResult->m_bWin)
	{
		pCombatPart->OnKillMonster(pCombatCnt->uidEnemy);

		if(CombatData.m_bOver )
		{						
			if(bUpgrade)
			{	
				//发布升级剑决事件
				SS_UpSecretLevel UpSecretLevel;
				UpSecretLevel.m_Level = SwordLevel;

				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpSecretLevel);
				m_pActor->OnEvent(msgID,&UpSecretLevel,sizeof(UpSecretLevel));

			}

			//发布通过副本事件
			SS_ThroughFuBen ThroughFuBen;
			ThroughFuBen.m_FuBenID = m_EnterFuBenID;
			ThroughFuBen.m_FuBenType = enFuBenType_GoldSword;
			ThroughFuBen.m_Level   = SwordLevel;		

			UINT32 msgID2 = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThroughFuBen);
			m_pActor->OnEvent(msgID2,&ThroughFuBen,sizeof(ThroughFuBen));
		}
	}
}


void FuBenPart::OnSynGuardCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	ISyndicateMgr * pSyndicateMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSyndicateMgr){
		TRACE("<error> %s : %d line 获取不到SyndicateMgr",__FUNCTION__,__LINE__);
		return; 
	}

	//当前模式
	const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfg(m_pActor->GetCrtProp(enCrtProp_Level));

	if(pSynCombatCnfg==0)
	{
		TRACE("<error> %s : %d line 找不到帮战模式配置信息, 等级 = %d ",__FUNCTION__,__LINE__ ,m_pActor->GetCrtProp(enCrtProp_Level));
		return;
	}

	SC_CombatCombatNpcInFuBen_Rsp Rsp;
	Rsp.m_Result = enCombatCode_OK;

	CombatNpcInFuBenData  CombatData;
	CombatData.m_FuBenID = m_EnterFuBenID;


	//发布事件
	SS_SynCombat SynCombat;

	OBuffer4k ob;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(m_EnterFuBenID);
	if( 0 == pFuBenCnfg){
		TRACE("<error> %s : %d 行 获取副本配置信息失败！！,副本ID = %d", __FUNCTION__, __LINE__, m_EnterFuBenID);
		return;
	}

	SFuBenProgressInfo * pFuBenProgress = GetFuBenProgressInfo(m_EnterFuBenID);

	TMapID mapID = pFuBenProgress->m_SceneID.GetMapID();

	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(mapID);
	if(pVect==0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, MapID = %d ",__FUNCTION__,__LINE__ ,mapID);
		return;
	}


	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(mapID);
	if( 0 == pMapConfig){
		TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, mapID);
		return;
	}




	ICombatPart * pCombatPart = m_pActor->GetCombatPart();


	if(pCombatResult->m_bWin)
	{
		this->AddSynFuBenEnterNum(pFuBenProgress);

		++pFuBenProgress->m_KillMonsterNum;

		//判断本场景的怪是否都击杀完毕了
		if( pFuBenProgress->m_KillMonsterNum >= pVect->size())
		{
			//是否还有场景可换
			int Index = 0;
			for(;Index<pFuBenCnfg->m_MapID.size();)
			{
				++Index;
				if(pFuBenCnfg->m_MapID[Index - 1] == mapID )
				{
					break;
				}
			}

			//下个场景
			int NextIndex = Index +1;

			if(Index>=pFuBenCnfg->m_MapID.size())
			{
				//结束了
				CombatData.m_bOver = true;

				ISyndicate * pSyn = m_pActor->GetSyndicate();
				if( 0 != pSyn){
					//获得打通的帮派经验和打完内阁得到的帮派贡献
					ISyndicateMember * pSynMember = pSyn->GetSynMember(m_pActor->GetUID());
					if( 0 != pSynMember){
						pCombatResult->m_SynContribution = pSynCombatCnfg->m_NeiGeContribution;
						pCombatResult->m_SynExp			 = pSynCombatCnfg->m_DaTongExp;	

						pSyn->AddSynExp(pSynCombatCnfg->m_DaTongExp);

						pSynMember->AddContribution(pSynCombatCnfg->m_NeiGeContribution);						
					}
				}

				SynCombat.m_Mode = pSynCombatCnfg->m_Mode;

				pFuBenProgress->m_Level = 1;
				pFuBenProgress->m_KillMonsterNum = 0;
			}
			else
			{

				this->SetFuBenFinished(pFuBenProgress->m_FuBenID, true);

				//注册事件
				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

				m_pActor->SubscribeEvent(msgID,this,"FuBenPart::CombatNpcSyn");

				pFuBenProgress->m_Level = pFuBenProgress->m_Level + 1;

				pFuBenProgress->m_KillMonsterNum = 0;

				ISyndicate * pSyn = m_pActor->GetSyndicate();
				if( 0 != pSyn){
					//获得打通的帮派经验和打完内阁得到的帮派贡献
					ISyndicateMember * pSynMember = pSyn->GetSynMember(m_pActor->GetUID());
					if( 0 != pSynMember){
						//获得打完大院的帮派贡献
						pCombatResult->m_SynContribution = pSynCombatCnfg->m_DaYuanContribution;

						pSynMember->AddContribution(pSynCombatCnfg->m_DaYuanContribution);
					}
				}
			}
		}
	}


	OBuffer4k ob2;

	ob2 <<  SCombatHeader(enCombatCmd_CombatNpcInFuBen,sizeof(Rsp) + sizeof(CombatData)) << Rsp << CombatData ;

	m_pActor->SendData(ob2.TakeOsb());

	if(pCombatResult->m_bWin)
	{
		pCombatPart->OnKillMonster(pCombatCnt->uidEnemy);

		if(pCombatResult->m_SynExp>0)
		{
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_SynCombat);
			m_pActor->OnEvent(msgID,&SynCombat,sizeof(SynCombat));

		}
	}
}

//普通单人副本战斗胜利
bool	FuBenPart::CommonSingleCombatWin(SFuBenProgressInfo * pFuBenProgress, bool & bOver)
{
	this->AddCommonFuBenEnterNum(pFuBenProgress);

	pFuBenProgress->m_KillMonsterNum++;

	//判断是否需要切换到下一场
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

	if(pFuBenCnfg == 0 )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
		return false;
	}

	TMapID MapID = pFuBenCnfg->m_MapID[pFuBenProgress->m_Level-1];

	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);
	if(pVect == 0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, FuBenID = %d MapID=%d",__FUNCTION__,__LINE__ ,pFuBenCnfg->m_FuBenID,MapID);
		return false;
	}

	if(pFuBenProgress->m_KillMonsterNum == pVect->size())
	{
		pFuBenProgress->m_Level++; //升一级
		pFuBenProgress->m_KillMonsterNum = 0;

		//是否还有下一级的配置
		if(pFuBenProgress->m_Level<=pFuBenCnfg->m_MapID.size())
		{
			//注册事件
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

			m_pActor->SubscribeEvent(msgID,this,"FuBenPart::CombatNpcCommon");

		}
		else
		{
			if ( enCombatIndexMode_Com == m_CombatMode )
			{
				this->SetFuBenFinished(pFuBenProgress->m_FuBenID, true);
			}
			else
			{
				this->SetHardFuBenFinished(pFuBenProgress->m_FuBenID, true);
			}

			pFuBenProgress->m_Level=1;
			pFuBenProgress->m_KillMonsterNum = 0;

			bOver = true;
		}
	}

	return true;
}

//普通组队副本战斗胜利
bool	FuBenPart::CommonTeamCombatWin(const SCombatContext * pCombatCnt, SFuBenProgressInfo * pFuBenProgress, bool & bOver)
{
	//判断是否需要切换到下一场
	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

	if(pFuBenCnfg == 0 )
	{
		TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
		return false;
	}

	TMapID MapID = pFuBenCnfg->m_MapID[pFuBenProgress->m_Level-1];

	const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);
	if(pVect == 0)
	{
		TRACE("<error> %s : %d line 找不到地图怪物配置信息, FuBenID = %d MapID=%d",__FUNCTION__,__LINE__ ,pFuBenCnfg->m_FuBenID,MapID);
		return false;
	}

	if ( !pCombatCnt->bIsAutoFighte )
	{
		//非自动模式的才加进度和次数
		this->AddCommonFuBenEnterNum(pFuBenProgress);

		pFuBenProgress->m_KillMonsterNum++;

		//注册事件
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

		m_pActor->SubscribeEvent(msgID,this,"FuBenPart::CommonTeamCombatWin");

		if(pFuBenProgress->m_KillMonsterNum == pVect->size())
		{
			++pFuBenProgress->m_Level; //升一级
			pFuBenProgress->m_KillMonsterNum = 0;

			//是否还有下一级的配置
			if( pFuBenProgress->m_Level <= pFuBenCnfg->m_MapID.size())
			{
			}
			else
			{			
				pFuBenProgress->m_Level = 1;

				bOver = true;
			}
		}

		//同步下队友进度
		ITeamPart * pTeamPart = m_pActor->GetTeamPart();

		if ( 0 == pTeamPart )
			return false;

		IActor * pMember = pTeamPart->GetTeamMember();

		if ( 0 != pMember )
		{
			IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

			if ( 0 == pFuBenPart )
				return false;

			SFuBenProgressInfo * pMemberProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

			if ( 0 == pMemberProgress )
				return false;

			pFuBenPart->AddCommonFuBenEnterNum(pMemberProgress);

			this->SynTeamLeaderProgress(pFuBenProgress, pMember);
		}
	}
	else
	{
		//队长自动模式后，检测下队友是否是自动模式，不是的话，给加进度和次数
		for ( int i = 0; i < pCombatCnt->vectActor.size(); ++i )
		{
			IFighter* pFighter = pCombatCnt->vectActor[i];

			if ( 0 == pFighter )
				continue;

			if ( pFighter->IsAutoFighte() )
				//自动模式，不给算进度
				continue;

			IActor * pMemberActor = g_pGameServer->GetGameWorld()->FindActor(pFighter->GetUID());

			if ( 0 == pMemberActor || pMemberActor == m_pActor || pMemberActor->GetMaster() != 0 )
				continue;

			IFuBenPart * pFuBenPart = pMemberActor->GetFuBenPart();

			if ( 0 == pFuBenPart )
				return false;

			SFuBenProgressInfo * pMemberProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

			if ( 0 == pMemberProgress )
				return false;

			pFuBenPart->AddCommonFuBenEnterNum(pMemberProgress);

			++pMemberProgress->m_KillMonsterNum;

			if(pMemberProgress->m_KillMonsterNum == pVect->size())
			{
				++pMemberProgress->m_Level; //升一级
				pMemberProgress->m_KillMonsterNum = 0;

				//是否还有下一级的配置
				if( pMemberProgress->m_Level <= pFuBenCnfg->m_MapID.size())
				{
				}
				else
				{			
					pMemberProgress->m_Level = 1;
				}
			}
		}	
	}


	return true;


	////同步下队友进度
	//ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	//if ( 0 == pTeamPart )
	//	return false;

	//IActor * pMember = pTeamPart->GetTeamMember();

	//if ( 0 != pMember )
	//{
	//	IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	//	if ( 0 == pFuBenPart )
	//		return false;

	//	SFuBenProgressInfo * pMemberProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

	//	if ( 0 == pMemberProgress )
	//		return false;

	//	pFuBenPart->AddCommonFuBenEnterNum(pMemberProgress);

	//	this->SynTeamLeaderProgress(pFuBenProgress, pMember);
	//}
	//else
	//{
	//	//队友离队了,从参战角色中获取
	//	for ( int i = 0; i < pCombatCnt->vectActor.size(); ++i )
	//	{
	//		IFighter* pFighter = pCombatCnt->vectActor[i];

	//		if ( 0 == pFighter )
	//			continue;

	//		if ( pFighter->IsAutoFighte() )
	//			//自动模式，不给算进度
	//			continue;

	//		IActor * pMemberActor = g_pGameServer->GetGameWorld()->FindActor(pFighter->GetUID());

	//		if ( 0 == pMemberActor || pMemberActor->GetMaster() != 0)
	//			continue;

	//		if ( pMemberActor == m_pActor )
	//			continue;

	//		IFuBenPart * pFuBenPart = pMemberActor->GetFuBenPart();

	//		if ( 0 == pFuBenPart )
	//			return false;

	//		SFuBenProgressInfo * pMemberProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

	//		if ( 0 == pMemberProgress )
	//			return false;

	//		pFuBenPart->AddCommonFuBenEnterNum(pMemberProgress);

	//		++pMemberProgress->m_KillMonsterNum;

	//		if(pMemberProgress->m_KillMonsterNum == pVect->size())
	//		{
	//			++pMemberProgress->m_Level; //升一级
	//			pMemberProgress->m_KillMonsterNum = 0;

	//			//是否还有下一级的配置
	//			if( pMemberProgress->m_Level <= pFuBenCnfg->m_MapID.size())
	//			{
	//			}
	//			else
	//			{			
	//				pMemberProgress->m_Level = 1;
	//			}
	//		}

	//		//this->SynTeamProgress(pFuBenProgress, pMemberActor);
	//	}
	//}
	//
	//return true;







	//ITeamPart * pTeamPart = m_pActor->GetTeamPart();

	//if ( 0 == pTeamPart )
	//	return false;

	//IActor * pMember = pTeamPart->GetTeamMember();

	//if ( 0 == pMember )
	//{
	//	//此时队友已离队
	//	return false;
	//}

	

	//IFuBenPart * pFuBenPart = pMember->GetFuBenPart();

	//if ( 0 == pFuBenPart )
	//	return false;

	//SFuBenProgressInfo * pMemberProgress = pFuBenPart->GetFuBenProgressInfo(pFuBenProgress->m_FuBenID);

	//if ( 0 == pMemberProgress )
	//{
	//	TRACE("<error> %s : %d 找不到队友的副本进度", __FUNCTION__, __LINE__);
	//	return false;
	//}

	//pFuBenPart->AddCommonFuBenEnterNum(pMemberProgress);

	//if ( pFuBenProgress->m_Level > pMemberProgress->m_Level
	//	|| (pFuBenProgress->m_Level == pMemberProgress->m_Level && pFuBenProgress->m_KillMonsterNum >= pMemberProgress->m_KillMonsterNum) )
	//{
	//	++pFuBenProgress->m_KillMonsterNum;

	//	pMemberProgress->m_KillMonsterNum = pFuBenProgress->m_KillMonsterNum;
	//	pMemberProgress->m_Level		  = pFuBenProgress->m_Level;
	//	pMemberProgress->m_SceneID		  = INVALID_SCENE_ID;
	//}
	//else
	//{
	//	++pMemberProgress->m_KillMonsterNum;

	//	pFuBenProgress->m_KillMonsterNum = pMemberProgress->m_KillMonsterNum;
	//	pFuBenProgress->m_Level			 = pMemberProgress->m_Level;
	//	pFuBenProgress->m_SceneID		 = pMemberProgress->m_SceneID;
	//	pMemberProgress->m_SceneID		 = INVALID_SCENE_ID;	
	//}

	////判断是否需要切换到下一场
	//const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pFuBenProgress->m_FuBenID);

	//if(pFuBenCnfg == 0 )
	//{
	//	TRACE("<error> %s : %d line 找不到副本配置信息 pFuBenProgress->m_FuBenID = %d",__FUNCTION__,__LINE__,pFuBenProgress->m_FuBenID);
	//	return false;
	//}

	//TMapID MapID = pFuBenCnfg->m_MapID[pFuBenProgress->m_Level-1];

	//const std::vector<SMonsterOutput> * pVect = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);
	//if(pVect == 0)
	//{
	//	TRACE("<error> %s : %d line 找不到地图怪物配置信息, FuBenID = %d MapID=%d",__FUNCTION__,__LINE__ ,pFuBenCnfg->m_FuBenID,MapID);
	//	return false;
	//}

	////注册事件
	//UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

	//m_pActor->SubscribeEvent(msgID,this,"FuBenPart::CommonTeamCombatWin");

	//if(pFuBenProgress->m_KillMonsterNum == pVect->size())
	//{
	//	++pFuBenProgress->m_Level; //升一级
	//	pFuBenProgress->m_KillMonsterNum = 0;

	//	++pMemberProgress->m_Level;
	//	pMemberProgress->m_KillMonsterNum = 0;

	//	//是否还有下一级的配置
	//	if(pFuBenProgress->m_Level<=pFuBenCnfg->m_MapID.size())
	//	{
	//		NULL;
	//	}
	//	else
	//	{			
	//		pFuBenProgress->m_Level = 1;

	//		pMemberProgress->m_Level = 1;

	//		bOver = true;
	//	}
	//}

	return true;
}

//设置困难副本完成记录
void	FuBenPart::SetHardFuBenFinished(TFuBenID FuBenID, bool bFinish)
{
	m_HardFinishedFuben.set(FuBenID, bFinish);
}

//得到困难副本是否完成过
bool	FuBenPart::GetHardFuBenIsFinished(TFuBenID FuBenID)
{
	return m_HardFinishedFuben.get(FuBenID);
}
