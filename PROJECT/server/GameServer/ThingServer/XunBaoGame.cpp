
#include "XunBaoGame.h"
#include "TalismanGameProtocol.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "RandomService.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "XDateTime.h"
#include "IGoodsServer.h"

XunBaoGame::XunBaoGame(UID uidTalisman): TalismanGame(uidTalisman)
{
	          m_CurDetectedNum=0;  //当前已探测次数
	          m_ObtainGoodsNum=0;  //已获得宝物数量
	          m_GameBegin=0;       //游戏开始时间
}


XunBaoGame::~XunBaoGame()
{
}



			//游戏消息
void XunBaoGame::OnMessage(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	switch(nSubCmd)
	{
	case enGameXunBaoCmd_Detect:  //打击气团
		{
			OnDetect(pActor, nSubCmd,ib);
		}
		break;	
	default:
		break;
	}
}


		//子类需要实现的接口

	//初始化客户端
 void XunBaoGame::NotifyInitClient()
 {
	 SC_XunBaoGame_Init GameInitCnt;


	 const SXunBaoGameParam & XunBaoGameParam = g_pGameServer->GetConfigServer()->GetXunBaoGameParam();

	 const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	 const SXunBaoGameConfig * pXunBaoGameConfig = g_pGameServer->GetConfigServer()->GetXunBaoGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	 if(pXunBaoGameConfig==0)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);
		 return;
	 }

	 

	 GameInitCnt.m_XDetectNum = XunBaoGameParam.m_XDetectPoint;
	 GameInitCnt.m_YDetectNum = XunBaoGameParam.m_YDetectPoint;
	 GameInitCnt.m_StartAfterTime    = ConfigParam.m_TamlismanGameReadyCountDown;
	 GameInitCnt.m_GameTotalTime     = this->m_pTalismanWorldCnfg->m_TotalGameTime;
	 GameInitCnt.m_TotalDetectNum = pXunBaoGameConfig->m_TotalDetectNum;

	 for(int i=0; i<ARRAY_SIZE(GameInitCnt.m_ColorRange) && i<XunBaoGameParam.m_vectColorRange.size();i++)
	 {
		 GameInitCnt.m_ColorRange[i] = XunBaoGameParam.m_vectColorRange[i];
	 }

	
	 SendGameMsgToClient(enGameXunBaoCmd_InitClient,&GameInitCnt,sizeof(GameInitCnt));
 }

	//游戏开始
 void XunBaoGame::GameStart()
 {
	  Super::GameStart();

	 const SXunBaoGameConfig * pXunBaoGameConfig = g_pGameServer->GetConfigServer()->GetXunBaoGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	  const SXunBaoGameParam & XunBaoGameParam = g_pGameServer->GetConfigServer()->GetXunBaoGameParam();

	 if(pXunBaoGameConfig==0)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);
		 return;
	 }

	  //选取宝物放到探测点
	 if(pXunBaoGameConfig->m_vectGoods.empty() || pXunBaoGameConfig->m_TotalProbability<1)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏宝物配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);

		 return;
	 }

	 for(int i=0; i<pXunBaoGameConfig->m_BaoGoodsNum;i++)
	 {
		 UINT32 Random = RandomService::GetRandom() % pXunBaoGameConfig->m_TotalProbability;
		 for(int j=0; j<pXunBaoGameConfig->m_vectGoods.size()/3;j++)
		 {
			 if(Random<pXunBaoGameConfig->m_vectGoods[j*3+2])
			 {
				 SBaoGoodsInfo Info;
				 Info.m_DetectPoint =  RandomService::GetRandom() % ( XunBaoGameParam.m_XDetectPoint*XunBaoGameParam.m_YDetectPoint);
				 Info.m_GoodsID = pXunBaoGameConfig->m_vectGoods[j*3];
				 Info.m_PileNum = pXunBaoGameConfig->m_vectGoods[j*3+1];
				 break;
			 }
		 }
	 }

	SendGameMsgToClient(enGameXunBaoCmd_Start,0,0);

	m_GameBegin = CURRENT_TIME();
 }

	//游戏结束
 void XunBaoGame::GameOver()
 {
	  Super::GameOver();
 }

 void XunBaoGame::OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)
 {
	 SC_XunBaoGame_Over Rsp;
	 Rsp.m_AdventureAwardID = AdventureAwardID;
	 Rsp.m_FinishLevel = FinishLevel;
	 Rsp.m_TotalGoodsNum = m_ObtainGoodsNum;

	 this->SendGameMsgToClient(enGameXunBaoCmd_Over,&Rsp,sizeof(Rsp));
 }

	//完成级别
UINT8 XunBaoGame::GetFinishLevel(INT32 & QualityPoint)
{
	const std::vector<SXunBaoGameAwardCnfg> & vectGameAwardCnfg = g_pGameServer->GetConfigServer()->GetXunBaoGameAwardCnfg();

	 const SXunBaoGameConfig * pXunBaoGameConfig = g_pGameServer->GetConfigServer()->GetXunBaoGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	 time_t gameTime = CURRENT_TIME() - m_GameBegin;

	//从最高级别开始判断
	for( int i = vectGameAwardCnfg.size() - 1; i >= 0; --i)
	{
		//获得宝物数量
		if( vectGameAwardCnfg[i].m_ObtainGoodsNum == -1){
			if(pXunBaoGameConfig->m_BaoGoodsNum != m_ObtainGoodsNum){
				continue;
			}
		}
		else if(m_ObtainGoodsNum< pXunBaoGameConfig->m_BaoGoodsNum)
		{
			continue;
		}

		if(vectGameAwardCnfg[i].m_TotalTimeLimit != -1)
		{
		
		  if( gameTime > vectGameAwardCnfg[i].m_TotalTimeLimit)
		  {
			continue;
		  }
		}
		
		QualityPoint = vectGameAwardCnfg[i].m_AwardQuality;

		return vectGameAwardCnfg[i].m_FinishLevel;
	}

	
	return 0;
}

void XunBaoGame::OnDetect(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	CS_XunBaoGame_Detect_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端数据长度有误 nSubCmd=%d",__FUNCTION__,__LINE__,nSubCmd);
		return;
	}

	enTalismanGameState State =  this->GetGameState();

	if(State != enTalismanGameState_Start)
	{
		return;
	}

	SC_XunBaoGame_Detect_Rsp Rsp;
	Rsp.m_DetectPoint = Req.m_DetectPoint;

	//最小距离
	INT32 minDistance = INT_MAX;

	for(int i=0; i<m_vectBaoGoods.size();i++)
	{
		INT32 Distance = CalculateDistance(Req.m_DetectPoint,m_vectBaoGoods[i].m_DetectPoint);

		minDistance = std::min(minDistance,Distance);

		if(minDistance==0)
		{
			Rsp.m_GoodsID = m_vectBaoGoods[i].m_GoodsID;

			Rsp.m_GoodsNum =  m_vectBaoGoods[i].m_PileNum;

			m_ObtainGoodsNum++;

			//移除
			m_vectBaoGoods.erase(m_vectBaoGoods.begin()+i);
			break;
		}
	}

	 const SXunBaoGameParam & XunBaoGameParam = g_pGameServer->GetConfigServer()->GetXunBaoGameParam();

	if(minDistance==0)
	{
		if(false ==pActor->GetPacketPart()->AddGoods(Rsp.m_GoodsID,Rsp.m_GoodsNum))
		{
			//发邮件
			NULL;
		}else{
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_TalismanWorld,Rsp.m_GoodsID,UID(),Rsp.m_GoodsNum,"法宝世界获得物品");
		}
	}
	else
	{
		for(int i= XunBaoGameParam.m_vectColorRange.size()-1; i>=0; i--)
		{
			if(minDistance <= XunBaoGameParam.m_vectColorRange[i])
			{
				Rsp.m_IndicatorColor = i;
				break;
			}
		}
	}


	SendGameMsgToClient(enGameXunBaoCmd_Detect,&Rsp,SIZE_OF(Rsp));

	m_CurDetectedNum++;

	const SXunBaoGameConfig * pXunBaoGameConfig = g_pGameServer->GetConfigServer()->GetXunBaoGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);


	//判断游戏是否结束
	if(m_vectBaoGoods.size()==0 || m_CurDetectedNum==pXunBaoGameConfig->m_TotalDetectNum)
	{
		GameOver();
	}

}

//计算两个探测点的距离
INT32 XunBaoGame::CalculateDistance(INT32 first,INT32 second)
{
	 const SXunBaoGameParam & XunBaoGameParam = g_pGameServer->GetConfigServer()->GetXunBaoGameParam();

	//距离 = |row1-row2| + |col1-col2|
	//行号
	INT32 row1 = first/XunBaoGameParam.m_XDetectPoint;
	INT32 row2 = second/XunBaoGameParam.m_XDetectPoint;

	//列号
	INT32 col1 = first/XunBaoGameParam.m_YDetectPoint;
	INT32 col2 = second/XunBaoGameParam.m_YDetectPoint;

	return std::abs(row1-row2) + std::abs(col1-col2);
}
