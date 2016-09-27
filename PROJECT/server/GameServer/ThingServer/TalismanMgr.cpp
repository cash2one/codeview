
#include "TalismanMgr.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "ITalismanPart.h"
#include "XiuLianGame.h"
#include "IActor.h"
#include "QingLiGame.h"
#include "XunBaoGame.h"

TalismanMgr::TalismanMgr()
{
}

TalismanMgr::~TalismanMgr()
{
}

bool TalismanMgr::Create(IGameWorld * pGameWorld)
{
	if ( !m_TalismanWorldFuBen.Create(pGameWorld) )
		return false;

	if ( !m_TalismanWorldFuBen.OnSetDBData() )
		return false;

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_TalismanWorld,this);
}

void TalismanMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_TalismanWorld,this);
}

		//收到MSG_ROOT消息
 void TalismanMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
 {
	 	 typedef  void (TalismanMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enTalismanWorldCmd_Max]=
	 {
		/*enTalismanWorldCmd_Open = 0,			//打开法宝世界
		enTalismanWorldCmd_EnterTW,				//进入法宝世界
		enTalismanWorldCmd_Combat,				//战斗
		enTalismanWorldCmd_ChangePhoto,			//通知更改图片
		enTalismanWorldCmd_SC_PopWaitCombat,	//弹出等待战斗框
		enTalismanWorldCmd_CancelWait,			//取消等待
		enTalismanWorldCmd_CancelReplace,		//取消替换图片*/
		& TalismanMgr::OpenTalismanWorld,
		& TalismanMgr::EnterTalismanWorld,
		& TalismanMgr::TWCombat,
		NULL,
		NULL,
		& TalismanMgr::CancelWait,
		NULL,
		& TalismanMgr::GetTWInfo,
		NULL,
		& TalismanMgr::ViewRec,
		& TalismanMgr::GetPreOccupationInfo,
		//////////////下面的没用了
		& TalismanMgr::EnterTalismanWorld,	
		& TalismanMgr::LeaveTalismanWorld,
		& TalismanMgr::TalismanWorldGame,
	 };

	 if(nCmd>= ARRAY_SIZE(s_funcProc) || 0==s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
 }

////进入法宝世界
//void  TalismanMgr::EnterTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib)
//{
//	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();
//
//	CS_TalismanWorldEnter_Req Req;
//
//	ib >> Req;
//
//	if(ib.Error())
//	{
//		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
//		return ;
//	}
//
//	pTalismanPart->EnterTalismanWorld(Req);
//}

//离开法宝世界
void  TalismanMgr::LeaveTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	CS_TalismanWorldLeave_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTalismanPart->LeaveTalismanWorld(Req.m_GameID);
}


//法宝世界游戏
void  TalismanMgr::TalismanWorldGame(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	CSC_TalismanWorldGame Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTalismanPart->GameMessage(Req,ib);
 
}

bool TalismanMgr::AddGame(ITalismanGame* pTalismanGame)
{
	if(m_mapTalismanGame.insert(std::make_pair(pTalismanGame->GetGameID(),pTalismanGame)).second == false)
	{
		return false;
	}

	return true;
}


void TalismanMgr::RemoveGame( UINT32 GameID)
{
	MAP_TALISMAN_GAME::iterator it = m_mapTalismanGame.find(GameID);

	if(it == m_mapTalismanGame.end())
	{
		return;
	}

	ITalismanGame* pTalismanGame = (*it).second;

	m_mapTalismanGame.erase(it);

	pTalismanGame->Close();

	delete pTalismanGame;
}

//查找
ITalismanGame * TalismanMgr::GetTalismanGame(UINT32 GameID)
{
	MAP_TALISMAN_GAME::iterator it = m_mapTalismanGame.find(GameID);

	if(it == m_mapTalismanGame.end())
	{
		return 0;
	}

	ITalismanGame* pTalismanGame = (*it).second;

	return pTalismanGame;
}

//占领
void TalismanMgr::Occupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	if ( !pTalismanPart->Occupation(uidMonster, Level, floor) ){
		return;
	}

	if ( !m_TalismanWorldFuBen.Occupation(pActor, uidMonster, Level, floor) ){

		pTalismanPart->OverOccupation();
		return;
	}
}

//占领结束
void TalismanMgr::OverOccupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor, bool bBeaten)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	if ( !m_TalismanWorldFuBen.OverOccupation(pActor, uidMonster, Level, floor) )
		return;

	pTalismanPart->OverOccupation(bBeaten);
}

//取消法宝世界等待战斗
void TalismanMgr::Cancel_WaitCombat(UID uidUser)
{
	m_TalismanWorldFuBen.Cancel_WaitCombat(uidUser);
}

//关服后，玩家上线获取玩家的占领信息
void TalismanMgr::GetUserOccupation(UID uidUser, UINT8 Level, UINT8 Floor, UID & uidMonster, UINT32 & OccupatTime)
{
	m_TalismanWorldFuBen.GetUserOccupation(uidUser, Level, Floor, uidMonster, OccupatTime);
}

//打开法宝世界
void	TalismanMgr::OpenTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	pTalismanPart->OpenTalismanWorld();
}

//进入法宝世界
void	TalismanMgr::EnterTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_EnterTW Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_TalismanWorldFuBen.EnterTalismanWorld(pActor, Req.m_Floor);
}

//战斗
void	TalismanMgr::TWCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_TWCombat Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_TalismanWorldFuBen.Combat(pActor, Req.uidEnemy, Req.m_Level, Req.m_Floor);
}

//取消等待
void	TalismanMgr::CancelWait(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_TalismanWorldFuBen.Cancel_WaitCombat(pActor->GetUID());
}

//客户端请求法宝世界信息
void	TalismanMgr::GetTWInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_GetTWInfo Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	SC_GetTWInfo Rsp;

	Rsp.m_leftEnterNum = GameParam.m_MaxEnterTWNum - pTalismanPart->GetEnterTWNumToday();
	Rsp.m_VipTotalEnterNum = pActor->GetVipValue(enVipType_AddEnterTalismanNum);
	Rsp.m_VipLeftEnterNum = Rsp.m_VipTotalEnterNum - pTalismanPart->GetVipEnterTWNumToday();

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_GetTWInfo,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	//同步替换信息
	m_TalismanWorldFuBen.SynReplaceInfoMap(pActor, Req.Level, Req.Floor);
}

ITalismanGame * TalismanMgr::CreateTalismanGame(UINT8  WorldType,UID uidTalisman)
{
	ITalismanGame * pTalismanGame = 0;

	switch(WorldType)
	{
	case enTalismanWorldType_XiuLian:                          //修炼类
		pTalismanGame = new XiuLianGame(uidTalisman);
		break;

	case enTalismanWorldType_Clear:                             //清理类
		pTalismanGame = new QingLiGame(uidTalisman);
		break;

	case enTalismanWorldType_Gather:                            //采集
		pTalismanGame = new XiuLianGame(uidTalisman);
		break;

	case enTalismanWorldType_Answer:                             //答题
		pTalismanGame = new XiuLianGame(uidTalisman);
		break;

	case enTalismanWorldType_Treasure:                          //寻宝
		pTalismanGame = new XunBaoGame(uidTalisman);
		break;

	default:
		break;
	}

	return pTalismanGame;

}

//查看回放
void TalismanMgr::ViewRec(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewRec Req;

	ib >> Req;

	if(ib.Error()){
		TRACE("<error> %s : %d Line 斗法客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}
	m_TalismanWorldFuBen.ViewRec(pActor,Req.m_Rank);

}

//获得法宝世界占领时被谁击败
void	TalismanMgr::GetPreOccupationInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_TalismanWorldFuBen.GetPreOccupationInfo(pActor);
}
