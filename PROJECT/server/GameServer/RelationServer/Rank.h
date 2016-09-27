#ifndef __RELATIONSERVER_RANK_H__
#define __RELATIONSERVER_RANK_H__

#include "IMessageDispatch.h"
#include <vector>
#include "ITimeAxis.h"
#include "IDBProxyClient.h"
#include "RankCmd.h"
#include "IEventServer.h"
#include "ISyndicateMgr.h"

class Rank : public IMsgRootDispatchSink,public ITimerSink,public IDBProxyClientSink, public IEventListener
{
	enum enTimer VC_PACKED_ONE
	{
		enTimer_Flush = 0,	//刷新的定时器ID
	}PACKED_ONE;

public:
	Rank();
	~Rank();

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	virtual void OnTimer(UINT32 timerID);

		// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual void	OnEvent(XEventData & EventData);

public:

	//更新帮派排行榜
	void FlushSynRank();


	//更新帮战排行榜
	void FlushSynWarRank();

private:
	//打开排行榜
	void OpenRank(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//打开玩家排行
	void OpenUserRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum);

	//打开夺宝排行
	void OpenDuoBaoRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum);

	//打开帮派排行
	void OpenSynRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum);

	//打开帮战排行
	void OpenSynWarRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum);

	//打开挑战排行榜
	void OpenChallengeRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum);

	void HandleLoadUserRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//更新玩家排行榜
	void FlushUserRank();

	//加载夺宝排行
	void HandleLoadDuoBaoRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//更新夺宝排行榜
	void FlushDuoBaoRank();

	//点击我的排行
	void ClickMyRand(IActor * pActor, INT16 nNum, enRank_Type Rank_Type);

	void HandleClickMyRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleClickMyDuoBaoRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//点击玩家，弹出玩家帮派信息
	void ClickUser(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	void HandleOpenChallengRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//打开我的帮派排行榜
	void OpenMySynRank(IActor * pActor, INT16 nNum);

	void HandleMyChallengeRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//仅得到我的排行
	void HandleOnlyGetMyRankChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//打开我的帮战排行榜
	void OpenMySynWarRank(IActor * pActor, INT16 nNum);

	//更新排行数据
	void UpdateRankData();


private:
	//帮派排行榜
	std::vector<SynRankSynInfo>			m_vectSynRank;

	//帮战排行榜
	std::vector<SynRankSynWarInfo>			m_vectSynWarRank;

	//是否初始化过帮派排行榜
	bool								m_bInitSynRank;

	//是否初始化过帮战排行榜
	bool								m_bInitSynWarRank;

};


#endif
