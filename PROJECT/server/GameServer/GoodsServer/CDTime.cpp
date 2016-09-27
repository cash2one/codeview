
#include "IActor.h"
#include "CDTime.h"

#include "time.h"
#include "GoodsServer.h"
#include "IConfigServer.h"

CDTimeMgr::CDTimeMgr()
{
}

CDTimeMgr::~CDTimeMgr()
{

}

bool	CDTimeMgr::Create()
{
	g_pGameServer->GetTimeAxis()->SetTimer(enCDTimeTimerID, this, 1000,"CDTime::Create");

	return true;
}

void	CDTimeMgr::Close()
{
	g_pGameServer->GetTimeAxis()->KillTimer(enCDTimeTimerID, this);
}
//////////////////////////////////////////////////////////////////////////
//登记冷却的信息(会检测配置表中是否有，没有则不登记)
//type:配置表中的类型(比如，聊天为1，物品使用为2)
//flag:配置表中的标识(比如，聊天为消息类别，物品使用为物品ID)
//func:CD时间到时的回调函数
//////////////////////////////////////////////////////////////////////////
void	CDTimeMgr::RegistCDTime(IActor * pActor, TCDTimerID CDTime_ID, ICDTimeEndHandler * pCDTimeEndHandler)
{
	//获取配置表，从而得到冷却时间多少
	UINT32 nTimeLong = g_pGameServer->GetConfigServer()->GetCDTimeCnfg(CDTime_ID);
	if( 0 == nTimeLong){
		return;
	}

	SCDTimeInfo CDTimeInfo;
	CDTimeInfo.m_EndTime			= (UINT32)time(0) + nTimeLong;
	CDTimeInfo.m_pCDTimeEndHandler  = pCDTimeEndHandler;

	std::map<UID, MAPCDTIME>::iterator iter = m_mapCDTime.find(pActor->GetUID());
	if( iter == m_mapCDTime.end()){
		MAPCDTIME map_CDTime;
		map_CDTime[CDTime_ID] = CDTimeInfo;
		m_mapCDTime[pActor->GetUID()] = map_CDTime;
	}else{
		MAPCDTIME & map_CDTime = iter->second;
		map_CDTime[CDTime_ID] = CDTimeInfo;
	}
}

//////////////////////////////////////////////////////////////////////////
//检测指定的CD时间是否已到
//type:配置表中的类型(比如，聊天为1，物品使用为2)
//flag:配置表中的标识(比如，聊天为消息类别，物品使用为物品ID)
//func:CD时间到时的回调函数
//CD时间已到返回true,CD时间未到返回false
//////////////////////////////////////////////////////////////////////////
bool	CDTimeMgr::IsCDTimeOK(IActor * pActor, TCDTimerID CDTime_ID)
{
	std::map<UID, MAPCDTIME>::iterator iter = m_mapCDTime.find(pActor->GetUID());
	if( iter == m_mapCDTime.end()){
		return true;
	}
	MAPCDTIME & map_CDTime = iter->second;

	MAPCDTIME::iterator it = map_CDTime.find(CDTime_ID);
	if( it == map_CDTime.end()){
		return true;
	}

	return false;
}

//卸载这个玩家身上的所有冷却时间
void	CDTimeMgr::UnLoadCDTime(IActor * pActor)
{
	std::map<UID, MAPCDTIME>::iterator iter = m_mapCDTime.find(pActor->GetUID());
	if( iter == m_mapCDTime.end()){
		return;
	}

	m_mapCDTime.erase(iter);
}

//卸载这个玩家身上指定的冷却时间
void	CDTimeMgr::UnLoadCDTime(IActor * pActor, TCDTimerID CDTime_ID)
{
	std::map<UID, MAPCDTIME>::iterator iter = m_mapCDTime.find(pActor->GetUID());
	if( iter == m_mapCDTime.end()){
		return;
	}

	MAPCDTIME & map_CDTime = iter->second;

	MAPCDTIME::iterator it = map_CDTime.find(CDTime_ID);                                                                      
	if( it == map_CDTime.end()){
		return;
	}

	map_CDTime.erase(it);
}

void	CDTimeMgr::OnTimer(UINT32 timerID)
{
	UINT32 NowTime = (UINT32)time(0);

	if( enCDTimeTimerID == timerID){
		//遍历，看看有没有时间到的，并调用回调函数，并从集合中删除
		for( std::map<UID, MAPCDTIME>::iterator iter = m_mapCDTime.begin(); iter != m_mapCDTime.end(); ++iter)
		{
			MAPCDTIME & map_CDTime = iter->second;

			for( MAPCDTIME::iterator it = map_CDTime.begin(); it != map_CDTime.end();)
			{
				SCDTimeInfo & CDTimeInfo = it->second;
				if( CDTimeInfo.m_EndTime < NowTime){
					//调用回调函数
					if( 0 != CDTimeInfo.m_pCDTimeEndHandler){
						CDTimeInfo.m_pCDTimeEndHandler->CDTimeEndBackFunc(iter->first, it->first);
					}
					//从集合中删除
					map_CDTime.erase(it++);
					continue;
				}

				++it;
			}
		}
	}
}
