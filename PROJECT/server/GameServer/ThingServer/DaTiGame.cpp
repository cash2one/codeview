#include "DaTiGame.h"
#include "ThingServer.h"
#include <set>
#include "IBasicService.h"
#include "RandomService.h"
#include "XDateTime.h"


DaTiGame::DaTiGame(UID uidTalisman) : TalismanGame(uidTalisman)
{
	m_NowTiMuIndex = 0;

	m_TotalRightTiMuNum = 0;

	m_GameBeginTime = 0;
}

DaTiGame::~DaTiGame()
{
}

void DaTiGame::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_TimeOut:
		{
			//问题回答超时
			this->SendGameMsgToClient(enGamDaTiCmd_TimeOut, 0, 0);

			//下一道题
			this->__SendNextTiMuData();		
		}
		break;
	default:
		Super::OnTimer(timerID);
		break;
	}
}

//游戏消息
void DaTiGame::OnMessage(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	switch(nSubCmd)
	{
	case enGamDaTiCmd_DaTi:
		{
			this->__UserDaTi(pActor, nSubCmd, ib);
		}
		break;
	default:
		TRACE("<warning> %s : %d 行 收到意外的命令字 %d！！", __FUNCTION__,__LINE__,nSubCmd);
		break;
	}
}


//初始化客户端
void DaTiGame::NotifyInitClient()
{
	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	const SDaTiGameCnfg * pDaTiGameCnfg = g_pGameServer->GetConfigServer()->GetDaTiCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pDaTiGameCnfg){
		return;
	}

	SC_DaTiGame_Init DaTiGame_Init;
	DaTiGame_Init.m_GameTotalTime  = this->m_pTalismanWorldCnfg->m_TotalGameTime;
	DaTiGame_Init.m_StartAfterTime = ConfigParam.m_TamlismanGameReadyCountDown;;
	DaTiGame_Init.m_TotalTiMu	   = pDaTiGameCnfg->m_GameTiMuNum + pDaTiGameCnfg->m_JXCQTiMuNum + pDaTiGameCnfg->m_NJJZWTiMuNum + pDaTiGameCnfg->m_XianXiaTiMuNum + pDaTiGameCnfg->m_ZheXueTiMuNum;

	this->SendGameMsgToClient(enGamDaTiCmd_InitClient, &DaTiGame_Init, sizeof(DaTiGame_Init));
}

//游戏开始
void DaTiGame::GameStart()
{
	//先用随机方法，获得游戏要用到的所有题目ID
	if( !this->__GetAllTiMu()){
		return;
	}

	//通知客户端游戏开始
	this->SendGameMsgToClient(enGamDaTiCmd_Start, 0, 0);

	//发送第一道题
	if( !this->__SendNextTiMuData()){
		return;
	}

	m_GameBeginTime = CURRENT_TIME();
}

//游戏结束
void DaTiGame::GameOver()
{
	Super::GameOver();

	g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_TimeOut,this);

	
}

void DaTiGame::OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)
{

}

//完成级别,返回完成级别和获得的品质点
UINT8 DaTiGame::GetFinishLevel(INT32 & QualityPoint)
{
	const std::vector<SDaTiGameAwardCnfg> vectDaTiAward = g_pGameServer->GetConfigServer()->GetDaTiGameAward();

	//从最高完成级别开始判断
	for( int i = vectDaTiAward.size() - 1; i >= 0; --i)
	{
		if( vectDaTiAward[i].m_RateTiMu != 0){
			const SDaTiGameCnfg * pDaTiGameCnfg = g_pGameServer->GetConfigServer()->GetDaTiCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
			if( 0 == pDaTiGameCnfg){
				return 0;
			}
			//题目总数
			UINT16 TotalTiMu = pDaTiGameCnfg->m_GameTiMuNum + pDaTiGameCnfg->m_JXCQTiMuNum + pDaTiGameCnfg->m_NJJZWTiMuNum + pDaTiGameCnfg->m_XianXiaTiMuNum + pDaTiGameCnfg->m_ZheXueTiMuNum;

			//按比例算答对数量来判断
			if( m_TotalRightTiMuNum < (TotalTiMu * vectDaTiAward[i].m_RateTiMu / 100)){
				continue;
			}
		}else{
			//按完成数量判断
			if( m_TotalRightTiMuNum < vectDaTiAward[i].m_RightTiMuNum){
				continue;
			}
		}

		//判断时间
		if( vectDaTiAward[i].m_TimeGameOver == -1 || CURRENT_TIME() - m_GameBeginTime <= vectDaTiAward[i].m_TimeGameOver){
			QualityPoint = vectDaTiAward[i].m_AwardQuality;
			return vectDaTiAward[i].m_FinishLevel;
		}
	}
	return 0;
}

//用随机方法获得玩家要答的所有题目
bool DaTiGame::__GetAllTiMu()
{
	const SDaTiGameCnfg * pDaTiGameCnfg = g_pGameServer->GetConfigServer()->GetDaTiCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pDaTiGameCnfg){
		return false;
	}

	int nCountTiMu = 0;	//题目个数

	for( int i = (int)enTiMuType_JXCQ; i < (int)enTiMuType_Max; ++i)
	{
		if( i == (int)enTiMuType_JXCQ){
			nCountTiMu = pDaTiGameCnfg->m_JXCQTiMuNum;
		}else if( i == (int)enTiMuType_Game){
			nCountTiMu = pDaTiGameCnfg->m_GameTiMuNum;
		}else if( i == (int)enTiMuType_XianXia){
			nCountTiMu = pDaTiGameCnfg->m_XianXiaTiMuNum;
		}else if( i == (int)enTiMuType_ZheXue){
			nCountTiMu = pDaTiGameCnfg->m_ZheXueTiMuNum;
		}else if( i == (int)enTiMuType_NJJZW){
			nCountTiMu = pDaTiGameCnfg->m_NJJZWTiMuNum;
		}

		const std::hash_map<UINT16/*TiMuID*/, STiMuData> * pmapTiMu = g_pGameServer->GetConfigServer()->GetTiMuListCnfg((enTiMuType)i);
		if( 0 == pmapTiMu){
			TRACE("<error> %s : %d 行 找不到题目类型%d的题库！", __FUNCTION__,__LINE__,i);
			return false;
		}

		if( 0 == pmapTiMu->size()){
			TRACE("<error> %s : %d 行 该类题目数量为0!!,类型为%d", __FUNCTION__, __LINE__, i);
			return false;
		}

		if( nCountTiMu > pmapTiMu->size()){
			TRACE("<error> %s : %d 行 答题游戏的题目库配置有错！！　题目类型%d的题目数%d小于要答的数目数%d！", __FUNCTION__,__LINE__,i,pmapTiMu->size(),nCountTiMu);
			return false;
		}

		std::set<UINT32> setRandom;

		//直接获取nCountTiMu个随机数
		for( int k = 0; k < nCountTiMu; ++k)
		{
			int nRandom = RandomService::GetRandom() % pmapTiMu->size();
			
			//要插入nCountTiMu个不同的
			while( !setRandom.insert(nRandom).second){
				nRandom = RandomService::GetRandom() % pmapTiMu->size();
			}
		}

		std::hash_map<UINT16/*TiMuID*/, STiMuData>::const_iterator it = pmapTiMu->begin();

		for( std::set<UINT32>::iterator iter = setRandom.begin(); iter != setRandom.end(); ++iter)
		{
			std::advance(it, *iter);
			if( it == pmapTiMu->end()){
				return false;
			}

			STiMuInfo TiMuInfo;
			TiMuInfo.m_TiMuID = it->first;
			TiMuInfo.m_TiMuType = (enTiMuType)i;

			m_vectTiMu.push_back(TiMuInfo);
		}
		
	}
	return true;
}

//发送下一道题给客户端题目信息
bool DaTiGame::__SendNextTiMuData()
{
	if( m_vectTiMu.size() < ++m_NowTiMuIndex){
		//答题全部完成，提前结束游戏
		this->GameOver();
		return true;
	}

	const STiMuData * pTiMuData = this->__GetTiMuData(m_vectTiMu[m_NowTiMuIndex - 1]);
	if( 0 == pTiMuData){
		TRACE("<error> %s : %d 行 获取题目配置失败！！题目ID=%d,题目类型=%d", __FUNCTION__,__LINE__,m_vectTiMu[m_NowTiMuIndex - 1].m_TiMuID,m_vectTiMu[m_NowTiMuIndex - 1].m_TiMuType);
		return false;
	}

	SC_BornTiMu BornTiMu;
	BornTiMu.m_TiMuIndex = m_NowTiMuIndex;

	OBuffer2k ob;

	ob << BornTiMu;
	ob.Push(pTiMuData->m_TiMuText.c_str(), pTiMuData->m_TiMuText.length() + 1);
	ob.Push(pTiMuData->m_AnserA.c_str(), pTiMuData->m_AnserA.length() + 1);
	ob.Push(pTiMuData->m_AnserB.c_str(), pTiMuData->m_AnserB.length() + 1);
	ob.Push(pTiMuData->m_AnserC.c_str(), pTiMuData->m_AnserC.length() + 1);
	ob.Push(pTiMuData->m_AnserD.c_str(), pTiMuData->m_AnserD.length() + 1);

	OBuffer2k ob2;
	ob2 << ob;

	this->SendGameMsgToClient(enGamDaTiCmd_BornTiMu, (void *)ob2.Buffer(), ob2.Size());

	//先删除旧定时器
	if( m_NowTiMuIndex != 1){
		g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_TimeOut,this);
	}

	const SDaTiGameParam & DaTiGameParam = g_pGameServer->GetConfigServer()->GetDaTiParam();

	//设置答题的超时定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_TimeOut,this,DaTiGameParam.m_MaxDaTiTime,"DaTiGame::GameStart");

	return true;
}

//得到题目信息
const STiMuData * DaTiGame::__GetTiMuData(STiMuInfo TiMuInfo)
{
	const std::hash_map<UINT16/*TiMuID*/, STiMuData> * pmapTiMu = g_pGameServer->GetConfigServer()->GetTiMuListCnfg(TiMuInfo.m_TiMuType);
	if( 0 == pmapTiMu){
		TRACE("<error> %s : %d 行 找不到题目类型%d的题库！", __FUNCTION__,__LINE__,(int)TiMuInfo.m_TiMuType);
		return 0;
	}

	const std::hash_map<UINT16/*TiMuID*/, STiMuData>::const_iterator iter = pmapTiMu->find(TiMuInfo.m_TiMuID);
	if( iter == pmapTiMu->end()){
		return 0;
	}

	return &(iter->second);;
}

//答题
void DaTiGame::__UserDaTi(IActor*,UINT8 nSubCmd,IBuffer & ib)
{
	CS_DaTi_Req DaTi_Req;
	ib >> DaTi_Req;

	SC_DaTi_Rsp DaTi_Rsp;
	DaTi_Rsp.m_TiMuIndex = m_NowTiMuIndex;

	do
	{
		//先验证题目索引是不是一至，不一至为作弊
		if( DaTi_Req.m_TiMuIndex != m_NowTiMuIndex){
			break;
		}

		const STiMuData * pTiMuData = this->__GetTiMuData(m_vectTiMu[m_NowTiMuIndex]);
		if( 0 == pTiMuData){
			break;
		}

		if( pTiMuData->m_RightAnser != DaTi_Req.m_nDaAnIndex){
			break;
		}

		DaTi_Rsp.m_bRight = true;

		++m_TotalRightTiMuNum;
	}
	while(0);

	this->SendGameMsgToClient(enGamDaTiCmd_DaTi, &DaTi_Rsp, sizeof(DaTi_Rsp));

	//下一道题
	this->__SendNextTiMuData();		
}

//得到游戏奖励
void DaTiGame::__GetGameAward()
{
	const SDaTiGameCnfg * pDaTiGameCnfg = g_pGameServer->GetConfigServer()->GetDaTiCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pDaTiGameCnfg){
		return;
	}

	const SDaTiGameLevelAward * pDaTiLevelAward = g_pGameServer->GetConfigServer()->GetDaTiGameLevelAward(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pDaTiLevelAward){
		return;
	}

	const SDaTiGameParam & DaTiGameParam = g_pGameServer->GetConfigServer()->GetDaTiParam();

	//题目总数
	UINT16 TotalTiMu = pDaTiGameCnfg->m_GameTiMuNum + pDaTiGameCnfg->m_JXCQTiMuNum + pDaTiGameCnfg->m_NJJZWTiMuNum + pDaTiGameCnfg->m_XianXiaTiMuNum + pDaTiGameCnfg->m_ZheXueTiMuNum;

	//从最高三级开始
	if( DaTiGameParam.m_RightTiMuNumThreeLevel == -1){
		if( m_TotalRightTiMuNum == TotalTiMu){
			//奖励礼券
			
		}
	}
}
