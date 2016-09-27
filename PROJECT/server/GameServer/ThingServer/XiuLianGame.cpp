
#include "XiuLianGame.h"
#include "GameSrvProtocol.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "RandomService.h"
#include "XDateTime.h"
#include "ITalisman.h"
#include "IBasicService.h"

XiuLianGame::XiuLianGame(UID uidTalisman) : TalismanGame(uidTalisman)
{
	m_HitWhiteAirMassNum = 0;  //击中的白气团数
	m_HitBlackAirMassNum = 0;  //击中的黑气团数
	m_TotalWhiteAirMassNum = 0; //总共的白气团数
	m_TotalBlackAirMassNum = 0; //总共的黑气团数

	m_TimeSection		 = 0;
}

 XiuLianGame::~XiuLianGame()
 {
 }

	//子类需要实现的接口

	//初始化客户端
 void XiuLianGame::NotifyInitClient()
 {
	 SC_XiuLianGame_Init InitCnt;

	 const SXiuLianGameParam & XiuLianGameParam = g_pGameServer->GetConfigServer()->GetXiuLianGameParam();

	  const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	 
	 InitCnt.m_BlackAirMassScore = XiuLianGameParam.m_BlackAirMassNimbus;
	 InitCnt.m_WhiteAirMassScore = XiuLianGameParam.m_WhiteAirMassNimbus;
	 InitCnt.m_StartAfterTime    = ConfigParam.m_TamlismanGameReadyCountDown;
	 InitCnt.m_TotalGameTime     = this->m_pTalismanWorldCnfg->m_TotalGameTime;

	 SendGameMsgToClient(enGameXiuLianCmd_InitClient,&InitCnt,sizeof(InitCnt));
	 
 }

	//游戏开始
void XiuLianGame::GameStart()
{
	Super::GameStart();

	const SXiuLianGameParam XiuLianGameParam = g_pGameServer->GetConfigServer()->GetXiuLianGameParam();
	//随机得到时间轴
	if( 0 == XiuLianGameParam.m_vectAirMassTimeScale.size()){
		TRACE("<error> %s : %d 行 时间轴配置有错误!!", __FUNCTION__, __LINE__);
		return;
	}

	UINT32 TimeShaft = RandomService::GetRandom() % XiuLianGameParam.m_vectAirMassTimeScale.size();
	m_vectTimeShaft = XiuLianGameParam.m_vectAirMassTimeScale[TimeShaft];

	//启动定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_AirMass,this,XiuLianGameParam.m_MinGenerateAirMassIntervalTime,"XiuLianGame::GameStart");

	SendGameMsgToClient(enGameXiuLianCmd_Start,0,0);
}

void XiuLianGame::OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)
{
	 const SXiuLianGameParam & GameParam = g_pGameServer->GetConfigServer()->GetXiuLianGameParam();

	  QualityPoint = 0;

	 SC_XiuLianGame_Over Rsp;

	 Rsp.m_FinishLevel = FinishLevel;
	 Rsp.m_AdventureAwardID = AdventureAwardID;

	 Rsp.m_HitBlackAirMassNum = m_HitBlackAirMassNum;
	 Rsp.m_HitWhiteAirMassNum = m_HitWhiteAirMassNum;
	 Rsp.m_TotalBlackAirMassNum = m_TotalBlackAirMassNum;
	 Rsp.m_TotalWhiteAirMassNum =m_TotalWhiteAirMassNum;
	 Rsp.m_TotalNimbus = (m_TotalWhiteAirMassNum-m_HitWhiteAirMassNum)*GameParam.m_WhiteAirMassNimbus - (m_TotalBlackAirMassNum-m_HitBlackAirMassNum)*GameParam.m_BlackAirMassNimbus;

	this->SendGameMsgToClient(enGameXiuLianCmd_Over,&Rsp,sizeof(Rsp));
}

	//游戏结束
 void XiuLianGame::GameOver()
 {
	 Super::GameOver();

	 g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_AirMass,this);	
 }

	//完成级别
UINT8 XiuLianGame::GetFinishLevel(INT32 & QualityPoint)
{
	std::vector<SXiuLianGameAwardCnfg> & vectGameAwardCnfg = g_pGameServer->GetConfigServer()->GetXiuLianGameAwardCnfg();

	//从最高级别开始判断
	for( int i = vectGameAwardCnfg.size() - 1; i >= 0; --i)
	{
		if( vectGameAwardCnfg[i].m_DeterBlackAirMassNum == -1){
			if( m_TotalBlackAirMassNum > 0){
				continue;
			}
		}
		
		if( m_HitBlackAirMassNum < vectGameAwardCnfg[i].m_DeterBlackAirMassNum){
			continue;
		}
		
		if( vectGameAwardCnfg[i].m_AllowWhiteAirMassNum != -1){
			if( m_HitWhiteAirMassNum > vectGameAwardCnfg[i].m_AllowWhiteAirMassNum){
				continue;
			}
		}

		QualityPoint = vectGameAwardCnfg[i].m_AwardQuality;

		return vectGameAwardCnfg[i].m_FinishLevel;
	}

	return 0;
}


void XiuLianGame::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_AirMass:
		  GenerateAirMass();
		break;
	default:
		Super::OnTimer(timerID);
		break;
	}
}

//游戏消息
void XiuLianGame::OnMessage(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	switch(nSubCmd)
	{
	case enGameXiuLianCmd_Hit:  //打击气团
		{
			OnHitAirMass(pActor, nSubCmd,ib);
		}
		break;
	case enGameXiuLianCmd_EndPoint:  //气团到达终点
		{
			OnAirMassEndPoint(pActor, nSubCmd,ib);
		}
		break;
	default:
		TRACE("<warning> %s : %d 行 收到意外的命令字 %d！！", __FUNCTION__,__LINE__,nSubCmd);
		break;
	}
}

//产生气团
void  XiuLianGame::GenerateAirMass()
{
	UINT64 NowTime = CURRENT_TIME();

	const SXiuLianGameParam & GameParam = g_pGameServer->GetConfigServer()->GetXiuLianGameParam();

	if(GameParam.m_AirMassLifeTimeByAddr.size()==0)
	{
		return;
	}

	//随机取一个产生气团地址
	UINT32 randnum = RandomService::GetRandom() % GameParam.m_AirMassLifeTimeByAddr.size();
	if(randnum >= m_vectLastGenerateTime.size())
	{
		m_vectLastGenerateTime.resize(1+randnum);
	}else
	{
		if( (NowTime * 1000 - m_vectLastGenerateTime[randnum]) < GameParam.m_SameAddrGenerateAirMassIntervalTime ){
			//时间间隔不满足的，返回等下轮
			return;
		}
	}

	if( m_vectAirMassType.size() == 0){
		//重新生成按时间比例的云团集合
		this->GenerateAirMassVect();
	}

	//颜色索引
	UINT8 nIndexColor = RandomService::GetRandom() % m_vectAirMassType.size();

	static UINT32 s_AirMass = 0;

	SAirMassInfo MassInfo;

	MassInfo.m_AirMassID = ++ s_AirMass;

	MassInfo.m_AirMassType = m_vectAirMassType[nIndexColor];
	//删除这个索引
	std::vector<enAirMassType>::iterator iter = m_vectAirMassType.begin();
	m_vectAirMassType.erase(iter + nIndexColor);

	MassInfo.m_EndTime = NowTime * 1000 + GameParam.m_AirMassLifeTimeByAddr[randnum] ;

	m_mapAirMass[MassInfo.m_AirMassID] = MassInfo;

	m_vectLastGenerateTime[randnum] = MassInfo.m_EndTime ;

	if(MassInfo.m_AirMassType == enAirMassType_White)
	{
		m_TotalWhiteAirMassNum++;
	}
	else
	{
		m_TotalBlackAirMassNum++;
	}

	SC_XiuLianGame_BornAirMass BornAirMass;

	BornAirMass.m_AirMassID = MassInfo.m_AirMassID ;
	BornAirMass.m_AirMassType = MassInfo.m_AirMassType;
	BornAirMass.m_BornAddr = randnum;
	BornAirMass.m_EndTime = MassInfo.m_EndTime;

	this->SendGameMsgToClient(enGameXiuLianCmd_BornAirMass,&BornAirMass,sizeof(BornAirMass));


}

void XiuLianGame::OnHitAirMass(IActor*,UINT8 nSubCmd,IBuffer & ib) //击打气团
{
	CS_XiuLianGame_HitAirMass_Req Req;
	ib >> Req;
	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端数据长度有误 nSubCmd=%d",__FUNCTION__,__LINE__,nSubCmd);
		return;
	}

	SAirMassInfo * pAirMassInfo = GetAirMassInfo(Req.m_AirMassID);

	if(pAirMassInfo==0)
	{
		return ;
	}

	if(pAirMassInfo->m_AirMassType == enAirMassType_White)
	{
		m_HitWhiteAirMassNum++;
	}
	else
	{
		m_HitBlackAirMassNum++;
	}

	m_mapAirMass.erase(Req.m_AirMassID);

	CS_XiuLianGame_HitAirMass_Rsp Rsp;
	Rsp.m_AirMassID = Req.m_AirMassID;

	this->SendGameMsgToClient(enGameXiuLianCmd_Hit,&Rsp,sizeof(Rsp));

}

void XiuLianGame::OnAirMassEndPoint(IActor*,UINT8 nSubCmd,IBuffer & ib)  //击打气团
{
	CSC_XiuLianGame_EndPoint Req;
	ib >> Req;
	if(ib.Error())
	{
		return;
	}

	SAirMassInfo * pAirMassInfo = GetAirMassInfo(Req.m_AirMassID);

	if(pAirMassInfo==0)
	{
		return ;
	}

	m_mapAirMass.erase(Req.m_AirMassID);

	CSC_XiuLianGame_EndPoint Rsp;
	Rsp.m_AirMassID = Req.m_AirMassID;

	this->SendGameMsgToClient(enGameXiuLianCmd_EndPoint,&Rsp,sizeof(Rsp));

}

SAirMassInfo * XiuLianGame::GetAirMassInfo(UINT32 AirMassID)
{
	MAP_AIRMASS::iterator it = m_mapAirMass.find(AirMassID);

	if(it != m_mapAirMass.end())
	{
		return 0;
	}
	
	return &(*it).second;
}

//预先生成时间轴的第几阶段的气团类型集合
void	XiuLianGame::GenerateAirMassVect()
{
	if( m_TimeSection >= m_vectTimeShaft.size()){
		return;
	}

	//这个阶段的持续时间
	UINT32 nNumSecond = 0;
	if( m_TimeSection >= 2){
		nNumSecond = m_vectTimeShaft[m_TimeSection] - m_vectTimeShaft[m_TimeSection - 2];
	}else{
		nNumSecond = m_vectTimeShaft[m_TimeSection];
	}

	const SXiuLianGameParam XiuLianGameParam = g_pGameServer->GetConfigServer()->GetXiuLianGameParam();
	//计算这个阶段会生成多少个云团
	UINT32 nCount = nNumSecond * 1000 / XiuLianGameParam.m_MinGenerateAirMassIntervalTime;

	//获得生成黑白云团的比例
	UINT16 Proportion = m_vectTimeShaft[m_TimeSection + 1];

	//获得就生成多少个白云团
	UINT32 nWhiteNum  = nCount * Proportion / 100;
	//获得就生成多少个黑云团
	UINT32 nBlackNum  = nCount - nWhiteNum;

	for( int i = 0; i < nWhiteNum; ++i)
	{	//放入nWhiteNum个白云团
		m_vectAirMassType.push_back(enAirMassType_White);
	}

	for( int k = 0; k < nBlackNum; ++k)
	{	//放入nWhiteNum个黑云团
		m_vectAirMassType.push_back(enAirMassType_Black);
	}

	m_TimeSection += 2;
}
