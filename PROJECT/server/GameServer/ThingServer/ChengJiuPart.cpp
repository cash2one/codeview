
#include "IActor.h"
#include "ChengJiuPart.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "XDateTime.h"
#include "ChengJiuCmd.h"
#include <sstream>
#include <string>



ChengJiuPart::ChengJiuPart()
{
	 m_pActor = 0;
	 m_ChengJiuPoint = 0;  //获得的成就点
	 m_GetChengJiuNum = 0; //已获得成就数量
	 m_ActiveTitleID = INVALID_TITLE_ID;  //当前启用的称号ID
	 m_bClientOpen = 0;
}


ChengJiuPart::~ChengJiuPart()
{
}



//推进成就进度
void ChengJiuPart::AdvanceProgress(TChengJiuID  ChengJiuID )
{
	const SChengJiuCnfg * pChengJiuCnfg = g_pGameServer->GetConfigServer()->GetChengJiuCnfg(ChengJiuID);
	if( 0 == pChengJiuCnfg){
		TRACE("<error> %s : %d 行 成就配置文件获取失败!!!,成就ID=%d", __FUNCTION__,__LINE__,ChengJiuID);
		return;
	}

	MAP_CHENGJIU::iterator iter = m_mapChengJiu.find(ChengJiuID);
	if( iter == m_mapChengJiu.end()){
		//增加新成就数据
		if( !this->AddChengJiu(pChengJiuCnfg)){
			return;
		}
	}else{
		SChengJiuData & ChengJiuData = iter->second;

		++ChengJiuData.m_Progress;

		if( pChengJiuCnfg->m_AttainNum <= ChengJiuData.m_Progress){
			//完成
			ChengJiuData.m_FinishTime = CURRENT_TIME();

			//奖励
			this->Award(pChengJiuCnfg);

			++m_GetChengJiuNum;
		}
		
		ChengJiuData.m_bUpdate = true;
	}

	//通知更改成就状态
	this->NoticeUpdateChengJiu(ChengJiuID);
}

//复位成就进度
void ChengJiuPart::ResetProgress(TChengJiuID  ChengJiuID )
{
	MAP_CHENGJIU::iterator iter = m_mapChengJiu.find(ChengJiuID);
	if( iter == m_mapChengJiu.end()){
		return;
	}

	SChengJiuData & ChengJiuData = iter->second;

	ChengJiuData.m_Progress = 0;

	ChengJiuData.m_bUpdate = true;

	//通知更改成就状态
	this->NoticeUpdateChengJiu(ChengJiuID);
}

//是否已获得某成就
bool ChengJiuPart::IsAttainChengJiu(TChengJiuID  ChengJiuID) 
{
	SChengJiuData * pChengJiuData = GetChengJiuData(ChengJiuID);

	if(pChengJiuData != 0 && pChengJiuData->m_FinishTime != 0)
	{
		return true;
	}

	return false;
}

		//打开成就栏
void ChengJiuPart::OpenChengJiuPanel()
{
	SC_ChengJiuOpenChengJiu_Rsp Rsp;

	Rsp.m_ActiveTitleID = m_ActiveTitleID;
	Rsp.m_ChengJiuPoint = m_ChengJiuPoint;
	Rsp.m_TotalChengJiuPoint = 0;
	Rsp.m_OkChengJiuNum = m_GetChengJiuNum;

	const std::hash_map<TChengJiuID,SChengJiuCnfg> & mapChengJiu = g_pGameServer->GetConfigServer()->GetAllChengJiuCnfg();

	Rsp.m_ChengJiuNum   = mapChengJiu.size();

	const STitleCnfg * pTitleCnfg = g_pGameServer->GetConfigServer()->GetTitleCnfg(m_ActiveTitleID);
	const std::string * pTitleName;
	if(pTitleCnfg == 0)
	{
		pTitleName = 0;
	}
	else
	{
		pTitleName = (const std::string * )g_pGameServer->GetGameWorld()->GetLanguageStr(pTitleCnfg->m_TitleNameLangID);
	}
	//const std::string * pTitleName = g_pGameServer->GetConfigServer()->GetTitleName(m_ActiveTitleID);
	if( pTitleName == 0){
		memset(Rsp.m_szTitleName, 0, sizeof(Rsp.m_szTitleName));
	}else{
		strncpy(Rsp.m_szTitleName, (*pTitleName).c_str(), sizeof(Rsp.m_szTitleName));
	}

	OBuffer1k ob;

	MAP_CHENGJIU::iterator it;

	std::hash_map<TChengJiuID,SChengJiuCnfg>::const_iterator iter = mapChengJiu.begin();

	for( ; iter != mapChengJiu.end(); ++iter)
	{
		const SChengJiuCnfg & ChengJiuCnfg = iter->second;

		SChengJiuDataCnfg  ChengJiuDataCnfg;
		ChengJiuDataCnfg.m_AwardGoodsNum = ChengJiuCnfg.m_vectAwardGoods.size() / 2;
		ChengJiuDataCnfg.m_ChengJiuID	 = ChengJiuCnfg.m_ChengJiuID;
		ChengJiuDataCnfg.m_ChengJiuPoint = ChengJiuCnfg.m_ChengJiuPoint;
		ChengJiuDataCnfg.m_Class		 = ChengJiuCnfg.m_Class;
		ChengJiuDataCnfg.m_SubClass		 = ChengJiuCnfg.m_SubClass;
		ChengJiuDataCnfg.m_TargetCount	 = ChengJiuCnfg.m_AttainNum;
		ChengJiuDataCnfg.m_TitleID		 = ChengJiuCnfg.m_TitleID;
		ChengJiuDataCnfg.m_ResID		 = ChengJiuCnfg.m_ResID;
		
		Rsp.m_TotalChengJiuPoint += ChengJiuDataCnfg.m_ChengJiuPoint;

		it = m_mapChengJiu.find(iter->first);

		if( it != m_mapChengJiu.end()){
			SChengJiuData & ChengJiuData = it->second;
			ChengJiuDataCnfg.m_CurCount		 = ChengJiuData.m_Progress;
			ChengJiuDataCnfg.m_FinishTime	 = ChengJiuData.m_FinishTime;
		}else{
			ChengJiuDataCnfg.m_CurCount		 = 0;
			ChengJiuDataCnfg.m_FinishTime	 = 0;		
		}

		ob << ChengJiuDataCnfg;
		
		for( int i = 0; i + 1 < ChengJiuCnfg.m_vectAwardGoods.size(); i += 2)
		{
			ob.Push(&ChengJiuCnfg.m_vectAwardGoods[i],sizeof(ChengJiuCnfg.m_vectAwardGoods[i]));

			ob.Push(&ChengJiuCnfg.m_vectAwardGoods[i + 1],sizeof(ChengJiuCnfg.m_vectAwardGoods[i + 1]));

			m_pActor->SendGoodsCnfg(ChengJiuCnfg.m_vectAwardGoods[i]);
		}
		

		// fly add	20121106

		ob.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(ChengJiuCnfg.m_ChengJiuLangID), strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(ChengJiuCnfg.m_ChengJiuLangID)) + 1);
		ob.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(ChengJiuCnfg.m_GetTargetLangID), strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(ChengJiuCnfg.m_GetTargetLangID)) + 1);
		//ob.Push(ChengJiuCnfg.m_strName.c_str(), ChengJiuCnfg.m_strName.length() + 1);
		//ob.Push(ChengJiuCnfg.m_strTarget.c_str(), ChengJiuCnfg.m_strTarget.length() + 1);
		ob.Push(ChengJiuCnfg.m_strProbDesc.c_str(), ChengJiuCnfg.m_strProbDesc.length() + 1);

		const STitleCnfg * pTitleCnfg = g_pGameServer->GetConfigServer()->GetTitleCnfg(ChengJiuCnfg.m_TitleID);
		const std::string * pTitleName;
		if(pTitleCnfg == 0)
		{
			pTitleName = 0;
		}
		else
		{
			pTitleName = (const std::string * )g_pGameServer->GetGameWorld()->GetLanguageStr(pTitleCnfg->m_TitleNameLangID);			
		}
		//const std::string * pTitleName = g_pGameServer->GetConfigServer()->GetTitleName(ChengJiuCnfg.m_TitleID);
		if( pTitleName == 0){
			char none = '\0';
			ob.Push(&none,1);
		}else{
			ob.Push((*pTitleName).c_str(), (*pTitleName).length() + 1);
		}
	}

	OBuffer4k ob2;
	ob2 << ChengJiuHeader(enChengJiuCmd_CS_OpenTask, sizeof(Rsp) + ob.Size()) << Rsp << ob;
	m_pActor->SendData(ob2.TakeOsb());	

	//打开过成就栏
	m_bClientOpen = true;
}

	//更换称号
void ChengJiuPart::ChangeTitle(TTitleID  TitleID)
{
	SC_ChengJiuChangeTitle_Rsp ChengJiuChangeTitle_Rsp;

	if( TitleID == INVALID_TITLE_ID){
		//不显示称号
		m_ActiveTitleID = TitleID;

		ChengJiuChangeTitle_Rsp.m_TitleID = TitleID;
	}else{
		MAP_CHENGJIU::iterator iter = m_mapChengJiu.begin();

		for( ; iter != m_mapChengJiu.end(); ++iter)
		{
			const SChengJiuCnfg * pChengJiuCnfg = g_pGameServer->GetConfigServer()->GetChengJiuCnfg(iter->first);
			if( 0 == pChengJiuCnfg){
				TRACE("<warning> %s : %d 行,找不到成就配置文件信息!!,成就ID = %d", __FUNCTION__, __LINE__, iter->first);
				continue;
			}

			if( pChengJiuCnfg->m_TitleID == TitleID){
				break;
			}
		}

		if( iter == m_mapChengJiu.end()){
			ChengJiuChangeTitle_Rsp.m_Result = enChengJiuRetCode_ErrTitleID;
		}else{
			m_ActiveTitleID = TitleID;

			ChengJiuChangeTitle_Rsp.m_TitleID = TitleID;
		}
	}

	OBuffer1k ob;
	ob << ChengJiuHeader(enChengJiuCmd_ChangeTitle, sizeof(ChengJiuChangeTitle_Rsp)) << ChengJiuChangeTitle_Rsp;
	m_pActor->SendData(ob.TakeOsb());	
}

//增加成就
bool	ChengJiuPart::AddChengJiu(const SChengJiuCnfg * pChengJiuCnfg)
{
	SChengJiuData ChengJiuData;
	ChengJiuData.m_bUpdate = true;
	ChengJiuData.m_ChengJiuID = pChengJiuCnfg->m_ChengJiuID;
	ChengJiuData.m_Progress = 1;

	if( pChengJiuCnfg->m_AttainNum <= ChengJiuData.m_Progress){
		//完成
		ChengJiuData.m_FinishTime = CURRENT_TIME();

		//奖励
		this->Award(pChengJiuCnfg);

		++m_GetChengJiuNum;
	}

	if( !m_mapChengJiu.insert(MAP_CHENGJIU::value_type(pChengJiuCnfg->m_ChengJiuID, ChengJiuData)).second){
		TRACE("<error> %s : %d 行 插入成就数据失败,成就ID＝%d", __FUNCTION__, __LINE__, pChengJiuCnfg->m_ChengJiuID);
		return false;
	}

	//自动获取的成就
	this->AutoGetChengJiu(pChengJiuCnfg->m_vectAutoGetChengJiuID);

	return true;
}

//获得成就数据
SChengJiuData * ChengJiuPart::GetChengJiuData(TChengJiuID ChengJiuID )
{
	MAP_CHENGJIU::iterator it = m_mapChengJiu.find(ChengJiuID);
	if(it != m_mapChengJiu.end())
	{
		return &(*it).second;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool ChengJiuPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || 0 == pContext || nLen < sizeof(DB_ChengJiuPart)){
		TRACE("<error>  %s : %d 行 创建任务PART失败", __FUNCTION__, __LINE__);
		return false;
	}

	m_pActor = (IActor *)pMaster;

	SChengJiuDBData * pChengJiuDBData = (SChengJiuDBData *)pContext;

	DB_ChengJiuPart * pChengJiuPart = (DB_ChengJiuPart *)pChengJiuDBData->m_pData;

	m_ChengJiuPoint  = pChengJiuPart->m_ChengJiuPoint;
	m_GetChengJiuNum = pChengJiuPart->m_GetChengJiuNum;
	m_ActiveTitleID  = pChengJiuPart->m_ActiveTitleID;

	INT32 nChengJiuNum = (nLen - sizeof(DB_ChengJiuPart)) / sizeof(DB_ChengJiu);
	
	if( nChengJiuNum < 1){
		return true;
	}

	DB_ChengJiu * pChengJiu = (DB_ChengJiu *)(pChengJiuPart + 1);

	for( int i = 0 ; i < nChengJiuNum; ++i,++pChengJiu)
	{
		if( pChengJiu->m_ChengJiuID == INVALID_CHENGJIU_ID){
			continue;
		}

		SChengJiuData ChengJiuData;

		ChengJiuData.m_bUpdate	  = false;
		ChengJiuData.m_ChengJiuID = pChengJiu->m_ChengJiuID;
		ChengJiuData.m_FinishTime = pChengJiu->m_FinishTime;
		ChengJiuData.m_Progress	  = pChengJiu->m_CurCount;

		m_mapChengJiu[pChengJiu->m_ChengJiuID] = ChengJiuData;
	}

	return true;
}

//释放
void ChengJiuPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart ChengJiuPart::GetPartID(void)
{
	return enThingPart_Actor_ChengJiu;
}

//取得本身生物
IThing*		ChengJiuPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool ChengJiuPart::OnGetDBContext(void * buf, int &nLen)
{
	if( 0 == buf || nLen != sizeof(SDB_Update_ChengJiuPart_Req)){
		return false;
	}

	SDB_Update_ChengJiuPart_Req * pUpdate_ChengJiuPart_Req = (SDB_Update_ChengJiuPart_Req *)buf;

	pUpdate_ChengJiuPart_Req->m_ActiveTitleID = m_ActiveTitleID;
	pUpdate_ChengJiuPart_Req->m_ChengJiuPoint = m_ChengJiuPoint;
	pUpdate_ChengJiuPart_Req->m_GetChengJiuNum = m_GetChengJiuNum;

	pUpdate_ChengJiuPart_Req->m_uidUser = m_pActor->GetUID().ToUint64();

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void ChengJiuPart::InitPrivateClient()
{
}


//玩家下线了，需要关闭该ThingPart
void ChengJiuPart::Close()
{
}

//保存数据
void ChengJiuPart::SaveData()
{
	//保存PART数据
	SDB_Update_ChengJiuPart_Req Update_ChengJiuPart_Req;

	int nLen = sizeof(SDB_Update_ChengJiuPart_Req);

	if( !this->OnGetDBContext(&Update_ChengJiuPart_Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Update_ChengJiuPart_Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateChengJiuPartInfo, ob.TakeOsb(), 0, 0);

	//保存单个成就
	for(MAP_CHENGJIU::iterator iter = m_mapChengJiu.begin(); iter != m_mapChengJiu.end(); ++iter)
	{
		SChengJiuData & ChengJiuData = iter->second;

		if( !ChengJiuData.m_bUpdate){
			//没更新过的不保存
			continue;
		}

		SDB_Update_ChengJiu_Req Update_ChengJiu_Req;
		
		Update_ChengJiu_Req.m_ChengJiuID = ChengJiuData.m_ChengJiuID;
		Update_ChengJiu_Req.m_CurCount	 = ChengJiuData.m_Progress;
		Update_ChengJiu_Req.m_FinishTime = ChengJiuData.m_FinishTime;

		Update_ChengJiu_Req.m_uidUser	 = m_pActor->GetUID().ToUint64();

		ChengJiuData.m_bUpdate = false;	//保存过的就不再保存了

		ob.Reset();
		ob << Update_ChengJiu_Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateChengJiuInfo, ob.TakeOsb(), 0, 0);
	}
}

//获得当前称号
std::string ChengJiuPart::GetTitle()
{
	const STitleCnfg * pTitleCnfg = g_pGameServer->GetConfigServer()->GetTitleCnfg(m_ActiveTitleID);
	const std::string * pString;
	if(pTitleCnfg == 0)
	{
		pString = 0;
	}
	else
	{
		pString = (const std::string * )g_pGameServer->GetGameWorld()->GetLanguageStr(pTitleCnfg->m_TitleNameLangID);
	}
	//const std::string * pString = g_pGameServer->GetConfigServer()->GetTitleName(m_ActiveTitleID);
	if( 0 == pString){
		return std::string();
	}

	return *pString;
}

//得到当前称号ID
TTitleID	ChengJiuPart::GetTitleID()
{
	return m_ActiveTitleID;
};

//获取成就完成时间
UINT32		ChengJiuPart::GetChengJiuFinishTime(TChengJiuID ChengJiuID)
{
	std::hash_map<TChengJiuID,SChengJiuData>::iterator iter = m_mapChengJiu.find(ChengJiuID);

	if( iter == m_mapChengJiu.end()){
		return 0;
	}

	return (iter->second).m_FinishTime;
}

//通知更新成就
void ChengJiuPart::NoticeUpdateChengJiu(TChengJiuID ChengJiuID)
{
	if( !m_bClientOpen){
		return;
	}

	MAP_CHENGJIU::iterator iter = m_mapChengJiu.find(ChengJiuID);
	if( iter == m_mapChengJiu.end()){
		return;
	}

	SChengJiuData & ChengJiuData = iter->second;

	SC_ChengJiuUpdateChengJiu ChengJiuUpdate;

	ChengJiuUpdate.m_ChengJiuID = ChengJiuID;
	ChengJiuUpdate.m_FinishedTime = ChengJiuData.m_FinishTime;
	ChengJiuUpdate.m_nCurCount	= ChengJiuData.m_Progress;

	OBuffer1k ob;
	ob << ChengJiuHeader(enChengJiuCmd_SC_Update, sizeof(ChengJiuUpdate)) << ChengJiuUpdate;
	m_pActor->SendData(ob.TakeOsb());
}

//自动获得的成就
void	ChengJiuPart::AutoGetChengJiu(const std::vector<TChengJiuID> & vectChengJiuID)
{
	MAP_CHENGJIU::iterator iter;

	for( int i = 0; i < vectChengJiuID.size(); ++i)
	{
		iter = m_mapChengJiu.find(vectChengJiuID[i]);

		if( iter != m_mapChengJiu.end()){
			continue;
		}

		const SChengJiuCnfg * pChengJiuCnfg = g_pGameServer->GetConfigServer()->GetChengJiuCnfg(vectChengJiuID[i]);
		if( 0 == pChengJiuCnfg){
			TRACE("<warning> %s : %d 行,找不到成就配置文件信息!!,成就ID = %d", __FUNCTION__, __LINE__, iter->first);
			continue;
		}

		this->AddChengJiu(pChengJiuCnfg);
	}
}

//奖励
void	ChengJiuPart::Award(const SChengJiuCnfg * pChengJiuCnfg)
{
	m_ChengJiuPoint += pChengJiuCnfg->m_ChengJiuPoint;

	//物品奖励
	if( pChengJiuCnfg->m_vectAwardGoods.size() > MAX_MAIL_GOODS_NUM * 2){
		TRACE("<error> %s : %d 行 奖励物品超出4件，邮件最多奖励4件,成就ID=%d", __FUNCTION__, __LINE__, pChengJiuCnfg->m_ChengJiuID);
	}

	if( pChengJiuCnfg->m_vectAwardGoods.size() < 2){
		//没物品
		return;
	}

	SWriteSystemData SystemMail;

	SystemMail.m_DestUID = m_pActor->GetUID();

	// fly add
	TLanguageID ChengJiuLangID;
	ChengJiuLangID = pChengJiuCnfg->m_ChengJiuLangID;
	const SLanguageTypeCnfg * pChengJiuLangConfig = g_pGameServer->GetConfigServer()->GetLanguageTypeCnfg(ChengJiuLangID);
	if( 0 == pChengJiuLangConfig){
		TRACE("<error> %s ; %d 行 获取语言类型配置数据出错!!语言ID = %d", __FUNCTION__, __LINE__, ChengJiuLangID);
		return;
	}

	// fly add	20121106
	sprintf_s(SystemMail.m_szThemeText, sizeof(SystemMail.m_szThemeText), g_pGameServer->GetGameWorld()->GetLanguageStr(10044), g_pGameServer->GetGameWorld()->GetLanguageStr(pChengJiuCnfg->m_ChengJiuLangID));
	//sprintf_s(SystemMail.m_szThemeText, sizeof(SystemMail.m_szThemeText), "%s成就奖励", pChengJiuCnfg->m_strName.c_str());

	std::string strContent;
	std::ostringstream os;
	os << g_pGameServer->GetGameWorld()->GetLanguageStr(pChengJiuCnfg->m_ChengJiuLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10045);
	//os << pChengJiuCnfg->m_strName.c_str() << "成就完成，获得";

	std::vector<IGoods * > vecGoods;

	for( int i = 0; (i + 1 < pChengJiuCnfg->m_vectAwardGoods.size()) && (i / 2 < MAX_MAIL_GOODS_NUM); i += 2)
	{
		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pChengJiuCnfg->m_vectAwardGoods[i]);
		if( 0 == pGoodsCnfg){
			continue;
		}

		SCreateGoodsContext GoodsContext;
		GoodsContext.m_Binded = false;
		GoodsContext.m_GoodsID = pGoodsCnfg->m_GoodsID;
		GoodsContext.m_Number  = pChengJiuCnfg->m_vectAwardGoods[i + 1];

		IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsContext);
		if( 0 == pGoods){
			continue;
		}

		vecGoods.push_back(pGoods);

		// fly add	20121106
		os << "," << (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10055);
	}

	strContent = os.str();

	sprintf_s(SystemMail.m_szContentText, "%s", strContent.c_str());

	//写系统邮件
	g_pGameServer->GetGameWorld()->WriteSystemMail(SystemMail, vecGoods);
}

//发送自己的成就数据给别的玩家（成就对比用）
void		ChengJiuPart::SendMeChengJiuToOtherUser(IActor * pRecvActor)
{
	SC_GetUserChengJiu Rsp;

	Rsp.m_ActiveTitleID = m_ActiveTitleID;

	OBuffer4k ob;

	MAP_CHENGJIU::iterator iter = m_mapChengJiu.begin();

	for( ; iter != m_mapChengJiu.end(); ++iter)
	{
		SChengJiuData & ChengJiuData = iter->second;

		if( ChengJiuData.m_FinishTime > 0){

			UserChengJiuData CJData;

			CJData.m_ChengJiuID = ChengJiuData.m_ChengJiuID;
			CJData.m_FinishTime = ChengJiuData.m_FinishTime;

			ob << CJData;

			++Rsp.m_Num;
		}
	}

	OBuffer4k ob2;
	ob2 << ChengJiuHeader(enChengJiuCmd_Get_UserChengJiu, sizeof(Rsp) + ob.Size()) << Rsp;

	if( ob.Size() > 0){
		ob2 << ob;
	}

	pRecvActor->SendData(ob2.TakeOsb());
}
