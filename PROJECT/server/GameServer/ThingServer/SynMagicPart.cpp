
#include "IThing.h"

#include "SynMagicPart.h"

#include "DBProtocol.h"
#include "IStatusPart.h"
#include "IActor.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include <vector>
#include "IBasicService.h"

SynMagicPart::SynMagicPart()
{
	m_pActor = 0;
}

SynMagicPart::~SynMagicPart()
{
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool SynMagicPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || pMaster->GetThingClass() != enThing_Class_Actor || nLen < sizeof(SDBSynMagicPanelData)){
		return false;
	}

	m_pActor = (IActor *)pMaster;

	SDBSynMagicPanelData * pSynMagicPanelData = (SDBSynMagicPanelData *)pContext;

	for(int i = 0; i < pSynMagicPanelData->m_SynMagicNum; ++i)
	{
		SSynMagicInfo SynMagicInfo;
		SynMagicInfo.m_SynMagicID	 = pSynMagicPanelData->m_SynMagicData[i].m_SynMagicID;
		SynMagicInfo.m_SynMagicLevel = pSynMagicPanelData->m_SynMagicData[i].m_SynMagicLevel;

		m_mapSynMagic[SynMagicInfo.m_SynMagicID] = SynMagicInfo;
	}
	return true;
}

//释放
void SynMagicPart::Release(void)
{
	delete this;
}

//取得部件ID那你欺负欺负我，你载我
enThingPart SynMagicPart::GetPartID(void)
{
	return enThingPart_Actor_SynMagic;
}

//取得本身生物
IThing*		SynMagicPart::GetMaster(void)
{
	return (IThing *)m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool SynMagicPart::OnGetDBContext(void * buf, int &nLen)
{
	if(0 == buf || nLen < sizeof(SDBSynMagicPanelData))
	{
		return false;
	}

	SDB_Update_SynMagicData_Req * pReq = (SDB_Update_SynMagicData_Req *)buf;

	pReq->Uid_User = m_pActor->GetUID().ToUint64();

	SDBSynMagicPanelData * pSynMagicPanelData =   (SDBSynMagicPanelData *)&pReq->SynMagicData;

	pSynMagicPanelData->m_SynMagicNum = 0;

	SDBSynMagicData * pSynMagicData = (SDBSynMagicData *)pSynMagicPanelData->m_SynMagicData;

	std::hash_map<TSynMagicID, SSynMagicInfo>::iterator iter = m_mapSynMagic.begin();
	while( iter != m_mapSynMagic.end())
	{
		SSynMagicInfo & SynMagicInfo	= iter->second;

		pSynMagicData->m_SynMagicID		= SynMagicInfo.m_SynMagicID;
		pSynMagicData->m_SynMagicLevel	= SynMagicInfo.m_SynMagicLevel;

		++pSynMagicData;
		++pSynMagicPanelData->m_SynMagicNum;

		++iter;
	}
	nLen = sizeof(SDB_Update_SynMagicData_Req);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void	SynMagicPart::InitPrivateClient()
{
}


//玩家下线了，需要关闭该ThingPart
void	SynMagicPart::Close()
{

}

//保存数据
void	SynMagicPart::SaveData()
{
	SDB_Update_SynMagicData_Req Req;
	
	int nLen = sizeof(SDB_Update_SynMagicData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateSynMagicInfo,ob.TakeOsb(),0,0);
}

//玩家习得此技能
void	SynMagicPart::LearnSynMagicOK(const SynMagicData & MagicData)
{
	SSynMagicInfo SynMagicInfo;
	SynMagicInfo.m_SynMagicID	 = MagicData.m_SynMagicID;
	SynMagicInfo.m_SynMagicLevel = MagicData.m_SynMagicLevel;

	const SSynMagicCnfg * pSynMagicCnfg = g_pGameServer->GetConfigServer()->GetSynMagicCnfg(MagicData.m_SynMagicID, MagicData.m_SynMagicLevel);
	if( 0 == pSynMagicCnfg){
		TRACE("<error> %s : %d 行 帮派技能配置信息获取失败！！帮派技能ID = %d,等级=%d",__FUNCTION__, __LINE__, MagicData.m_SynMagicID,MagicData.m_SynMagicLevel);
		return;	
	}

	//给玩家加上效果
	IStatusPart * pStatusPart =  (IStatusPart *)m_pActor->GetPart(enThingPart_Crt_Status);
	if( 0 == pStatusPart){
		return;
	}

	std::hash_map<TSynMagicID, SSynMagicInfo>::iterator iter = m_mapSynMagic.find(MagicData.m_SynMagicID);
	if( iter != m_mapSynMagic.end()){
		SSynMagicInfo & SynMagicInfo = iter->second;

		const SSynMagicCnfg * pOldSynMagicInfo = g_pGameServer->GetConfigServer()->GetSynMagicCnfg(SynMagicInfo.m_SynMagicID, SynMagicInfo.m_SynMagicLevel);
		if( 0 == pOldSynMagicInfo){
			TRACE("<error> %s : %d 行 帮派技能配置信息获取失败！！,帮派技能ID=%d,帮派技能等级=%d",__FUNCTION__, __LINE__,SynMagicInfo.m_SynMagicID, SynMagicInfo.m_SynMagicLevel);
		}else{
			//删除效果
			for( int i = 0; i < pOldSynMagicInfo->m_vectEffect.size(); ++i)
			{
				pStatusPart->RemoveEffect(pOldSynMagicInfo->m_vectEffect[i]);
			}
		}

		m_mapSynMagic.erase(iter);
	}

	for( int i = 0; i < pSynMagicCnfg->m_vectEffect.size(); ++i)
	{
		pStatusPart->AddEffect(pSynMagicCnfg->m_vectEffect[i]);
	}

	m_mapSynMagic[SynMagicInfo.m_SynMagicID] = SynMagicInfo;

//	m_pActor->RecalculateProp();
}

//把帮派技能的效果加给玩家
void	SynMagicPart::UserSynMagicEffect()
{
	IStatusPart * pStatusPart = (IStatusPart *)m_pActor->GetPart(enThingPart_Crt_Status);
	if( 0 == pStatusPart){
		return;
	}
	
	std::hash_map<TSynMagicID, SSynMagicInfo>::iterator iter = m_mapSynMagic.begin();

	for( ;iter != m_mapSynMagic.end(); ++iter)
	{
		SSynMagicInfo & SynMagicInfo = iter->second;
		const std::vector<SSynMagicCnfg> * pvectSynMagic = g_pGameServer->GetConfigServer()->GetSynMagicCnfg(SynMagicInfo.m_SynMagicID);
		if( 0 == pvectSynMagic){
			TRACE("<error> %s : %d 行 帮派技能配置信息获取失败！！帮派技能ID = %d",__FUNCTION__, __LINE__, SynMagicInfo.m_SynMagicID);
			continue;
		}
		const SSynMagicCnfg * pSynMagicCnfg = 0;

		for( int i = 0; i < pvectSynMagic->size(); ++i)
		{
			if( (*pvectSynMagic)[i].m_SynMagicLevel == SynMagicInfo.m_SynMagicLevel){
				pSynMagicCnfg = &(*pvectSynMagic)[i];
			}
		}

		if( 0 == pSynMagicCnfg){
			return;
		}

		for(int i  = 0; i < pSynMagicCnfg->m_vectEffect.size(); ++i)
		{
			pStatusPart->AddEffect(pSynMagicCnfg->m_vectEffect[i]);
		}
	}
}

//打开帮派技能栏
void	SynMagicPart::OpenSynMagicPanel()
{
	SC_OpenSynMagic Rsp;

	Rsp.m_Num = m_mapSynMagic.size();

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_OpenSynMagic,sizeof(Rsp) + Rsp.m_Num * sizeof(SynMagicInfo_Rsp)) << Rsp;

	std::hash_map<TSynMagicID, SSynMagicInfo>::iterator iter = m_mapSynMagic.begin();

	for(; iter != m_mapSynMagic.end(); ++iter)
	{
		SSynMagicInfo & MagicInfo = iter->second;

		SynMagicInfo_Rsp MagicRsp;

		MagicRsp.m_Level = MagicInfo.m_SynMagicLevel;
		MagicRsp.m_SynMagicID = MagicInfo.m_SynMagicID;

		ob << MagicRsp;

		//发送配置信息
		m_pActor->SendSynMagicCnfg(MagicInfo.m_SynMagicID);
	}

	m_pActor->SendData(ob.TakeOsb());
}

//获得玩家的当前技能等级
UINT8	SynMagicPart::GetSynMagicLevel(TSynMagicID SynMagicID)
{
	std::hash_map<TSynMagicID, SSynMagicInfo>::iterator iter = m_mapSynMagic.find(SynMagicID);

	if( iter == m_mapSynMagic.end()){
		return 0;
	}

	return (iter->second).m_SynMagicLevel;
}
