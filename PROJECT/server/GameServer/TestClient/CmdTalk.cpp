#include "CmdTalk.h"
#include "Client.h"
#include "RandomService.h"
#include "GameSrvProtocol.h"

CmdTalk::CmdTalk(Client * pClient): ICmdBase(pClient)
{
	m_bFirst = true;
}

//发送命令给服务器
void CmdTalk::SendCmd()
{
	//只发送 世界聊天和帮派聊天
	int nCmd = RandomService::GetRandom() % 2;

	static UINT32 last_send_time = 0;

	if(::time(0)-last_send_time < 40)
	{
		return;
	}

	OBuffer1k ob;

	if ( m_bFirst )
	{
		//第一次，用来调命令
		CS_WorldTalk_Req Req;
		sprintf_s(Req.m_TalkContent,sizeof(Req.m_TalkContent), "/lv,60",m_pClient->GetUserUID().ToString() );

		ob << TalkHeader(enTalkCmd_WorldTalk, sizeof(Req)) << Req;

		m_pClient->SendData(ob.TakeOsb());

		ob.Reset();
		m_bFirst = false;
		return;
	}

	switch(nCmd)
	{
	case 0:
		{
			//世界聊天
			CS_WorldTalk_Req Req;
			sprintf_s(Req.m_TalkContent,sizeof(Req.m_TalkContent), "我是测试客户端[%s]",m_pClient->GetUserUID().ToString() );

			ob << TalkHeader(enTalkCmd_WorldTalk, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送世界聊天!!");
			}
		}
		break;
	case 1:
		{
			//帮派聊天
			CS_SynTalk_Req Req;
			strncpy(Req.m_TalkContent, "我是测试客户端", sizeof(Req.m_TalkContent));

			ob << TalkHeader(enTalkCmd_SynTalkRet, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送帮派聊天!!");
			}
		}
		break;
	}

	last_send_time = ::time(0);
}

//数据到达,
void CmdTalk::OnNetDataRecv(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enTalkCmd_WorldTalk:
		{
			SC_WorldTalkRet_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 世界聊天消息读取错误", __FUNCTION__, __LINE__);
			}
		}
		break;
	case enTalkCmd_SynTalkRet:
		{
			SC_SynTalkRetCode_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 帮派聊天消息读取错误", __FUNCTION__, __LINE__);
			}
		}
		break;
	}
}
