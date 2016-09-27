
#include "IActor.h"

#include "DouFaPart.h"
#include "string.h"
#include "ThingServer.h"
#include "GameSrvProtocol.h"
#include "ICombatPart.h"
#include "IBasicService.h"
#include "DMsgSubAction.h"
#include "ISyndicate.h"
#include "ISyndicateMgr.h"
#include "IConfigServer.h"
#include "time.h"
#include "RandomService.h"
#include "IGameScene.h"
#include "XDateTime.h"
#include "ICDTimerPart.h"


DouFaPart::DouFaPart()
{
	m_pActor			= 0;

	m_GetHonorToday		= 0;

	m_bListenDouFa		= false;

	m_bListenQieCuo		= false;

	m_MaxGetHonorDouFaToday		= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGetHonorDouFaToday;

	m_ChallengeData.m_JoinChallengeNum			= 0;

	m_ChallengeData.m_LastJoinChallengeTime		= 0;

	m_ChallengeData.m_MaxChallengeNumToday		= 0;

	m_LastGetHonorTime			= 0;

	m_LastFLushDouFaUpLevel		= 0;
}
//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool DouFaPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || 0 == pContext || nLen < sizeof(SDB_Get_DouFaPartInfo_Rsp)){
		return false;
	}

	m_pActor = (IActor *)pMaster;

	SDB_Get_DouFaPartInfo_Rsp * pDouFaPartInfo = (SDB_Get_DouFaPartInfo_Rsp *)pContext;

	time_t NowTime = CURRENT_TIME();

	m_LastGetHonorTime = pDouFaPartInfo->m_LastGetHonorTime;
	m_LastFLushDouFaUpLevel = pDouFaPartInfo->m_LastFLushDouFaUpLevel;

	if ( 0 == m_LastFLushDouFaUpLevel ){
		m_LastFLushDouFaUpLevel = m_pActor->GetCrtProp(enCrtProp_Level) + 1;
	}
	
	if( XDateTime::GetInstance().IsSameDay( m_LastGetHonorTime,NowTime)){
		//同一天
		m_GetHonorToday = pDouFaPartInfo->m_GetHonorToday;
	}else{
		//不同一天
		m_GetHonorToday  = 0;
	}

	m_ChallengeData.m_LastJoinChallengeTime = pDouFaPartInfo->m_LastJoinChallengeTime;

	if( XDateTime::GetInstance().IsSameDay( m_ChallengeData.m_LastJoinChallengeTime,NowTime)){
		//同一天
		m_ChallengeData.m_JoinChallengeNum = pDouFaPartInfo->m_JoinChallengeNum;

		m_ChallengeData.m_MaxChallengeNumToday = pDouFaPartInfo->m_MaxChallengeNumToday;

		if ( 0 == m_ChallengeData.m_MaxChallengeNumToday){
			
			this->ResetMaxChallengeNum();
		}
	}else{
		//不同一天
		m_ChallengeData.m_JoinChallengeNum = 0;

		this->ResetMaxChallengeNum();
	}

	m_DuoBaoData.m_CreditUp = pDouFaPartInfo->m_CreditUp;

	m_DuoBaoData.m_GetCredit = pDouFaPartInfo->m_GetCredit;

	m_DuoBaoData.m_LastChangeCreditUp = pDouFaPartInfo->m_LastChangeCreditUp;

	//帮派福利增加每日最多可获取荣誉
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return false;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());
	if( 0 != pSyndicate){
		INT32 nValue = pSyndicate->GetWelfareValue(enWelfare_MaxHonorOneDay);

		m_MaxGetHonorDouFaToday += (m_MaxGetHonorDouFaToday *  nValue / 100);
	}

	m_MaxGetHonorDouFaToday += m_pActor->GetVipValue(enVipType_AddMaxHonorToday);

	DB_DouFaQieCuoEnemyData * pEnemyData = (DB_DouFaQieCuoEnemyData *)&pDouFaPartInfo->m_DouFaQieCuoEnemy;
	if( 0 == pEnemyData){
		return false;
	}


	for( int i = 0; i < DOUFA_ENEMY_NUM; ++i)
	{
		DB_DouFaEnemyInfo & EnemyInfo = pEnemyData->m_DouFaEnemy[i];

		if( !UID(EnemyInfo.m_uidEnemy).IsValid()){
			continue;
		}

		DouFaEnemyInfo DouFaData;
		DouFaData.m_bFinish = EnemyInfo.m_bFinish;
		DouFaData.m_bWin	= EnemyInfo.m_bWin;
		strncpy(DouFaData.m_Name, (char *)EnemyInfo.m_EnemyName, sizeof(DouFaData.m_Name));
		DouFaData.m_UserEnemy = UID(EnemyInfo.m_uidEnemy);
		DouFaData.m_Facade  = EnemyInfo.m_Facade;
		DouFaData.m_vipLevel = EnemyInfo.m_VipLevel;

		m_mapEnemy[DouFaData.m_UserEnemy] = DouFaData;
	}

	for( int i = 0; i < QIECUO_ENEMY_NUM; ++i)
	{
		DB_QieCuoEnemyInfo & EnemyInfo = pEnemyData->m_QieCuoEnemy[i];

		if( !UID(EnemyInfo.m_uidEnemy).IsValid()){
			continue;
		}

		QieCuoUserData QieCuoData;

		QieCuoData.m_bFinish = EnemyInfo.m_bFinish;
		QieCuoData.m_bWin	 = EnemyInfo.m_bWin;
		QieCuoData.m_Layer	 = EnemyInfo.m_Layer;
		QieCuoData.m_Level	 = EnemyInfo.m_Level;
		strncpy(QieCuoData.m_Name, (char *)EnemyInfo.m_EnemyName, sizeof(QieCuoData.m_Name));
		QieCuoData.m_Level	 = EnemyInfo.m_Level;
		QieCuoData.m_UidUser = UID(EnemyInfo.m_uidEnemy);
		QieCuoData.m_vipLevel = EnemyInfo.m_VipLevel;

		m_mapQieCuo[QieCuoData.m_UidUser] = QieCuoData;
	}


	if( !m_pActor->GetUseFlag(enUseFlag_First_FlushDouFaEnemy)){
		//第一次自动刷新下斗法对手
		this->FlushDouFaEnemy();

		this->FlushQieCuoEnemy();

		m_pActor->SetUseFlag(enUseFlag_First_FlushDouFaEnemy, true);
	}

	//设置定时器,在线的玩家，晚上0点0分0秒更新
	time_t nCurTime = CURRENT_TIME();

	tm * pTm = localtime(&nCurTime);

	UINT32 LeftTime = XDateTime::SECOND_OF_DAY - (pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec);

	g_pGameServer->GetTimeAxis()->SetTimer(enDouFaTimer_ID_Challenge,this,LeftTime * 1000,"DouFaPart::Create");

	//监听自己离队的事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitTeam);

	m_pActor->SubscribeEvent(msgID, this, "DouFaPart::Create");

	return true;
}

//释放
void DouFaPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart DouFaPart::GetPartID(void)
{
	return enThingPart_Actor_DouFa;
}

//取得本身生物
IThing*		DouFaPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool DouFaPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf == 0 || nLen < sizeof(SDB_Update_DouFaPartInfo_Req))
	{
		return false;
	}

	SDB_Update_DouFaPartInfo_Req * pDouFaPartInfo = (SDB_Update_DouFaPartInfo_Req *)buf;

	pDouFaPartInfo->m_UserUID = m_pActor->GetUID().ToUint64();
	pDouFaPartInfo->m_LastGetHonorTime = m_LastGetHonorTime;
	pDouFaPartInfo->m_GetHonorToday = m_GetHonorToday;
	pDouFaPartInfo->m_JoinChallengeNum = m_ChallengeData.m_JoinChallengeNum;
	pDouFaPartInfo->m_LastJoinChallengeTime = m_ChallengeData.m_LastJoinChallengeTime;
	pDouFaPartInfo->m_MaxChallengeNumToday  = m_ChallengeData.m_MaxChallengeNumToday;
	pDouFaPartInfo->m_LastFLushDouFaUpLevel = m_LastFLushDouFaUpLevel;
	pDouFaPartInfo->m_CreditUp = m_DuoBaoData.m_CreditUp;
	pDouFaPartInfo->m_GetCredit = m_DuoBaoData.m_GetCredit;
	pDouFaPartInfo->m_LastChangeCreditUp = m_DuoBaoData.m_LastChangeCreditUp;
	
	DB_DouFaQieCuoEnemyData DouFaQieCuoEnemyData;

	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.begin();

	for( ; iter != m_mapEnemy.end(); ++iter)
	{
		DouFaEnemyInfo & EnemyInfo = iter->second;

		for( int i = 0; i < DOUFA_ENEMY_NUM; ++i)
		{
			if( !UID(DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_uidEnemy).IsValid()){
				DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_bFinish = EnemyInfo.m_bFinish;
				DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_bWin	   = EnemyInfo.m_bWin;
				strncpy(DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_EnemyName, EnemyInfo.m_Name, sizeof(DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_EnemyName));
				DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_uidEnemy = EnemyInfo.m_UserEnemy.ToUint64();
				DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_Facade   = EnemyInfo.m_Facade;
				DouFaQieCuoEnemyData.m_DouFaEnemy[i].m_VipLevel = EnemyInfo.m_vipLevel;
				break;
			} 
		}
	}

	std::map<UID, QieCuoUserData>::iterator it = m_mapQieCuo.begin();

	for( ; it != m_mapQieCuo.end(); ++it)
	{
		QieCuoUserData & EnemyInfo = it->second;

		for( int i = 0; i < QIECUO_ENEMY_NUM; ++i)
		{
			if( !UID(DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_uidEnemy).IsValid()){
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_bFinish = EnemyInfo.m_bFinish;
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_bWin	= EnemyInfo.m_bWin;
				strncpy(DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_EnemyName, EnemyInfo.m_Name, sizeof(DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_EnemyName));
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_Layer   = EnemyInfo.m_Layer;
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_Level	= EnemyInfo.m_Level;
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_TitleID = EnemyInfo.m_TitleID;
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_uidEnemy = EnemyInfo.m_UidUser.ToUint64();
				DouFaQieCuoEnemyData.m_QieCuoEnemy[i].m_VipLevel = EnemyInfo.m_vipLevel;
				break;
			}
		}
	}


	memcpy(&pDouFaPartInfo->m_DouFaQieCuoEnemy, &DouFaQieCuoEnemyData, sizeof(pDouFaPartInfo->m_DouFaQieCuoEnemy));

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void DouFaPart::InitPrivateClient()
{
	if ( GetUserLvIndex() != INVALID_INDEX || GetGroupID() != INVALID_GROUPID )
	{
		g_pThingServer->GetDouFaMgr().QuitDuoBao(m_pActor);
	}
}


//玩家下线了，需要关闭该ThingPart
void DouFaPart::Close()
{
}

//保存数据
void DouFaPart::SaveData()
{
	SDB_Update_DouFaPartInfo_Req Update_Req;

	int nLen = sizeof(SDB_Update_DouFaPartInfo_Req);

	if( false == this->OnGetDBContext(&Update_Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Update_Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateDouFaPartInfo, ob.TakeOsb(), 0, 0);
}

// nRetCode: 取值于 enDBRetCode
void DouFaPart::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	}

	switch(ReqCmd)
	{
	case enDBCmd_Flush_QieCuoEnemy:
		{
			//切磋，刷新对手
			this->HandleQieCuoGetEnemy(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Flush_DouFaEnemy:
		{
			//斗法，刷新对手
			this->HandleFlushDouFaEnemy(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_MyRank_Challenge:
		{
			//得到我的挑战排行，只得到排行
			this->HandleGetMyRankChallenge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_ChallengeRecordBuf:
		{
			//得到我的挑战回放
			this->HandleGetMyRankChallengeRecordBuf(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
    default:
	    {
		   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	    }
	    break;
	}
}

void DouFaPart::OnEvent(XEventData & EventData)
{
	if ( 0 == EventData.m_pContext )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitTeam);

	if ( msgID == EventData.m_MsgID )
	{
		//离队事件
		SS_QuitTeam * pQuitTeam = (SS_QuitTeam *)EventData.m_pContext;

		if ( pQuitTeam->m_bMeQuit )
		{
			IActor * pMember = g_pGameServer->GetGameWorld()->FindActor(pQuitTeam->m_uidTeamMember);

			IActor * pLeader = g_pGameServer->GetGameWorld()->FindActor(pQuitTeam->m_uidTeamLeader);

			if ( 0 == pMember || 0 == pLeader )
			{
				TRACE("<error> %s : %d Line 找不到队友或者队长！！", __FUNCTION__, __LINE__);
				return;
			}

			//离队当退出处理
			g_pThingServer->GetDouFaMgr().QuitDuoBao(m_pActor);
		}

		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);
	
	if ( msgID == EventData.m_MsgID )
	{
		//创建角色事件
		SS_ActoreCreateContext * pActoreCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;

		if( true == m_bListenQieCuo){
			//切磋事件分支
			this->OnEventQieCuo(UID(pActoreCreateContext->m_uidActor));
		}

		if( true == m_bListenDouFa){
			//其它的走斗法事件分支
			this->OnEventDouFa(UID(pActoreCreateContext->m_uidActor));	
		}
		return;
	}
}

void DouFaPart::OnTimer(UINT32 timerID)
{
	if ( enDouFaTimer_ID_Challenge == timerID )
	{
		this->ResetMaxChallengeNum();

		g_pGameServer->GetTimeAxis()->KillTimer(enDouFaTimer_ID_Challenge,this);

		g_pGameServer->GetTimeAxis()->SetTimer(enDouFaTimer_ID_Challenge,this,XDateTime::SECOND_OF_DAY * 1000,"DouFaPart::OnTimer");
	}
	else if ( enDouFaTimer_ID_Run == timerID )
	{
		m_DuoBaoData.m_RunBeginTime = 0;

		g_pGameServer->GetTimeAxis()->KillTimer(enDouFaTimer_ID_Run,this);
	}
}

//打开斗法标签
void DouFaPart::OpenDouFaLabel()
{
	//同步斗法数据
	this->SynDouFaData();
}

//打开切磋标签
void DouFaPart::OpenQieCuoLabel()
{
	//同步切磋数据
	this->SynQieCuoData();
}


//斗法
void DouFaPart::DouFaCombat(UID uidEnemy)
{
	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.find(uidEnemy);
	if( iter == m_mapEnemy.end()){
		return;
	}

	DouFaEnemyInfo & EnemyInfo = iter->second;

	SC_DouFaDouFa_Rsp Rsp;
	if( EnemyInfo.m_bFinish)
	{
		Rsp.m_Result = enDouFaRetCode_ErrFinished;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrInDuoBao2;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrWaitTeam;
	}
	else
	{
		IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
		if( 0 == pTargetActor){

			this->StartListenUserCreate(enListenType_DouFa);

			g_pGameServer->LoadActor(uidEnemy);

			return;
		}

		this->BeginDouFaCombat(uidEnemy);	
	}

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_DouFaCombat, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//获得今天参加的挑战次数
INT16 DouFaPart::GetChallengeNum()
{
	if( !XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_ChallengeData.m_LastJoinChallengeTime))
	{
		m_ChallengeData.m_JoinChallengeNum = 0;
	}

	return m_ChallengeData.m_JoinChallengeNum;
}

//得到今天获得的荣誉值
INT32	DouFaPart::GetGetHonorToday()
{
	if(! XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_LastGetHonorTime)){
		 m_GetHonorToday = 0;
	}

	return m_GetHonorToday;	
}

//得到今天最大可获得荣誉值
INT32	DouFaPart::GetMaxGetHonorToday()
{
	INT32 MaxHonor = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGetHonorDouFaToday;

	ISyndicate * pSyndicate = m_pActor->GetSyndicate();

	if ( 0 != pSyndicate){
		
		INT32 nValue = pSyndicate->GetWelfareValue(enWelfare_MaxHonorOneDay);

		MaxHonor = MaxHonor + MaxHonor * nValue / 100;
	}

	MaxHonor += m_pActor->GetVipValue(enVipType_AddMaxHonorToday);

	return MaxHonor;
}

//刷新切磋对手
void DouFaPart::FlushQieCuoEnemy()
{
	ICDTimerPart * pCDTimerPart = m_pActor->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	SC_FlushQieCuo_Rsp Rsp;

	if( !pCDTimerPart->IsCDTimeOK(g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoCDTimerID)){
		//冷却中
		Rsp.m_Result = enDouFaRetCode_ErrCDTimer;
	}else{
		UINT8	nLevel = m_pActor->GetCrtProp(enCrtProp_Level);

		SDB_Flush_QieCuoEnemy Req;

		Req.m_UidUser = m_pActor->GetUID().ToUint64();
		Req.m_Num = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoEnemyNum;

		switch(nLevel / 10)
		{
		case 0:
			{
				Req.m_BeginLevel = 1;
				Req.m_EndLevel	 = 9;
			}
			break;
		case 1:
			{
				Req.m_BeginLevel = 10;
				Req.m_EndLevel	 = 19;
			}
			break;
		case 2:
			{
				Req.m_BeginLevel = 20;
				Req.m_EndLevel	 = 29;
			}
			break;
		case 3:
			{
				Req.m_BeginLevel = 30;
				Req.m_EndLevel	 = 39;
			}
			break;
		case 4:
			{
				Req.m_BeginLevel = 40;
				Req.m_EndLevel	 = 49;
			}
			break;
		case 5:
		case 6:
			{
				Req.m_BeginLevel = 50;
				Req.m_EndLevel	 = 60;
			}
			break;
		default:
			TRACE("<error> %s : %d 程序错误！！，玩家等级=%d", __FUNCTION__, __LINE__, nLevel);
			return;
		}

		m_mapQieCuo.clear();

		OBuffer1k ob;
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Flush_QieCuoEnemy, ob.TakeOsb(), &g_pThingServer->GetDouFaMgr(), m_pActor->GetUID().ToUint64());	

		pCDTimerPart->RegistCDTime(g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoCDTimerID);

		return;
	}

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_Flush_QieCuo, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//开始切磋
void DouFaPart::BeginQieCuo(UID uidEnemy)
{
	ICombatPart * pCombatPart = m_pActor->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}

	//先把对手数据发给客户端
	this->SendActorInfo(uidEnemy);

	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QiCuoMapID;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	TSceneID SceneID =pGameScene->GetSceneID();

	if( 0 == pGameScene){
		return ;
	}	

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(MapID);
	if( 0 == pMapConfig){
		TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, MapID);
		return;
	}

	pCombatPart->CombatWithScene(enCombatType_QieCuo,uidEnemy, m_QieCuoCombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID),pGameScene, enJoinPlayer_Oneself);
	
}

//开始斗法战斗
void	DouFaPart::BeginDouFaCombat(UID uidEnemy)
{
	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.find(uidEnemy);
	if( iter == m_mapEnemy.end()){
		return;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
	if( 0 == pTargetActor){
		return;
	}

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}

	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaMapID;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	TSceneID SceneID =pGameScene->GetSceneID();

	if( 0 == pGameScene){
		return ;
	}

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(MapID);
	if( 0 == pMapConfig){
		TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, MapID);
		return;
	}

	pCombatPart->CombatWithScene(enCombatType_DouFa,uidEnemy, m_DouFaCombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID),pGameScene,enJoinPlayer_Oneself);

}

//斗法每场比赛结果处理
void	DouFaPart::DouFaForward(UID uidEnemy, bool bWin,const OBuffer4k & ob)
{
	bool bOverWin = false;	//

	bool bAlreadyOver = this->IsDouFaOver(bOverWin);

	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.find(uidEnemy);
	if( iter == m_mapEnemy.end()){
		return;
	}

	DouFaEnemyInfo & EnemyInfo = iter->second;

	EnemyInfo.m_bFinish = true;
	EnemyInfo.m_bWin = bWin;

	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
	if( 0 == pEnemy){
		return;
	}

	SC_DouFaDouFa_Rsp Rsp;
	Rsp.m_Result = enDouFaRetCode_Ok;

	DouFaCombatData CombatData;
	CombatData.m_uidEnemy	 = uidEnemy;
	CombatData.m_Credit		 = 0;

	
	////胜利方加的声望值
	//INT32 nAddCredit = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaWinGetCredit;

	////失败方减的声望值
	//INT32 nDesCredit = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaFailLostCredit;

	//if( true == bWin){
	//	m_pActor->AddCrtPropNum(enCrtProp_ActorCredit, nAddCredit);

	//	//对手的当前声望值
	//	INT32 nEnemyCredit = pEnemy->GetCrtProp(enCrtProp_ActorCredit);
	//	//实际扣除声望值
	//	nDesCredit = (nEnemyCredit - nDesCredit >= 0) ?  nDesCredit : nEnemyCredit;

	//	pEnemy->AddCrtPropNum(enCrtProp_ActorCredit, -nDesCredit);

	//	CombatData.m_Credit = nAddCredit + m_pActor->GetVipValue(enVipType_AddGetCredit);
	//}else{
	//	pEnemy->AddCrtPropNum(enCrtProp_ActorCredit, nAddCredit);

	//	//自己的当前声望值
	//	INT32 nCredit = m_pActor->GetCrtProp(enCrtProp_ActorCredit);
	//	//实际扣除声望值
	//	nDesCredit = (nCredit - nDesCredit >= 0) ?  nDesCredit : nCredit;

	//	m_pActor->AddCrtPropNum(enCrtProp_ActorCredit, -nDesCredit);

	//	CombatData.m_Credit = -nDesCredit;
	//}

	OBuffer4k ob2;
	ob2 <<  DouFaHeader(enDouFaCmd_DouFaCombat, sizeof(Rsp) + sizeof(CombatData) ) << Rsp << CombatData ;
	m_pActor->SendData(ob2.TakeOsb());

	//检测斗法是否结束
	if( !bAlreadyOver && this->IsDouFaOver(bOverWin)){
		this->DouFaOverForward(bOverWin);
	}
}

//检测斗法是结束
bool	DouFaPart::IsDouFaOver(bool & bWin)
{
	if( m_mapEnemy.size() < g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaRandEnemyNum){
		return true;
	}

	//3局2胜制
	INT32 nWinNum  = 0;
	INT32 nFailNum = 0;

	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.begin();

	for( ; iter != m_mapEnemy.end(); ++iter)
	{
		DouFaEnemyInfo & EnemyInfo = iter->second;

		if( EnemyInfo.m_bFinish == false){
			continue;
		}

		if( true == EnemyInfo.m_bWin){
			++nWinNum;

			if( nWinNum >= 2){
				bWin = true;
				return true;
			}		
		}else{
			++nFailNum;

			if( nFailNum >= 2){
				bWin = false;
				return true;
			}		
		}
	}

	return false;
}

//斗法结束处理
void	DouFaPart::DouFaOverForward(bool bWin)
{
	//斗法全部结束
	SC_DouFaOver_Rsp Rsp;

	INT32 MaxGetHonor = this->GetMaxGetHonorToday();
	
	if( bWin){
		Rsp.m_bWin  = true;

		INT32 GetHonor = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaWinGetHonor;

		GetHonor += m_pActor->GetVipValue(enVipType_AddGetHonor);

		INT32 TodayGetHonor = this->GetGetHonorToday();

		if( (GetHonor + TodayGetHonor) > MaxGetHonor){
			GetHonor = MaxGetHonor - TodayGetHonor;
		}

		Rsp.m_Honor = GetHonor;
	}else{
		Rsp.m_bWin  = false;
		Rsp.m_Honor = 0;
	}

	this->AddHonor(Rsp.m_Honor);

	Rsp.m_GetHonorToday = this->GetGetHonorToday();;
	Rsp.m_Max_GetHonorToday = MaxGetHonor;

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_SC_DouFaOver, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//OnEnvent的切磋分支
void	DouFaPart::OnEventQieCuo(UID uidEnemy)
{
	std::map<UID, QieCuoUserData>::iterator iter = m_mapQieCuo.find(uidEnemy);
	if( iter == m_mapQieCuo.end()){
		return;
	}

	//取消监听
	this->RemoveListenUserCreate(enListenType_QieCuo);

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
	if( 0 == pActor){
		return;
	}

	//开始切磋战斗
	this->BeginQieCuo(uidEnemy);
}

//OnEnvent的斗法分支
void	DouFaPart::OnEventDouFa(UID uidEnemy)
{
	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.find(uidEnemy);
	if( iter == m_mapEnemy.end()){
		return;
	}

	//取消监听
	this->RemoveListenUserCreate(enListenType_DouFa);

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
	if( 0 == pActor){
		return;
	}

	//开始斗法战斗
	this->BeginDouFaCombat(uidEnemy);	
}

//发送玩家信息给客户端
void	DouFaPart::SendActorInfo(UID uidUser)
{
	IActor * pDouFaActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);
	if( 0 == pDouFaActor){
		return;
	}

	ICombatPart * pCombatPart = pDouFaActor->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}
	
	std::vector<SCreatureLineupInfo> vectCreature = pCombatPart->GetJoinBattleActor();

	for( int i = 0; i < vectCreature.size(); ++i)
	{
		switch(vectCreature[i].m_pCreature->GetThingClass())
		{
		case enThing_Class_Actor:
			{
				m_pActor->NoticClientCreatePublicThing(vectCreature[i].m_pCreature->GetUID());	
			}
			break;
		case enThing_Class_Monster:
			{
				m_pActor->NoticClientCreatePrivateThing(vectCreature[i].m_pCreature->GetUID());	
			}
			break;
		}

	}
}

//进入切磋场景
bool	DouFaPart::EnterQieCuoScene()
{
	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QiCuoMapID;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	TSceneID SceneID =pGameScene->GetSceneID();

	if( 0 == pGameScene){
		return false;
	}

	pGameScene->EnterScene(m_pActor);

	SC_EnterQieCuoScene Rsp;
	Rsp.m_Result = enDouFaRetCode_Ok;

	OBuffer1k ob;
	ob << DouFaHeader(enDouFaCmd_SC_EnterQieCuo,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	return  true;
}

//进入斗法场景
bool	DouFaPart::EnterDouFaScene()
{
	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DouFaMapID;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	TSceneID SceneID =pGameScene->GetSceneID();

	if( 0 == pGameScene){
		return false;
	}

	pGameScene->EnterScene(m_pActor);

	SC_EnterDouFaScene Rsp;
	Rsp.m_Result = enDouFaRetCode_Ok;

	OBuffer1k ob;
	ob << DouFaHeader(enDouFaCmd_SC_EnterDouFa,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	return  true;
}

//切磋得到敌人
void	DouFaPart::HandleQieCuoGetEnemy(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());
	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_FlushQieCuo_Rsp Rsp;

	INT32 RealNum = m_mapQieCuo.size();

	if( OutParam.retCode == enDBRetCode_OK){

		INT32 nNum = RspIb.Remain() / sizeof(SDB_QieCuoEnemy);

		for( int i = 0; i < nNum; ++i)
		{
			SDB_QieCuoEnemy DBEnemy;

			RspIb >> DBEnemy;
			if( RspIb.Error()){
				TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,RspIb.Capacity());
				continue;
			}

			QieCuoUserData UserData;

			UserData.m_bFinish = false;
			UserData.m_bWin	   = false;
			UserData.m_Layer   = DBEnemy.m_Layer;
			UserData.m_Level   = DBEnemy.m_Level;
			UserData.m_vipLevel= DBEnemy.m_vipLevel;
			strncpy(UserData.m_Name, DBEnemy.m_Name, sizeof(UserData.m_Name));
			UserData.m_TitleID = DBEnemy.m_TitleID;
			UserData.m_UidUser = UID(DBEnemy.m_UidUser);

			IActor * pEnemyActor = g_pGameServer->GetGameWorld()->FindActor(UserData.m_UidUser);
			if( 0 != pEnemyActor){
				UserData.m_Level = pEnemyActor->GetCrtProp(enCrtProp_Level);
				UserData.m_Layer = pEnemyActor->GetCrtProp(enCrtProp_ActorLayer);
				UserData.m_vipLevel = pEnemyActor->GetCrtProp(enCrtProp_TotalVipLevel);
			}

			if( m_mapQieCuo.insert(std::map<UID, QieCuoUserData>::value_type(UserData.m_UidUser, UserData)).second){
				++RealNum;
				if( RealNum >= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoEnemyNum){
					break;
				}
			}
		}
	}

	if( RealNum < g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoEnemyNum){
		//该等级的人数少于切磋指定人数，继续从上级获取玩家补充
		SDB_Flush_QieCuoEnemy ReqEnemy;
		ReqIb >> ReqEnemy;
		if( ReqIb.Error()){
			TRACE("<error> %s : %d 行 数据错误!!", __FUNCTION__, __LINE__);
		}else if( !(ReqEnemy.m_BeginLevel == 0 && ReqEnemy.m_EndLevel == 60)){
			//获取上一级的等级开始和结束
			SDB_Flush_QieCuoEnemy Req;

			Req.m_UidUser = m_pActor->GetUID().ToUint64();
			Req.m_Num = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_QieCuoEnemyNum - m_mapQieCuo.size();

			Req.m_BeginLevel = ReqEnemy.m_BeginLevel - 10;
			Req.m_EndLevel   = ReqEnemy.m_BeginLevel - 1;

			if( ReqEnemy.m_BeginLevel <= 0){
				//则从所有玩家中选出指定数量的切磋对象
				Req.m_BeginLevel = 0;
				Req.m_EndLevel	 = 60;
			}
		
			OBuffer1k ob;
			ob << Req;
			g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Flush_QieCuoEnemy, ob.TakeOsb(), &g_pThingServer->GetDouFaMgr(), m_pActor->GetUID().ToUint64());
			return;
		}
	}

	//同步
	this->SynQieCuoData();
	
	OBuffer1k ob2;
	ob2 << DouFaHeader(enDouFaCmd_Flush_QieCuo,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob2.TakeOsb());
}

//同步切磋数据
void	DouFaPart::SynQieCuoData()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_SynQieCuo SynQieCuo;

	SynQieCuo.m_Num = m_mapQieCuo.size();

	OBuffer1k ob;
	ob << DouFaHeader(enDouFaCmd_SynQieCuo,sizeof(SynQieCuo) + SynQieCuo.m_Num * sizeof(QieCuoUserData)) << SynQieCuo;

	std::map<UID, QieCuoUserData>::iterator iter = m_mapQieCuo.begin();

	for(; iter != m_mapQieCuo.end(); ++iter)
	{
		QieCuoUserData & userData = iter->second;

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(userData.m_UidUser);
		if( 0 != pSyndicate){
			strncpy(userData.m_SynName, pSyndicate->GetSynName(), sizeof(userData.m_SynName));
		}

		IActor * pEnemyActor = g_pGameServer->GetGameWorld()->FindActor(userData.m_UidUser);
		if( 0 != pEnemyActor){
			userData.m_Level = pEnemyActor->GetCrtProp(enCrtProp_Level);
			userData.m_Layer = pEnemyActor->GetCrtProp(enCrtProp_ActorLayer);
			userData.m_vipLevel = pEnemyActor->GetCrtProp(enCrtProp_TotalVipLevel);
		}

		ob << userData;
	}

	m_pActor->SendData(ob.TakeOsb());	
}

//切磋战斗
void DouFaPart::QieCuoBattle(UID uidEnemy)
{
	std::map<UID, QieCuoUserData>::iterator iter = m_mapQieCuo.find(uidEnemy);
	if( iter == m_mapQieCuo.end()){
		return;
	}

	QieCuoUserData & userdata = iter->second;

	SC_QieCuoBattle Rsp;
	Rsp.m_Result = enDouFaRetCode_Ok;

	if( userdata.m_bFinish)
	{
		Rsp.m_Result = enDouFaRetCode_ErrFinished;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrInDuoBao2;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrWaitTeam;
	}
	else
	{
		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);
		if( 0 == pEnemy){
			this->StartListenUserCreate(enListenType_QieCuo);

			//把玩家载入内存
			g_pGameServer->LoadActor(uidEnemy);

			return;
		}

		this->BeginQieCuo(uidEnemy);
	}

	OBuffer1k ob2;
	ob2 << DouFaHeader(enDouFaCmd_QieCuoBattle,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob2.TakeOsb());
}

//刷新斗法的对手
void DouFaPart::FlushDouFaEnemy()
{
	bool bWin = false;

	SC_Flush_DouFa_Rsp Rsp;
	OBuffer1k ob;

	if( !this->IsDouFaOver(bWin)){
		Rsp.m_Result = enDouFaRetCode_ErrNotOver;
	}else{
		UINT8 Level = m_pActor->GetCrtProp(enCrtProp_Level);

		//获取新的配对玩家
		SDB_Flush_DouFaEnemy Req;

		std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.begin();

		for ( ; iter != m_mapEnemy.end(); ++iter )
		{
			const UID & uidEnemy = iter->first;

			if ( !UID(Req.m_uidEnemy1).IsValid() ){

				Req.m_uidEnemy1 = uidEnemy.ToUint64();

			} else if ( !UID(Req.m_uidEnemy2).IsValid() ){

				Req.m_uidEnemy2 = uidEnemy.ToUint64();

			} else if ( !UID(Req.m_uidEnemy3).IsValid() ){

				Req.m_uidEnemy3 = uidEnemy.ToUint64();
				break;

			} 
		}

		if ( m_mapEnemy.size() == 0 || bWin || ( !bWin && m_LastFLushDouFaUpLevel <= 1) ){
			m_LastFLushDouFaUpLevel = Level + 1;
		} else {
			m_LastFLushDouFaUpLevel -= 1;
		}

		Req.m_LimitUpLv = m_LastFLushDouFaUpLevel;

		Req.m_UidUser = m_pActor->GetUID().ToUint64();

		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Flush_DouFaEnemy, ob.TakeOsb(), &g_pThingServer->GetDouFaMgr(), m_pActor->GetUID().ToUint64());

		m_mapEnemy.clear();

		return;
	}

	ob << DouFaHeader(enDouFaCmd_Flush_DouFa,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//增加荣誉值
void	DouFaPart::AddHonor(INT32 nHonor)
{
	if( nHonor > 0){

		if( !XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_LastGetHonorTime)){
			m_GetHonorToday = 0;
		}

		m_GetHonorToday += nHonor;
		m_pActor->AddCrtPropNum(enCrtProp_ActorHonor, nHonor);
	}

	m_LastGetHonorTime = CURRENT_TIME();
}

//参加挑战
void	DouFaPart::JoinChallenge()
{
	m_ChallengeData.m_LastJoinChallengeTime = CURRENT_TIME();

	m_ChallengeData.m_JoinChallengeNum += 1;
}

//清除挑战次数
void	DouFaPart::ClearChallengeNum()
{
	m_ChallengeData.m_JoinChallengeNum = 0;
}

//最大可参加挑战次数
UINT8	DouFaPart::MaxChallengeNum()
{
	return m_ChallengeData.m_MaxChallengeNumToday;
}

//监听玩家的创建
void	DouFaPart::StartListenUserCreate(enListenType ListenType)
{
	if( m_bListenDouFa == false && m_bListenQieCuo == false)
	{
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
		if( 0 == pEnventServer){
			return;
		}

		pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"DouFaPart::StartListenUserCreate");	
	}

	if( ListenType == enListenType_DouFa){
		m_bListenDouFa = true;
	}else{
		m_bListenQieCuo = true;
	}
}

//取消监听玩家的创建
void	DouFaPart::RemoveListenUserCreate(enListenType ListenType)
{
	if( m_bListenDouFa == false || m_bListenQieCuo == false){
		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
		if( 0 == pEnventServer){
			return;
		}
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);
		pEnventServer->RemoveListener(this, msgID, enEventSrcType_Actor, 0);
	}

	if( ListenType == enListenType_DouFa){
		m_bListenDouFa = false;
	}else{
		m_bListenQieCuo = false;
	}
}

//切磋每场比赛结果处理
void	DouFaPart::QieCuoForward(UID uidEnemy, bool bWin, const OBuffer4k & ob)
{
	std::map<UID, QieCuoUserData>::iterator iter = m_mapQieCuo.find(uidEnemy);
	if( iter == m_mapQieCuo.end()){
		return;
	}

	QieCuoUserData & userdata = iter->second;

	userdata.m_bFinish = true;
	userdata.m_bWin	   = bWin;

	SC_QieCuoBattle Rsp;

	Rsp.m_Result = enDouFaRetCode_Ok;
		
	OBuffer4k ob2;
	ob2 <<  DouFaHeader(enDouFaCmd_QieCuoBattle, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob2.TakeOsb());	
}

//同步斗法数据
void	DouFaPart::SynDouFaData()
{
	SC_SynDouFa Rsp;

	Rsp.m_GetHonorOfDay = this->GetGetHonorToday();;
	Rsp.m_MaxGetHonorOfDay = this->GetMaxGetHonorToday();
	Rsp.m_Num = m_mapEnemy.size();

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_SynDouFa, sizeof(Rsp) + Rsp.m_Num * sizeof(DouFaUser)) << Rsp;

	std::map<UID,  DouFaEnemyInfo>::iterator iter = m_mapEnemy.begin();
	
	for( ; iter != m_mapEnemy.end(); ++iter)
	{
		DouFaEnemyInfo & doufaUser = iter->second;

		DouFaUser userdata;
		userdata.m_bFinish = doufaUser.m_bFinish;
		userdata.m_bWin	   = doufaUser.m_bWin;
		userdata.m_uidUser = doufaUser.m_UserEnemy;
		userdata.m_Facade  = doufaUser.m_Facade;
		userdata.m_VipLevel = doufaUser.m_vipLevel;
		strncpy(userdata.m_Name, doufaUser.m_Name, sizeof(userdata.m_Name));

		ob << userdata;
	}
	
	m_pActor->SendData(ob.TakeOsb());
}

void	DouFaPart::HandleFlushDouFaEnemy(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
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

	SC_Flush_DouFa_Rsp Rsp;
	OBuffer1k ob;

	if( OutParam.retCode != 0){
		Rsp.m_Result = enDouFaRetCode_ErrNoEnemy;
	}else{
		int num = RspIb.Remain() / sizeof(SDB_DouFaEnemy);

		for ( int i = 0; i < num; ++i )
		{
			SDB_DouFaEnemy DouFaEnemy;
			RspIb >> DouFaEnemy;

			if( RspIb.Error()){
				TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
				return;
			}

			if ( !UID(DouFaEnemy.m_UidEnemy).IsValid() ){
				break;
			}

			DouFaEnemyInfo EnemyInfo;

			EnemyInfo.m_bFinish = false;
			EnemyInfo.m_bWin	= false;

			EnemyInfo.m_UserEnemy = UID(DouFaEnemy.m_UidEnemy);
			EnemyInfo.m_Facade	  = DouFaEnemy.m_Facade;
			strncpy(EnemyInfo.m_Name, DouFaEnemy.m_NameEnemy, sizeof(EnemyInfo.m_Name));
			EnemyInfo.m_vipLevel  = DouFaEnemy.m_VipLevel;
			
			m_mapEnemy.insert(std::map<UID,  DouFaEnemyInfo>::value_type(EnemyInfo.m_UserEnemy,EnemyInfo)); 
		}

		this->SynDouFaData();
	}

	ob.Reset();
	ob <<  DouFaHeader(enDouFaCmd_Flush_DouFa, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//重新设置今天最多可挑战次数
void	DouFaPart::ResetMaxChallengeNum()
{
	SDB_Get_MyRank_Challenge_Req Req;

	Req.m_uidUser = m_pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Get_MyRank_Challenge, ob.TakeOsb(),&g_pThingServer->GetDouFaMgr(), m_pActor->GetUID().ToUint64());
}

void	DouFaPart::HandleGetMyRankChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_MyRank  MyRank;
	RspIb >> RspHeader >> OutParam >> MyRank;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != 0){
		return;
	}

	m_ChallengeData.m_MaxChallengeNumToday = g_pGameServer->GetConfigServer()->GetMaxChallengeNum(MyRank.m_Rank);
}

//设置今天最大挑战次数
void	DouFaPart::SetMaxChallengeToday(UINT16 MaxChallengeNum)
{
	m_ChallengeData.m_MaxChallengeNumToday = MaxChallengeNum;
}


//得到本周声望上限
UINT32 DouFaPart::GetMaxCreditWeek()
{
	UINT32 CurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameWeek(CurTime,m_DuoBaoData.m_LastChangeCreditUp) )
	{
		const SDuoBaoCnfg * pDuoBaoCnfg = g_pGameServer->GetConfigServer()->GetDuoBaoCnfg(m_pActor->GetCrtProp(enCrtProp_DuoBaoLevel));

		if ( 0 == pDuoBaoCnfg )
		{
			TRACE("<error> %s : %d Line 获取夺宝配置失败！！夺宝等级%d",__FUNCTION__, __LINE__, m_pActor->GetCrtProp(enCrtProp_DuoBaoLevel));
			return 0;
		}

		m_DuoBaoData.m_CreditUp = pDuoBaoCnfg->m_CreditUp + m_pActor->GetVipValue(enVipType_AddDuoBaoCreditUp);
		m_DuoBaoData.m_GetCredit = 0;
		m_DuoBaoData.m_LastChangeCreditUp = CurTime;
	}

	return m_DuoBaoData.m_CreditUp;
}

//得到本周获得声望
UINT32 DouFaPart::GetCreditWeek()
{
	UINT32 CurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameWeek(CurTime,m_DuoBaoData.m_LastChangeCreditUp) )
	{
		const SDuoBaoCnfg * pDuoBaoCnfg = g_pGameServer->GetConfigServer()->GetDuoBaoCnfg(m_pActor->GetCrtProp(enCrtProp_DuoBaoLevel));

		if ( 0 == pDuoBaoCnfg )
		{
			TRACE("<error> %s : %d Line 获取夺宝配置失败！！夺宝等级%d",__FUNCTION__, __LINE__, m_pActor->GetCrtProp(enCrtProp_DuoBaoLevel));
			return 0;
		}

		m_DuoBaoData.m_CreditUp = pDuoBaoCnfg->m_CreditUp + m_pActor->GetVipValue(enVipType_AddDuoBaoCreditUp);
		m_DuoBaoData.m_GetCredit = 0;
		m_DuoBaoData.m_LastChangeCreditUp = CurTime;
	}

	return m_DuoBaoData.m_GetCredit;
}

//夺宝获得声望
void   DouFaPart::AddCreditDuoBao(INT32 AddCredit)
{
	if ( AddCredit != 0 )
	{
		INT32 MaxGetCredit = this->GetMaxCreditWeek();

		INT32 GetCredit = this->GetCreditWeek();

		INT32 VipAdd = m_pActor->GetVipValue(enVipType_AddGetCredit);

		if ( GetCredit + AddCredit + VipAdd > MaxGetCredit ){
			AddCredit = MaxGetCredit - (GetCredit + VipAdd);
		}

		m_pActor->AddCrtPropNum(enCrtProp_ActorCredit, AddCredit);

		m_DuoBaoData.m_GetCredit += (AddCredit + VipAdd);	
	}
}

//是否正在夺宝中
bool   DouFaPart::IsInDuoBao()
{
	return (m_DuoBaoData.m_UserLvIndex != INVALID_INDEX || m_DuoBaoData.m_GroupID != INVALID_GROUPID );
}

//设置玩家参加夺宝的玩家等级组下标
void   DouFaPart::SetUserLvIndex(INT8 UserLvIndex)
{
	m_DuoBaoData.m_UserLvIndex = UserLvIndex;
}

//得到玩家参加夺宝的玩家等级组下标
INT8   DouFaPart::GetUserLvIndex()
{
	return m_DuoBaoData.m_UserLvIndex;
}

//得到配对组ID
TGroupID DouFaPart::GetGroupID()
{
	return m_DuoBaoData.m_GroupID;
}

//设置配对组ID
void	DouFaPart::SetGroupID(TGroupID groupID)
{
	m_DuoBaoData.m_GroupID = groupID;
}

//是否在逃跑惩罚期间
bool	DouFaPart::IsInRunTime()
{
	return (m_DuoBaoData.m_RunBeginTime > 0);
}

//开启逃跑惩罚
void	DouFaPart::StartRunTime()
{
	m_DuoBaoData.m_RunBeginTime = CURRENT_TIME();

	UINT32 TimerSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DuoBaoRunTimeNum;

	g_pGameServer->GetTimeAxis()->SetTimer(enDouFaTimer_ID_Run,this,TimerSpace * 1000,"DouFaPart::StartRunTime");
}

//掉线，下线要做的一些事
void	DouFaPart::LeaveDoSomeThing()
{	
	if ( GetUserLvIndex() != INVALID_INDEX || GetGroupID() != INVALID_GROUPID )
	{
		//在夺宝中
		g_pThingServer->GetDouFaMgr().QuitDuoBao(m_pActor);
	}
}

//得到逃跑惩罚剩余时间
UINT32	DouFaPart::GetLeftRunTime()
{
	if ( m_DuoBaoData.m_RunBeginTime == 0 )
		return 0;

	UINT32 TimerSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DuoBaoRunTimeNum;

	UINT32 CurTime = CURRENT_TIME();

	if ( m_DuoBaoData.m_RunBeginTime + TimerSpace < CurTime )
	{
		m_DuoBaoData.m_RunBeginTime = 0;
		return 0;
	}

	return m_DuoBaoData.m_RunBeginTime + TimerSpace - CurTime;
}


//是否组队夺宝
bool	DouFaPart::IsTeamDuoBao()
{
	return m_DuoBaoData.m_bTeam;
}

//设置是否组队夺宝
void	DouFaPart::SetIsTeamDuoBao(bool bIsTeam)
{
	m_DuoBaoData.m_bTeam = bIsTeam;
}

//清除夺宝数据
void	DouFaPart::ClearDuoBaoData()
{
	m_DuoBaoData.m_GroupID = INVALID_GROUPID;
	m_DuoBaoData.m_UserLvIndex = INVALID_INDEX;
	m_DuoBaoData.m_bTeam = false;
	m_DuoBaoData.m_DuoBaoLvIndex = INVALID_INDEX;

	TRACE("玩家%s成功清除夺宝数据！", m_pActor->GetName());
}

//设置玩家参加夺宝的夺宝等级组下标
void	DouFaPart::SetDuoBaoLvIndex(INT8 DuoBaoLvIndex)
{
	m_DuoBaoData.m_DuoBaoLvIndex = DuoBaoLvIndex;
}

//得到玩家参加夺宝的夺宝等级组下标
INT8	DouFaPart::GetDuoBaoLvIndex()
{
	return m_DuoBaoData.m_DuoBaoLvIndex;
}

	//战斗结束了
void DouFaPart::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	if ( pCombatCnt->uidSource != pCombatCnt->uidUser )
		return;

	if(m_pActor->GetUID() != pCombatCnt->uidSource)
	{
		return;
	}

    if(m_DouFaCombatID == pCombatCnt->CombatID)
	{
		if( pCombatResult->m_bWin){
			this->DouFaForward(pCombatCnt->uidEnemy, true, pCombatCnt->ob);
		}else{
			this->DouFaForward(pCombatCnt->uidEnemy, false, pCombatCnt->ob);
		
		}

		//发布事件
		SS_DouFa DouFa;
		DouFa.m_bWin = pCombatResult->m_bWin;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DouFa);
		m_pActor->OnEvent(msgID,&DouFa,sizeof(DouFa));
	}
	else if(m_QieCuoCombatID == pCombatCnt->CombatID )
	{
		if( pCombatResult->m_bWin){
			this->QieCuoForward(pCombatCnt->uidEnemy, true, pCombatCnt->ob);
		}else{
			this->QieCuoForward(pCombatCnt->uidEnemy, false, pCombatCnt->ob);
		}		
	}


}


void	DouFaPart::HandleGetMyRankChallengeRecordBuf(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
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

	SC_ViewChallengeRecord Rsp;
	OBuffer6k ob;
	
	if( OutParam.retCode != 0){
		Rsp.m_Result = enDouFaRetCode_ErrNoRecord;
		ob <<  DouFaHeader(enDouFaCmd_ViewChallengeRecord, sizeof(Rsp)) << Rsp;
	}else{
		SDB_ChallengeRecordBuf DBChallengeRecordBuf;
		RspIb >> DBChallengeRecordBuf;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}
		
		ob.Push((const unsigned char*)DBChallengeRecordBuf.m_RecordBuf,DBChallengeRecordBuf.m_BufLen);		
	}

	
	
	m_pActor->SendData(ob.TakeOsb());
}
