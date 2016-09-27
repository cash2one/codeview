
#include "ChengJiuMgr.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IChengJiuPart.h"
#include "ChengJiuCmd.h"
#include "IActor.h"
#include "IBasicService.h"
#include "MagicLvSecChengJiu.h"
#include "ChengJiuCmd.h"
#include "ThingServer.h"

ChengJiuMgr::ChengJiuMgr()
{
}

ChengJiuMgr::~ChengJiuMgr()
{
}

	
bool ChengJiuMgr::Create()
{
	const std::hash_map<TChengJiuID,SChengJiuCnfg> & mapChengJiu =  g_pGameServer->GetConfigServer()->GetAllChengJiuCnfg();

	for(std::hash_map<TChengJiuID,SChengJiuCnfg>::const_iterator it = mapChengJiu.begin() ; it != mapChengJiu.end() ; ++it)
	{
		const SChengJiuCnfg & ChengJiuCnfg  = (*it).second; 

		IChengJiu * pChengJiu  = CreateChengJiu(&ChengJiuCnfg);

		if(pChengJiu)
		{
		   m_vectChengJiu.push_back(pChengJiu);
		}		
	}

	g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_ChengJiu,this);

	return true;
}

void ChengJiuMgr::Close()
{
	for(int i=0; i < m_vectChengJiu.size(); i++)
	{
		m_vectChengJiu[i]->Release();
	}

	m_vectChengJiu.clear();

	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_ChengJiu,this);
}

		//收到MSG_ROOT消息
void ChengJiuMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (ChengJiuMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enChengJiuCmd_Max] = 
	{

		&ChengJiuMgr::OpenChengJiuPanel,	
		NULL,
		&ChengJiuMgr::ChangeTitle,
		&ChengJiuMgr::GetUserChengJiuData,

	};
	if(nCmd >= ARRAY_SIZE(s_funcProc) || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

	// nRetCode: 取值于 enDBRetCode
void ChengJiuMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata/* = 0*/)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_GetChengJiuInfo:
			{
				HandleGetUserChengJiuData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}
}

		//打开成就栏
void ChengJiuMgr::OpenChengJiuPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
	if(pChengJiuPart == 0){
		return ;
	}

	pChengJiuPart->OpenChengJiuPanel();
}

	//更改称号
void ChengJiuMgr::ChangeTitle(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
	if(pChengJiuPart == 0){
		return ;
	}

	CS_ChengJiuChangeTitle_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pChengJiuPart->ChangeTitle(Req.m_TitleID);

}

//得到某角色有进度或者完成了的成就数据
void ChengJiuMgr::GetUserChengJiuData(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_GetUserChengJiu Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行　客户端数据包长度有误 len = %d !", __FUNCTION__, __LINE__, ib.Capacity());
		return;
	}

	IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(UID(Req.m_uidUser));
	if( 0 == pUser){
		//玩家不在线上,数据从数据库中查找
		SDB_Get_ChengJiuData_Req ReqChengJiu;
		ReqChengJiu.m_uidUser = Req.m_uidUser.ToUint64();

		OBuffer1k ob;
		ob << ReqChengJiu;
		g_pGameServer->GetDBProxyClient()->Request(Req.m_uidUser.ToUint64(),enDBCmd_GetChengJiuInfo,ob.TakeOsb(),this, pActor->GetUID().ToUint64());
	}else{
		//在线上，发送自己的成就数据给别的玩家
		IChengJiuPart * pChengJiuPart = pUser->GetChengJiuPart();
		if(pChengJiuPart == 0){
			return ;
		}

		pChengJiuPart->SendMeChengJiuToOtherUser(pActor);
	}
}


IChengJiu * ChengJiuMgr::CreateChengJiu(const SChengJiuCnfg * pChengJiuCnfg)
{
	IChengJiu * pChengJiu = 0;

	switch(pChengJiuCnfg->m_Type)
	{
	case enChengJiuType_Event:
		{
			pChengJiu = new EventChengJiu();
		}
		break;
	case enChengJiuType_MagicLvSec:
		{
			pChengJiu = new MagicLvSecChengJiu();
		};
		break;
	default:
		return 0;
	}

	if(pChengJiu == 0)
	{
		return 0;
	}

	if(pChengJiu->Create(pChengJiuCnfg)==false)
	{
		delete pChengJiu;
		return 0;
	}

	return pChengJiu;
}

void	ChengJiuMgr::HandleGetUserChengJiuData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());
	DBRspPacketHeader RspHeader;
	DB_ChengJiuPart	  DBChengJiuPart;
	RspIb >> RspHeader >> DBChengJiuPart;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_GetUserChengJiu Rsp;

	Rsp.m_ActiveTitleID = DBChengJiuPart.m_ActiveTitleID;

	INT32 nNum = RspIb.Remain() / sizeof(DB_ChengJiu);

	OBuffer4k ob;

	for( int i = 0; i < nNum; ++i)
	{
		DB_ChengJiu DBData;
		RspIb >> DBData;
		if( RspIb.Error()){
			TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		if( DBData.m_FinishTime > 0){

			UserChengJiuData ChengJiuData;

			ChengJiuData.m_ChengJiuID = DBData.m_ChengJiuID;
			ChengJiuData.m_FinishTime = DBData.m_FinishTime;

			ob << ChengJiuData;

			++Rsp.m_Num;
		}
	}

	OBuffer4k ob2;
	ob2 << ChengJiuHeader(enChengJiuCmd_Get_UserChengJiu, sizeof(Rsp) + ob.Size()) << Rsp;

	if( ob.Size() > 0){
		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}
