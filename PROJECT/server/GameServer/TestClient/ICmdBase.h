#ifndef __TESTCLIENT_ICMDBASE_H__
#define __TESTCLIENT_ICMDBASE_H__

#include "BclHeader.h"
#include "TBuffer.h"
#include "GameSrvProtocol.h"
#include "Client.h"
struct ICmdBase
{
	ICmdBase(Client * pClient)
	{
		m_pClient = pClient;
	}

	//发送命令给服务器
	virtual void SendCmd() = 0;

	//数据到达,
	virtual void OnNetDataRecv(UINT8 nCmd, IBuffer & ib) = 0;

	virtual ~ICmdBase(){};

	//发送聊天消息，控制台命令
	virtual bool SendMsg(const char * szMsg)
	{
		//世界聊天
			CS_WorldTalk_Req Req;
			sprintf_s(Req.m_TalkContent,sizeof(Req.m_TalkContent), "%s",szMsg );

			OBuffer1k ob;

			ob << TalkHeader(enTalkCmd_WorldTalk, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())==false)
			{
				return false;				
			}

			return true;
	}

protected:
	Client *		m_pClient;
};

#endif
