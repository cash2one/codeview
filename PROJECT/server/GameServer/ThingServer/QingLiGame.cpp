#include "QingLiGame.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "XDateTime.h"
#include "RandomService.h"
#include "IActor.h"


QingLiGame::QingLiGame(UID uidTalisman) : TalismanGame(uidTalisman)
{
	m_HitDiShuGuaiNum = 0;
	m_HitDiShuJingNum = 0;
	m_HitShuJingNum	  = 0;

	m_CanHitNum = g_pGameServer->GetConfigServer()->GetQingLiGameParam().m_CanHitNum;

	m_GetExp = 0;
}
QingLiGame::~QingLiGame()
{
}

void QingLiGame::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_BornDiShu:
		{
			//通知客户端产生地鼠
			this->__NoticeClientCreateDiShu();
		}
		break;
	case enTimerID_IsDeleteTimer:
		{
			UINT32 nCurTime = CURRENT_TIME();

			const SQingLiGameParam & QingLiGameParam = g_pGameServer->GetConfigServer()->GetQingLiGameParam();

			for(MAP_INMAPDISHUINFO::iterator iter = m_mapNowDiShu.begin(); iter != m_mapNowDiShu.end();)
			{
				SInMapDiShuInfo & InMapDiShuInfo = iter->second;

				if( InMapDiShuInfo.m_QingLiDiShuType == enQingLiDiShuType_DiShuGuai){
					if( (InMapDiShuInfo.m_BornTime + QingLiGameParam.m_DiShuGuaiTime) <= nCurTime){
						//删除该地鼠
						m_mapNowDiShu.erase(iter++);
						this->__NoticeClientDeleteDiShu(InMapDiShuInfo.m_DiShuID);
						continue;
					}
				}else if( InMapDiShuInfo.m_QingLiDiShuType == enQingLiDiShuType_DiShuJing){
					if( (InMapDiShuInfo.m_BornTime + QingLiGameParam.m_DiShuJingTime) <= nCurTime){
						//删除该地鼠
						m_mapNowDiShu.erase(iter++);
						this->__NoticeClientDeleteDiShu(InMapDiShuInfo.m_DiShuID);
						continue;
					}
				}else if( InMapDiShuInfo.m_QingLiDiShuType == enQingLiDiShuType_ShuJing){
					if( (InMapDiShuInfo.m_BornTime + QingLiGameParam.m_ShuJingTime) <= nCurTime){
						//删除该地鼠
						m_mapNowDiShu.erase(iter++);
						this->__NoticeClientDeleteDiShu(InMapDiShuInfo.m_DiShuID);
						continue;
					}
				}

				++iter;
			}
		};
		break;
	default:
		Super::OnTimer(timerID);
		break;
	}
}

//游戏消息
void QingLiGame::OnMessage(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	switch(nSubCmd)
	{
	case enGameQingLiCmd_Hit:  //打击地鼠
		{
			this->__OnHitDiShuMass(pActor, nSubCmd, ib);
		}
		break;
	default:
		TRACE("<warning> %s : %d 行 收到意外的命令字 %d！！", __FUNCTION__,__LINE__,nSubCmd);
		break;
	}
}

//初始化客户端
void QingLiGame::NotifyInitClient()
{
	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	SC_QingLiGame_Init QingLiGame_Init;
	QingLiGame_Init.m_GameTotalTime = this->m_pTalismanWorldCnfg->m_TotalGameTime;
	QingLiGame_Init.m_StartAfterTime = ConfigParam.m_TamlismanGameReadyCountDown;

	this->SendGameMsgToClient(enGameQingLiCmd_InitClient,&QingLiGame_Init,sizeof(QingLiGame_Init));
}

//游戏开始
void QingLiGame::GameStart()
{
	//先按数量把地鼠全部创建
	if( !this->__CreateAllDiShu()){
		return;
	}

	const SQingLiGameParam & QingLiGameParam = g_pGameServer->GetConfigServer()->GetQingLiGameParam();

	//设置定时器产生地鼠
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_BornDiShu,this,QingLiGameParam.m_BornTimeLong,"QingLiGame::GameStart");

	//设置定时器看地鼠持续时间是否结束
	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_IsDeleteTimer, this, QingLiGameParam.m_TimerDiShuIsDelete, "QingLiGame::GameStart");

	SendGameMsgToClient(enGameQingLiCmd_Start,0,0);
}

//创建游戏中的所有地鼠
bool QingLiGame::__CreateAllDiShu()
{
	const SQingLiGameConfig * pQingLiGameConfig = g_pGameServer->GetConfigServer()->GetQingLiGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pQingLiGameConfig){
		TRACE("<error> %s : %d 行,获取清理土地游戏配置文件失败!! 法宝世界ID = %d", __FUNCTION__, __LINE__, m_pTalismanWorldCnfg->m_TalismanWorldID);
		return false;
	}

	static UINT32 s_DiShuID = 1;

	SDiShuInfo DiShuInfo;

	//地鼠怪
	DiShuInfo.m_QingLiDiShuType = enQingLiDiShuType_DiShuGuai;
	for( int i = 0; i < pQingLiGameConfig->m_DiShuGuaiNum; ++i)
	{
		DiShuInfo.m_DiShuID = s_DiShuID;
		m_mapDiShuInfo[DiShuInfo.m_DiShuID] = DiShuInfo;
		++s_DiShuID;
	}

	//地鼠精
	DiShuInfo.m_QingLiDiShuType = enQingLiDiShuType_DiShuJing;
	for( int i = 0; i < pQingLiGameConfig->m_DiShuJingNum; ++i)
	{
		DiShuInfo.m_DiShuID = s_DiShuID;
		m_mapDiShuInfo[DiShuInfo.m_DiShuID] = DiShuInfo;
		++s_DiShuID;		
	}

	//树精
	DiShuInfo.m_QingLiDiShuType = enQingLiDiShuType_ShuJing;
	for( int i = 0; i < pQingLiGameConfig->m_ShuJingNum; ++i)
	{
		DiShuInfo.m_DiShuID = s_DiShuID;
		m_mapDiShuInfo[DiShuInfo.m_DiShuID] = DiShuInfo;
		++s_DiShuID;			
	}

	return true;
}

//通知客户端产生一个地鼠
void QingLiGame::__NoticeClientCreateDiShu()
{
	if( m_mapDiShuInfo.size() < 1){
		return;
	}

	//从已创建的地鼠中随机选一个
	INT32 nRandom = RandomService::GetRandom() % m_mapDiShuInfo.size();

	MAP_DISHUINFO::iterator iter = m_mapDiShuInfo.begin();

	std::advance(iter, nRandom);

	SDiShuInfo & DiShuInfo = iter->second;
	
	SC_QingLiGame_BornDiShu BornDiShu;
	BornDiShu.m_DiShuID = DiShuInfo.m_DiShuID;
	BornDiShu.m_QingLiDiShuType = DiShuInfo.m_QingLiDiShuType;

	const SQingLiGameParam & QingLiGameParam = g_pGameServer->GetConfigServer()->GetQingLiGameParam();

	//接下来随机选择位置
	time_t CurTime = CURRENT_TIME();
	do
	{
		if( m_CanHitNum < 1){
			TRACE("<error> %s : %d 行 可击打位置数小于1!!,可击打数等于%d", __FUNCTION__, __LINE__, m_CanHitNum);
			return;
		}

		nRandom = RandomService::GetRandom() % m_CanHitNum;

		SPosInfo & PosInfo = m_mapPosInfo[nRandom];
		
		if( PosInfo.m_QingLiDiShuType == enQingLiDiShuType_DiShuJing){
			//上次是出地鼠精,则在地鼠精消失后一定内不能出现任何类型的地鼠
			if( (CurTime - PosInfo.m_LastFlushTime + QingLiGameParam.m_DiShuJingTime) < QingLiGameParam.m_SamePosDiShuJingNum){
				continue;
			}
		}else if( PosInfo.m_QingLiDiShuType == enQingLiDiShuType_DiShuGuai || PosInfo.m_QingLiDiShuType == enQingLiDiShuType_ShuJing){
			//上次是出地鼠怪,则同一个地方出地鼠的时间间隔(s)
			if( CurTime - PosInfo.m_LastFlushTime < QingLiGameParam.m_SamePosTimeNum){
				continue;
			}
		}
	}
	while(1);

	BornDiShu.m_Pos = nRandom;

	m_mapPosInfo[nRandom].m_LastFlushTime = CurTime;
	m_mapPosInfo[nRandom].m_QingLiDiShuType = DiShuInfo.m_QingLiDiShuType;

	//删除该地鼠
	m_mapDiShuInfo.erase(iter);

	SInMapDiShuInfo InMapDiShuInfo;
	InMapDiShuInfo.m_BornTime = CURRENT_TIME();
	InMapDiShuInfo.m_DiShuID  = DiShuInfo.m_DiShuID;
	InMapDiShuInfo.m_HitNum   = 0;
	InMapDiShuInfo.m_Pos	  = nRandom;
	InMapDiShuInfo.m_QingLiDiShuType = DiShuInfo.m_QingLiDiShuType;

	//加到正在地图的地鼠中
	m_mapNowDiShu[DiShuInfo.m_DiShuID] = InMapDiShuInfo;

	this->SendGameMsgToClient(enGameQingLiCmd_BornDiShu, &BornDiShu, sizeof(BornDiShu));
}

//验证客户端发来的打中地鼠消息
void QingLiGame::__OnHitDiShuMass(IActor* pActor,UINT8 nSubCmd,IBuffer & ib)
{
	CS_QingLiGame_HitDiShu_Req  HitDiShu_Req;
	ib >> HitDiShu_Req;

	SC_QingLiGame_HitDiShu_Rsp HitDiShu_Rsp;
	HitDiShu_Rsp.m_DiShuID = HitDiShu_Req.m_DiShuID;

	MAP_INMAPDISHUINFO::iterator iter = m_mapNowDiShu.find(HitDiShu_Req.m_DiShuID);

	do
	{
		if( iter == m_mapNowDiShu.end()){
			//有错
			HitDiShu_Rsp.m_bHit = false;
			break;
		}

		SInMapDiShuInfo & InMapDiShuInfo = iter->second;
		if( InMapDiShuInfo.m_Pos != HitDiShu_Req.m_HitPos || InMapDiShuInfo.m_DiShuID != HitDiShu_Req.m_DiShuID){
			//有错
			HitDiShu_Rsp.m_bHit = false;
			break;
		}

		bool bDelete = true;
		//计算经验
		const SQingLiGameParam & QingLiGameParam = g_pGameServer->GetConfigServer()->GetQingLiGameParam();

		if( HitDiShu_Req.m_QingLiDiShuType == enQingLiDiShuType_DiShuGuai){

			HitDiShu_Rsp.m_GetExpNum = pActor->GetCrtProp(enCrtProp_Level) * QingLiGameParam.m_DiShuGuaiExpParam;
		}else if( HitDiShu_Req.m_QingLiDiShuType == enQingLiDiShuType_ShuJing){

			HitDiShu_Rsp.m_GetExpNum = pActor->GetCrtProp(enCrtProp_Level) * QingLiGameParam.m_DiShuJingExpParam;
		}else if( HitDiShu_Req.m_QingLiDiShuType == enQingLiDiShuType_DiShuJing){

			HitDiShu_Rsp.m_GetExpNum = pActor->GetCrtProp(enCrtProp_Level) * QingLiGameParam.m_DiShuJingExpParam;
			//超过地鼠精可击打次数，则可删除
			++InMapDiShuInfo.m_HitNum;
			if( InMapDiShuInfo.m_HitNum < QingLiGameParam.m_DiShuJingCanHitNum){
				bDelete = false;
			}
		}

		m_GetExp += HitDiShu_Rsp.m_GetExpNum;

		if( bDelete){
			//删除地鼠
			this->__NoticeClientDeleteDiShu(HitDiShu_Req.m_DiShuID);
			m_mapNowDiShu.erase(HitDiShu_Req.m_DiShuID);
		}
	}
	while(0);

	this->SendGameMsgToClient(enGameQingLiCmd_Hit ,&HitDiShu_Rsp , sizeof(HitDiShu_Rsp));
}


//通知客户端删除地鼠
void QingLiGame::__NoticeClientDeleteDiShu(UINT32 DiShuID)
{
	SC_QingLiGame_DeleteDiShu DeleteDiShu;
	DeleteDiShu.m_DiShuID = DiShuID;

	this->SendGameMsgToClient(enGameQingLiCmd_DelDiShu ,&DeleteDiShu , sizeof(DeleteDiShu));
}


//游戏结束
void QingLiGame::GameOver()
{
	Super::GameOver();

	g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_BornDiShu,this);

	//获得的经验
	for( int i = 0; i < m_vectActor.size(); ++i)
	{
		IActor * pActor = m_vectActor[i];
		if( 0 == pActor){
			continue;
		}

		pActor->AddCrtPropNum(enCrtProp_Level, m_GetExp / m_vectActor.size());
	}
}

void QingLiGame::OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)
{
	const SQingLiGameConfig * pQingLiGameConfig = g_pGameServer->GetConfigServer()->GetQingLiGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pQingLiGameConfig){
		TRACE("<error> %s : %d 行,获取清理土地游戏配置文件失败!! 法宝世界ID = %d", __FUNCTION__, __LINE__, m_pTalismanWorldCnfg->m_TalismanWorldID);
		return;
	}
	SC_QingLiGame_Over Rsp;
	Rsp.m_TotalExp = m_GetExp;
	Rsp.m_FinishLevel = FinishLevel;
	Rsp.m_TotalDiShuGuai = pQingLiGameConfig->m_DiShuGuaiNum;
	Rsp.m_TotalDiShuJing = pQingLiGameConfig->m_DiShuJingNum;
	Rsp.m_TotalShuJing	 = pQingLiGameConfig->m_ShuJingNum;
	Rsp.m_TotalHitDiShuGuai = m_HitDiShuGuaiNum;
	Rsp.m_TotalHitDiShuJing = m_HitDiShuJingNum;
	Rsp.m_TotalHitShuJing   = m_HitShuJingNum;
	Rsp.m_AdventureAwardID  = AdventureAwardID;

	this->SendGameMsgToClient(enGameXunBaoCmd_Over,&Rsp,sizeof(Rsp));
}

//完成级别,返回完成级别和获得的品质点
UINT8 QingLiGame::GetFinishLevel(INT32 & QualityPoint)
{
	const std::vector<SQingLiGameAwardCnfg> & vectGameAwardCnfg = g_pGameServer->GetConfigServer()->GetQingLiAwardCnfg();

	const SQingLiGameConfig * pQingLiGameConfig = g_pGameServer->GetConfigServer()->GetQingLiGameCnfg(m_pTalismanWorldCnfg->m_TalismanWorldID);
	if( 0 == pQingLiGameConfig){
		TRACE("<error> %s : %d 行,获取清理土地游戏配置文件失败!! 法宝世界ID = %d", __FUNCTION__, __LINE__, m_pTalismanWorldCnfg->m_TalismanWorldID);
		return false;
	}

	//从最高级别开始判断
	for( int i = vectGameAwardCnfg.size() - 1; i >= 0; --i)
	{
		//获得宝物数量
		if( vectGameAwardCnfg[i].m_DestroyDiShuNum == -1){
			if((pQingLiGameConfig->m_DiShuGuaiNum + pQingLiGameConfig->m_DiShuJingNum) != (m_HitDiShuGuaiNum + m_HitDiShuJingNum)){
				continue;
			}
		}
		else if((pQingLiGameConfig->m_DiShuGuaiNum + pQingLiGameConfig->m_DiShuJingNum) > (m_HitDiShuGuaiNum + m_HitDiShuJingNum))
		{
			continue;
		}

		if(vectGameAwardCnfg[i].m_WuShangShuJingNum != -1)
		{
			if( vectGameAwardCnfg[i].m_WuShangShuJingNum < m_HitShuJingNum){
				continue;
			}
		}
		
		QualityPoint = vectGameAwardCnfg[i].m_AwardQuality;

		return vectGameAwardCnfg[i].m_FinishLevel;
	}

	return 0;
}
