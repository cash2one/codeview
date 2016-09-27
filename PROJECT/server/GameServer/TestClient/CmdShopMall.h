#ifndef __TESTCLIENT_CMDSHOPMALL_H__
#define __TESTCLIENT_CMDSHOPMALL_H__

#include "ICmdBase.h"
#include <vector>
#include "DSystem.h"
#include "GameSrvProtocol.h"

class Client;

class CmdShopMall : public ICmdBase
{
public:
	CmdShopMall(Client * pClient);

	//发送命令给服务器
	virtual void SendCmd();

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib);

private:
	//商城打开标签可买的物品ID
	std::vector<TGoodsID>	m_vectGoodsID;

	//当前所在商城标签
	enShopMallLabel			n_InLabel;

	
};

#endif
