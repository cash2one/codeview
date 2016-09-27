
#include "IActor.h"
#include "XiuLianPart.h"

#include "GameSrvProtocol.h"
#include "XDateTime.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IGameWorld.h"
#include "IMagicPart.h"
#include "IPacketPart.h"
#include "IConfigServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IFriendPart.h"
#include "XDateTime.h"
#include "DMsgSubAction.h"
#include "ISystemMsg.h"

XiuLianPart::XiuLianPart()
{
	m_pActor = 0;
	m_bAloneTimerInCreateUser = false;
	m_bTwoTimerInCreateUser = false;
}

XiuLianPart::~XiuLianPart()
{
}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool XiuLianPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster==0 || pContext==0 || nLen<sizeof(SDB_Get_XiuLianData_Rsp))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	SDB_Get_XiuLianData_Rsp * pDBXiuLianData = (SDB_Get_XiuLianData_Rsp *)pContext;

	m_XiuLianData.m_AloneXiuLianState   = (enXiuLianState)pDBXiuLianData->m_AloneXiuLianState;
	m_XiuLianData.m_EndTime				= pDBXiuLianData->m_AloneEndTime;
	m_XiuLianData.m_Hours				= pDBXiuLianData->m_AloneHours;
	m_XiuLianData.m_lastGetNimbusTime	= pDBXiuLianData->m_AloneLastGetNimbusTime;
	m_XiuLianData.m_MagicXLSeq			= pDBXiuLianData->m_MagicXLSeq;
	m_XiuLianData.m_TwoEndTime			= pDBXiuLianData->m_TwoXLEndTime;
	m_XiuLianData.m_TwoLastGetNimbusTime= pDBXiuLianData->m_TwoXLLastGetNimbusTime;
	m_XiuLianData.m_TwoTotalNimbus		= pDBXiuLianData->m_TwoXLTotalNimbus;
	m_XiuLianData.m_TwoXiuLianHours		= pDBXiuLianData->m_TwoXLHours;
	m_XiuLianData.m_TwoXiuLianState		= (enXiuLianState)pDBXiuLianData->m_TwoXiuLianState;
	m_XiuLianData.m_uidActor			= UID(pDBXiuLianData->m_TwoXLUidActor);
	m_XiuLianData.m_uidFriend			= UID(pDBXiuLianData->m_TwoXLUidFriend);
	m_XiuLianData.m_FriendFacade		= pDBXiuLianData->m_FriendFacade;
	m_XiuLianData.m_GetGodSwordNimbus   = pDBXiuLianData->m_GetGodSwordNimbus;
	strncpy(m_XiuLianData.m_FriendName, (char *)pDBXiuLianData->m_FriendName, sizeof(m_XiuLianData.m_FriendName));

	m_XiuLianData.m_AloneXLData[0].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor);
	m_XiuLianData.m_AloneXLData[0].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus;

	m_XiuLianData.m_AloneXLData[1].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor2);
	m_XiuLianData.m_AloneXLData[1].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus2;

	m_XiuLianData.m_AloneXLData[2].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor3);
	m_XiuLianData.m_AloneXLData[2].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus3;

	m_XiuLianData.m_AloneXLData[3].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor4);
	m_XiuLianData.m_AloneXLData[3].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus4;

	m_XiuLianData.m_AloneXLData[4].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor5);
	m_XiuLianData.m_AloneXLData[4].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus5;

	m_XiuLianData.m_AloneXLData[5].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor6);
	m_XiuLianData.m_AloneXLData[5].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus6;

	m_XiuLianData.m_AloneXLData[6].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor7);
	m_XiuLianData.m_AloneXLData[6].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus7;

	m_XiuLianData.m_AloneXLData[7].m_ActorUID  = UID(pDBXiuLianData->m_AloneXLActor8);
	m_XiuLianData.m_AloneXLData[7].m_GetNimbus = pDBXiuLianData->m_AloneXLGetNimbus8;

	// 做些处理
	//nLen -= sizeof(SDB_Get_XiuLianData_Rsp);

	//INT32 XiuLianRecordNum = nLen/sizeof(SDB_Get_XiuLianData_Record);

	////法术修
	//STwoXiuLianData * pMagicXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	//if(pMagicXiuLianData==0)
	//{
	//	m_XiuLianData.m_MagicXLSeq = 0;
	//}
	//else if(pMagicXiuLianData->m_XiuLianState != enXiuLianState_Ask )
	//{
	//	if( pMagicXiuLianData->m_bStudyMagic)
	//	{
	//		if(pMagicXiuLianData->m_uidSource==m_pActor->GetUID() && pMagicXiuLianData->m_Mode == enXiuLianMode_Study
	//			|| pMagicXiuLianData->m_uidFriend==m_pActor->GetUID()  && pMagicXiuLianData->m_Mode == enXiuLianMode_Teach)
	//		{
	//			pMagicXiuLianData->m_bUpdate = true;
	//			IMagicPart * pMagicPart = m_pActor->GetMagicPart();
	//			if(pMagicPart !=0)
	//			{
	//				pMagicPart->StudyMagic(pMagicXiuLianData->m_MagicID);
	//				pMagicXiuLianData->m_bStudyMagic = false;
	//			}
	//		}
	//	}

	//	if(pMagicXiuLianData->m_XiuLianState != enXiuLianState_Doing )
	//	{
	//		m_XiuLianData.m_MagicXLSeq = 0;
	//	}	
	//}

	//当角色创建完成后,再收取单修和双修灵气
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_ActoreCreate);

	m_pActor->SubscribeEvent(msgID,this,"XiuLianPart::Create");

	//监听解雇
	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_UnloadEmployee);
	
	m_pActor->SubscribeEvent(msgID,this,"XiuLianPart::Create");

	return true;
}

//释放
void XiuLianPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart XiuLianPart::GetPartID(void)
{
	return enThingPart_Actor_XiuLian;
}

//取得本身生物
IThing*		XiuLianPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool XiuLianPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SDB_Update_XiuLianData_Req))
	{
		return false;
	}

	SDB_Update_XiuLianData_Req * pDBXiuLianData = (SDB_Update_XiuLianData_Req *)buf;

	pDBXiuLianData->m_MagicXLSeq        = m_XiuLianData.m_MagicXLSeq;
	pDBXiuLianData->m_AloneEndTime		= m_XiuLianData.m_EndTime;
	pDBXiuLianData->m_AloneHours		= m_XiuLianData.m_Hours;
	pDBXiuLianData->m_AloneXiuLianState	= m_XiuLianData.m_AloneXiuLianState;
	pDBXiuLianData->m_AloneLastGetNimbusTime = m_XiuLianData.m_lastGetNimbusTime;
	pDBXiuLianData->m_TwoXiuLianState	= m_XiuLianData.m_TwoXiuLianState;
	pDBXiuLianData->m_TwoXLEndTime		= m_XiuLianData.m_TwoEndTime;
	pDBXiuLianData->m_TwoXLHours		= m_XiuLianData.m_TwoXiuLianHours;
	pDBXiuLianData->m_TwoXLLastGetNimbusTime = m_XiuLianData.m_TwoLastGetNimbusTime;
	pDBXiuLianData->m_TwoXLTotalNimbus	= m_XiuLianData.m_TwoTotalNimbus;
	pDBXiuLianData->m_TwoXLUidActor		= m_XiuLianData.m_uidActor.ToUint64();
	pDBXiuLianData->m_TwoXLUidFriend	= m_XiuLianData.m_uidFriend.ToUint64();
	pDBXiuLianData->Uid_User			= m_pActor->GetUID().ToUint64();
	pDBXiuLianData->m_FriendFacade		= m_XiuLianData.m_FriendFacade;
	pDBXiuLianData->m_GetGodSwordNimbus = m_XiuLianData.m_GetGodSwordNimbus;
	strncpy(pDBXiuLianData->m_FriendName, m_XiuLianData.m_FriendName, sizeof(pDBXiuLianData->m_FriendName));

	pDBXiuLianData->m_AloneXLActor      = m_XiuLianData.m_AloneXLData[0].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus  = m_XiuLianData.m_AloneXLData[0].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor2 = m_XiuLianData.m_AloneXLData[1].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus2 = m_XiuLianData.m_AloneXLData[1].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor3 = m_XiuLianData.m_AloneXLData[2].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus3 = m_XiuLianData.m_AloneXLData[2].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor4 = m_XiuLianData.m_AloneXLData[3].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus4 = m_XiuLianData.m_AloneXLData[3].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor5 = m_XiuLianData.m_AloneXLData[4].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus5 = m_XiuLianData.m_AloneXLData[4].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor6 = m_XiuLianData.m_AloneXLData[5].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus6 = m_XiuLianData.m_AloneXLData[5].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor7 = m_XiuLianData.m_AloneXLData[6].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus7 = m_XiuLianData.m_AloneXLData[6].m_GetNimbus;

	pDBXiuLianData->m_AloneXLActor8 = m_XiuLianData.m_AloneXLData[7].m_ActorUID.ToUint64();
	pDBXiuLianData->m_AloneXLGetNimbus8 = m_XiuLianData.m_AloneXLData[7].m_GetNimbus;

	nLen = sizeof(SDB_Update_XiuLianData_Req);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void XiuLianPart::InitPrivateClient()
{

}


//玩家下线了，需要关闭该ThingPart
void XiuLianPart::Close()
{
}

//保存数据
void XiuLianPart::SaveData()
{
	SDB_Update_XiuLianData_Req Req;

	int nLen = sizeof(SDB_Update_XiuLianData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateXiuLianInfo,ob.TakeOsb(),0,0);

	for(ASK_DATA::iterator it = m_AskXiuLianData.begin(); it != m_AskXiuLianData.end(); ++it)
	{
		g_pThingServer->GetXiuLianMgr().UnloadXiuLianRecord(*it);
	}

}

void XiuLianPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_ActoreCreate);

	if( EventData.m_MsgID == msgID){

		SS_ActoreCreateContext * pActoreCreateCnt = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == pActoreCreateCnt){
			return;
		}

		if( pActoreCreateCnt->m_uidActor != m_pActor->GetUID().ToUint64()){
			return;
		}

		if( m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing){
			//获取单修灵气
			this->Take_AloneXLNimbus();
		}

		if( m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing){
			//还没结束,则启动定时器
			this->StartTimer(enXiuLianTimerID_AloneXL, true);

			//通知客户端显示修炼公告
			this->NoticeViewXiuLianMsg(enXiuLianType_Alone, m_XiuLianData.m_EndTime - CURRENT_TIME());
		}

		if( m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing){
				//获取双修灵气
			this->Take_TwoXLNimbus();
		}

		if( m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing){
			//还没结束,则启动定时器
			this->StartTimer(enXiuLianTimerID_TwoXL, true);

			//通知客户端显示修炼公告
			this->NoticeViewXiuLianMsg(enXiuLianType_Two, m_XiuLianData.m_TwoEndTime -  CURRENT_TIME());
		}

		//取消对玩家创建事件的接听
		m_pActor->UnsubscribeEvent(msgID, this);

		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_UnloadEmployee);

	if( EventData.m_MsgID == msgID){
		//解雇,判断解雇角色是否在修炼中
		SS_UnloadEmployee * pUnloadEmployee = (SS_UnloadEmployee *)EventData.m_pContext;
		if( 0 == pUnloadEmployee){
			return;
		}

		for( int i = 0; i < MAX_ALONE_XL_NUM; ++i)
		{
			if( m_XiuLianData.m_AloneXLData[i].m_ActorUID.ToUint64() == pUnloadEmployee->m_uidActor){
				m_XiuLianData.m_AloneXLData[i].m_ActorUID = UID();
				m_XiuLianData.m_AloneXLData[i].m_GetNimbus = 0;

				//判断下,还有没有角色在修炼,没有则停止修炼
				bool bHave = false;
				for( int k = 0; k < MAX_ALONE_XL_NUM; ++k)
				{
					if( m_XiuLianData.m_AloneXLData[k].m_ActorUID.IsValid()){
						bHave = true;
						break;
					}
				}
				
				if( bHave){
					break;	
				}

				//没人在修炼,取消修炼
				this->CancelAloneXiuLian();

				break;
			}
		}

		if( m_XiuLianData.m_uidActor.ToUint64() == pUnloadEmployee->m_uidActor){
			//取消双修
			this->CancelTwoXiuLian();
		}
	}
}

void		XiuLianPart::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__, __LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_GetActorFacade:
		{
			this->HandleTwoXiuLian(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
    default:
	    {
			TRACE("<warning> %s : %d 行 意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",__FUNCTION__, __LINE__,ReqCmd,userID,nRetCode);
	    }
	    break;
	}
}

//打开
void XiuLianPart::Open(CS_OpenXiuLian_Req & Req)
{
	//返回修炼数据
	switch(Req.m_XiuLianType)
	{
	case enXiuLianType_Alone:
		{
			SyncAloneXiuLianData();
		}
		break;
	case enXiuLianType_Two:
		{
			SyncTwoXiuLianData();
		};
		break;
	case enXiuLianType_Magic:
		{
			SyncMagicXiuLianData();
		}
		break;
	default:
		break;
	}
}


//获取独自修炼人数
INT32 XiuLianPart::GetAloneXiuLianActorNum()
{
	int num =0;

	for( int i = 0; i < ARRAY_SIZE(m_XiuLianData.m_AloneXLData); ++i)
	{
		if( !m_XiuLianData.m_AloneXLData[i].m_ActorUID.IsValid())
		{
			continue;
		}

		++num;
	}

	return num;
}

//同步独自修炼数据
void XiuLianPart::SyncAloneXiuLianData()
{
	SC_AloneXiuLianData_Sync Rsp;

	OBuffer1k ob;

	if ( m_XiuLianData.m_AloneXiuLianState != enXiuLianState_Non)
	{
		Rsp.m_XiuLianState = enXiuLianState_Doing;
		Rsp.m_TotalTime	   = m_XiuLianData.m_Hours * 60 * 60;
		Rsp.m_RemianTime   = m_XiuLianData.m_EndTime - CURRENT_TIME();

		if ( Rsp.m_RemianTime < 0){

			Rsp.m_RemianTime = 0;
		}

		Rsp.m_GetGodSwordNimbus = m_XiuLianData.m_GetGodSwordNimbus;

		for ( int i=0; i<ARRAY_SIZE(m_XiuLianData.m_AloneXLData);i++)
		{
			if ( !m_XiuLianData.m_AloneXLData[i].m_ActorUID.IsValid()){

				continue;
			}

			IActor * pTmpActor = g_pGameServer->GetGameWorld()->FindActor(m_XiuLianData.m_AloneXLData[i].m_ActorUID);

			if ( 0 == pTmpActor){

				break;
			}

			SC_AloneXLData  AloneXLData;

			const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTmpActor->GetCrtProp(enCrtProp_ActorLayer));

			if ( 0 == pLayerCnfg){

				return;
			}

			AloneXLData.m_NimbusUp = pLayerCnfg->m_NimbusUp;

			AloneXLData.m_uidActor = m_XiuLianData.m_AloneXLData[i].m_ActorUID;
			AloneXLData.m_GetNimbus = m_XiuLianData.m_AloneXLData[i].m_GetNimbus;

			Rsp.m_ActorNum++;
			ob << AloneXLData;
		}			
	}
	else
	{
		Rsp.m_XiuLianState = enXiuLianState_Non;
	}


	OBuffer1k ob2;
	ob2 << XiuLian_Header(enXiuLianCmd_AloneXiuLian_Sync,sizeof(Rsp)+ob.Size()) << Rsp;
	if(ob.Size()>0)
	{
		ob2.Push(ob.Buffer(),ob.Size());
	}

	m_pActor->SendData(ob2.TakeOsb());
}

//同步双修炼数据
void XiuLianPart::SyncTwoXiuLianData()
{
	SC_TwoXiuLianData_Sync Rsp;

	OBuffer1k ob;

	if( m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Non){
		Rsp.m_XiuLianState = enXiuLianState_Non;

		ob << XiuLian_Header(enXiuLianCmd_TwoXiuLian_Sync,sizeof(Rsp)) << Rsp;
	}else{
		TwoXiuLianData_Rsp RspData;

		Rsp.m_XiuLianState	  = m_XiuLianData.m_TwoXiuLianState;
		RspData.m_RemianTime  = m_XiuLianData.m_TwoEndTime - CURRENT_TIME();
		RspData.m_TotalTime   = m_XiuLianData.m_TwoXiuLianHours * 60 * 60;
		RspData.m_TotalNimbus = m_XiuLianData.m_TwoTotalNimbus;
		RspData.m_uidActor	  = m_XiuLianData.m_uidActor;
		RspData.m_uidFriend	  = m_XiuLianData.m_uidFriend;
		RspData.m_FriendFacade = m_XiuLianData.m_FriendFacade;
		strncpy(RspData.m_FriendName, m_XiuLianData.m_FriendName, sizeof(RspData.m_FriendName));

		IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(m_XiuLianData.m_uidActor);
		if( 0 == pTargetActor){
			return;
		}

		const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorLayer));
		if( 0 == pLayerCnfg){
			return;
		}

		RspData.m_NimbusUp = pLayerCnfg->m_NimbusUp;

		ob << XiuLian_Header(enXiuLianCmd_TwoXiuLian_Sync,sizeof(Rsp) + sizeof(RspData)) << Rsp << RspData;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//同步法术修炼数据
void XiuLianPart::SyncMagicXiuLianData()
{
	SC_MagicXiuLianData_Sync Rsp;
	MagicLianXiuLianData_Rsp RspData;
	OBuffer1k ob;

	STwoXiuLianData * pTwoXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	if(pTwoXiuLianData==0  || (pTwoXiuLianData->m_XiuLianState != enXiuLianState_Doing && pTwoXiuLianData->m_XiuLianState != enXiuLianState_Ask))
	{
		Rsp.m_XiuLianState = enXiuLianState_Non;

		for( ASK_DATA::iterator iter = m_AskXiuLianData.begin(); iter != m_AskXiuLianData.end(); ++iter){
			STwoXiuLianData * pTmpTwoXiuLianData = GetAskXiuLianData(*iter);
			if( pTmpTwoXiuLianData != 0 && pTmpTwoXiuLianData->m_XiuLianState == enXiuLianState_Ask &&
				pTmpTwoXiuLianData->m_XiuLianType == enXiuLianType_Magic && pTmpTwoXiuLianData->m_uidSource == m_pActor->GetUID())
			{
				//发出请求未处理状态
				Rsp.m_XiuLianState = enXiuLianState_Ask;
				break;
			}
		}
		
		ob << XiuLian_Header(enXiuLianCmd_MagicXiuLian_Sync,sizeof(Rsp)) << Rsp;
	}
	else 	
	{
		Rsp.m_XiuLianState = pTwoXiuLianData->m_XiuLianState;

		RspData.m_AskSeq	 = pTwoXiuLianData->m_AskSeq;
		RspData.m_RemianTime = GetXiuLianRemainTime(enXiuLianType_Magic);
		RspData.m_MagicID	 = pTwoXiuLianData->m_MagicID;
		RspData.m_Mode		 = pTwoXiuLianData->m_Mode;

		if(pTwoXiuLianData->m_uidSource==m_pActor->GetUID())         //需要判断是否是发起方
		{			
			RspData.m_uidActor = pTwoXiuLianData->m_uidSourceActor;	  
			RspData.m_uidFriend =pTwoXiuLianData->m_uidFriend;
			strncpy(RspData.m_szFriendName,pTwoXiuLianData->m_szFriendName,sizeof(RspData.m_szFriendName));			
		}
		else
		{
			RspData.m_uidActor = pTwoXiuLianData->m_uidFriendActor;	  
			RspData.m_uidFriend =pTwoXiuLianData->m_uidSource;
			strncpy(RspData.m_szFriendName,pTwoXiuLianData->m_szSourceName,sizeof(RspData.m_szFriendName));	
		}

		ob << XiuLian_Header(enXiuLianCmd_MagicXiuLian_Sync,sizeof(Rsp) + sizeof(RspData)) << Rsp << RspData;
	}

	m_pActor->SendData(ob.TakeOsb());
}


		//获得双修剩余时间
UINT32 XiuLianPart::GetTwoXiuLianRemainTime()
{
	return GetXiuLianRemainTime(enXiuLianType_Two);
}

//获得单修剩余时间
 UINT32 XiuLianPart::GetAloneXiuLianRemainTime()
 {
	 return GetXiuLianRemainTime(enXiuLianType_Alone);
 }

//得到正在修炼中的角色UID
void   XiuLianPart::GetInXiuLianActorUID()
{
	SC_InXLActorUID Rsp;

	Rsp.m_TwoXLActorUID = m_XiuLianData.m_uidActor;

	OBuffer1k ob;

	for( int i = 0; i < MAX_ALONE_XL_NUM; ++i)
	{
		if ( m_XiuLianData.m_AloneXLData[i].m_ActorUID.IsValid()){

			ob << m_XiuLianData.m_AloneXLData[i].m_ActorUID;

			++Rsp.m_Num;
		}
	}

	OBuffer1k ob2;
	ob2 << XiuLian_Header(enXiuLianCmd_InXiuLianActor,sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){

		ob2 << ob;
	}

	m_pActor->SendData(ob2.TakeOsb());	
}

//获得修炼余下时间
UINT32  XiuLianPart::GetXiuLianRemainTime(enXiuLianType XiuLianType)
{
	UINT32 nCurTime = CURRENT_TIME();

	UINT32 EndTime = 0;

	if(XiuLianType == enXiuLianType_Alone)
	{
		EndTime = m_XiuLianData.m_EndTime;
	}
	else if(XiuLianType == enXiuLianType_Two)
	{
		EndTime = m_XiuLianData.m_TwoEndTime;
	}
	else if(XiuLianType == enXiuLianType_Two )
	{
		STwoXiuLianData * pTwoXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);
		if(pTwoXiuLianData != 0 && pTwoXiuLianData->m_XiuLianState == enXiuLianState_Doing)
		{
			EndTime = pTwoXiuLianData->m_EndTime;
		}
	}


	if(nCurTime>EndTime)
	{
		return 0;
	}
	return EndTime - nCurTime;
}


//独自修炼请求
void XiuLianPart::AloneXiuLian(const CS_AlongXiuLian_Req * pAlongXiuLianReq,const UID* pUidActor)
{
	SC_AloneXiuLian_Rsp Rsp;

	Rsp.m_Result = enXiuLianRetCode_OK;


	if(GetAloneXiuLianActorNum()>0)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveAloneXL;	
	}
	else if(pAlongXiuLianReq->m_ActorNum<1)
	{
		//参数有误
		return;
	}
	else
	{
		for(int i=0; i<pAlongXiuLianReq->m_ActorNum;i++)
		{
			IActor * pEmployee = m_pActor;
			//判断角色是否有效
			if(IsValidActor(pUidActor[i])==false)
			{
				Rsp.m_Result = enXiuLianRetCode_ErrorActor;
				break;
			}

			enXiuLianType XiuLianType = GetXiuLianType(pUidActor[i]);

			//判断角色是否已在双休为修炼法术
			if(XiuLianType == enXiuLianType_Two)
			{
				Rsp.m_Result = enXiuLianRetCode_HaveTwoXL;
				break;
			}
			else if(XiuLianType == enXiuLianType_Magic)
			{
				Rsp.m_Result = enXiuLianRetCode_HaveMagicXL;
				break;
			}			
		}

		if(Rsp.m_Result == enXiuLianRetCode_OK)
		{
			if( m_pActor->GetCrtProp(enCrtProp_ActorStone) < GetAloneXLCharge(pAlongXiuLianReq->m_ActorNum,pAlongXiuLianReq->m_Hours)){
				//费用不足
				Rsp.m_Result = enXiuLianRetCode_NoSpiritStone;
			}
			else
			{
				m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -GetAloneXLCharge(pAlongXiuLianReq->m_ActorNum,pAlongXiuLianReq->m_Hours));

				for( int i = 0; i < pAlongXiuLianReq->m_ActorNum && i < MAX_ALONE_XL_NUM; ++i,++pUidActor)
				{
					m_XiuLianData.m_AloneXLData[i].m_ActorUID = *pUidActor;
				}

				AloneXiuLian(pAlongXiuLianReq->m_Hours);

				SyncAloneXiuLianData();

				//通知客户端显示修炼公告
				this->NoticeViewXiuLianMsg(enXiuLianType_Alone, pAlongXiuLianReq->m_Hours * 3600);

				//修炼完成，发布事件
				SS_XiuLianFinish XiuLianFinish;
				XiuLianFinish.m_XiuLianType = enXiuLianType_Alone;

				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_XiuLian);
				m_pActor->OnEvent(msgID,&XiuLianFinish,sizeof(XiuLianFinish));
			}
		}		
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_AloneXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

}

INT32 XiuLianPart::AloneXiuLian(INT32 nHours) 
{
	UINT32 nCurTime  = CURRENT_TIME() ;
	m_XiuLianData.m_EndTime = nCurTime + nHours*3600;
	m_XiuLianData.m_Hours = nHours;
	m_XiuLianData.m_lastGetNimbusTime = nCurTime;
	m_XiuLianData.m_AloneXiuLianState = enXiuLianState_Doing;

	//启动定时器
	StartTimer(enXiuLianTimerID_AloneXL);

	return 0;
}

//取消独自修炼
void XiuLianPart::CancelAloneXiuLian()
{
	SC_CancelAloneXL_Rsp Rsp;
	OBuffer1k ob;

	Rsp.m_Result = enXiuLianRetCode_OK;

	INT32 ActorNum = GetAloneXiuLianActorNum();

	if(ActorNum==0)
	{
		Rsp.m_Result = enXiuLianRetCode_NoAloneXL;
		ob << XiuLian_Header(enXiuLianCmd_CancelAloneXiuLian,sizeof(Rsp)) << Rsp;
	}
	else
	{
		Rsp.m_ActorNum		= ActorNum;
		Rsp.m_Hours			= m_XiuLianData.m_Hours;
		Rsp.m_RealHours		= m_XiuLianData.m_Hours - (m_XiuLianData.m_EndTime - time(0) + 59) / 60;
		Rsp.m_Stone			= this->GetAloneXLCharge(ActorNum, m_XiuLianData.m_Hours);

		ob << XiuLian_Header(enXiuLianCmd_CancelAloneXiuLian,sizeof(Rsp) + ActorNum * sizeof(SC_CancelSendName_Rsp)) << Rsp;

		for( int k = 0; k < MAX_ALONE_XL_NUM; ++k)
		{
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_XiuLianData.m_AloneXLData[k].m_ActorUID);
			if( 0 == pActor){
				continue;
			}
			SC_CancelSendName_Rsp RspName;
			strncpy(RspName.m_szActorName, pActor->GetName(), sizeof(RspName.m_szActorName));
			ob << RspName;
		}

		if( m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing){
			//停止定时器		
			StopTimer(enXiuLianTimerID_AloneXL);
		}
		
		for( int i = 0; i < ARRAY_SIZE(m_XiuLianData.m_AloneXLData); ++i)
		{
			m_XiuLianData.m_AloneXLData[i].m_ActorUID = UID();
			m_XiuLianData.m_AloneXLData[i].m_GetNimbus = 0;
		}
	}

	m_XiuLianData.m_AloneXiuLianState = enXiuLianState_Non;

	m_XiuLianData.m_EndTime = 0;
	m_XiuLianData.m_Hours = 0;
	m_XiuLianData.m_lastGetNimbusTime = 0;
	m_XiuLianData.m_GetGodSwordNimbus = 0;

	//通知客户端取消公告
	this->NoticeCancelViewXLMsg(enXiuLianType_Alone);

	m_pActor->SendData(ob.TakeOsb());

	return;
}

//完成独自修炼
void XiuLianPart::FinishAloneXiuLian()
{
	if(m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing)
	{
		//修炼完成
		m_XiuLianData.m_AloneXiuLianState = enXiuLianState_Finish;

		//停止定时器		
		StopTimer(enXiuLianTimerID_AloneXL);
	}

	return ;
}


//判断角色是否是有效的角色
bool XiuLianPart::IsValidActor(UID uidActor)
{
	if(uidActor  != m_pActor->GetUID() && m_pActor->GetEmployee(uidActor)==0)
	{
		return false;
	}

	return true;
}

//发起双修请求
void XiuLianPart::TwoXiuLian(CS_TwoXiuLian_Req & Req)
{
	SC_TwoXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	enXiuLianType XiuLianType = GetXiuLianType(Req.m_uidActor);					

	if(IsValidActor(Req.m_uidActor)==false)
	{
		Rsp.m_Result = enXiuLianRetCode_ErrorActor;
	}
	else if(m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveTwoXL;
	}
	//判断角色是否已在双休为修炼法术
	else if(XiuLianType == enXiuLianType_Alone)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveAloneXL;				
	}
	else if(XiuLianType == enXiuLianType_Magic)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveMagicXL;
	}	
	else if(m_pActor->GetCrtProp(enCrtProp_ActorStone) < GetTwoXLCharge(Req.m_Hours))
	{
		Rsp.m_Result = enXiuLianRetCode_NoSpiritStone;
	}
	else
	{
		IFriendPart * pFriendPart = m_pActor->GetFriendPart();
		if( 0 == pFriendPart){
			return;
		}

		if ( !pFriendPart->IsFriend(Req.m_uidFriend) )
		{
			TRACE("<error> %s : %d Line 不是好友，不能双修！！我的UID:%s,好友的UID:%s", __FUNCTION__, __LINE__, m_pActor->GetUID().ToString(), Req.m_uidFriend.ToString());
			return;
		}
		
		strncpy(m_XiuLianData.m_FriendName, pFriendPart->GetFriendName(Req.m_uidFriend), sizeof(m_XiuLianData.m_FriendName));
		m_XiuLianData.m_uidFriend		= Req.m_uidFriend;
		m_XiuLianData.m_TwoEndTime		= CURRENT_TIME() + Req.m_Hours * 60 * 60;
		m_XiuLianData.m_TwoXiuLianHours = Req.m_Hours;
		m_XiuLianData.m_uidActor		= Req.m_uidActor;
		m_XiuLianData.m_TwoLastGetNimbusTime = CURRENT_TIME();
		m_XiuLianData.m_TwoTotalNimbus		= 0;

		//获得好友的外观
		SDB_GetActorFacade_Req GetActorFacade_Req;
		GetActorFacade_Req.m_ActorUID = Req.m_uidFriend.ToUint64();

		OBuffer1k ob;
		ob << GetActorFacade_Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_GetActorFacade,ob.TakeOsb(),&g_pThingServer->GetXiuLianMgr(),m_pActor->GetUID().ToUint64());
		
		return;
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_AskTwoXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//请求双休
INT32 XiuLianPart::AskTwoXiuLian(UID uidFriend,INT32 nHours, const char * FriendName)
{
	STwoXiuLianData  XiuLianData;

	XiuLianData.m_XiuLianType = enXiuLianType_Two;
	XiuLianData.m_XiuLianState = enXiuLianState_Ask;
	XiuLianData.m_AskSeq = GetNextAskSeq();
	XiuLianData.m_AskTime = CURRENT_TIME();
//	XiuLianData.m_EndTime = XiuLianData.m_AskTime + nHours*3600;
	XiuLianData.m_EndTime = 0;
	XiuLianData.m_Hours = nHours;
	XiuLianData.m_lastGetNimbusTime = 0;
	XiuLianData.m_MagicID = INVALID_MAGIC_ID;
	XiuLianData.m_Mode = enXiuLianMode_Max;
//	XiuLianData.m_szFriendName[0] = 0;
	XiuLianData.m_uidFriend = uidFriend;
	XiuLianData.m_TotalNimbus = 0;
	XiuLianData.m_uidFriendActor = UID();
	XiuLianData.m_uidSourceActor = m_pActor->GetUID();
	XiuLianData.m_bUpdate = true;

	XiuLianData.m_uidSource = m_pActor->GetUID();

	strncpy(XiuLianData.m_szFriendName, FriendName, sizeof(XiuLianData.m_szFriendName));
	strncpy(XiuLianData.m_szSourceName,m_pActor->GetName(),sizeof(XiuLianData.m_szSourceName));

	XiuLianData.m_FriendNimbusSpeed = 0;
	XiuLianData.m_SourceNimbusSpeed = m_pActor->GetCrtProp(enCrtProp_ActorNimbusSpeed);
	XiuLianData.m_FriendLayer		= 0;
	XiuLianData.m_SourceLayer		= m_pActor->GetCrtProp(enCrtProp_ActorLayer);

	AddAskXiuLianData(XiuLianData);

	//如果好友在线的话，往好友修炼请求加上
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidFriend);
	if( 0 != pFriend){
		IXiuLianPart * pXiuLianPart = pFriend->GetXiuLianPart();
		if( 0 != pXiuLianPart){
			pXiuLianPart->AddToAskList(XiuLianData.m_AskSeq);
		}
	}


	//发送邮件给对方
	this->SendEmail(enXiuLianType_Two,uidFriend);

	return enXiuLianRetCode_OK;
}

//增加一个修炼请求
bool  XiuLianPart::AddAskXiuLianData(STwoXiuLianData & XiuLianData)
{
	if(m_AskXiuLianData.insert(XiuLianData.m_AskSeq).second==false)
	{
		return false;
	}

	if(g_pThingServer->GetXiuLianMgr().AddAskXiuLianData(XiuLianData)==false)
	{
		m_AskXiuLianData.erase(XiuLianData.m_AskSeq);
		return false;
	}

	return true;

}


//删除一个修炼请求
void  XiuLianPart::DelAskXiuLianData(STwoXiuLianData & XiuLianData)
{
	if(g_pThingServer->GetXiuLianMgr().DelAskXiuLianData(XiuLianData.m_AskSeq)==false)
	{
		return;
	}
	m_AskXiuLianData.erase(XiuLianData.m_AskSeq);
}

//获得角色的修炼状态
enXiuLianType XiuLianPart::GetXiuLianType(UID uidActor)
{
	//正在双修的数据
	STwoXiuLianData * pMagicXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	if(m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing)
	{
		for(int i=0; i<ARRAY_SIZE(m_XiuLianData.m_AloneXLData);++i)
		{
			if(m_XiuLianData.m_AloneXLData[i].m_ActorUID == uidActor)
			{
				return enXiuLianType_Alone;
			}
		}
	}
	else if(m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing && m_XiuLianData.m_uidActor == uidActor)
	{
		return enXiuLianType_Two;
	}
	else if(pMagicXiuLianData && (pMagicXiuLianData->m_uidSourceActor==uidActor || pMagicXiuLianData->m_uidFriendActor==uidActor))
	{
		return enXiuLianType_Magic;
	}

	return enXiuLianType_Non;
}

//停止双修
void XiuLianPart::FinishTwoXiuLian()
{	
	if( m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing){
		m_XiuLianData.m_TwoXiuLianState = enXiuLianState_Finish;

		//停止定时器		
		StopTimer(enXiuLianTimerID_TwoXL);
	}
}

//检测这个请求号是否在玩家的请求列表中
bool XiuLianPart::IsInAskList(UINT32 AskReqID)
{
	ASK_DATA::iterator iter = m_AskXiuLianData.find(AskReqID);
	if( iter == m_AskXiuLianData.end()){
		return false;
	}

	return true;
}

//取消双修
void XiuLianPart::CancelTwoXiuLian()
{
	SC_CancelTwoXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;
	if( m_XiuLianData.m_TwoXiuLianState != enXiuLianState_Doing && m_XiuLianData.m_TwoXiuLianState != enXiuLianState_Finish){
		Rsp.m_Result = enXiuLianRetCode_NoTwoXL;
	}else{
		Rsp.m_Hours	   = m_XiuLianData.m_TwoXiuLianHours;
		Rsp.m_RealTime = m_XiuLianData.m_TwoXiuLianHours * 60 * 60 - (m_XiuLianData.m_TwoEndTime - m_XiuLianData.m_TwoLastGetNimbusTime);
		Rsp.m_Stone	   = this->GetTwoXLCharge(m_XiuLianData.m_TwoXiuLianHours);
		Rsp.m_TotalNimbus  = m_XiuLianData.m_TwoTotalNimbus;
		Rsp.m_XiuLianActor = m_XiuLianData.m_uidActor;

		if( m_XiuLianData.m_AloneXiuLianState == enXiuLianState_Doing){
			//停止定时器		
			StopTimer(enXiuLianTimerID_TwoXL);
		}

		//加好友度
		IFriendPart * pFriendPart = m_pActor->GetFriendPart();
		if( 0 != pFriendPart){
			pFriendPart->AddRelationNum(m_XiuLianData.m_uidFriend, m_XiuLianData.m_FriendName, enXiuLianOneHour, Rsp.m_RealTime / 3600);
		}
		
		//数据重置下
		m_XiuLianData.m_TwoXiuLianState = enXiuLianState_Non;
		m_XiuLianData.m_uidActor	= UID();
		m_XiuLianData.m_uidFriend	= UID();
		m_XiuLianData.m_TwoXiuLianHours	= 0;
		m_XiuLianData.m_TwoTotalNimbus	= 0;
		m_XiuLianData.m_TwoLastGetNimbusTime = 0;
		m_XiuLianData.m_TwoEndTime = 0;
		m_XiuLianData.m_FriendFacade = 0;
		memset(m_XiuLianData.m_FriendName, 0, ARRAY_SIZE(m_XiuLianData.m_FriendName));

		//通知客户端取消公告
		this->NoticeCancelViewXLMsg(enXiuLianType_Two);
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLainCmd_CancelTwoXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}


//发起修炼法术请求
void XiuLianPart::MagicXiuLian(CS_MagicXiuLian_Req & Req)
{
	SC_MagicXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Req.m_uidActor));

	//法术part
	IMagicPart * pMagicPart = 0;

	//背包
	IPacketPart * pPacketPart = 0;

	IFriendPart * pFriendPart = 0;

	if(pActor != 0)
	{
		pMagicPart = pActor->GetMagicPart();

		pPacketPart = pActor->GetPacketPart();

		pFriendPart = pActor->GetFriendPart();
	}

	if( 0 == pMagicPart || 0 == pPacketPart || 0 == pFriendPart){
		return;
	}

	STwoXiuLianData * pTwoXiuLianData =GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	enXiuLianType XiuLianType = GetXiuLianType(UID(Req.m_uidActor));	

	//获得教学法术所需的好友度
	INT32 nNeedRelation = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MagicXiuLianNeedRelation;

	if(IsValidActor(UID(Req.m_uidActor))==false)
	{
		Rsp.m_Result = enXiuLianRetCode_ErrorActor;
	}
	else if(pTwoXiuLianData != 0)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveMagicXL;
	}
	//判断角色是否已在双休为修炼法术
	else if(XiuLianType == enXiuLianType_Alone)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveAloneXL;				
	}
	else if(XiuLianType == enXiuLianType_Two)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveTwoXL;				
	}
	else if(Req.m_Mode == enXiuLianMode_Teach && pMagicPart->GetMagic(Req.m_MagicID)==0) //如果是传授，判断是否已学会该法术
	{
		Rsp.m_Result = enXiuLianRetCode_NoMagic;		
	}
	else if(pPacketPart->DestroyGoods(GetMagicXiuLianNeedGoodsID())==false)  //判断是否有传功丹
	{
		Rsp.m_Result = enXiuLianRetCode_NoGoods;		
	}
	else if(pFriendPart->GetRelationNum(UID(Req.m_uidFriend)) < nNeedRelation)	//好友度判断
	{
		Rsp.m_Result = enXiuLianRetCode_ErrRelation;
	}
	else
	{
		//一次只能发一份双修请求，检查是否有请求未处理
		for(ASK_DATA::iterator iter = m_AskXiuLianData.begin(); iter != m_AskXiuLianData.end(); ++iter)
		{
			 STwoXiuLianData * pTwoXiuLianData = GetAskXiuLianData((*iter));
			 if( pTwoXiuLianData->m_XiuLianState == enXiuLianState_Ask && pTwoXiuLianData->m_XiuLianType == enXiuLianType_Magic){
				 Rsp.m_Result = enXiuLianRetCode_ErrXiuLianMagic;
				 break;
			 }
		}

		if( enXiuLianRetCode_OK == Rsp.m_Result){
			Rsp.m_Result = (enXiuLianRetCode)AskMagicXiuLian(UID(Req.m_uidFriend),Req.m_Mode,Req.m_MagicID, Req.m_szFriendName);

			if(Rsp.m_Result == enXiuLianRetCode_OK )
			{
				SyncMagicXiuLianData();
				return;
			}
		}

	}


	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_AskMagicXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//获取修炼法术需要的物品GoodsID
TGoodsID XiuLianPart::GetMagicXiuLianNeedGoodsID()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_gidMagicXLPropID;
}

//法术修炼时间
INT32 XiuLianPart::GetMagicXiuLianNeedTime()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MagicXLTime;
}

//请求修炼法术
INT32 XiuLianPart::AskMagicXiuLian(UID uidFriend,enXiuLianMode Mode,TMagicID MagicID,const char * FriendName)
{
	STwoXiuLianData  XiuLianData;

	XiuLianData.m_XiuLianType = enXiuLianType_Magic;
	XiuLianData.m_XiuLianState = enXiuLianState_Ask;
	XiuLianData.m_AskSeq = GetNextAskSeq();
	XiuLianData.m_AskTime = CURRENT_TIME();
	XiuLianData.m_Hours = GetMagicXiuLianNeedTime();
	XiuLianData.m_EndTime = XiuLianData.m_AskTime + XiuLianData.m_Hours*3600;	
	XiuLianData.m_lastGetNimbusTime = 0;
	XiuLianData.m_MagicID = MagicID;
	XiuLianData.m_Mode = Mode;
//	XiuLianData.m_szFriendName[0] = 0;
	XiuLianData.m_uidFriend = uidFriend;
	XiuLianData.m_TotalNimbus = 0;
	XiuLianData.m_uidFriendActor = UID();
	XiuLianData.m_uidSourceActor = m_pActor->GetUID();
	XiuLianData.m_bUpdate = true;

	XiuLianData.m_uidSource = m_pActor->GetUID();

	strncpy(XiuLianData.m_szFriendName, FriendName, sizeof(XiuLianData.m_szFriendName));
	strncpy(XiuLianData.m_szSourceName,m_pActor->GetName(),sizeof(XiuLianData.m_szSourceName));

	XiuLianData.m_FriendNimbusSpeed = 0;
	XiuLianData.m_SourceNimbusSpeed = m_pActor->GetCrtProp(enCrtProp_ActorNimbusSpeed);
	XiuLianData.m_FriendLayer		= 0;
	XiuLianData.m_SourceLayer		= m_pActor->GetCrtProp(enCrtProp_ActorLayer);

	AddAskXiuLianData(XiuLianData);

	//如果好友在线的话，往好友修炼请求加上
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidFriend);
	if( 0 != pFriend){
		IXiuLianPart * pXiuLianPart = pFriend->GetXiuLianPart();
		if( 0 != pXiuLianPart){
			pXiuLianPart->AddToAskList(XiuLianData.m_AskSeq);
		}
	}

	//发送邮件给对方
	this->SendEmail(enXiuLianType_Magic, uidFriend, MagicID);

	return enXiuLianRetCode_OK;
}

//同意修炼法术
void XiuLianPart::AcceptMagicXiuLian(CS_AcceptMagicXiuLian_Req & Req)
{
	SC_AcceptMagicXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	//别人发过来的请求
	STwoXiuLianData * pAskXiuLianData = GetAskXiuLianData(Req.m_AskSeq);

	STwoXiuLianData * pMagicXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	enXiuLianType XiuLianType = GetXiuLianType(Req.m_uidActor);

	IFriendPart * pFriendPart = m_pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	INT32 nNeedRelation = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MagicXiuLianNeedRelation;

	if(pAskXiuLianData==0)
	{
		Rsp.m_Result = enXiuLianRetCode_NoAskData;
	}
	else if(IsValidActor(Req.m_uidActor)==false) 
	{
		Rsp.m_Result = enXiuLianRetCode_ErrorActor;
	}
	else if(XiuLianType == enXiuLianType_Alone)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveAloneXL;
	}
	else if(XiuLianType == enXiuLianType_Two)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveTwoXL;;
	}
	else if(XiuLianType == enXiuLianType_Magic)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveMagicXL;;
	}
	else if(pMagicXiuLianData != 0)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveMagicXL;
	}
	else if(pFriendPart->GetRelationNum(pAskXiuLianData->m_uidSource) < nNeedRelation)
	{
		Rsp.m_Result = enXiuLianRetCode_ErrRelation;
	}
	else if(pAskXiuLianData->m_XiuLianState != enXiuLianState_Ask)
	{
		Rsp.m_Result = enXiuLianRetCode_HaveAloneXL;
	}
	else
	{
		this->CancelMagicAsk();

		//开始修炼法术
		pAskXiuLianData->m_uidFriendActor	 = Req.m_uidActor;
		strncpy(pAskXiuLianData->m_szFriendName,m_pActor->GetName(),sizeof(pAskXiuLianData->m_szFriendName));
		pAskXiuLianData->m_EndTime			 = time(0) + pAskXiuLianData->m_Hours * 3600;
		pAskXiuLianData->m_XiuLianState		 = enXiuLianState_Doing;

		StartMagicXiuLian(pAskXiuLianData);

		SyncMagicXiuLianData();

		//如果修炼法术的好友也在线，则也要同步下
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pAskXiuLianData->m_uidSource);
		if( 0 != pActor){
			IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
			if( 0 != pXiuLianPart){
				pXiuLianPart->StartMagicXiuLian(pAskXiuLianData);
				pXiuLianPart->SyncMagicXiuLianData();
			}
		}
	}


	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_AcceptMagicXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//取消前面发起的修炼法术请求
void XiuLianPart::CancelMagicAsk()
{
	STwoXiuLianData * pAskXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);
	if( 0 != pAskXiuLianData && pAskXiuLianData->m_XiuLianState == enXiuLianState_Ask){
		pAskXiuLianData->m_XiuLianState = enXiuLianState_Cancel;
	}
}

////开始修炼法术
void XiuLianPart::StartMagicXiuLian(STwoXiuLianData * pAskXiuLianData)
{
	pAskXiuLianData->m_bUpdate = true;

	m_XiuLianData.m_MagicXLSeq	= pAskXiuLianData->m_AskSeq;
}

//停止修炼法术
void XiuLianPart::StopMagicXiuLian()
{

	//根据序列号获得请求数据
	STwoXiuLianData * pDBXiuLianData =GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	if(pDBXiuLianData)
	{
		pDBXiuLianData->m_XiuLianState = enXiuLianState_Stop;
		pDBXiuLianData->m_bUpdate = true;
	}
	
	m_XiuLianData.m_MagicXLSeq	= 0;
}

//拒绝修炼法术
void XiuLianPart::RejectMagicXiuLian(CS_RejectMagicXiuLian_Req & Req)
{
	SC_RejectTwoXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	//别人发过来的请求
	STwoXiuLianData * pAskXiuLianData = GetAskXiuLianData(Req.m_AskSeq);

	if(pAskXiuLianData == 0)
	{
		Rsp.m_Result = enXiuLianRetCode_NoAskData;
	}
	else if(pAskXiuLianData->m_XiuLianType != enXiuLianType_Magic
		|| pAskXiuLianData->m_XiuLianState != enXiuLianState_Ask)
	{
		Rsp.m_Result = enXiuLianRetCode_NoAskData;
	}
	else
	{
		//不在线，怎么处理
		pAskXiuLianData->m_XiuLianState = enXiuLianState_Cancel;
		pAskXiuLianData->m_bUpdate = true;
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_RejectMagicXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//取消修炼法术
void XiuLianPart::CancelMagicXiuLian(CS_CancelMagicXiuLian_Req & Req)
{
	SC_CancelMagicXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	STwoXiuLianData * pMagicXiuLianData = GetAskXiuLianData(m_XiuLianData.m_MagicXLSeq);

	if(pMagicXiuLianData==0
		|| (pMagicXiuLianData->m_XiuLianState != enXiuLianState_Doing && pMagicXiuLianData->m_XiuLianState != enXiuLianState_Ask))
	{
		Rsp.m_Result = enXiuLianRetCode_NoMagicXL;
	}
	else
	{	
		Rsp.m_Hours = pMagicXiuLianData->m_Hours;
		Rsp.m_RealHours = pMagicXiuLianData->m_Hours - (pMagicXiuLianData->m_EndTime - time(0) + 59) / 60;

		if( m_pActor->GetUID() == pMagicXiuLianData->m_uidSource){
			strncpy(Rsp.m_szActorName, pMagicXiuLianData->m_szSourceName, sizeof(Rsp.m_szActorName));
			strncpy(Rsp.m_szFriendName, pMagicXiuLianData->m_szFriendName, sizeof(Rsp.m_szFriendName));
			Rsp.m_XiuLianMode = pMagicXiuLianData->m_Mode;
		}else{
			strncpy(Rsp.m_szActorName, pMagicXiuLianData->m_szFriendName, sizeof(Rsp.m_szActorName));
			strncpy(Rsp.m_szFriendName, pMagicXiuLianData->m_szSourceName, sizeof(Rsp.m_szFriendName));
			if( enXiuLianMode_Study == pMagicXiuLianData->m_Mode){
				Rsp.m_XiuLianMode = enXiuLianMode_Teach;
			}else{
				Rsp.m_XiuLianMode = enXiuLianMode_Study;
			}
		}
		Rsp.m_MagicID = pMagicXiuLianData->m_MagicID;

		StopMagicXiuLian();
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_CancelMagicXiuLian,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	//如果好友在线也要发消息
	IActor * pFriend = 0;
	if( m_pActor->GetUID() == pMagicXiuLianData->m_uidSource){
		pFriend = g_pGameServer->GetGameWorld()->FindActor(pMagicXiuLianData->m_uidFriend);
	}else{
		pFriend = g_pGameServer->GetGameWorld()->FindActor(pMagicXiuLianData->m_uidSource);
	}

	IXiuLianPart * pXiuLianPart = pFriend->GetXiuLianPart();
	if( 0 == pXiuLianPart || 0 == pFriend){
		return;
	}

	SC_CancelMagicXiuLian_Rsp RspFriend;
	RspFriend.m_Hours		= Rsp.m_Hours;
	RspFriend.m_RealHours	= Rsp.m_RealHours;
	RspFriend.m_Result		= Rsp.m_Result;
	strncpy(RspFriend.m_szActorName, Rsp.m_szFriendName, sizeof(RspFriend.m_szActorName));
	strncpy(RspFriend.m_szFriendName, Rsp.m_szActorName, sizeof(RspFriend.m_szFriendName));
	if( enXiuLianMode_Study == Rsp.m_XiuLianMode){
		RspFriend.m_XiuLianMode = enXiuLianMode_Teach;
	}else{
		RspFriend.m_XiuLianMode = enXiuLianMode_Study;
	}
	RspFriend.m_MagicID = pMagicXiuLianData->m_MagicID;

	ob.Reset();
	ob << XiuLian_Header(enXiuLianCmd_CancelMagicXiuLian,sizeof(RspFriend)) << RspFriend;
	pFriend->SendData(ob.TakeOsb());

	pXiuLianPart->StopMagicXiuLian();
}


//查看请求数据
void XiuLianPart::ViewAskData(/*CS_ViewXiuLianAskData_Req & Req*/)
{
	SC_ViewXiuLianAskData_Rsp Rsp;
	Rsp.m_AskDataNum = 0;

	XiuLianAskData AskData;

	//ASK_DATA::iterator it = m_AskXiuLianData.begin();
	//if(Req.m_AskSeq>0)
	//{
	//	it = m_AskXiuLianData.find(Req.m_AskSeq);
	//	if(it != m_AskXiuLianData.end())
	//	{
	//		++it;
	//	}
	//}

	OBuffer4k obData;

	for(ASK_DATA::iterator it = m_AskXiuLianData.begin(); it != m_AskXiuLianData.end(); ++it)
	{
		STwoXiuLianData * pTwoXiuLianData = GetAskXiuLianData((*it));

		if(pTwoXiuLianData != 0)
		{
			AskData.m_AskSeq = pTwoXiuLianData->m_AskSeq;
			AskData.m_Hours = pTwoXiuLianData->m_Hours;
			AskData.m_MagicID = pTwoXiuLianData->m_MagicID;
			AskData.m_Mode = pTwoXiuLianData->m_Mode;
			AskData.m_RealTime = pTwoXiuLianData->m_Hours*3600;
			AskData.m_State = pTwoXiuLianData->m_XiuLianState;
			AskData.m_uidActor = pTwoXiuLianData->m_uidSource;
			AskData.m_uidAcceptor = pTwoXiuLianData->m_uidFriend;
			AskData.m_uidFriendActor = pTwoXiuLianData->m_uidFriendActor;
			strncpy(AskData.m_uidFriendName,pTwoXiuLianData->m_szFriendName,sizeof(AskData.m_uidFriendName));
			strncpy(AskData.m_SourceName, pTwoXiuLianData->m_szSourceName, sizeof(AskData.m_SourceName));

			//对方是否在线
			AskData.m_bIsLine = false;
			if( m_pActor->GetUID() == pTwoXiuLianData->m_uidSource){
				if( g_pGameServer->GetGameWorld()->FindActor(pTwoXiuLianData->m_uidFriend)){
					AskData.m_bIsLine = true;
				}
			}else{
				if( g_pGameServer->GetGameWorld()->FindActor(pTwoXiuLianData->m_uidSource)){
					AskData.m_bIsLine = true;
				}
			}

			obData << AskData;

			Rsp.m_AskDataNum++;
		}

	}


	OBuffer4k ob;
	ob << XiuLian_Header(enXiuLianCmd_ViewAskData,sizeof(Rsp)+obData.Size()) << Rsp;

	if(obData.Size()>0)
	{
		ob.Push(obData.Buffer(),obData.Size());
	}

	m_pActor->SendData(ob.TakeOsb());
}


//获得独自修炼费用
INT32 XiuLianPart::GetAloneXLCharge(INT32 ActorNum,INT32 nHours)
{
	//收取费用=35*创建的角色等级^(1/2)*小时数（8或24、48）*帮派福利(精打细算)
	INT32 ActorLevel = m_pActor->GetCrtProp(enCrtProp_Level);

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return 35 * pow(ActorLevel, 0.5) * nHours + 0.999999;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());
	if( 0 != pSyndicate){
		INT32 nNeedMoney = 35 * pow(ActorLevel,0.5) * nHours + 0.999999;
		return (nNeedMoney - nNeedMoney * pSyndicate->GetWelfareValue(enWelfare_ReduceXiuLianMoney) / 100);
	}

	return 35 * pow(ActorLevel, 0.5) * nHours + 0.999999;
}

//获得双休费用
INT32 XiuLianPart::GetTwoXLCharge(INT32 nHours)
{
	int nPrice = 0;
	//收取费用=10*角色的等级^(1/2)*小时数（8或24）*帮派福利(精打细算)
	INT32 ActorLevel = m_pActor->GetCrtProp(enCrtProp_Level);

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return 30 * pow(ActorLevel, 0.5) * nHours + 0.999999;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());
	if( 0 != pSyndicate){
		INT32 nNeedMoney = 30 * pow(ActorLevel, 0.5) * nHours + 0.999999;
		return (nNeedMoney - nNeedMoney * pSyndicate->GetWelfareValue(enWelfare_ReduceXiuLianMoney) / 100);
	}

	nPrice = 30 * pow(ActorLevel, 0.5) * nHours + 0.999999;
	return nPrice;
}


//启动修炼定时器,,bCreateUser为是否创建时创建的 定时器
void  XiuLianPart::StartTimer(enXiuLianTimerID TimerID, bool bCreateUser)
{
	//获取修炼每几分钟更新一次
	UINT16 UpdateMinuteNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_UpdateMinuteNum;

	if( !bCreateUser){
		g_pGameServer->GetTimeAxis()->SetTimer(TimerID,this,UpdateMinuteNum * 60 * 1000,"XiuLianPart::StartTimer");	
	}else{
		//角色上线时创建定时器
		if( TimerID == enXiuLianTimerID_AloneXL){
			//单修
			UINT32 nElap = UpdateMinuteNum * 60 - (CURRENT_TIME() - m_XiuLianData.m_lastGetNimbusTime) % (UpdateMinuteNum * 60);

			g_pGameServer->GetTimeAxis()->SetTimer(TimerID,this,nElap * 1000,"XiuLianPart::StartTimer");

			m_bAloneTimerInCreateUser = true;
		}else{
			//双修
			UINT32 nElap = UpdateMinuteNum * 60 - (CURRENT_TIME() - m_XiuLianData.m_TwoLastGetNimbusTime) % (UpdateMinuteNum * 60);

			g_pGameServer->GetTimeAxis()->SetTimer(TimerID,this,nElap * 1000,"XiuLianPart::StartTimer");

			m_bTwoTimerInCreateUser = true;
		}
	}
}

//停止定时器
void XiuLianPart::StopTimer(enXiuLianTimerID TimerID)
{
	g_pGameServer->GetTimeAxis()->KillTimer(TimerID,this);
}

void XiuLianPart::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enXiuLianTimerID_AloneXL:
		{
			//获取单修灵气
			this->Take_AloneXLNimbus();

			if( m_bAloneTimerInCreateUser){
				//创建角色时创建的定时器时间间隔不同，要重新创建定时器
				this->StopTimer(enXiuLianTimerID_AloneXL);

				this->StartTimer(enXiuLianTimerID_AloneXL);

				m_bAloneTimerInCreateUser = false;
			}
		}
		break;
	case enXiuLianTimerID_TwoXL:
		{
			//获取双修灵气
			this->Take_TwoXLNimbus();

			if( m_bTwoTimerInCreateUser){
				//创建角色时创建的定时器时间间隔不同，要重新创建定时器
				this->StopTimer(enXiuLianTimerID_TwoXL);

				this->StartTimer(enXiuLianTimerID_TwoXL);

				m_bTwoTimerInCreateUser = false;
			}
		}
		break;
	default:
		break;
	}
}


//获得下一个序列号
UINT32  XiuLianPart::GetNextAskSeq()
{
	static UINT32 s_AskSeq = CURRENT_TIME();

	return ++s_AskSeq;

}

//根据序列号获得请求数据
STwoXiuLianData * XiuLianPart::GetAskXiuLianData(UINT32 AskSeq)
{
	return g_pThingServer->GetXiuLianMgr().GetAskXiuLianData(AskSeq);
}

 //发送邮件
 void XiuLianPart::SendEmail(enXiuLianType XiuLianType, const UID & uidDestUser, TMagicID MagicID)
 {
	 if( enXiuLianType_Two != XiuLianType && enXiuLianType_Magic != XiuLianType){
		return;
	 }

	SWriteData WriteData;

	WriteData.m_MailType = enMailType_Person;
	WriteData.m_UidDestUser = uidDestUser;


	switch(XiuLianType)
	{
	case enXiuLianType_Two:
		{

			// fly add	20121106
			sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10023), m_pActor->GetName());
			strncpy(WriteData.m_szThemeText,g_pGameServer->GetGameWorld()->GetLanguageStr(10000), sizeof(WriteData.m_szThemeText));
			//sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText), "%s邀请您进行双人修炼，请前往“修炼”页面查", m_pActor->GetName());
			//strncpy(WriteData.m_szThemeText,"双人修炼", sizeof(WriteData.m_szThemeText));
		}
		break;
	case enXiuLianType_Magic:
		{
			IMagicPart * pMagicPart = m_pActor->GetMagicPart();
			if( 0 == pMagicPart){
				return;
			}
			const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(MagicID);
			if( 0 == pMagicCnfg){
				TRACE("<error>%s,%d行,获取法术配置信息错误,MagicID = %d", __FUNCTION__,__LINE__,MagicID);
				return;
			}

			IMagic * pMagic = pMagicPart->GetMagic(MagicID);
			// fly add	20121106
			if( 0 == pMagic){
				sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10024), m_pActor->GetName(), g_pGameServer->GetGameWorld()->GetLanguageStr(pMagicCnfg->m_MagicLangID));				
				//sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText),"%s希望能学%s法术，请前往“修炼”页面查看", m_pActor->GetName(), pMagicCnfg->m_strName.c_str());
			}else{
				sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10024), m_pActor->GetName(), g_pGameServer->GetGameWorld()->GetLanguageStr(pMagicCnfg->m_MagicLangID));									
				//sprintf_s(WriteData.m_szContentText, sizeof(WriteData.m_szContentText),"%s希望能教%s法术，请前往“修炼”页面查看", m_pActor->GetName(), pMagicCnfg->m_strName.c_str());
			}
			strncpy(WriteData.m_szThemeText,g_pGameServer->GetGameWorld()->GetLanguageStr(10001), sizeof(WriteData.m_szThemeText));
			//strncpy(WriteData.m_szThemeText,"修炼法术", sizeof(WriteData.m_szThemeText));
		}
		break;
	}

	IMailPart * pMailPart = m_pActor->GetMailPart();
	if( 0 == pMailPart){
		return;
	}

	pMailPart->WriteMail(WriteData);
 }

//往请求列表加条记录
bool XiuLianPart::AddToAskList(UINT32 AskID)
{
	if( m_AskXiuLianData.insert(AskID).second == false){
		return false;
	}
	return true;
}

//移除请求列表中的记录
void XiuLianPart::RemoveAsk(UINT32 AskID)
{
	m_AskXiuLianData.erase(AskID);
}

 //获取双修灵气
 void XiuLianPart::Take_TwoXLNimbus()
 {
	IFriendPart * pFriendPart = m_pActor->GetFriendPart();
	if( 0 == pFriendPart){
	    return;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(m_XiuLianData.m_uidActor);
	if( 0 == pTargetActor){
		return;
	}

 	if( m_XiuLianData.m_TwoXiuLianState != enXiuLianState_Doing)
	{
		return;
	}

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	UINT32 nCurTime = CURRENT_TIME();

	nCurTime = std::min(nCurTime,m_XiuLianData.m_TwoEndTime);

	//获取修炼每几分钟更新一次
	UINT16 UpdateMinuteNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_UpdateMinuteNum;

	INT32 interval = (nCurTime - m_XiuLianData.m_TwoLastGetNimbusTime) / (60 * UpdateMinuteNum);

	const SXiuLianFriendAdd * pFriendAdd = g_pGameServer->GetConfigServer()->GetXiuLianFriendAddCnfg(pFriendPart->GetRelationNum(m_XiuLianData.m_uidFriend));
	if( 0 == pFriendAdd){
		return;
	}

	INT32 nAddNimbus = GameParam.m_TwoXLNimbusParam * (pTargetActor->GetCrtProp(enCrtProp_ActorNimbusSpeed) / 1000.0f) * interval;
	nAddNimbus += (nAddNimbus * pFriendAdd->m_AddNum / 100);

	m_XiuLianData.m_TwoLastGetNimbusTime = nCurTime;

	nAddNimbus = nAddNimbus * m_pActor->GetWallowFactor() + 0.99999;  //防沉迷

	const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorLayer));
	if( 0 == pLayerCnfg){
		TRACE("<error> %s : %d 行, 境界配置表获取错误!!,境界=%d",__FUNCTION__, __LINE__, pTargetActor->GetCrtProp(enCrtProp_ActorLayer));
		return;
	}
	//灵气上限验证
	if( (pTargetActor->GetCrtProp(enCrtProp_ActorNimbus) + nAddNimbus) > pLayerCnfg->m_NimbusUp){
		nAddNimbus = pLayerCnfg->m_NimbusUp - pTargetActor->GetCrtProp(enCrtProp_ActorNimbus);
	}

	m_XiuLianData.m_TwoTotalNimbus += nAddNimbus;

	//玩家获得灵气值
	if(nAddNimbus>0)
	{	
		pTargetActor->AddCrtPropNum(enCrtProp_ActorNimbus, nAddNimbus);
	}

	//同步
	this->SyncTwoXiuLianData();

	if( nCurTime >= m_XiuLianData.m_TwoEndTime){
		this->FinishTwoXiuLian();
	}
 }

 //获取单修灵气
 void XiuLianPart::Take_AloneXLNimbus()
 {
	 const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

 	if(m_XiuLianData.m_AloneXiuLianState != enXiuLianState_Doing){
		return;
	}

	UINT32 nCurTime = CURRENT_TIME();

	nCurTime = std::min(nCurTime,m_XiuLianData.m_EndTime);

	//获取修炼每几分钟更新一次
	UINT16 UpdateMinuteNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_UpdateMinuteNum;

	INT32 interval = (nCurTime - m_XiuLianData.m_lastGetNimbusTime)/(UpdateMinuteNum * 60);

	if(interval > 0)
	{
		m_XiuLianData.m_lastGetNimbusTime += interval * UpdateMinuteNum * 60;

		for(int i=0; i < ARRAY_SIZE(m_XiuLianData.m_AloneXLData); ++i)
		{
			if( !m_XiuLianData.m_AloneXLData[i].m_ActorUID.IsValid()){
				break;
			}

			IActor * pTmpActor = g_pGameServer->GetGameWorld()->FindActor(m_XiuLianData.m_AloneXLData[i].m_ActorUID);
			if( 0 == pTmpActor){
				break;
			}

			INT32 GetNimbus = interval * GameParam.m_AloneXLNimbusParam * (pTmpActor->GetCrtProp(enCrtProp_ActorNimbusSpeed) / 1000.0f);

			GetNimbus = (GetNimbus + GetNimbus * pTmpActor->GetCrtProp(enCrtProp_SynMagicAloneXLParam) / 1000.0f + GetNimbus * m_pActor->GetVipValue(enVipType_AddXLNimbus) / 100.0f) * m_pActor->GetWallowFactor() + 0.99999;  //防沉迷

			const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTmpActor->GetCrtProp(enCrtProp_ActorLayer));
			if( 0 == pLayerCnfg){
				TRACE("<error> %s : %d 行, 境界配置表获取错误!!,境界=%d",__FUNCTION__, __LINE__, pTmpActor->GetCrtProp(enCrtProp_ActorLayer));
				return;
			}
			//灵气上限验证
			if ( (pTmpActor->GetCrtProp(enCrtProp_ActorNimbus) + GetNimbus) > pLayerCnfg->m_NimbusUp){
				GetNimbus = pLayerCnfg->m_NimbusUp - pTmpActor->GetCrtProp(enCrtProp_ActorNimbus);
			}

			m_XiuLianData.m_AloneXLData[i].m_GetNimbus += GetNimbus;

			if(GetNimbus > 0)
			{
				pTmpActor->AddCrtPropNum(enCrtProp_ActorNimbus,GetNimbus );
			}
		}

		////获得的仙剑灵气
		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return;
		}

		INT32 nAddGodSwordNimbus = interval * GameParam.m_AloneXLGodSwordNimbusParam;

		ISyndicate * pSyn = pSynMgr->GetSyndicate(m_pActor->GetUID());
		if( 0 != pSyn){
			nAddGodSwordNimbus += (interval * (GameParam.m_AloneXLGodSwordNimbusParam * pSyn->GetWelfareValue(enWelfare_AloneXiuLianGodSwordNimbus) / 100.0f) + 0.99999);
		}

		nAddGodSwordNimbus = (nAddGodSwordNimbus + (nAddGodSwordNimbus * m_pActor->GetVipValue(enVipType_AddXLNimbus) / 100.0f)) * m_pActor->GetWallowFactor() + 0.99999;	//防沉迷

		INT32 nMaxGodSwordNimbus = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;
		if( (nAddGodSwordNimbus + m_pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus)) > nMaxGodSwordNimbus){
			//超过仙剑灵气的最大值
			nAddGodSwordNimbus = nMaxGodSwordNimbus - m_pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus);
		}

		m_XiuLianData.m_GetGodSwordNimbus += nAddGodSwordNimbus;

		m_pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, nAddGodSwordNimbus);

		this->SyncAloneXiuLianData();

		TRACE("玩家%s,修炼获得仙剑灵气 : %d", m_pActor->GetName(), nAddGodSwordNimbus);
	}


	if(nCurTime >= m_XiuLianData.m_EndTime)
	{
		//结束了
		this->FinishAloneXiuLian();

		this->SyncAloneXiuLianData();
	}
 }

 //请求双修数据库回调
void	XiuLianPart::HandleTwoXiuLian(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	DB_GetActorFacade  GetActorFacade;
	RspIb >> RspHeader >> OutParam >> GetActorFacade;

	if( RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_TwoXiuLian_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	if( m_XiuLianData.m_TwoXiuLianState == enXiuLianState_Doing){
		return;
	}else if(m_XiuLianData.m_uidFriend.ToUint64() != GetActorFacade.m_ActorUID){
		return;
	}else{
		m_pActor->AddCrtPropNum(enCrtProp_ActorStone,-GetTwoXLCharge(m_XiuLianData.m_TwoXiuLianHours));

		m_XiuLianData.m_TwoXiuLianState = enXiuLianState_Doing;
		m_XiuLianData.m_FriendFacade	= GetActorFacade.m_ActorFacade;
   
		SyncTwoXiuLianData();

		//开启定时器
		this->StartTimer(enXiuLianTimerID_TwoXL);

		//通知客户端显示修炼公告
		this->NoticeViewXiuLianMsg(enXiuLianType_Two, m_XiuLianData.m_TwoXiuLianHours * 3600);

		IFriendPart * pFriendPart = m_pActor->GetFriendPart();
		if( 0 == pFriendPart){
			return;
		}

		const SFriendBasicInfo * pFriendBasicInfo = pFriendPart->GetFriendInfo(m_XiuLianData.m_uidFriend);
		if( 0 == pFriendBasicInfo){
			return;
		}

		//发布事件
		SS_XiuLianFinish XiuLianFinish;
		XiuLianFinish.m_XiuLianType = enXiuLianType_Two;

		if( m_pActor->GetCrtProp(enCrtProp_ActorUserID) == pFriendBasicInfo->m_Sex){
			XiuLianFinish.m_bYiXing = false;
		}else{
			XiuLianFinish.m_bYiXing = true;
		}


		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_XiuLian);
		m_pActor->OnEvent(msgID,&XiuLianFinish,sizeof(XiuLianFinish));
	}

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_AskTwoXiuLian,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//通知客户端显示修炼公告
void	XiuLianPart::NoticeViewXiuLianMsg(enXiuLianType XiuLianType, UINT32 RemainTime)
{
	ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
	if( 0 == pSystemMsg){
		return;
	}

	SXiuLianMsg XiuLianMsg;
	XiuLianMsg.m_XiuLianType = XiuLianType;
	XiuLianMsg.m_RemainTime  = RemainTime;

	pSystemMsg->ViewMsg(m_pActor, enMsgType_XiuLian, &XiuLianMsg);
}

//中途取消显示修炼公告
void	XiuLianPart::NoticeCancelViewXLMsg(enXiuLianType m_XiuLianType)
{
	ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
	if( 0 == pSystemMsg){
		return;
	}

	pSystemMsg->CancelViewXiuLianMsg(m_pActor, m_XiuLianType);
}
