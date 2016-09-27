#ifndef __TESTCLIENT_CMDPACKET_H__
#define __TESTCLIENT_CMDPACKET_H__

#include "ICmdBase.h"
#include <set>
#include "UniqueIDGenerator.h"

class Client;

//背包消息
class CmdPacket : public ICmdBase
{
public:
	CmdPacket(Client * pClient);

	//发送命令给服务器
	virtual void SendCmd();

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib);

private:
	//玩家背包里的物品UID
	std::set<UID>	m_setGoodsUID;

};

#endif
