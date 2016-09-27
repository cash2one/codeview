#include "SynPart.h"
#include "XDateTime.h"
#include "ThingServer.h"
#include "IBasicService.h"


SynPart::SynPart()
{
	m_MaxJoinCombatNumToday = 0;

	m_JoinCombatNumToday = 0;

	m_LastJoinSynWarTime = 0;

	m_GetCreditWeek = 0;

	m_LastGetCreditTime = 0;

	m_LastSetMaxCombatNumTime = 0;
}
//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool SynPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if ( 0 == pMaster || 0 == pContext || nLen < sizeof(SDB_SynPart))
		return false;

	m_pActor = (IActor *)pMaster;

	SDB_SynPart * pDBSynPart = (SDB_SynPart *)pContext;

	m_JoinCombatNumToday = pDBSynPart->m_JoinCombatNumToday;

	m_LastJoinSynWarTime = pDBSynPart->m_LastJoinSynWarTime;

	m_MaxJoinCombatNumToday = pDBSynPart->m_MaxJoinCombatNumToday;

	m_LastSetMaxCombatNumTime = pDBSynPart->m_LastSetMaxCombatNumTime;

	m_GetCreditWeek = pDBSynPart->m_GetCreditWeek;

	m_LastGetCreditTime = pDBSynPart->m_LastGetCreditTime;

	return true;
}

//释放
void SynPart::Release(void)
{

}

//取得部件ID
enThingPart SynPart::GetPartID(void)
{
	return enThingPart_Actor_Syn;
}

//取得本身生物
IThing*		SynPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool	SynPart::OnGetDBContext(void * buf, int &nLen)
{
	if ( nLen != sizeof(SDB_Update_SynPart_Req) )
		return false;

	SDB_Update_SynPart_Req * pReq = (SDB_Update_SynPart_Req *)buf;

	pReq->m_uidUser = m_pActor->GetUID().ToUint64();
	pReq->m_JoinCombatNumToday = m_JoinCombatNumToday;
	pReq->m_LastJoinSynWarTime = m_LastJoinSynWarTime;
	pReq->m_MaxJoinCombatNumToday = m_MaxJoinCombatNumToday;
	pReq->m_LastSetMaxCombatNumTime = m_LastSetMaxCombatNumTime;
	pReq->m_GetCreditWeek = m_GetCreditWeek;
	pReq->m_LastGetCreditTime = m_LastGetCreditTime;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void SynPart::InitPrivateClient()
{
}


//玩家下线了，需要关闭该ThingPart
void SynPart::Close()
{
}

//保存数据
void SynPart::SaveData()
{
	SDB_Update_SynPart_Req DBReq;

	int nLen = sizeof(SDB_Update_SynPart_Req);

	if ( !this->OnGetDBContext(&DBReq, nLen) )
	{
		TRACE("<error> %s : %d Line 保存数据失败！！", __FUNCTION__, __LINE__);
		return;
	}

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_Update_SynPart,ob.TakeOsb(),0,0);
}

//获得今天参加帮战的战斗次数
UINT16	SynPart::GetJoinCombatNumToday()
{
	time_t nCurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameDay(nCurTime,m_LastJoinSynWarTime) ){
		m_JoinCombatNumToday = 0;
	}

	return m_JoinCombatNumToday;
}

//获得今天最多可参加帮战的战斗次数
UINT16	SynPart::GetMaxJoinCombatNumToday()
{
	time_t nCurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameDay(nCurTime,m_LastSetMaxCombatNumTime) ){
		m_MaxJoinCombatNumToday = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_MaxSynWarCombatNum;
	}

	return m_MaxJoinCombatNumToday;
}

//设置今天最多可参加帮战的战斗次数
void	SynPart::SetMaxJoinCombatNumToday(UINT16 MaxJoinCombatNum)
{
	m_MaxJoinCombatNumToday = MaxJoinCombatNum;

	m_LastSetMaxCombatNumTime = CURRENT_TIME();
}

//获得今天得到的声望
INT32	SynPart::GetGetNumWeek()
{
	time_t nCurTime = CURRENT_TIME();

	if ( !XDateTime::GetInstance().IsSameWeek(nCurTime,m_LastGetCreditTime) ){
		m_GetCreditWeek = 0;
	}

	return m_GetCreditWeek;
}

//增加今天的战斗次数
void	SynPart::AddCombatNumToday(INT16 AddNum)
{
	m_JoinCombatNumToday += AddNum;

	m_LastJoinSynWarTime = CURRENT_TIME();
}

//增加今天获得的声望
void	SynPart::AddGetCredit(INT32 Credit)
{
	this->GetGetNumWeek();

	m_pActor->AddCrtPropNum(enCrtProp_ActorCredit, Credit);

	m_GetCreditWeek += (Credit + m_pActor->GetVipValue(enVipType_AddGetCredit));

	m_LastGetCreditTime = CURRENT_TIME();
}

