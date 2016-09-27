
#include "TalismanGame.h"
#include "IActor.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IGameWorld.h"
#include "ITalisman.h"
#include "IPacketPart.h"
#include "IBasicService.h"
#include "RandomService.h"
#include "IGoodsServer.h"

TalismanGame::TalismanGame(UID uidTalisman) : m_uidTalisman(uidTalisman)
{
	static UINT32 s_GameID = 0;

	m_GameID = ++s_GameID;

	if(m_GameID == 0)
	{
		m_GameID = ++s_GameID;
	}
	
	m_GameState = enTalismanGameState_Max;

	m_pTalismanWorldCnfg = 0;

	ITalisman * pTalisman = g_pGameServer->GetGameWorld()->GetTalisman(m_uidTalisman);

	if(pTalisman != 0)
	{
		 const SGoodsCnfg * pGoodsCnfg = pTalisman->GetGoodsCnfg(); 

		m_pTalismanWorldCnfg = g_pGameServer->GetConfigServer()->GetTalismanWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);
	}
}

TalismanGame::~TalismanGame()
{
}


void TalismanGame::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_Ready:
		{
			GameStart();			
		}
		break;
	case enTimerID_Over:
		{
			GameOver();
		}
		break;
	default:
		break;
	}
}

//法宝界配置
const STalismanWorldCnfg * TalismanGame::GetTalismanWorldCnfg()
{
	return m_pTalismanWorldCnfg;
}

//发送消息给客户端
void  TalismanGame::SendGameMsgToClient(UINT8 SubCmd,void * buffer,INT32 len)
{
	CSC_TalismanWorldGame  WorldGame;
	WorldGame.m_GameID = GetGameID();
	WorldGame.m_SubCmd = SubCmd;

	OBuffer1k ob;

	ob << TalismanWorldHeader(enTalismanWorldCmd_Game,sizeof(CSC_TalismanWorldGame)+len) << WorldGame;

	if(buffer && len>0)
	{
		ob.Push(buffer,len);
	}

	m_vectActor[0]->SendData(ob.TakeOsb());
}

//游戏开始
 void TalismanGame::GameStart()
 {
	 m_GameState = enTalismanGameState_Start;

	 g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Ready,this);

	//启动定时器
	
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Over,this,m_pTalismanWorldCnfg->m_TotalGameTime*1000,"TalismanGame::GameStart[enTimerID_Over]");
 }

 //游戏结束
 void TalismanGame::GameOver()
 {
	  g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Over,this);
	  m_GameState = enTalismanGameState_Over;

	  INT32 QualityPoint = 0;

	 UINT8 FinishLevel = GetFinishLevel(QualityPoint);

	 UINT32 AdventureAwardID = 0;

	  	 //获得的奇遇ID
	 if( 0 != FinishLevel){
			
		const std::hash_map<UINT16, SAdventureAwardCnfg> * pAwardCnfg = g_pGameServer->GetConfigServer()->GetAdventureAwardCnfgVect(m_pTalismanWorldCnfg->m_TalismanWorldID);

		if( 0 == pAwardCnfg){
			return;
		}

		if( (*pAwardCnfg).size() == 0){
			TRACE("<error> %s : %d 行 奇遇配置错误!!", __FUNCTION__, __LINE__);
			return;
		}

		UINT32 nRandom = RandomService::GetRandom() % (*pAwardCnfg).size();

		std::hash_map<UINT16, SAdventureAwardCnfg>::const_iterator iter = pAwardCnfg->begin();
		
		std::advance(iter,nRandom);;

		AdventureAwardID = (*iter).first;		

		//给玩家奖励(级别)
	   this->GiveFinishLevelAward(QualityPoint);
	  //给玩家奖励(奇遇)
	  this->GiveFinishAdventureAward(AdventureAwardID);
	 }

	 OnNoticClientGameOver(FinishLevel, AdventureAwardID, QualityPoint);

	//发布事件
	SS_TalismanWorld TalismanWorld;
	TalismanWorld.m_TalismanWorldID = m_pTalismanWorldCnfg->m_TalismanWorldID;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TalismanWorld);

	for( int  i = 0; i < m_vectActor.size(); ++i)
	{
		IActor * pActor = m_vectActor[i];
		if( 0 == pActor){
			continue;
		}

		if( 0 != pActor->GetMaster()){
			continue;
		}

		m_vectActor[i]->OnEvent(msgID,&TalismanWorld,sizeof(TalismanWorld));
	}
 }

//给奖励(级别和奇遇)
bool TalismanGame::GiveFinishLevelAward(INT32 QualityPoint)
{
	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(m_uidTalisman);
	if( 0 == pGoods){
		return false;
	}

	ITalisman * pTalisman = (ITalisman *)pGoods;

	//完成级别奖励

	if( pTalisman->IsSpawnGoods()){
		//只有在孕育时才加品质上去
		pTalisman->UpgradeQuality(m_vectActor[0], QualityPoint);
	}
	return true;
}

//给奖励(奇遇)
bool TalismanGame::GiveFinishAdventureAward(UINT16 AdventureAwardID)
{
	//奇遇奖励
	const SAdventureAwardCnfg * pAdventureAwardCnfg = g_pGameServer->GetConfigServer()->GetAdventureAwardCnfg(AdventureAwardID);
	if( 0 == pAdventureAwardCnfg){
		TRACE("<error> %s : %d Line 找不到奇遇配置信息 奇遇ID = %d",__FUNCTION__,__LINE__,AdventureAwardID);
		return false;	
	}

	switch(pAdventureAwardCnfg->m_RewardType)
	{
	case enRewardType_Goods:
		{
			IActor * pActor = m_vectActor[0];
			if( 0 == pActor){
				return false;
			}

			IPacketPart * pPacketPart = pActor->GetPacketPart();
			if( 0 == pPacketPart){
				return false;
			}

			pPacketPart->AddGoods(pAdventureAwardCnfg->m_Param);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_TalismanWorld,pAdventureAwardCnfg->m_Param,UID(),1,"法宝世界获得物品");
		}
		break;
	default:
		{
		TRACE("<error> %s : %d Line 奇遇没有这种奖励类别 奇遇类别 = %d",__FUNCTION__,__LINE__,pAdventureAwardCnfg->m_RewardType);
		}
		break;
	}
	return true;
}

//启动游戏
bool TalismanGame::Start(std::vector<IActor*> & vectActor)
{
	if(m_pTalismanWorldCnfg == 0)
	{
		return false;
	}

	m_vectActor = vectActor;
	if(m_vectActor.size()==0)
	{
		return false;
	}

	m_GameState = enTalismanGameState_Ready;

	//启动定时器
	 const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
	
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Ready,this,ConfigParam.m_TamlismanGameReadyCountDown*1000,"TalismanGame::Start[enTimerID_Ready]");

	//通知客户端初始化
	NotifyInitClient();

	return true;
}

//获得游戏状态
enTalismanGameState TalismanGame::GetGameState()
{
	return m_GameState;
}


//关闭游戏
void TalismanGame::Close()
{
	g_pGameServer->GetTimeAxis()->KillAllTimer(this);
	m_GameState = enTalismanGameState_Max;
}

//获得游戏ID
UINT32 TalismanGame::GetGameID()
{
	return m_GameID ;
} 
