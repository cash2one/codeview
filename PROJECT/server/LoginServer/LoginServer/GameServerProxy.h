
#ifndef __LOGINSERVER_GAMESERVER_PROXY_H__
#define __LOGINSERVER_GAMESERVER_PROXY_H__

#include "ISocketSystem.h"
#include "DSystem.h"

class GameServerProxy : public ISocketSink
{
public:
	GameServerProxy(TServerID ServerID,ISocketSystem * pSocketSystem);

	~GameServerProxy();

public:
	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

private:
	void HandleReportStateReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

private:
	TServerID m_ServerID;
	ISocketSystem * m_pSocketSystem;
};




#endif
