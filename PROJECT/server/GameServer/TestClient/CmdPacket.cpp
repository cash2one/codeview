#include "CmdPacket.h"
#include "GameSrvProtocol.h"
#include "Client.h"
#include "RandomService.h"


CmdPacket::CmdPacket(Client* pClient): ICmdBase(pClient)
{
	
}

//发送命令给服务器
void CmdPacket::SendCmd()
{
	//只用到使用物品和丢弃物品
	int nCmd = RandomService::GetRandom() % 2;

	OBuffer1k ob;

	switch(nCmd)
	{
	case 0:
		{
			if( m_setGoodsUID.size() == 0){
				return;
			}

			//使用物品
			CS_PacketUseGoods_Req Req;

			//随机选择物品
			std::set<UID>::iterator iter = m_setGoodsUID.begin();

			int nRand = RandomService::GetRandom() % m_setGoodsUID.size();

			for( int i = 0; i < nRand; ++i){
				++iter;
			}

			Req.m_uidUseGoods = *iter;
			Req.m_uidTarget	  = m_pClient->GetUserUID();
			Req.m_UseNum	  = 1;

			ob << Packet_Header(enPacketCmd_UseGoods, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送使用物品消息!!");
			}
		}
		break;
	case 1:
		{
			if( m_setGoodsUID.size() == 0){
				return;
			}

			//销毁物品
			CS_PacketDiscardGoods_Req Req;

			//随机选择物品
			std::set<UID>::iterator iter = m_setGoodsUID.begin();

			int nRand = RandomService::GetRandom() % m_setGoodsUID.size();

			for( int i = 0; i < nRand; ++i){
				++iter;
			}

			if( iter == m_setGoodsUID.end()){
				return;
			}

			Req.m_uidGoods = *iter;

			ob << Packet_Header(enPacketCmd_DiscardGoods, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送销毁物品消息!!");
			}
		}
		break;
	}
}

//数据到达,
void CmdPacket::OnNetDataRecv(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enPacketCmd_Sync:
		{
			SC_PacketData_Sync Rsp;
			ib >> Rsp;
			
			if( ib.Error()){
				break;
			}
			
			for(int i = 0; i < Rsp.m_GoodsNum; ++i)
			{
				UID uidGoods;
				ib >> uidGoods;
				m_setGoodsUID.insert(uidGoods);
			}
		}
		break;
	case enPacketCmd_DiscardGoods:
		{
			SC_PacketDiscardGoods_Rsp Rsp;
			ib >> Rsp;
			
			if( ib.Error()){
				break;
			}

			if( Rsp.m_Result != enPacketRetCode_OK){
				TRACE("物品丢弃不成功,返回码:%d", Rsp.m_Result);
			}
		}
		break;
	case enPacketCmd_UseGoods:
		{
			SC_PacketUseGoods_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 使用物品消息读取错误", __FUNCTION__, __LINE__);
			}
		}
		break;
	}
}
