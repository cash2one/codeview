#include "ThingServer.h"
#include "TargetTimeToLevel.h"
#include "IBasicService.h"
#include "XDateTime.h"

static 	const int SECOND_30_DAY = 30 * XDateTime::SECOND_OF_DAY;

bool TargetTimeToLevel::Create(const  SActivityCnfg * pActivityCnfg, bool bRegistEvent)
{
	if( !EventActivity::Create(pActivityCnfg, false)){
		return false;
	}

	const SConfigParam & Param = g_pGameServer->GetConfigParam();

	//先获取服务器开启时间,即第一次开启服务器的时间
	SDB_Get_FirstOpenServerTimeReq Req;

	Req.m_ServerID = Param.m_ServerID;
	Req.m_PlatformID = Param.m_GamePlatform;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(Req.m_ServerID, enDBCmd_Get_FirstOpenServerTime, ob.TakeOsb(), this, 0);

	return true;
}

void TargetTimeToLevel::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK){
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	case enDBCmd_Get_FirstOpenServerTime:
		{
			this->HandleGetFirstOpenServerTime(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	}
}

void TargetTimeToLevel::OnEvent(XEventData & EventData)
{
	const SServer_Info & ServerInfo = g_pGameServer->GetServerInfo();

	UINT32 nCurTime = CURRENT_TIME();

	if( (nCurTime - ServerInfo.m_OpenServiceTime) / XDateTime::SECOND_OF_DAY >= SECOND_30_DAY){
		return;
	}

	EventActivity::OnEvent(EventData);
}

void TargetTimeToLevel::HandleGetFirstOpenServerTime(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());
	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	FirstOpenServerTime DBTime;
	RspIb >> RspHeader >> OutParam >> DBTime;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		return;
	}

	UINT32 nCurTime = CURRENT_TIME();

	if( (nCurTime - DBTime.m_time) / XDateTime::SECOND_OF_DAY >= SECOND_30_DAY){
		return;
	}

	//注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pActivityCnfg->m_EventID);

	g_pGameServer->GetEventServer()->AddListener(this,msgID,enEventSrcType_Actor,0,"TargetTimeToLevel::HandleGetFirstOpenServerTime");
}
