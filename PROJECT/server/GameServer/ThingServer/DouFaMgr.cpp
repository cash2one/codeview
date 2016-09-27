

#include "DouFaMgr.h"
#include "IActor.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IDouFaPart.h"
#include "time.h"
#include "XDateTime.h"
#include "ITeamPart.h"


DouFaMgr::DouFaMgr()
{
	m_bFirstTimer = true;
	m_TotalNum = 0;

}

DouFaMgr::~DouFaMgr()
{
}


bool DouFaMgr::Create()
{
	this->SetEverydayTimer();
	//this->SetSundayTimer();

	if ( !m_DuoBaoWar.Create() )
	{
		return false;
	}



	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_DouFa,this);
}

void DouFaMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_DouFa,this);
}

//收到MSG_ROOT消息
void DouFaMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (DouFaMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enDouFaCmd_Max] = 
	{
		NULL,
		&DouFaMgr::OpenDouFaLabel,
		&DouFaMgr::OpenQieCuoLabel,
		&DouFaMgr::DouFaCombat,
		NULL,
		NULL,
		NULL,
		NULL,
		&DouFaMgr::QieCuoBattle,
		&DouFaMgr::FlushDouFaEnemy,
		&DouFaMgr::FlushQieCuo,
		NULL,
		&DouFaMgr::FlushMyChalleng,
		&DouFaMgr::ViewChallengeLvGroup,
		&DouFaMgr::ChallengeUser,
		NULL,
		&DouFaMgr::ViewChallengeForward,

		//夺宝
		&DouFaMgr::OpenDuoBao,
		NULL,
		&DouFaMgr::ReadySelect,
		&DouFaMgr::JoinDuoBao,
		NULL,

		&DouFaMgr::FlushMyChallengRecord,
		&DouFaMgr::ViewChallengeRecord,

		NULL,
		&DouFaMgr::CancelDuoBao,
		NULL,
		&DouFaMgr::BuyCombatNum,
	};

	if(nCmd >= enDouFaCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

// nRetCode: 取值于 enDBRetCode
void DouFaMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK){
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	if( ReqCmd == enDBCmd_Get_ChallengeLevelRank || ReqCmd == enDBCmd_Get_ChallengeLevelTotalNum){

		OBuffer4k RspOb(RspOsb);
		OBuffer4k ReqOb(ReqOsb);

		switch(ReqCmd)
		{
		case enDBCmd_Get_ChallengeLevelRank:
			{
				this->HandleChallengeLvGroupForward(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_Get_ChallengeLevelTotalNum:
			{
				this->HandleChallengeLvGroupTotalNum(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		}
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	pDouFaPart->OnDBRet(userID,ReqCmd,nRetCode,RspOsb,ReqOsb,userdata);
}

void DouFaMgr::OnTimer(UINT32 timerID)
{
/*	if( enTimerID_Sunday == timerID){
		//挑战奖励结算
		this->ChallengeForwardLvGroup();
		this->ChallengeForwardLvGroup_B();

		//更新下挑战排行表
		SDB_Update_AllUserLevelChallenge DBReq;

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(timerID, enDBCmd_Update_AllUserLevelChallenge, ob.TakeOsb(), 0, 0);

		if( m_bFirstTimer){

			m_bFirstTimer = false;

			g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Sunday, this);

			this->SetSundayTimer();
		}
	}*/
	if( enTimerID_Everyday == timerID){
		//挑战奖励结算
		this->ChallengeForwardLvGroup();
		this->ChallengeForwardLvGroup_B();

		//更新下挑战排行表
		SDB_Update_AllUserLevelChallenge DBReq;

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(timerID, enDBCmd_Update_AllUserLevelChallenge, ob.TakeOsb(), 0, 0);

		if( m_bFirstTimer){

			m_bFirstTimer = false;

			g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Everyday, this);

			this->SetEverydayTimer();
		}
	}
}

//设置每日0点0时的定时器
void DouFaMgr::SetEverydayTimer()
{
	//设置定时器,晚上0点0分0秒发放挑战奖励
	if( m_bFirstTimer){
		time_t nCurrentTime = ::time(0);

		tm * pTm = ::localtime(&nCurrentTime);
		if( 0 == pTm){
			return;
		}

		UINT32 timeLong = 0;

		timeLong = XDateTime::SECOND_OF_DAY - (pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec);	

		g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Everyday,this,timeLong * 1000,"DouFaMgr::SetEverydayTimer");
	}else{
		//直接设置1天
		g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Everyday,this, XDateTime::SECOND_OF_DAY * 1000,"DouFaMgr::SetEverydayTimer");
	}
}

//设置每周日晚0点0时的定时器
void DouFaMgr::SetSundayTimer()
{
	if( m_bFirstTimer){
		time_t nCurrentTime = ::time(0);

		tm * pTm = ::localtime(&nCurrentTime);
		if( 0 == pTm){
			return;
		}

		UINT32 timeLong = 0;

		if( pTm->tm_wday != 0){
			timeLong = (7 - pTm->tm_wday) * XDateTime::SECOND_OF_DAY + (24 - pTm->tm_hour - 1) * XDateTime::SECOND_OF_HOUR + (60 - pTm->tm_min - 1) * 60 + (60 - pTm->tm_sec);	
		}else{
			//星期天
			timeLong = (24 - pTm->tm_hour - 1) * XDateTime::SECOND_OF_HOUR + (60 - pTm->tm_min - 1) * 60 + (60 - pTm->tm_sec);
		}

		g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Sunday,this,timeLong * 1000,"DouFaMgr::SetSundayTimer");
	}else{
		//直接设置1周
		g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Sunday,this,XDateTime::SECOND_OF_DAY * 7 * 1000,"DouFaMgr::SetSundayTimer");
	}
}

//退出或掉线或顶号或离队
void DouFaMgr::QuitDuoBao(IActor * pQuiter)
{
	m_DuoBaoWar.QuitDuoBao(pQuiter);
}

//打开斗法标签
void DouFaMgr::OpenDouFaLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	pDouFaPart->OpenDouFaLabel();

}

//打开切磋标签
void DouFaMgr::OpenQieCuoLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	pDouFaPart->OpenQieCuoLabel();
}

//斗法
void DouFaMgr::DouFaCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_DouFaDouFa_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}
	
	pDouFaPart->DouFaCombat(UID(Req.m_uidUser));
}

//刷新切磋对象
void DouFaMgr::FlushQieCuo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFa = pActor->GetDouFaPart();

	if( 0 == pDouFa){
		return;
	}

	pDouFa->FlushQieCuoEnemy();
}

//切磋战斗
void DouFaMgr::QieCuoBattle(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFa = pActor->GetDouFaPart();

	if( 0 == pDouFa){
		return;
	}

	CS_QieCuoBattle Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}

	pDouFa->QieCuoBattle(UID(Req.m_uidEnemy));
}

//刷新斗法对手
void DouFaMgr::FlushDouFaEnemy(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFa = pActor->GetDouFaPart();
	if( 0 == pDouFa){
		return;
	}

	pDouFa->FlushDouFaEnemy();
}

//刷新我的挑战排行
void DouFaMgr::FlushMyChalleng(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_Challenge.FlushMyChallenge(pActor);
}

//刷新我的挑战回放
void DouFaMgr::FlushMyChallengRecord(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_Challenge.FlushMyChallengeRecord(pActor);
}

//查看挑战等级组排行
void DouFaMgr::ViewChallengeLvGroup(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewLvGroup Req;

	ib >> Req;
	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}

	m_Challenge.ViewLvGroup(pActor, Req.m_lvGroup, Req.m_BeginPos, Req.m_Num);
}

//挑战玩家
void DouFaMgr::ChallengeUser(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_Challenge Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}
	
	m_Challenge.ChallengeUser(pActor, Req.m_uidEnemy);
}

//查看挑战奖励
void DouFaMgr::ViewChallengeForward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ChallengeForward Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;	
	}

	//挑战从第几名开始可获得奖励
	INT8 RankCanGetForward = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ChallengeRankGetForward;

	SC_ChallengeForward Rsp;

	OBuffer4k ob;

	const SChallengeForward * pChallengeForward = g_pGameServer->GetConfigServer()->GetChallengeForward(Req.m_lvGroup, Req.m_Rank);
	if( 0 == pChallengeForward){
		TRACE("<error> %s : %d 行 获取挑战奖励配置失败！！,等级组 %d,名次 %d", __FUNCTION__, __LINE__, Req.m_lvGroup, Req.m_Rank);
		return;
	}

	for( int k = 1; k + 2 < pChallengeForward->m_vectGoods.size(); k += 3)
	{
		GoodsForward forward;

		forward.m_GoodsID = pChallengeForward->m_vectGoods[k];
		forward.m_GoodsNum = pChallengeForward->m_vectGoods[k + 1];

		pActor->SendGoodsCnfg(pChallengeForward->m_vectGoods[k]);

		ob << forward;

		++Rsp.m_Num;
	}

	OBuffer4k ob2;
	ob2 <<  DouFaHeader(enDouFaCmd_ChallengeForward, sizeof(Rsp) + ob.Size()) << Rsp;

	if( ob.Size() > 0){
		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}


//每周日0点对区间挑战前5%名的奖励(前5名除外)
void DouFaMgr::ChallengeForwardLvGroup_B()
{

	SetTotalNumLvGroup();

}

//每周日0点对区间挑战前5名的奖励
void DouFaMgr::ChallengeForwardLvGroup()
{
	//得到各区间前5名
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//30-34级区间
	SDB_Get_ChallengeLevelRank DBReq;

	DBReq.m_BeginLevel = 30;
	DBReq.m_EndLevel   = 34;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);

	//35-39级区间
	DBReq.m_BeginLevel = 35;
	DBReq.m_EndLevel   = 39;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);

	//40-44级区间
	DBReq.m_BeginLevel = 40;
	DBReq.m_EndLevel   = 44;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);

	//45-49级区间
	DBReq.m_BeginLevel = 45;
	DBReq.m_EndLevel   = 49;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);

	//50-54级区间
	DBReq.m_BeginLevel = 50;
	DBReq.m_EndLevel   = 54;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);

	//55-60级区间
	DBReq.m_BeginLevel = 55;
	DBReq.m_EndLevel   = 60;
	DBReq.m_BeginPos   = 1;
	DBReq.m_Num		   = GameParam.m_ChallengeRankGetForward;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 0);
}

void DouFaMgr::HandleChallengeLvGroupForward(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		return;
	}

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	INT32 nNum = 0;
	//if(userdata != 0){

	//	nNum = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo) - GameParam.m_ChallengeRankGetForward;

	//}else{

		nNum = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo);

	//}

	for( int i = 0; i < nNum; ++i)
	{
		SDB_ChallengeRankInfo DBChallenge;
		RspIb >> DBChallenge;

		if( RspIb.Error()){
			TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}
	
		enLevelGroup lvGroup;

		if( DBChallenge.m_Level >= 30 && DBChallenge.m_Level <= 34){

			lvGroup = enLevelGroup_30_34;

		}else if( DBChallenge.m_Level >= 35 && DBChallenge.m_Level <= 39){

			lvGroup = enLevelGroup_35_39;

		}else if( DBChallenge.m_Level >= 40 && DBChallenge.m_Level <= 44){

			lvGroup = enLevelGroup_40_44;

		}else if(DBChallenge.m_Level >= 45 && DBChallenge.m_Level <= 49){

			lvGroup = enLevelGroup_45_49;

		}else if(DBChallenge.m_Level >= 50 && DBChallenge.m_Level <= 54){

			lvGroup = enLevelGroup_50_54;

		}else if(DBChallenge.m_Level >= 55 && DBChallenge.m_Level <= 60){

			lvGroup = enLevelGroup_55_60;

		}else{
			return;
		}

		if(userdata != 0){

			m_Challenge.LvGroupRankForward(UID(DBChallenge.m_uidUser), lvGroup, i + 1 + GameParam.m_ChallengeRankGetForward);
			
		}else{

			m_Challenge.LvGroupRankForward(UID(DBChallenge.m_uidUser), lvGroup, i + 1);

		}
	}
}

//打开夺宝
void DouFaMgr::OpenDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	SC_OpenDuoBao Rsp;

	Rsp.m_GetCredit = pDouFaPart->GetCreditWeek();
	Rsp.m_MaxCredit = pDouFaPart->GetMaxCreditWeek();
	Rsp.m_RunRemainTime = pDouFaPart->GetLeftRunTime();
	Rsp.m_bOpen		= m_DuoBaoWar.IsOpen();

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_OpenDuoBao, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//加入夺宝
void DouFaMgr::JoinDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_DuoBaoWar.JoinDuoBao(pActor);
}

//点击战斗准备框按钮
void DouFaMgr::ReadySelect(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ReadySelect Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;	
	}

	m_DuoBaoWar.ReadySelect(pActor, Req.m_bEnterCombat);
}

//查看战斗回放
void DouFaMgr::ViewChallengeRecord(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewChallengeRecord Req;

	ib >> Req;

	if(ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}

	SDB_Get_ChallengeRecordBuf DBReq;
	DBReq.m_Rank = Req.m_Rank;


	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_ChallengeRecordBuf, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
}

//设置区间挑战总人数
void DouFaMgr::SetTotalNumLvGroup()
{

	SDB_Get_ChallengeLevelTotalNum DBReq;
	OBuffer1k ob;

	//30-34级区间
	DBReq.m_BeginLevel = 30;
	DBReq.m_EndLevel   = 34;

	UINT64 LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;

	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

	//35-39级区间
	DBReq.m_BeginLevel = 35;
	DBReq.m_EndLevel   = 39;
	LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;
	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

	//40-44级区间
	DBReq.m_BeginLevel = 40;
	DBReq.m_EndLevel   = 44;
	LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;
	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

	//45-49级区间
	DBReq.m_BeginLevel = 45;
	DBReq.m_EndLevel   = 49;
	LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;
	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

	//50-54级区间
	DBReq.m_BeginLevel = 50;
	DBReq.m_EndLevel   = 54;
	LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;
	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

	//55-60级区间
	DBReq.m_BeginLevel = 55;
	DBReq.m_EndLevel   = 60;
	LvParam = DBReq.m_BeginLevel << 8;
	LvParam += DBReq.m_EndLevel;
	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelTotalNum, ob.TakeOsb(), this, LvParam);

}

void DouFaMgr::HandleChallengeLvGroupTotalNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_ChallengeLevelTotalNum TotalNum;
	RspIb >> RspHeader >> TotalNum;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//m_TotalNum = TotalNum.m_TotalNum;


	SDB_Get_ChallengeLevelRank DBReq;
	int Num = 0;
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
	Num = (float)TotalNum.m_TotalNum * GameParam.m_ChallengeRankGetForward_B/100 + 0.9999 - GameParam.m_ChallengeRankGetForward;
	DBReq.m_BeginLevel = userdata >> 8;
	DBReq.m_EndLevel   = (userdata & 0xff);
	DBReq.m_BeginPos   = GameParam.m_ChallengeRankGetForward + 1;
	DBReq.m_Num		   = Num;
	if(Num < 1){
		return;
	}
	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_BeginLevel, enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, 1);
	
}

//得到区间挑战总人数
INT32 DouFaMgr::GetTotalNumLvGroup()
{
	return m_TotalNum;
}

//取消夺宝
void DouFaMgr::CancelDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if( 0 == pDouFaPart)
	{
		return;
	}

	SC_CancelDuoBao Rsp;

	if ( pDouFaPart->IsTeamDuoBao() )
	{
		ITeamPart * pTeamPart = pActor->GetTeamPart();

		if ( 0 == pTeamPart )
			return;

		if ( pActor != pTeamPart->GetTeamLeader() )
		{
			//只有队长能选择取消
			Rsp.m_Result = enDouFaRetCode_ErrNeedLeader;
		}
		else
		{
			this->QuitDuoBao(pActor);
			return;
		}
	}
	else
	{
		this->QuitDuoBao(pActor);
		return;
	}

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_CancelDuoBao, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//购买战斗次数
void DouFaMgr::BuyCombatNum(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_Challenge.BuyCombatNum(pActor);
}
