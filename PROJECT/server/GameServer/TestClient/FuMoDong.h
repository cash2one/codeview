#ifndef __TESTCLIENT_CMDFUMODONG_H__
#define __TESTCLIENT_CMDFUMODONG_H__

#include "ICmdBase.h"
#include <set>
#include "UniqueIDGenerator.h"

class Client;

//背包消息
class CmdFuMoDong : public ICmdBase
{
public:
	CmdFuMoDong(Client * pClient);

	//发送命令给服务器
	virtual void SendCmd();

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib);

private:
	bool m_bCombat; //是否已发送了挑战消息

	
};

#endif
