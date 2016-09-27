#ifndef __TESTCLIENT_CMDTALK_H__
#define __TESTCLIENT_CMDTALK_H__

#include "ICmdBase.h"

class Client;

class CmdTalk : public ICmdBase
{
public:
	CmdTalk(Client * pClient);

	//发送命令给服务器
	virtual void SendCmd();

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib);

private:

	bool	m_bFirst;
	
};

#endif
