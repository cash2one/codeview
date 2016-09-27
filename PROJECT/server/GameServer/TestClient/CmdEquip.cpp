#include "CmdEquip.h"
#include "RandomService.h"
#include "GameSrvProtocol.h"
#include "Client.h"


CmdEquip::CmdEquip(Client * pClient) : ICmdBase(pClient)
{
	
}

//发送命令给服务器
void CmdEquip::SendCmd()
{
	int nCmd =  RandomService::GetRandom() % 1;

	OBuffer1k ob;

	switch(nCmd)
	{
	case 0:
		{
			//打开法术栏
			ob << Equip_Header(enEquipCmd_OpenMagic,0);

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送打开法术栏消息!!");
			}
		}
	case 1:
		{
			//境界升级
			CS_AddLayerLevel_Req Req;
			Req.uid_Actor = m_pClient->GetUserUID().ToUint64();
		
			ob << Equip_Header(enEquipCmd_AddLayerLevel, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送境界升级消息!!");
			}
		}
		break;
	}
}

//数据到达,
void CmdEquip::OnNetDataRecv(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enEquipCmd_OpenMagic:
		{
			SC_EquipOpenMagic_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 打开法术栏消息读取错误", __FUNCTION__, __LINE__);
			}
		}
		break;
	case enEquipCmd_AddLayerLevel:
		{
			SC_AddLayerLevel_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 境界升级消息读取错误", __FUNCTION__, __LINE__);
			}
		}
		break;
	}
}
