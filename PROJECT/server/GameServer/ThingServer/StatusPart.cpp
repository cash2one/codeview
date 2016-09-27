
#include "StatusPart.h"
#include "IActor.h"
#include "ICombatServer.h"
#include "ThingServer.h"
#include "IStatus.h"
#include "IEffect.h"
#include "IBasicService.h"
#include "IConfigServer.h"
#include "XDateTime.h"
#include <sstream>
#include "ISystemMsg.h"

StatusPart::StatusPart()
{
	m_pActor = 0;
}

StatusPart::~StatusPart()
{
	for( StatusMap::iterator iter = m_status.begin(); iter != m_status.end(); ++iter)
	{
		IStatus * pStatus = iter->second;
		if( 0 == pStatus){
			continue;
		}

		pStatus->Release();
	}

	m_status.clear();

	 for(EffectList::iterator it = m_effectList.begin(); it != m_effectList.end(); ++it)
	 {
		 IEffect * pEffect = *it;
		 if(pEffect)
		 {			 
			 pEffect->End();
			 pEffect->Release();			 
		 }
	 }

	 m_effectList.clear();
}




//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool StatusPart::Create(IThing *pMaster, void *pContext, int nLen)
{

	if(pMaster == 0 )
	{
		return false;
	}

	m_pActor = (ICreature*)pMaster;

	if( pContext == 0 ||  nLen < sizeof(SDB_Get_StatusNum_Rsp))
	{
		return true;
	}
	

	SStatusPart * pStatusPart = (SStatusPart *)pContext;

	if( 0 == pStatusPart->m_Len){
		return true;
	}

	if( 0 == pStatusPart->m_pData){
		return false;
	}

	UINT32 nNowTime = CURRENT_TIME();

	int nCount = pStatusPart->m_Len / sizeof(SDB_Get_StatusData_Rsp);

	SDB_Get_StatusData_Rsp * pStatusData_Rsp = (SDB_Get_StatusData_Rsp *)pStatusPart->m_pData;

	for(int i=0; i<nCount; ++i,pStatusData_Rsp++)
	{	

		if( nNowTime >= pStatusData_Rsp->m_EndStatusTime){
			continue;
		}

		this->__CreateStatus(*pStatusData_Rsp);
	}

	return true;
}

//释放
void StatusPart::Release(void)
{
	delete this;
}


//取得部件ID
enThingPart StatusPart::GetPartID(void)
{
	return enThingPart_Crt_Status;
}


//取得本身生物
IThing*		StatusPart::GetMaster(void)
{
	return m_pActor;
}


//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool StatusPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void StatusPart::InitPrivateClient()
{
	StatusMap::iterator iter = m_status.begin();

	for(; iter != m_status.end(); ++iter)
	{
		this->Check_ViewEffectMsg(iter->first);
	}
}



//玩家下线了，需要关闭该ThingPart
void StatusPart::Close()
{
}

//保存数据
void StatusPart::SaveData()
{
	if( m_pActor->GetThingClass() != enThing_Class_Actor){
		return;
	}

	SDB_Delete_AllStatus_Req DBReq;

	DBReq.m_uidUser = m_pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(((IActor *)m_pActor)->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteAllStatus,ob.TakeOsb(),0,0);	

	StatusMap::iterator iter = m_status.begin();

	for( ; iter != m_status.end(); ++iter)
	{
		IStatus* pStatus = iter->second;
		if( 0 == pStatus){
			continue;
		}

		pStatus->SaveData();
	}
}

//增加一个状态
 bool StatusPart::AddStatus(TStatusID lID, UID uidCreator,std::vector<UINT8> & vectDeleteStatusType, TMagicID MagicID,INT32 StatusRoundNum)
 {
	 INT32 size = m_effectList.size();

	const SStatusCnfg* pStatusInfo = g_pGameServer->GetConfigServer()->GetStatusCnfg(lID);

	if( pStatusInfo == 0){
		TRACE("<error> %s : %d line 找不到状态配置信息 StatusID = %d",__FUNCTION__,__LINE__,lID);
		return false;
	}

	if(StatusRoundNum==0)
	{
		StatusRoundNum = pStatusInfo->m_RoundNum;
	}

	IStatus* pStatus = FindStatus(pStatusInfo->m_StatusID);
	if( pStatus != 0 ){
		//已经拥有这个状态时
		this->UseAlreadyHaveStatus(pStatus);

		return true;
	}

	//状态组
	std::vector<TStatusGroupID>	 vectGroupID = pStatusInfo->m_vectGroupID;

	std::map<TStatusGroupID, std::vector<IStatus *>> mapGroupStatus;

	//得到相同状态组的状态
	this->__GetSameGroupStatus(vectGroupID, mapGroupStatus);

	bool bCreateNewStatus = false;

	//组处理
	if( this->__CalStatusGroup(pStatusInfo, mapGroupStatus, bCreateNewStatus, vectDeleteStatusType) && bCreateNewStatus){
		//加上新状态,并返回
		return this->__AddNewStatus(pStatusInfo, uidCreator, MagicID,StatusRoundNum);
	}

	return false; 
 }

//增加一个状态,通过使用物品得到的   bOk是否是点击确认使用
bool  StatusPart::AddStatus_UseGoods(TStatusID lID, UID uidCreator, SAddStatus & AddStatus)
{
	const SStatusCnfg* pStatusInfo = g_pGameServer->GetConfigServer()->GetStatusCnfg(lID);
	if(pStatusInfo==0){
		TRACE("<error> %s : %d line 找不到状态配置信息 StatusID = %d",__FUNCTION__,__LINE__,lID);
		return false;
	}

	//已存在该状态,则只是增加时间或回合数
	IStatus* pStatus = FindStatus(lID);
	if( pStatus !=0 ){
		//已经拥有这个状态时
		for( int i = 0; i < AddStatus.m_UseGoodsNum; ++i)
		{
			this->UseAlreadyHaveStatus(pStatus);
		}

		if( pStatusInfo->m_bAllActor == 1 && m_pActor->GetThingClass() == enThing_Class_Actor && ((IActor *)m_pActor)->GetMaster() == 0){
			//如果对所有角色有效，则也增加招募角色状态
			for( int  k = 0; k < MAX_EMPLOY_NUM; ++k)
			{
				IActor * pEmployee = ((IActor *)m_pActor)->GetEmployee(k);
				if( 0 == pEmployee){
					continue;
				}

				IStatusPart * pEmployStatusPart = (IStatusPart *)pEmployee->GetPart(enThingPart_Crt_Status);
				if( 0 == pEmployStatusPart){
					continue;
				}

				IStatus* pEmpStatus = pEmployStatusPart->FindStatus(lID);
				if( 0 == pEmpStatus){
					continue;
				}

				for( int i = 0; i < AddStatus.m_UseGoodsNum; ++i)
				{
					pEmployStatusPart->UseAlreadyHaveStatus(pEmpStatus);
				}
			}
		}

		
		AddStatus.m_PacketRetCode = enPacketRetCode_OK;

		//this->Check_CancelViewEffectMsg(lID);

		this->Check_ViewEffectMsg(lID);

		return true;
	}

	//按状态组处理(使用物品)
	if( this->__GroupHandle_UseGoods(pStatusInfo, uidCreator, AddStatus)){
		//如果加状态成功，则多个物品使用的话，叠加状态时间
		IStatus * pGoodsStatus = FindStatus(lID);
		if( 0 == pGoodsStatus){
			return false;
		}
		
		for( int i = 1; i < AddStatus.m_UseGoodsNum; ++i)
		{
			//已经拥有这个状态时
			this->UseAlreadyHaveStatus(pGoodsStatus);

			if( pStatusInfo->m_bAllActor == 1 && m_pActor->GetThingClass() == enThing_Class_Actor && ((IActor *)m_pActor)->GetMaster() == 0){
				//如果对所有角色有效，则也增加招募角色状态
				for( int  k = 0; k < MAX_EMPLOY_NUM; ++k)
				{
					IActor * pEmployee = ((IActor *)m_pActor)->GetEmployee(k);
					if( 0 == pEmployee){
						continue;
					}

					IStatusPart * pEmployStatusPart = (IStatusPart *)pEmployee->GetPart(enThingPart_Crt_Status);
					if( 0 == pEmployStatusPart){
						continue;
					}

					IStatus* pEmpStatus = pEmployStatusPart->FindStatus(lID);
					if( 0 == pEmpStatus){
						continue;
					}

					pEmployStatusPart->UseAlreadyHaveStatus(pEmpStatus);
				}
			}
		}

		this->Check_ViewEffectMsg(lID);

		return true;
	}

	return false;
}

//从数据库获取状态数据创建
bool  StatusPart::__CreateStatus(SDB_Get_StatusData_Rsp Rsp)
{
	const SStatusCnfg* pStatusInfo = g_pGameServer->GetConfigServer()->GetStatusCnfg(Rsp.m_StatusID);

	 if( 0 == pStatusInfo){
		 TRACE("<error> %s : %d line 找不到状态配置信息 StatusID = %d",__FUNCTION__,__LINE__,Rsp.m_StatusID);
		 return false;
	 }

	 if( pStatusInfo->m_RecordType != enStatusRocordType_Time)
	 {
	 	 const SStatusTypeCnfg * pStatusTypeCnfg = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(pStatusInfo->m_StatusType);
		 if( 0 == pStatusTypeCnfg){
			 TRACE("<error> %s : %d line 找不到状态类型配置信息 StatusID = %d",__FUNCTION__,__LINE__,Rsp.m_StatusID);
			 return false;
		 }
	 }


	 SStatusInfo StatusInfo;
	 StatusInfo.m_bAlreadyStart = false;
	 StatusInfo.m_EndStatusTime = Rsp.m_EndStatusTime;
	 StatusInfo.m_LeftRoundNum  = 0;
	 StatusInfo.m_MagicID		= 0;
	 StatusInfo.m_StatusID		= Rsp.m_StatusID;
	 StatusInfo.m_uidCreator	= UID(Rsp.m_UidCreator);

	 IStatus * pStatus = g_pGameServer->GetCombatServer()->CreateStatus(StatusInfo);
	 if( 0 == pStatus){
		return false;
	 }

	 if( false == __StartStatus(pStatus, StatusInfo.m_uidCreator,0,0)){
		return false;
	 }

	 return true;
}

bool StatusPart::__StartStatus(IStatus * pStatus,UID uidCreator,TMagicID MagicID,INT32  StatusRoundNum)
{
	const SStatusCnfg * pStatusInfo = pStatus->GetStatusCnfg();
	//启动
	if( pStatus->Start(this,uidCreator,MagicID) == false){

		TRACE("<error> %s : %d 启动状态失败 statusid = %d",__FUNCTION__,__LINE__,pStatusInfo->m_StatusID);

		return false;
	}

	if(m_status.insert(std::make_pair(pStatusInfo->m_StatusID, pStatus)).second==false)
	{
		TRACE("<error> %s : %d 增加状态失败 statusid = %d",__FUNCTION__,__LINE__,pStatusInfo->m_StatusID);
		pStatus->Release();
		return false;
	}

	pStatus->SetLeftRoundNum(StatusRoundNum);

	//通知客户端
	this->__SendAddStatus(pStatus);

	 UINT32 MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddStatus);
	 SS_AddStatus AddStts;
	 AddStts.m_uidActor = m_pActor->GetUID();
	 AddStts.m_StatusID = pStatusInfo->m_StatusID;
	 AddStts.m_StatusType = pStatusInfo->m_StatusType;
	 AddStts.m_MagicID = MagicID;
	 AddStts.m_StatusTime = pStatusInfo->m_TimeNum;

	 m_pActor->OnEvent(MsgID,&AddStts,sizeof(AddStts));

	return true;
}

//按组处理(物品使用)    bOk是否是点击确认使用
bool StatusPart::__GroupHandle_UseGoods(const SStatusCnfg* pStatusInfo, UID uidCreator, SAddStatus & AddStatus)
{
	const std::vector<TStatusGroupID> & vectGroupID = pStatusInfo->m_vectGroupID;

	bool bOk = true;

	++AddStatus.m_Index;

	for( ;AddStatus.m_Index <= vectGroupID.size(); ++AddStatus.m_Index)
	{
		const SStatusGroupCnfg * pGroupCnfg = g_pGameServer->GetConfigServer()->GetStatusGroupCnfg(vectGroupID[AddStatus.m_Index - 1]);
		if( 0 == pGroupCnfg){
			TRACE("<error> %s : %d line 找不到状态组配置信息 StatusGroupID = %d",__FUNCTION__,__LINE__,vectGroupID[AddStatus.m_Index - 1]);
			return false;
		}

		//获取状态组可容纳多少状态
		UINT8	MaxStatusNum = pGroupCnfg->m_MaxStatusNum;
		
		//身上已经有该状态组的状态
		std::vector<IStatus *> vectIStatus;

		for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
		{
			IStatus* pTmpStatus = (*it).second;

			const SStatusCnfg * pStatusCnfg2 = pTmpStatus->GetStatusCnfg();

			for( int n = 0; n < pStatusCnfg2->m_vectGroupID.size(); ++n)
			{
				if( pStatusCnfg2->m_vectGroupID[n] == vectGroupID[AddStatus.m_Index - 1]){
					vectIStatus.push_back(pTmpStatus);
				}
			}

		}
		
		if( vectIStatus.size() < MaxStatusNum){
			continue;
		}

		AddStatus.m_PacketRetCode = enPacketRetCode_FullStatusGroup;

		bOk = false;

		std::ostringstream os;

		if( vectIStatus.size() == 1){
			IStatus* pTmpStatus = vectIStatus[0];

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(pStatusInfo->m_StatusNameLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10056) << g_pGameServer->GetGameWorld()->GetLanguageStr(pTmpStatus->GetStatusCnfg()->m_StatusNameLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10057);		
			//os << pStatusInfo->m_strName << "将替换" << pTmpStatus->GetStatusCnfg()->m_strName << ",确定使用？";
			AddStatus.m_strUserDesc = os.str(); 

			AddStatus.m_bReplace = true;		
		}
		//else{
		//	os << "请先从 ";
		//	for( int k = 0; k < vectIStatus.size(); ++k)
		//	{
		//		os << vectIStatus[k]->GetStatusCnfg()->m_strName << ",";
		//	}

		//	os << "删除一种";
		//	AddStatus.m_strUserDesc = os.str();
		//}

		break;
	}

	if( bOk){
		//删除被替换的状态
		for( int i = 0; i < vectGroupID.size(); ++i)
		{
			const SStatusGroupCnfg * pGroupCnfg = g_pGameServer->GetConfigServer()->GetStatusGroupCnfg(vectGroupID[i]);
			if( 0 == pGroupCnfg){
				TRACE("<error> %s : %d line 找不到状态组配置信息 StatusGroupID = %d",__FUNCTION__,__LINE__,vectGroupID[i]);
				return false;
			}

			//获取状态组可容纳多少状态
			UINT8	MaxStatusNum = pGroupCnfg->m_MaxStatusNum;

			//身上已经有该状态组的状态
			std::vector<IStatus *> vectIStatus;

			for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
			{
				IStatus* pTmpStatus = (*it).second;

				const SStatusCnfg * pStatusCnfg2 = pTmpStatus->GetStatusCnfg();

				for( int n = 0; n < pStatusCnfg2->m_vectGroupID.size(); ++n)
				{
					if( pStatusCnfg2->m_vectGroupID[n] == vectGroupID[i]){
						vectIStatus.push_back(pTmpStatus);
					}
				}
			}

			if( vectIStatus.size() == 1 && vectIStatus.size() == MaxStatusNum){

				this->Check_CancelViewEffectMsg(vectIStatus[0]->GetStatusCnfg()->m_StatusID);

				this->RemoveStatus(vectIStatus[0]->GetStatusCnfg()->m_StatusID);
			}else if(vectIStatus.size() < MaxStatusNum){
				continue;
			}else{
				AddStatus.m_PacketRetCode = enPacketRetCode_Error;
				return false;
			}
		}

		//创建并增加到玩家身上
		if( this->__AddNewStatus(pStatusInfo, uidCreator, 0)){
			AddStatus.m_PacketRetCode = enPacketRetCode_OK;

			if( pStatusInfo->m_bAllActor){
				//如果状态对所有角色有效，则也给招募角色加上

				if( m_pActor->GetThingClass() == enThing_Class_Actor && ((IActor *)m_pActor)->GetMaster() == 0){
					
					for( int index = 0; index < MAX_EMPLOY_NUM; ++index)
					{
						IActor * pEmployee = ((IActor *)m_pActor)->GetEmployee(index);
						if( 0 == pEmployee){
							continue;
						}

						IStatusPart * pEmployStatusPart = (IStatusPart *)pEmployee->GetPart(enThingPart_Crt_Status);
						if( 0 == pEmployStatusPart){
							continue;
						}

						IStatus * pStatus = this->FindStatus(pStatusInfo->m_StatusID);
						if( 0 == pStatus){
							return false;
						}

						std::vector<UINT8> vectDeleteStatusType;

						pEmployStatusPart->AddStatus(pStatusInfo->m_StatusID, uidCreator, vectDeleteStatusType);
					}
				}
			}


			return true;
		}else{
			AddStatus.m_PacketRetCode = enPacketRetCode_Error;
		}
	}

	return false;
}

//给玩家加上一种新状态
bool StatusPart::__AddNewStatus(const SStatusCnfg* pStatusInfo, UID uidCreator,TMagicID MagicID,INT32 StatusRoundNum )
{
	if(StatusRoundNum==0)
	{
		StatusRoundNum = pStatusInfo->m_RoundNum;
	}
	IStatus * pStatus = g_pGameServer->GetCombatServer()->CreateStatus(pStatusInfo->m_StatusID);

	if( pStatus == 0){
		return false;
	}

	
	return __StartStatus(pStatus,uidCreator,MagicID,StatusRoundNum);
}

//发送同步状态消息
void StatusPart::__SendSycStatus(IStatus * pStatus)
{
	SC_SycStatus  SycStatus;
	SycStatus.m_EndTime		= pStatus->GetEndTimeNum();
	SycStatus.m_LeftRound	= pStatus->GetLeftRoundNum();
	SycStatus.m_RocordType	= pStatus->GetStatusRecordType();
	SycStatus.m_StatusID	= pStatus->GetStatusCnfg()->m_StatusID;

 	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_SycStatus,sizeof(SycStatus)) << SycStatus ;
	
	if(m_pActor->GetThingClass()==enThing_Class_Actor)
	{
		((IActor*)m_pActor)->SendData(ob.TakeOsb());	
	}
}

//发送增加玩家状态消息
void StatusPart::__SendAddStatus(IStatus * pStatus)
{
	SC_AddStatus AddStatus;
	AddStatus.m_StatusID = pStatus->GetStatusCnfg()->m_StatusID;

 	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_AddStatus,sizeof(AddStatus)) << AddStatus ;
	if(m_pActor->GetThingClass()==enThing_Class_Actor)
	{
		((IActor*)m_pActor)->SendData(ob.TakeOsb());	
	}
}

	//删除状态
 bool StatusPart::RemoveStatus(TStatusID lStatusID, UID uidCreator )
 {
	 StatusMap::iterator it = m_status.find(lStatusID);
	 if(it == m_status.end())
	 {
		 return false;
	 }
	 

	 IStatus * pStatus = (*it).second;

	 UINT32 MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RemoveStatus);
	 SS_RemoveStatus RemoveStts;
	 RemoveStts.m_uidActor = m_pActor->GetUID();
	 RemoveStts.m_StatusID = lStatusID;

	 if(pStatus)
	 {		    
	   
		 RemoveStts.m_StatusType = pStatus->GetStatusCnfg()->m_StatusType;

		 //释放状态
		 pStatus->Release();
	 }
	 
	 m_status.erase(it);

	 m_pActor->OnEvent(MsgID,&RemoveStts,sizeof(RemoveStts));

	 return true;
 }

	//根据状态ID进行查询
 IStatus* StatusPart::FindStatus(TStatusID lStatusID)
 {
	  StatusMap::iterator it = m_status.find(lStatusID);
	 if(it == m_status.end())
	 {
		 return 0;
	 }

	 IStatus * pStatus = (*it).second;

	 return pStatus;
 }

	//效果控制接口！直接给生物加效果，生物销毁的时候会自动销毁效果，外部不要控制效果的生命周期！！！！
 bool	StatusPart::AddEffect(TEffectID effID)
 {
	 IEffect * pEffect = g_pGameServer->GetCombatServer()->CreateEffect(effID);
	 if(pEffect==0)
	 {
		 return false;
	 }

	 if(pEffect->Start(m_pActor,m_pActor->GetUID(),0)==false)
	 {
		 pEffect->Release();
		 return false;
	 }

	 m_effectList.push_back(pEffect);

	  return true;
 }

 void	StatusPart::RemoveEffect(TEffectID effID)
 {
	 for(EffectList::iterator it = m_effectList.begin(); it != m_effectList.end(); ++it)
	 {
		 IEffect * pEffect = *it;
		 if(pEffect->GetEffectID() == effID)
		 {
			 m_effectList.erase(it);
			 pEffect->End();
			 pEffect->Release();
			 return ;
		 }
	 }

 }

 	//获得生物已有状态中优先级最高的一个状态类型
const SStatusTypeCnfg * StatusPart::GetMaxPriorityStatusTypeCnfg()
{
	const SStatusTypeCnfg * pTypeCnfg = 0;

	 for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
	 {
		  IStatus* pStatus = (*it).second;
		 const SStatusTypeCnfg * pStatusTypeCnfg2 = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(pStatus->GetStatusCnfg()->m_StatusType);
		
		 if(pTypeCnfg==0 || pStatusTypeCnfg2 && pStatusTypeCnfg2->m_Priority > pTypeCnfg->m_Priority )
		  {
			  pTypeCnfg = pStatusTypeCnfg2;			 
		  }		  
	 }

	return pTypeCnfg;
}

//打开人物面板，显示状态
void	StatusPart::ShowStatusOpenUserPanel()
{
	if( m_pActor->GetThingClass() != enThing_Class_Actor){
		return;
	}

	SC_ViewActorStatus_Rsp Rsp;

	OBuffer2k ob;

	for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
	{
		IStatus * pStatus = it->second;
		if( 0 == pStatus){
			continue;
		}

		const SStatusCnfg * pStatusCnfg = pStatus->GetStatusCnfg();
		if( 0 == pStatusCnfg){
			continue;
		}

		if( 0 == pStatusCnfg->m_bShowInUserPanel){
			continue;
		}

		for( int i = 0; i < pStatusCnfg->m_vectEffect.size(); ++i)
		{
			const SEffectCnfg * pEffectCnfg = g_pGameServer->GetConfigServer()->GetEffectCnfg(pStatusCnfg->m_vectEffect[i]);
			if( 0 == pEffectCnfg){
				continue;
			}

			StatusShowInfo StatusInfo;

			// fly add	20121106
			strncpy(StatusInfo.m_Name, g_pGameServer->GetGameWorld()->GetLanguageStr(pEffectCnfg->m_EffectDescLangID), sizeof(StatusInfo.m_Name));
			//strncpy(StatusInfo.m_Name, pEffectCnfg->m_Descript.c_str(), sizeof(StatusInfo.m_Name));
			StatusInfo.m_RemainTime = pStatus->GetEndTimeNum() - CURRENT_TIME();
			StatusInfo.m_StatusID	= pStatusCnfg->m_StatusID;
			StatusInfo.m_TotalTime  = StatusInfo.m_RemainTime + (pStatusCnfg->m_TimeNum - StatusInfo.m_RemainTime % pStatusCnfg->m_TimeNum);

			ob << StatusInfo;

			++Rsp.m_Num;
		}
	}

	//if( ((IActor*)m_pActor)->GetMaster() != 0)
	//{
	//	//获取主角身上对所有角色共用的状态
	//	IActor * pMaster = ((IActor *)m_pActor)->GetMaster();
	//	if( 0 == pMaster){
	//		return;
	//	}

	//	IStatusPart * pStatusPart = (IStatusPart *)pMaster->GetPart(enThingPart_Crt_Status);
	//	if( 0 == pStatusPart){
	//		return;
	//	}

	//	std::vector<StatusShowInfo> vectStatus = pStatusPart->GetAllActorStatus();

	//	for( int i = 0; i < vectStatus.size(); ++i)
	//	{
	//		ob << vectStatus[i];

	//		++Rsp.m_Num;
	//	}
	//}

	OBuffer2k ob2;
	ob2 << Equip_Header(enEquipCmd_ViewActorStatus,sizeof(Rsp) + ob.Size()) << Rsp;

	if( ob.Size() > 0){
		ob2 << ob;
	}

	((IActor *)m_pActor)->SendData(ob2.TakeOsb());
}

//检测是否要通知客户端取消效果公告
void StatusPart::Check_CancelViewEffectMsg(TStatusID lStatusID)
{
	IStatus * pStatus = this->FindStatus(lStatusID);
	if( 0 == pStatus){
		return;
	}

	if( pStatus->GetStatusRecordType() == enStatusRocordType_Time && m_pActor->GetThingClass() == enThing_Class_Actor)
	{
		const SStatusCnfg * pStatusCnfg = pStatus->GetStatusCnfg();
		if( 0 == pStatusCnfg){
			return;
		}

		ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
		if( 0 == pSystemMsg){
			return;
		}

		for( int i = 0; i < pStatusCnfg->m_vectEffect.size(); ++i)
		{
			const SEffectCnfg * pEffectCnfg = g_pGameServer->GetConfigServer()->GetEffectCnfg(pStatusCnfg->m_vectEffect[i]);
			if( 0 == pEffectCnfg){
				continue;
			}

			if( !pEffectCnfg->m_bShowMsg){
				continue;
			}

			pSystemMsg->CancelViewEffectMsg((IActor *)m_pActor, pStatusCnfg->m_vectEffect[i]); 
		}
	}
}

//检测是否要显示公告，要的话并显示
void StatusPart::Check_ViewEffectMsg(TStatusID lStatusID)
{
	IStatus * pStatus = this->FindStatus(lStatusID);
	if( 0 == pStatus){
		return;
	}
	
	const SStatusCnfg* pStatusInfo = pStatus->GetStatusCnfg();
	if( 0 == pStatusInfo){
		return;
	}

	//检测是否要显示在跑马灯公告栏
	if( m_pActor->GetThingClass() == enThing_Class_Actor && pStatusInfo->m_RecordType == enStatusRocordType_Time && ((IActor *)m_pActor)->GetMaster() == 0){
		ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
		if( 0 == pSystemMsg){
			return;
		}

		for( int i = 0; i < pStatusInfo->m_vectEffect.size(); ++i)
		{
			const SEffectCnfg * pEffectCnfg = g_pGameServer->GetConfigServer()->GetEffectCnfg(pStatusInfo->m_vectEffect[i]);
			if( 0 == pEffectCnfg){
				continue;
			}

			if( !pEffectCnfg->m_bShowMsg){
				continue;
			}

			SEffectMsg EffectMsg;
			EffectMsg.m_EffectID = pEffectCnfg->m_EffectID;
			EffectMsg.m_RemainTime = pStatus->GetEndTimeNum() - CURRENT_TIME();

			// fly add	20121106

			strncpy(EffectMsg.m_EffectName, g_pGameServer->GetGameWorld()->GetLanguageStr(pEffectCnfg->m_EffectDescLangID), sizeof(EffectMsg.m_EffectName));
			//strncpy(EffectMsg.m_EffectName, pEffectCnfg->m_Descript.c_str(), sizeof(EffectMsg.m_EffectName));

			if( pStatusInfo->m_bAllActor && ((IActor *)m_pActor)->GetMaster() != 0){
				continue;
			}

			pSystemMsg->ViewMsg((IActor *)m_pActor, enMsgType_Effect, &EffectMsg);
		}
	}
}


//得到对所有角色共用的状态
std::vector<StatusShowInfo> StatusPart::GetAllActorStatus()
{
	std::vector<StatusShowInfo> vectStatus;

	for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
	{
		IStatus * pStatus = it->second;
		if( 0 == pStatus){
			continue;
		}

		const SStatusCnfg * pStatusCnfg = pStatus->GetStatusCnfg();
		if( 0 == pStatusCnfg){
			continue;
		}

		if( 0 == pStatusCnfg->m_bShowInUserPanel || 0 == pStatusCnfg->m_bAllActor){
			continue;
		}

		StatusShowInfo StatusInfo;

		// fly add	20121106
		strncpy(StatusInfo.m_Name, g_pGameServer->GetGameWorld()->GetLanguageStr(pStatusCnfg->m_StatusNameLangID), sizeof(StatusInfo.m_Name));
		//strncpy(StatusInfo.m_Name, pStatusCnfg->m_strName.c_str(), sizeof(StatusInfo.m_Name));
		StatusInfo.m_RemainTime = pStatus->GetEndTimeNum() - CURRENT_TIME();
		StatusInfo.m_StatusID	= pStatusCnfg->m_StatusID;
		StatusInfo.m_TotalTime  = pStatusCnfg->m_TimeNum;

		vectStatus.push_back(StatusInfo);
	}

	return vectStatus;
}

//使用已有的状态
void StatusPart::UseAlreadyHaveStatus(IStatus * pStatus)
{
	const SStatusCnfg * pStatusCnfg = pStatus->GetStatusCnfg();
	if( 0 == pStatusCnfg){
		TRACE("<error> %s : %d 行　获取状态配置失败！！", __FUNCTION__, __LINE__);
		return;
	}

	if( enStatusRocordType_Interval == pStatusCnfg->m_RecordType){
		//回合数设成状态的回合数
		pStatus->SetLeftRoundNum(pStatusCnfg->m_RoundNum);	

	}else if( enStatusRocordType_Time == pStatusCnfg->m_RecordType
		      || enStatusRocordType_Count == pStatusCnfg->m_RecordType){
		//时间的则延长时间
		UINT32 nLeftTime = pStatus->GetEndTimeNum() - ::time(0);

		if( nLeftTime < 0){
			this->RemoveStatus(pStatusCnfg->m_StatusID);
			return;
		}

		pStatus->SetEndTimeNum(pStatusCnfg->m_TimeNum + CURRENT_TIME() + nLeftTime);

		if(enStatusRocordType_Count == pStatusCnfg->m_RecordType)
		{
			pStatus->SetLeftEffectCount(pStatusCnfg->m_EffectCount);	
		}
	}
	
	
}

//得到相同状态组的状态
void StatusPart::__GetSameGroupStatus(const std::vector<TStatusGroupID> & vectGroupID, std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus)
{
	for (StatusMap::iterator it = m_status.begin(); it != m_status.end(); ++it)
	{
		IStatus * pStatus = (*it).second;
		if( 0 == pStatus){
			continue;
		}

			
		const std::vector<TStatusGroupID> & vectTmpGoodsID = pStatus->GetStatusCnfg()->m_vectGroupID;

		for( int i = 0; i < vectGroupID.size(); ++i)
		{
			for( int index = 0; index < vectTmpGoodsID.size(); ++index)
			{
				if( vectGroupID[i] != vectTmpGoodsID[index]){
					continue;
				}

				std::map<TStatusGroupID, std::vector<IStatus *>>::iterator iter = mapGroupStatus.find(vectGroupID[i]);
				if( iter == mapGroupStatus.end()){
					
					std::vector<IStatus *> vectStatus;
					vectStatus.push_back(pStatus);
					mapGroupStatus[vectGroupID[i]] = vectStatus;
				}else{
					
					std::vector<IStatus *> & vectStatus = iter->second;
					vectStatus.push_back(pStatus);
				}
			}
		}
	}
}


//组处理
bool StatusPart::__CalStatusGroup(const SStatusCnfg* pStatusInfo, std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus, bool & bCreateNewStatus, std::vector<UINT8> & vectDeleteStatusType)
{
	bCreateNewStatus = true;

	std::map<TStatusGroupID, std::vector<IStatus *>>::const_iterator iter = mapGroupStatus.begin();

	for( ; iter != mapGroupStatus.end(); ++iter)
	{
		const SStatusGroupCnfg * pGroupCnfg = g_pGameServer->GetConfigServer()->GetStatusGroupCnfg(iter->first);
		if( 0 == pGroupCnfg){
			TRACE("<error> %s : %d line 找不到状态组配置信息 StatusGroupID = %d",__FUNCTION__,__LINE__,iter->first);
			return false;
		}

		const std::vector<IStatus *> & vectStatus = iter->second;

		if( vectStatus.size() < pGroupCnfg->m_MaxStatusNum){
			continue;
		}

		switch(pGroupCnfg->m_FullHandleType)
		{
		case enFullCalType_ReplaceMinPriority:
			{
				//替换低优先级的那个效果

				//得到该组中最低优先级的状态
				int nMin = this->__GetMinPriorityStatusGroup(vectStatus);

				if( nMin > (vectStatus.size() - 1) || vectStatus[nMin] == 0){
					return false;
				}

				//与要新加的状态比较优先级
				const SStatusTypeCnfg * pStatusTypeCnfg1 = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(vectStatus[nMin]->GetStatusCnfg()->m_StatusType);
				if(pStatusTypeCnfg1 == 0){
					TRACE("<error> %s : %d line 找不到状态类型配置信息 StatusID = %d",__FUNCTION__,__LINE__,vectStatus[nMin]->GetStatusCnfg()->m_StatusID);
					return false;
				}

				const SStatusTypeCnfg * pStatusTypeCnfg2 = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(pStatusInfo->m_StatusType);
				if(pStatusTypeCnfg2 == 0){
					TRACE("<error> %s : %d line 找不到状态类型配置信息 StatusID = %d",__FUNCTION__,__LINE__,pStatusInfo->m_StatusID);
					return false;
				}

				if( pStatusTypeCnfg1->m_Priority > pStatusTypeCnfg2->m_Priority){
					bCreateNewStatus = false;
					return true;
				}

				vectDeleteStatusType.push_back(vectStatus[nMin]->GetStatusCnfg()->m_StatusType);

				this->__DeleteStatus_Group(mapGroupStatus, vectStatus[nMin]);
			}
			break;
		case enFullCalType_ReplaceMinEndTime:
			{
				//int nMin = 0;

				//for( int index = 0; index + 1 < vectStatus.size(); ++index)
				//{
				//	if( vectStatus[index] == 0){
				//		continue;
				//	}

				//	if( vectStatus[index + 1] == 0){
				//		++index;
				//		continue;
				//	}

				//	if( vectStatus[index]->GetEndTimeNum() > vectStatus[index + 1]->GetEndTimeNum()){
				//		nMin = index + 1;
				//	}
				//}

				//if( nMin > (vectStatus.size() - 1)){
				//	return false;
				//}

				////与要新加的状态比较结束时间
				//if( vectStatus[nMin]->GetEndTimeNum() > pStatusInfo->m_TimeNum + CURRENT_TIME()){
				//	bCreateNewStatus = false;
				//	return true;
				//}

				//vectDeleteStatusType.push_back(vectStatus[nMin]->GetStatusCnfg()->m_StatusType);


				//this->__DeleteStatus_Group(mapGroupStatus, vectStatus[nMin]);
				int Size = vectStatus.size();
				if( Size <= 0){
					return false;
				}

				IStatus * pStatus = vectStatus[Size - 1];
				if( pStatus == 0){
					return false;
				}

				const SStatusCnfg* pOldStatusCnfg = pStatus->GetStatusCnfg();
				if( 0 == pOldStatusCnfg){
					return false;
				}

				UINT8 type = pOldStatusCnfg->m_StatusType;

				vectDeleteStatusType.push_back(pStatus->GetStatusCnfg()->m_StatusType);

				this->__DeleteStatus_Group(mapGroupStatus, pStatus);
			}
			break;
		case enFullCalType_NoReplace:
			{
				bCreateNewStatus = false;
			}
			break;
		}
	}

	return true;
}

//得到组中最低优先级的状态下标
int  StatusPart::__GetMinPriorityStatusGroup(const std::vector<IStatus *> & vectStatus)
{
	int nMin = 0;

	for( int index = 0; index + 1 < vectStatus.size(); ++index)
	{
		IStatus * pStatus1 = vectStatus[index];
		if( 0 == pStatus1){
			continue;
		}

		IStatus * pStatus2 = vectStatus[index + 1];
		if( 0 == pStatus2){
			++index;
			continue;
		}

		if( pStatus1->GetStatusCnfg() == 0 || pStatus2->GetStatusCnfg() == 0){
			return 0;
		}

		const SStatusTypeCnfg * pStatusTypeCnfg1 = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(pStatus1->GetStatusCnfg()->m_StatusType);
		if(pStatusTypeCnfg1 == 0){
			TRACE("<error> %s : %d line 找不到状态类型配置信息 StatusID = %d",__FUNCTION__,__LINE__,pStatus1->GetStatusCnfg()->m_StatusID);
			return 0;
		}

		const SStatusTypeCnfg * pStatusTypeCnfg2 = g_pGameServer->GetConfigServer()->GetStatusTypeCnfg(pStatus2->GetStatusCnfg()->m_StatusType);
		if(pStatusTypeCnfg2 == 0){
			TRACE("<error> %s : %d line 找不到状态类型配置信息 StatusID = %d",__FUNCTION__,__LINE__,pStatus2->GetStatusCnfg()->m_StatusID);
			return 0;
		}

		if( pStatusTypeCnfg1->m_Priority > pStatusTypeCnfg2->m_Priority){
			nMin = index + 1;
		}
	}

	return nMin;
}

//删除状态组中的状态
void StatusPart::__DeleteStatus_Group(std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus, IStatus * pStatus)
{
	std::map<TStatusGroupID, std::vector<IStatus *>>::iterator iter = mapGroupStatus.begin();

	for( ; iter != mapGroupStatus.end(); ++iter)
	{
		std::vector<IStatus *> & vectStatus = iter->second;

		for( int i = 0; i < vectStatus.size(); ++i)
		{
			if( vectStatus[i] != pStatus){
				continue;
			}

			vectStatus[i] = 0;
		}
	}

	this->RemoveStatus(pStatus->GetStatusCnfg()->m_StatusID);
}
