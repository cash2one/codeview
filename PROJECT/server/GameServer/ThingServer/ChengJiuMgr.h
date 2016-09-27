
#ifndef __THINGSERVER_CHENGJIUMGR_H__
#define __THINGSERVER_CHENGJIUMGR_H__


#include "IMessageDispatch.h"
#include "IDBProxyClient.h"
#include "EventChengJiu.h"
#include "IConfigServer.h"
#include <vector>

class ChengJiuMgr : public IMsgRootDispatchSink, public IDBProxyClientSink
{
public:
    ChengJiuMgr();

	~ChengJiuMgr();

	
	bool Create();

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata = 0);

private:
	IChengJiu * CreateChengJiu(const SChengJiuCnfg * pChengJiuCnfg);

		//打开成就栏
	void OpenChengJiuPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//更改称号
	void ChangeTitle(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到某角色有进度或者完成了的成就数据(成就对比有用到)
	void GetUserChengJiuData(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	void	HandleGetUserChengJiuData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

private:
	std::vector<IChengJiu*>  m_vectChengJiu;
};



#endif

