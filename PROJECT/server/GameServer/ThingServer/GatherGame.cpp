
#include "GatherGame.h"
#include "TalismanGameProtocol.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "RandomService.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "XDateTime.h"
#include "IGoodsServer.h"

GatherGame::GatherGame(UID uidTalisman): TalismanGame(uidTalisman)
{
	          m_CurGatherIndex= -1;  
	       
	          m_MaxGatherNum=0;      
}


GatherGame::~GatherGame()
{
}



			//游戏消息
void GatherGame::OnMessage(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	switch(nSubCmd)
	{
	case enGameGatherCmd_Detect:  //探测
		{
			OnDetect(pActor, nSubCmd,ib);
		}
		break;	

	 case enGameGatherCmd_Gather:  //采集
	 {
		OnGather(pActor, nSubCmd,ib);
	  }
		break;
	default:
		break;
	}
}


		//子类需要实现的接口

	//初始化客户端
 void GatherGame::NotifyInitClient()
 {
	 SC_GatherGame_Init GameInitCnt;


	 const SGatherGameParam & GatherGameParam = g_pGameServer->GetConfigServer()->GetGatherGameParam();

	 const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	 const SGatherGameConfig * pGatherGameConfig = g_pGameServer->GetConfigServer()->GetGatherGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	 if(pGatherGameConfig==0)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);
		 return;
	 }

	 

	 GameInitCnt.m_XDetectNum = GatherGameParam.m_XDetectPoint;
	 GameInitCnt.m_YDetectNum = GatherGameParam.m_YDetectPoint;
	 GameInitCnt.m_StartAfterTime    = ConfigParam.m_TamlismanGameReadyCountDown;
	 GameInitCnt.m_GameTotalTime     = this->m_pTalismanWorldCnfg->m_TotalGameTime;

	 for(int i=0; i<ARRAY_SIZE(GameInitCnt.m_ColorRange) && i < GatherGameParam.m_vectColorRange.size();i++)
	 {
		 GameInitCnt.m_ColorRange[i] = GatherGameParam.m_vectColorRange[i];
	 }

	
	 SendGameMsgToClient(enGameGatherCmd_InitClient,&GameInitCnt,sizeof(GameInitCnt));
 }

	//游戏开始
 void GatherGame::GameStart()
 {
	  Super::GameStart();

	 const SGatherGameConfig * pGatherGameConfig = g_pGameServer->GetConfigServer()->GetGatherGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	  const SGatherGameParam & GatherGameParam = g_pGameServer->GetConfigServer()->GetGatherGameParam();

	 if(pGatherGameConfig==0)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);
		 return;
	 }

	  //选取宝物放到探测点
	 if(pGatherGameConfig->m_vectGoods.empty() || pGatherGameConfig->m_TotalProbability<1)
	 {
		 TRACE("<error> %s : %d line 找不到寻宝类游戏宝物配置信息 TalismanWorldID=%d",__FUNCTION__,__LINE__,m_pTalismanWorldCnfg->m_TalismanWorldID);

		 return;
	 }

	 for(int i=0; i<pGatherGameConfig->m_BaoGoodsNum;i++)
	 {
		 UINT32 Random = RandomService::GetRandom() % pGatherGameConfig->m_TotalProbability;
		 for(int j=0; j<pGatherGameConfig->m_vectGoods.size()/2;j++)
		 {
			 if(Random<pGatherGameConfig->m_vectGoods[j*2+1])
			 {
				 SGatherGoodsInfo Info;
				 Info.m_DetectPoint =  RandomService::GetRandom() % ( GatherGameParam.m_XDetectPoint*GatherGameParam.m_YDetectPoint);
				 Info.m_GoodsID = pGatherGameConfig->m_vectGoods[j*2];
				 Info.m_GatherNum = 0;
				 break;
			 }
		 }
	 }

	SendGameMsgToClient(enGameGatherCmd_Start,0,0);
 }

	//游戏结束
 void GatherGame::GameOver()
 {
	  Super::GameOver();
 }

 void GatherGame::OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)
 {
	 SC_GatherGame_Over Rsp;
	 Rsp.m_AdventureAwardID = AdventureAwardID;
	 Rsp.m_FinishLevel = FinishLevel;
	 Rsp.m_MaxGatherNum = m_MaxGatherNum;

	 this->SendGameMsgToClient(enGameGatherCmd_Over,&Rsp,sizeof(Rsp));
 }

	//完成级别
UINT8 GatherGame::GetFinishLevel(INT32 & QualityPoint)
{
	const std::vector<SGatherGameAwardCnfg> & vectGameAwardCnfg = g_pGameServer->GetConfigServer()->GetGatherGameAwardCnfg();

	 const SGatherGameConfig * pGatherGameConfig = g_pGameServer->GetConfigServer()->GetGatherGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	 UINT8 FinishLevel = 0;

	//从最高级别开始判断
	for( int i = vectGameAwardCnfg.size() - 1; i >= 0; --i)
	{
		QualityPoint = vectGameAwardCnfg[i].m_AwardQuality;

		FinishLevel =  vectGameAwardCnfg[i].m_FinishLevel;

		//获得宝物数量
		if( m_MaxGatherNum >= vectGameAwardCnfg[i].m_GatherNum)
		{
			break;
		}

	}

	
	return FinishLevel;
}

void GatherGame::OnDetect(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	CS_GatherGame_Detect_Req Req;

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

	if(m_CurGatherIndex>=0)
	{
		OnGatherTimer();
	}

	if(State != enTalismanGameState_Start)
	{
		return;
	}

	SC_GatherGame_Detect_Rsp Rsp;
	Rsp.m_DetectPoint = Req.m_DetectPoint;

	//最小距离
	INT32 minDistance = INT_MAX;

	for(int i=0; i<m_vectBaoGoods.size();i++)
	{
		INT32 Distance = CalculateDistance(Req.m_DetectPoint,m_vectBaoGoods[i].m_DetectPoint);

		minDistance = std::min(minDistance,Distance);

		if(minDistance<=1)
		{
			Rsp.m_GoodsID = m_vectBaoGoods[i].m_GoodsID;
			
			m_CurGatherIndex = i;

			StartTimer();

			break;
		}
	}

	 const SGatherGameParam & GatherGameParam = g_pGameServer->GetConfigServer()->GetGatherGameParam();

	if(minDistance>1)
	{
		for(int i= GatherGameParam.m_vectColorRange.size()-1; i>=0; i--)
		{
			if(minDistance <= GatherGameParam.m_vectColorRange[i])
			{
				Rsp.m_IndicatorColor = i;
				break;
			}
		}
	}


	SendGameMsgToClient(enGameGatherCmd_Detect,&Rsp,SIZE_OF(Rsp));	

}

void GatherGame::OnGather(IActor*,UINT8 nSubCmd,IBuffer & ib)
{
	if(m_CurGatherIndex<0 || m_CurGatherIndex>=m_vectBaoGoods.size())
	{
		return ;
	}	

	CS_GatherGame_Gather_Req Req;

	ib >> Req;

	
	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端数据长度有误 nSubCmd=%d",__FUNCTION__,__LINE__,nSubCmd);
		return;
	}


	m_vectBaoGoods[m_CurGatherIndex].m_GatherNum++;

	SC_GatherGame_Gather_Rsp Rsp;
	Rsp.m_DetectPoint = Req.m_DetectPoint;
	Rsp.m_TotalGatherNum = m_vectBaoGoods[m_CurGatherIndex].m_GatherNum;

	SendGameMsgToClient(enGameGatherCmd_Gather,&Rsp,SIZE_OF(Rsp));	
}

//计算两个探测点的距离
INT32 GatherGame::CalculateDistance(INT32 first,INT32 second)
{
	 const SGatherGameParam & GatherGameParam = g_pGameServer->GetConfigServer()->GetGatherGameParam();

	//距离 = |row1-row2| + |col1-col2|
	//行号
	INT32 row1 = first/GatherGameParam.m_XDetectPoint;
	INT32 row2 = second/GatherGameParam.m_XDetectPoint;

	//列号
	INT32 col1 = first/GatherGameParam.m_YDetectPoint;
	INT32 col2 = second/GatherGameParam.m_YDetectPoint;

	return std::abs(row1-row2) + std::abs(col1-col2);
}

void GatherGame::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_Gather:
		  OnGatherTimer();
		break;
	default:
		Super::OnTimer(timerID);
		break;
	}
}

	//启动采集定时器
	void GatherGame::StartTimer()
	{
		const SGatherGameConfig * pGatherGameConfig = g_pGameServer->GetConfigServer()->GetGatherGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
		g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Gather,this,pGatherGameConfig->m_GatherTime*1000);
	}

	//停止定时器
	void GatherGame::StopTimer()
	{
		 g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Gather,this);
	}

void	GatherGame::OnGatherTimer()
{
	const SGatherGameConfig * pGatherGameConfig = g_pGameServer->GetConfigServer()->GetGatherGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);

	StopTimer();

	if(m_CurGatherIndex<0)
	{
		return;
	}

	  const SGatherGameParam & GatherGameParam = g_pGameServer->GetConfigServer()->GetGatherGameParam();

	  //获得物品数
	  INT32 GoodsNum = 0;

	  for(int i=0; i<GatherGameParam.m_vectAwardGoodsNum.size()/2; i++)
	  {
		  if(m_vectBaoGoods[m_CurGatherIndex].m_GatherNum<GatherGameParam.m_vectAwardGoodsNum[i*2+1])
		  {
			  break;
		  }

		  GoodsNum = GatherGameParam.m_vectAwardGoodsNum[i*2];
	  }

	  SC_GatherGame_ObtainGoods ObtainGoods;

	 ObtainGoods.m_DetectPoint = m_vectBaoGoods[m_CurGatherIndex].m_DetectPoint;
	 ObtainGoods.m_GoodsID = m_vectBaoGoods[m_CurGatherIndex].m_GoodsID;
	 ObtainGoods.m_GoodsNum = GoodsNum;

	  if(GoodsNum>0)
	  {
		  IActor * pActor = m_vectActor[0];
		  if( 0 == pActor){
				return;
		  }

		  IPacketPart * pPacketPart = pActor->GetPacketPart();

		  if(pPacketPart->AddGoods(ObtainGoods.m_GoodsID,ObtainGoods.m_GoodsNum)==false)
		  {
			  //发送邮件
			  NULL;
		  }else{
			  g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_TalismanWorld,ObtainGoods.m_GoodsID,UID(),ObtainGoods.m_GoodsNum,"法宝世界获得物品");
		  }

	  }



	 SendGameMsgToClient(enGameGatherCmd_ObtainGoods,&ObtainGoods,SIZE_OF(ObtainGoods));	


	m_CurGatherIndex = -1;

		//判断游戏是否结束
	if(m_vectBaoGoods.size()==0 )
	{
		GameOver();
	}

}
