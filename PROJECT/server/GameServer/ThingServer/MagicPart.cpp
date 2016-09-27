
#include "MagicPart.h"
#include "DBProtocol.h"
#include "ICombatServer.h"
#include "IBasicService.h"
#include "IMagic.h"
#include "ThingServer.h"
#include "IActor.h"
#include "IConfigServer.h"
#include "DMsgSubAction.h"

MagicPart::MagicPart()
{
	m_pActor = 0;
	memset(m_EquipMagic,0,sizeof(m_EquipMagic));
}

MagicPart::~MagicPart()
{
	//删除法术
	for(MAP_MAGIC::iterator it = m_mapMagic.begin(); it != m_mapMagic.end();it++)
	{
		IMagic * pMagic  = (*it).second;
		pMagic->Release();		
	}

	m_mapMagic.clear();

}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool MagicPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pContext || nLen < sizeof(SDBMagicPanelData))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	const SDBMagicPanelData * pDBMagicPanelData = (SDBMagicPanelData*)pContext;

	const SDBMagicInfo * pDBMagicInfo = pDBMagicPanelData->m_MagicInfo;

	ICombatServer * pCombatServer = g_pGameServer->GetCombatServer();

	for(int i=0; i < pDBMagicPanelData->m_MagicInfoNum ; i++ )
	{
		SCreateMagicCnt CreateMagicCnt;

		CreateMagicCnt.m_MagicID = pDBMagicInfo[i].m_MagicID;
		CreateMagicCnt.m_Level = pDBMagicInfo[i].m_Level;

		IMagic * pMagic = pCombatServer->CreateMagic(CreateMagicCnt);

		if(pMagic==0)
		{
			TRACE("<error> %s : %d line 创建玩家[%s]法术失败,MagicID=%d, Level=%d",__FUNCTION__,__LINE__,m_pActor->GetName(),CreateMagicCnt.m_MagicID,CreateMagicCnt.m_Level);
			continue;
		}

		m_mapMagic[CreateMagicCnt.m_MagicID] = pMagic;
	}

	//已加载的法术
	for(int i=0; i<ARRAY_SIZE(pDBMagicPanelData->m_EquipMagic); i++)
	{
		TMagicID MagicID = pDBMagicPanelData->m_EquipMagic[i];

		if(MagicID != INVALID_MAGIC_ID)
		{
			IMagic * pMagic = GetMagic(MagicID);     

			if(pMagic==0)
			{
				m_EquipMagic[i] = INVALID_MAGIC_ID;
				TRACE("<error> %s : %d line 创建玩家[%s]法术栏，找不到该法术,MagicID=%d",__FUNCTION__,__LINE__,m_pActor->GetName(),MagicID);
			}
			else
			{
				LoadMagic(MagicID,i);
			}
		}
	}
	return true;
}

//释放
void MagicPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart MagicPart::GetPartID(void)
{
	return enThingPart_Actor_Magic;
}

//取得本身生物
IThing*		MagicPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool MagicPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen < sizeof(SDB_Update_MagicPanelData_Req))
	{
		return false;
	}

	SDB_Update_MagicPanelData_Req * pReq  = (SDB_Update_MagicPanelData_Req*)buf;

	pReq->Uid_User = m_pActor->GetUID().ToUint64();

	SDB_Update_MagicPanelData * pMagicPanelData = (SDB_Update_MagicPanelData *)&pReq->MagicPanelData;

	pMagicPanelData->m_MagicInfoNum = 0;

	SDB_GetMagicInfo_Rsp * pDBMagicInfo = pMagicPanelData->m_MagicInfo;

	for(MAP_MAGIC::iterator it = m_mapMagic.begin(); it != m_mapMagic.end();it++)
	{
		IMagic * pMagic  = (*it).second;
		if(pMagic != 0)
		{
			pDBMagicInfo[pMagicPanelData->m_MagicInfoNum].MagicID	= pMagic->GetMagicID();
			pDBMagicInfo[pMagicPanelData->m_MagicInfoNum].Level		= pMagic->GetLevel();
			pDBMagicInfo[pMagicPanelData->m_MagicInfoNum].Position  = 0;

			for( int i = 0; i < MAX_EQUIP_MAGIC_NUM; ++i){
				if( pDBMagicInfo[pMagicPanelData->m_MagicInfoNum].MagicID == m_EquipMagic[i]){
					pDBMagicInfo[pMagicPanelData->m_MagicInfoNum].Position = i + 1;
				}
			}

			++pMagicPanelData->m_MagicInfoNum;
		}
	}

	nLen = sizeof(SDB_Update_MagicPanelData_Req);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void MagicPart::InitPrivateClient()
{
}


//玩家下线了，需要关闭该ThingPart
void MagicPart::Close()
{
}

//保存数据
void MagicPart::SaveData()
{
	SDB_Update_MagicPanelData_Req Req;

	int nLen = sizeof(SDB_Update_MagicPanelData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;

	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateMagicPanelInfo,ob.TakeOsb(),0,0);
}

//打开法术栏
void MagicPart::OpenMagicPanel()
{
	SC_EquipOpenMagic_Rsp Rsp;
	memcpy(Rsp.m_EquipMagic,m_EquipMagic,sizeof(Rsp.m_EquipMagic));
	Rsp.m_MagicInfoNum = 0;
	Rsp.m_uidActor = m_pActor->GetUID();

	OBuffer1k ob2;

	for(MAP_MAGIC::iterator it = m_mapMagic.begin(); it != m_mapMagic.end();it++)
	{
		IMagic * pMagic  = (*it).second;
		if(pMagic != 0)
		{
			SMagicInfo Info;
			Info.m_MagicID = pMagic->GetMagicID();
			Info.m_Level   = pMagic->GetLevel();
			Rsp.m_MagicInfoNum++;
			ob2 << Info;

			//发送法术配置信息给客户端
			m_pActor->SendMagicLevelCnfg(pMagic->GetMagicID());
		}
	}



	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_OpenMagic,sizeof(Rsp)+ob2.Size()) << Rsp;
	if(ob2.Size()>0)
	{
		ob.Push(ob2.Buffer(),ob2.Size());
	}

	m_pActor->SendData(ob.TakeOsb());

	IActor * pEmployee = 0;

	for(int i=0; 0!=(pEmployee=m_pActor->GetEmployee(i));i++)
	{
		IMagicPart * pMagicPart = pEmployee->GetMagicPart();
		if(pMagicPart !=0)
		{
			pMagicPart->OpenMagicPanel();
		}
	}
}

//升级法术
void MagicPart::UpgradeMagic(TMagicID MagicID)
{
	CS_EquipUpgradeMagic_Rsp Rsp;
	Rsp.m_MagicID = MagicID;
	Rsp.m_Result = enEquipRetCode_OK;
	Rsp.m_uidActor = m_pActor->GetUID();

	IMagic * pMagic = GetMagic(MagicID);
	if(pMagic==0)
	{
		Rsp.m_Result = enEquipRetCode_NoStudyMagic;
	}
	else
	{
		//当前等级
		UINT8 nCurLevel = pMagic->GetLevel();
		//获取下一级配置
		const  SMagicLevelCnfg*  pMagicLevelCnfg = g_pGameServer->GetConfigServer()->GetMagicLevelCnfg(MagicID,nCurLevel+1);

		if(pMagicLevelCnfg==0)
		{
			Rsp.m_Result = enEquipRetCode_LevelLimit;
		}		
		else if(m_pActor->GetCrtProp(enCrtProp_ActorLayer)< pMagicLevelCnfg->m_NeedLayer)
		{
			//境界未达到要求
			Rsp.m_Result = enEquipRetCode_LayerLimit;
		}
		else if(m_pActor->GetCrtProp(enCrtProp_ActorNimbus) < pMagicLevelCnfg->m_NeedNimbus)
		{
			//灵气不足
			Rsp.m_Result = enEquipRetCode_NoNimbus;
		}
		else
		{
			m_pActor->AddCrtPropNum(enCrtProp_ActorNimbus, -pMagicLevelCnfg->m_NeedNimbus);

			pMagic->Upgrade();

			Rsp.m_Level = pMagic->GetLevel();

			//发布事件
			SS_MagicUpLevel MagicUpLevel;
			MagicUpLevel.m_MagicID = Rsp.m_Level;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_MagicUpLevel);

			IActor * pMaster = m_pActor->GetMaster();

			if ( 0 == pMaster){
				
				pMaster = m_pActor;
			}

			pMaster->OnEvent(msgID,&MagicUpLevel,sizeof(MagicUpLevel));
		}		
	}	

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_Upgrade,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//加载法术
void MagicPart::LoadMagic(TMagicID MagicID,UINT8 pos)
{
	SC_EquipAddMagic_Rsp Rsp;
	Rsp.m_MagicID = MagicID;
	Rsp.m_Result = enEquipRetCode_OK;
	Rsp.m_uidActor = m_pActor->GetUID();

	//是否已经装备有相同法术，有则不能再装备该法术
	for( int i = 0; i < MAX_EQUIP_MAGIC_NUM; ++i)
	{
		if( m_EquipMagic[i] == MagicID && i != pos ){
			//已经装备有相同法术
			Rsp.m_Result = enEquipRetCode_ErrSameMagic;
		}
	}

	IMagic * pMagic = GetMagic(MagicID);

	if(Rsp.m_Result == enEquipRetCode_OK)
	{
		if(pMagic==0)
		{
			Rsp.m_Result = enEquipRetCode_NoStudyMagic;
		}
		else if(pos>=MAX_EQUIP_MAGIC_NUM)
		{
			Rsp.m_Result = enEquipRetCode_ErrorPos;
		}
		else
		{
			//该位置已有法术，需要先移除
			if(m_EquipMagic[pos] != INVALID_MAGIC_ID)
			{
				UnloadMagic(m_EquipMagic[pos],pos);
			}

			m_EquipMagic[pos] = MagicID;
			Rsp.m_Pos = pos;

			IMagic * pMagic =  GetMagic(MagicID);

			if(pMagic)
		 {
			 pMagic->OnEquip(m_pActor);
		 }

			//发布事件
			SS_EquipMagic EquipMagic;
			EquipMagic.m_MagicID = MagicID;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EuipMagic);

			IActor * pMaster = m_pActor->GetMaster();

			if ( 0 == pMaster){
				
				pMaster = m_pActor;
			}

			pMaster->OnEvent(msgID,&EquipMagic,sizeof(EquipMagic));	
		}
	}


	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_AddMagic,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//卸载法术
void MagicPart::UnloadMagic(TMagicID MagicID,UINT8 pos)
{
	SC_EquipRemoveMagic_Rsp Rsp;
	Rsp.m_MagicID = MagicID;
	Rsp.m_Result = enEquipRetCode_OK;
	Rsp.m_uidActor = m_pActor->GetUID();

	IMagic * pMagic = GetLoadedMagic(pos);
	if(pMagic==0)
	{
		Rsp.m_Result = enEquipRetCod_NoLoad;
	}
	else if(pos>=MAX_EQUIP_MAGIC_NUM)
	{
		Rsp.m_Result = enEquipRetCode_ErrorPos;
	}
	else
	{
		m_EquipMagic[pos] = INVALID_MAGIC_ID;
		Rsp.m_Pos = pos;

		pMagic->OnUnEquip(m_pActor);			
	}

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_RemoveMagic,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//玩家学会法术
IMagic * MagicPart::StudyMagic(TMagicID MagicID)
{
	SC_EquipStudyMagic_Rsp Rsp;
	Rsp.m_MagicID = MagicID;
	Rsp.m_Level = 1;
	Rsp.m_uidActor = m_pActor->GetUID();

	SCreateMagicCnt CreateMagicCnt;
	CreateMagicCnt.m_MagicID = MagicID;
	CreateMagicCnt.m_Level = 1;

	IMagic * pMagic = g_pGameServer->GetCombatServer()->CreateMagic(CreateMagicCnt);

	if(pMagic==0)
	{
		return 0;
	}

	m_mapMagic[MagicID] = pMagic;

	//发送法术配置信息给客户端
	m_pActor->SendMagicLevelCnfg(CreateMagicCnt.m_MagicID);

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_StudyMagic,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	return pMagic;
}

//获得玩家已学会的法术
IMagic * MagicPart::GetMagic(TMagicID MagicID)
{
	MAP_MAGIC::iterator it = m_mapMagic.find(MagicID);
	if(it == m_mapMagic.end())
	{
		return 0;
	}

	return (*it).second;
}


//获取已加载的法术
IMagic * MagicPart::GetLoadedMagic(UINT8 pos)
{
	if(pos >= MAX_EQUIP_MAGIC_NUM)
	{
		return 0;
	}

	TMagicID MagicID = m_EquipMagic[pos];

	if(MagicID != INVALID_MAGIC_ID)
	{
		return GetMagic(MagicID);
	}

	return 0;
}


//直接设置法术等级
bool MagicPart::SetMagicLevel(TMagicID MagicID, UINT8 level)
{
	IMagic * pMagic = this->GetMagic(MagicID);
	if( 0 == pMagic){
		return false;
	}

	return pMagic->SetLevel(level);
}
