#include "FuMoDong.h"
#include "GameSrvProtocol.h"
#include "Client.h"
#include "RandomService.h"
#include "BuildingCmd.h"


CmdFuMoDong::CmdFuMoDong(Client* pClient): ICmdBase(pClient)
{
    //先升个级啊
	int Level = RandomService::GetRandom() % 60 +1;

	Level = 20;

	char szTemp[100] = {0};

	sprintf_s(szTemp,sizeof(szTemp),"/lv,%d",Level);

	if(SendMsg(szTemp))
	{
		TRACE("成功发送升级[%d]消息!!",Level);
	}

	m_bCombat = false;
	
}

//发送命令给服务器
void CmdFuMoDong::SendCmd()
{
	if(m_bCombat)
	{
		return;
	}

	//伏魔洞层次
	int Level = RandomService::GetRandom() % 6;

	CS_KillMonster_Req Req;
	Req.m_Level = Level;

	OBuffer4k ob;
	ob << BuildingHeader(enBuildingCmd_KillMonster,sizeof(Req)) << Req;

	if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送挑战伏魔洞第[%d]层消息!!",Level);
				m_bCombat = true;
			}

	
}

//数据到达,
void CmdFuMoDong::OnNetDataRecv(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{	
	case enBuildingCmd_KillMonster:
		{
			SC_KillMonster_Rsp Rsp;
			ib >> Rsp;

			if( ib.Error()){
				TRACE("<ib.Error()> %s:%d 挑战伏魔洞消息读取错误", __FUNCTION__, __LINE__);
			}

			if(Rsp.m_Result == enKMRetCode_OK)
			{
				TRACE("挑战伏魔洞成功!");
			}
			else
			{
				TRACE("挑战伏魔洞成功!");
			}

			m_bCombat = false;

			//再次进入伏魔洞，让服务器删除战斗场景
			OBuffer4k ob;
	       ob << BuildingHeader(enBuildingCmd_EnterFuMoDong,0);

	       if( m_pClient->SendData(ob.TakeOsb())){
				TRACE("成功发送进入伏魔洞消息!!");
			}

			
		}
		break;
	}
}
