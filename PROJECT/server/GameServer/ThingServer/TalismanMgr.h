
#ifndef __THINGSERVER_TALISMANMGR_H__
#define __THINGSERVER_TALISMANMGR_H__

#include "IMessageDispatch.h"
#include "IGameWorld.h"
#include <hash_map>
#include "TalismanWorldFuBen.h"

struct ITalismanGame;

class TalismanMgr : public IMsgRootDispatchSink//,public IDBProxyClientSink
{
public:
	TalismanMgr();

	~TalismanMgr();

	bool Create(IGameWorld * pGameWorld);

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	bool AddGame(ITalismanGame*);

	void RemoveGame(UINT32 GameID);

	ITalismanGame * CreateTalismanGame(UINT8  WorldType,UID uidTalisman);

	//查找
	ITalismanGame * GetTalismanGame(UINT32 GameID);

public:
	//占领
	virtual void Occupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor);

	//占领结束
	virtual void OverOccupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor, bool bBeaten = false);

	//取消法宝世界等待战斗
	virtual void Cancel_WaitCombat(UID uidUser);

	//关服后，玩家上线获取玩家的占领信息
	void GetUserOccupation(UID uidUser, UINT8 Level, UINT8 Floor, UID & uidMonster, UINT32 & OccupatTime);

private:

	//打开法宝世界
	void	OpenTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//进入法宝世界
	void	EnterTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//战斗
	void	TWCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//取消等待
	void	CancelWait(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//客户端请求法宝世界信息
	void	GetTWInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看回放
	void	ViewRec(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//获得法宝世界占领时被谁击败
	void	GetPreOccupationInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	/*******************下面函数现在没用了*********************************************/
		//进入法宝世界
	//void  TalismanMgr::EnterTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//离开法宝世界
	void  TalismanMgr::LeaveTalismanWorld(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//法宝世界游戏
	void  TalismanMgr::TalismanWorldGame(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	typedef std::hash_map<UINT32,ITalismanGame*>  MAP_TALISMAN_GAME;
    MAP_TALISMAN_GAME   m_mapTalismanGame;

	TalismanWorldFuBen	m_TalismanWorldFuBen;
};









#endif
