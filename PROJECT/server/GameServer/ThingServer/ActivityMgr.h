
#ifndef __THINGSERVER_ACTIVITYMGR_H__
#define __THINGSERVER_ACTIVITYMGR_H__
#include "IMessageDispatch.h"
#include <vector>
#include "IDBProxyClient.h"
#include "IConfigServer.h"

struct IActivity;


enum enActivityType
{
	enActivityType_Event = 0,			//普通事件
	enActivityType_TargetTimeToLevel,	//30天内达40级
};


class ActivityMgr : public IMsgRootDispatchSink, public IDBProxyClientSink
{
public:
	ActivityMgr();
	virtual ~ActivityMgr();

public:
		
	bool Create();

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//删除活动
	void	DeleteActivity(UINT16	ActivityID);

	//添加活动
	void	AddActivity(UINT16	ActivityID);

	//修改活动
	void	ChangeActivity(UINT16 ActivityID);

private:
		void ActivityMgr::OpenDaily(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::OpenSignIn(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::SignIn(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::SignInAward(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::OpenActivity(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::ActivityAward(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::TakeOnlineAward(IActor *pActor,UINT8 nCmd, IBuffer & ib);
		void ActivityMgr::ChangeTicket(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		void ActivityMgr::OnlineAwardNotic(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		void HandleGetActivityCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
				OBuffer4k & ReqOb,UINT64 userdata);

		void HandleChangeTicket(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
				OBuffer4k & ReqOb,UINT64 userdata);

		bool CreateActivity(const SActivityCnfg & ActivityCnfg);

		//得到参数
		void ActivityMgr::GetActivityParam(char * pszParam, std::vector<INT32> & vectParam);

		void HandleGetOneActivityCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
				OBuffer4k & ReqOb,UINT64 userdata);

private:
	std::vector<IActivity*>  m_vectActivity;
};




#endif
