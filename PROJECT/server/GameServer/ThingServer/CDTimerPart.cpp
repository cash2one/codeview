#include "CDTimerPart.h"
#include "ThingServer.h"
#include "XDateTime.h"

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool CDTimerPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || 0 == pContext){
		return false;
	}

	m_pActor = (IActor *)pMaster;

	SCDTimerDBData * pCDTimerDBData = (SCDTimerDBData *)pContext;

	if( nLen < pCDTimerDBData->m_Len || pCDTimerDBData->m_pData == 0){
		return false;
	}

	int nNum = pCDTimerDBData->m_Len / sizeof(SDB_CDTimerData);

	if( nNum < 1){
		return true;
	}

	SDB_CDTimerData * pCDTimerData = (SDB_CDTimerData *)pCDTimerDBData->m_pData;

	for( int i = 0; i < nNum; ++i, ++pCDTimerData)
	{
		if( pCDTimerData->m_CDTimerID == INVALID_CDTIMER_ID){
			continue;
		}

		this->__RegistDBCDTime(pCDTimerData);
	}

	return true;
}

//释放
void CDTimerPart::Release(void)
{

}

//取得部件ID
enThingPart CDTimerPart::GetPartID(void)
{
	return enThingPart_Actor_CDTimer;
}

//取得本身生物
IThing*		CDTimerPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool CDTimerPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void CDTimerPart::InitPrivateClient()
{

}


//玩家下线了，需要关闭该ThingPart
void CDTimerPart::Close()
{

}

//保存数据
void CDTimerPart::SaveData()
{
	MAPCDTIMER::iterator iter = m_mapCDTimer.begin();

	OBuffer1k ob;

	SDB_Delete_UserCDTimer_Req Req;
	Req.m_uidUser = m_pActor->GetUID().ToUint64();

	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteCDTimer, ob.TakeOsb(), NULL, 0);

	for(; iter != m_mapCDTimer.end(); ++iter)
	{
		SCDTimeData & CDTimeInfo = iter->second;

		SDB_Update_UserCDTimer_Req Req;

		Req.m_uidUser	= m_pActor->GetUID().ToUint64();
		Req.m_CDTimerID = iter->first;
		Req.m_EndTime   = CDTimeInfo.m_EndTime;

		ob.Reset();
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateCDTimer, ob.TakeOsb(), NULL, 0);
	}
}

void CDTimerPart::OnTimer(UINT32 timerID)
{
	//定时器ID既为冷却时间ID
	MAPCDTIMER::iterator iter = m_mapCDTimer.find(timerID);

	if( iter != m_mapCDTimer.end()){
		//有回调函数的话，执行回调函数
		SCDTimeData & CDTimeInfo = iter->second;

		if( 0 != CDTimeInfo.m_pCDTimeEndHandler){
			CDTimeInfo.m_pCDTimeEndHandler->CDTimeEndBackFunc(m_pActor->GetUID(), timerID);
		}

		m_mapCDTimer.erase(iter);
	}

	//销毁定时器
	this->__StopTimer(timerID);
}

//////////////////////////////////////////////////////////////////////////
//登记冷却的信息(会检测配置表中是否有，没有则不登记)
//func:CD时间到时的回调函数
//bMaster:是否是所有角色共用的
//////////////////////////////////////////////////////////////////////////
void CDTimerPart::RegistCDTime(TCDTimerID CDTime_ID, bool bMaster, ICDTimerEndHandler * pCDTimeEndHandler)
{
	if( bMaster && m_pActor->GetMaster() != 0){
		//如果是所有角色共用的冷却时间，要放到主角身上
		ICDTimerPart * pCDTimerPart = m_pActor->GetMaster()->GetCDTimerPart();
		if( 0 == pCDTimerPart){
			return;
		}

		pCDTimerPart->RegistCDTime(CDTime_ID);
		return;
	}

	//获取配置表，从而得到冷却时间多少
	UINT32 nTimeLong = g_pGameServer->GetConfigServer()->GetCDTimeCnfg(CDTime_ID);
	if( 0 == nTimeLong){
		return;
	}

	//创建定时器
	if( !this->__StartTimer(CDTime_ID, nTimeLong)){
		return;
	}

	SCDTimeData CDTimeInfo;
	CDTimeInfo.m_EndTime = nTimeLong + CURRENT_TIME();
	CDTimeInfo.m_pCDTimeEndHandler = pCDTimeEndHandler;

	m_mapCDTimer[CDTime_ID] = CDTimeInfo;
}

//////////////////////////////////////////////////////////////////////////
//检测指定的CD时间是否已到
//bMaster:是否要获取主角的冷却时间(有些冷却是所有角色共用的)
//CD时间已到返回true,CD时间未到返回false
//////////////////////////////////////////////////////////////////////////
bool CDTimerPart::IsCDTimeOK(TCDTimerID CDTime_ID, bool bMaster)
{
	if( !bMaster){
		//获取自己的冷却
		return this->__IsCDTimeOK(CDTime_ID);
	}
	
	if( m_pActor->GetMaster() == 0){
		//获取主角的冷却，但自己就是主角
		return this->__IsCDTimeOK(CDTime_ID);
	}

	//获取主角的冷却
	ICDTimerPart * pCDTimerPart = m_pActor->GetMaster()->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return false;
	}

	return pCDTimerPart->IsCDTimeOK(CDTime_ID);
}

//卸载指定的冷却时间
void CDTimerPart::UnLoadCDTime(TCDTimerID CDTime_ID)
{
	MAPCDTIMER::iterator iter = m_mapCDTimer.find(CDTime_ID);
	if( iter == m_mapCDTimer.end()){
		return;
	}

	//冷却ID也是定时器的ID
	this->__StopTimer(CDTime_ID);

	m_mapCDTimer.erase(iter);
}

//卸载所有冷却时间
void CDTimerPart::UnLoadAllCDTime()
{
	MAPCDTIMER::iterator iter = m_mapCDTimer.begin();

	for( ; iter != m_mapCDTimer.end(); ++iter)
	{
		this->__StopTimer(iter->first);
	}

	m_mapCDTimer.clear();
}

//得到CD剩余时间
UINT32 CDTimerPart::GetRemainCDTime(TCDTimerID CDTime_ID)
{
	MAPCDTIMER::iterator iter = m_mapCDTimer.find(CDTime_ID);

	if ( iter == m_mapCDTimer.end() )
		return 0;

	UINT32 CurTime = CURRENT_TIME();

	SCDTimeData & CDData = iter->second;

	if ( CDData.m_EndTime <= CurTime )
		return 0;

	return CDData.m_EndTime - CurTime;
}

//CD时间是否已到
bool	CDTimerPart::__IsCDTimeOK(TCDTimerID CDTime_ID)
{
	MAPCDTIMER::iterator iter = m_mapCDTimer.find(CDTime_ID);
	if( iter == m_mapCDTimer.end()){
		return true;
	}

	return false;
}

//创建定时器
bool	CDTimerPart::__StartTimer(UINT32 TimerID, UINT32 TimeLong)
{
	//小于2天才创建定时器，防止时间间隔过大变成负数
	if( TimeLong > XDateTime::SECOND_OF_DAY * 2){
		return true;
	}

	return g_pGameServer->GetTimeAxis()->SetTimer(TimerID,this,TimeLong * 1000,"CDTimerPart::StartTimer");
}

//销毁定时器
void	CDTimerPart::__StopTimer(UINT32 TimerID)
{
	g_pGameServer->GetTimeAxis()->KillTimer(TimerID, this);
}

//登记数据库获得的冷却
void	CDTimerPart::__RegistDBCDTime(const SDB_CDTimerData * pCDTimerData)
{
	UINT32 nCurTime = CURRENT_TIME();

	if( nCurTime >= pCDTimerData->m_EndTime){
		return;
	}

	//获取配置表，从而得到冷却时间多少
	UINT32 nTimeLong = pCDTimerData->m_EndTime - nCurTime;

	//创建定时器
	if( !this->__StartTimer(pCDTimerData->m_CDTimerID, nTimeLong)){
		return;
	}


	SCDTimeData CDTimeInfo;
	CDTimeInfo.m_EndTime = pCDTimerData->m_EndTime;
	CDTimeInfo.m_pCDTimeEndHandler = 0;

	m_mapCDTimer[pCDTimerData->m_CDTimerID] = CDTimeInfo;
}
