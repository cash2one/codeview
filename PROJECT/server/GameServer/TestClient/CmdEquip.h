#ifndef __TESTCLIENT_CMDEQUIP_H__
#define __TESTCLIENT_CMDEQUIP_H__

#include "ICmdBase.h"

class Client;

class CmdEquip : public ICmdBase
{
public:
	CmdEquip(Client * pClient);

	//发送命令给服务器
	virtual void SendCmd();

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib);

private:
	
};

#endif
