#include "CmdShopMall.h"
#include "Client.h"
#include "RandomService.h"


CmdShopMall::CmdShopMall(Client * pClient): ICmdBase(pClient)
{
	
}

//发送命令给服务器
void CmdShopMall::SendCmd()
{
	//总共有3条命令,从这3条中抽取
	int nCmd = RandomService::GetRandom() % 3;

	OBuffer1k ob;

	switch(nCmd)
	{
	case enShopMallCmd_Enter:
		{
			ob << ShopMallHeader(enShopMallCmd_Enter, 0);

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送进入商城消息!!");
			}

			n_InLabel = enShopMall_Hot;
		}
		break;
	case enShopMallCmd_ChangeLabel:
		{
			CS_ChangeLaber_Req Req;
			Req.m_nLaber = (enShopMallLabel)(RandomService::GetRandom() % (enShopMall_Max - 1));
			ob << ShopMallHeader(enShopMallCmd_ChangeLabel, sizeof(Req)) << Req;

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送改变商城标签消息!!");
			}

			n_InLabel = Req.m_nLaber;
		}
		break;
	case enShopMallCmd_Buy:
		{
			if( m_vectGoodsID.size() > 0){
				CS_BuyItemShop_Req Req;
				Req.m_GoodsNum = 1;
				Req.m_ID  = 1;
				Req.m_Label	   = (enShopMallLabel)0;

				ob << ShopMallHeader(enShopMallCmd_Buy, sizeof(Req)) << Req;
			}else{
				ob << ShopMallHeader(enShopMallCmd_Enter, 0);

				n_InLabel = enShopMall_Hot;
			}

			if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送买商城物品消息!!");
			}
		}
		break;
	}

}

//数据到达,
void CmdShopMall::OnNetDataRecv(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enShopMallCmd_Enter:
	case enShopMallCmd_ChangeLabel:
		{
			SC_ShopMallData_Rsp RspNum;
			ib >> RspNum;
			
			if( ib.Error()){
				TRACE("<error> %s:%d商城消息读取错误,错误命令ID:%d", __FUNCTION__, __LINE__, nCmd);
				return;
			}

			m_vectGoodsID.clear();

			for( int i = 0; i < RspNum.m_nGoodsNum; ++i)
			{
				m_vectGoodsID.clear();

				SShopMallData GoodsID;
				ib >> GoodsID;

				m_vectGoodsID.push_back(GoodsID.m_idGoods);
			}
		}
		break;
	case enShopMallCmd_Buy:
		{
			//随机从集合中获取一个物品ID买
			SC_BuyShopMall_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<error> %s:%d商城买物品消息读取错误", __FUNCTION__, __LINE__);
				return;
			}
		}
		break;
	}
}
