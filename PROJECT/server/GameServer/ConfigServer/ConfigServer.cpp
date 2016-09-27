
#include "ConfigServer.h"
#include "IBasicService.h"
#include "RandomService.h"
#include <algorithm>  
#include "ICryptService.h"
#include "FileSystem.h"
#include "IEquipment.h"
#include "IGodSword.h"

IConfigServer * CreateConfigServer()
{
	return new ConfigServer();
}


ConfigServer::ConfigServer()/* : m_vecShopMallCnfgByLabel((INT32)enShopMall_Max)*/
{
}

ConfigServer::~ConfigServer()
{
}


void ConfigServer::Release()
{
	delete this;
}

bool  ConfigServer::Create()
{
	for(int i=0; i<sizeof(g_ConfigIniFile)/sizeof(g_ConfigIniFile[0]);i++)
	{
		FileConfig File;
		if(File.Open(g_ConfigIniFile[i],"GameServer")==false)
		{
			TRACE("打开配置文件[%s]失败!",g_ConfigIniFile[i]);
			return false;
		}
		if(g_OnLoadIniFunc[i] && (this->*g_OnLoadIniFunc[i])(File,g_ConfigIniFile[i])==false)
		{
			TRACE("读取配置文件[%s]失败!",g_ConfigIniFile[i]);
			return false;
		}
	}

	for(int i=0; i<sizeof(g_ConfigCsvFile)/sizeof(g_ConfigCsvFile[0]);i++)
	{
		FileCSV File;
		if(File.Open(g_ConfigCsvFile[i])==false)
		{
			TRACE("打开配置文件[%s]失败!",g_ConfigCsvFile[i]);
			return false;
		}
		if(g_OnLoadCsvFunc[i] && (this->*g_OnLoadCsvFunc[i])(File,g_ConfigCsvFile[i])==false)
		{
			TRACE("读取配置文件[%s]失败!",g_ConfigCsvFile[i]);
			return false;
		}
	}

	//计算客户端配置文件md5

	m_vectClientCnfgFile = FileSystem::GetAllFileName("Config/client/");

	std::vector<std::string> vectCommon = FileSystem::GetAllFileName("Config/clientcommon/");

	std::string strMd5;

	ICryptService * pCryptService =  ::GetBasicService()->GetCryptService();

	for(int i=0; i<m_vectClientCnfgFile.size();i++)
	{
		TMD5  md5 =  pCryptService->CalculateMD5FromFile(m_vectClientCnfgFile[i].c_str());

		strMd5 += md5.toString();
	}

	for(int i=0; i<vectCommon.size();i++)
	{
		TMD5  md5 =  pCryptService->CalculateMD5FromFile(vectCommon[i].c_str());

		strMd5 += md5.toString();
	}

	//最后再做一次md5

	this->m_strClientVersion = pCryptService->CalculateMD5((const UINT8*)strMd5.c_str(),strMd5.length()).toString();

	TRACE("客户端配置文件版本[%s]",this->m_strClientVersion.c_str());

	//计算客户端资源文件md5

	std::string strResMd5;

	m_vectClientResFile = FileSystem::GetAllFileName("Config/res/");
	
	for(int i=0; i<m_vectClientResFile.size();i++)
	{
		TMD5  md5 =  pCryptService->CalculateMD5FromFile(m_vectClientResFile[i].c_str());

		strResMd5 += md5.toString();
	}

	//最后再做一次md5

	this->m_strClientResVersion = pCryptService->CalculateMD5((const UINT8*)strResMd5.c_str(),strResMd5.length()).toString();

	TRACE("客户端资源文件版本[%s]",this->m_strClientResVersion.c_str());


	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigServer::OnLoadGameConfig(FileConfig & File,const char* szFileName )
{
	if(File.Read(m_GameConfigParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}


   //服务器端杂项游戏参数
  bool ConfigServer::OnLoadServerGameConfig(FileConfig & File,const char* szFileName)
  {
	  if(File.Read(m_GameConfigParam.m_ServerConfigParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
  }

//////////////////////////////////////////////////////////////////////////////////////////////
//杂项配置
const SGameConfigParam & ConfigServer::GetGameConfigParam()
{
	return m_GameConfigParam;
}

////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

bool  ConfigServer::OnLoadMap(FileCSV & File,const char* szFileName)
{
	if(File.GetTable(m_vectMapConfigInfo)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<m_vectMapConfigInfo.size();i++)
	{
		SMapConfigInfo & Info = m_vectMapConfigInfo[i];		
	}

	return true;
}



//获得地图所有配置信息
std::vector<SMapConfigInfo> & ConfigServer::GetAllMapConfigInfo()
{
	return m_vectMapConfigInfo;
}

//获得单个地图配置信息
const SMapConfigInfo * ConfigServer::GetMapConfigInfo(TMapID mapid)
{
	for(int i=0; i<m_vectMapConfigInfo.size();++i)
	{
		if(m_vectMapConfigInfo[i].m_MapID == mapid)
		{
			return &m_vectMapConfigInfo[i];
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////

bool ConfigServer::OnLoadCreateActor(FileCSV & File,const char* szFileName)
{
	return File.GetRow(1,m_CreateActorCnfg);
}

//创建角色数据配置
const SCreateActorCnfg & ConfigServer::GetCreateActorCnfg()
{
	return m_CreateActorCnfg;
}

///////////////////////////////////////////////////////////////////////////////////////////

//怪物配置
bool ConfigServer::OnLoadMonster(FileCSV & File,const char* szFileName)
{
	std::vector<SMonsterCnfg> vect;
	if(File.GetTable(vect)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vect.size();i++)
	{
		m_MapMonster[vect[i].m_MonsterID] = vect[i];
	}

	return true;
}

//获得怪物配置数据
const SMonsterCnfg*  ConfigServer::GetMonsterCnfg(TMonsterID MonsterID)
{
	MAP_MONSTER::iterator it = m_MapMonster.find(MonsterID);
	if(it == m_MapMonster.end() )
	{
		return NULL;
	}

	return &(it->second);
}

//增加怪物配置
void ConfigServer::Push_MonsterCnfg(const SMonsterCnfg & MonsterCnfg)
{
	m_MapMonster[MonsterCnfg.m_MonsterID] = MonsterCnfg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//伏魔洞
bool ConfigServer::OnLoadFuMoDong(FileCSV & File,const char* szFileName)
{	
	if(File.GetTable(m_vectFuMoDongConfigInfo)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	return true;
}

//获得伏魔洞配置数据
const SFuMoDongCnfg* ConfigServer::GetFuMoDongCnfg(UINT8 level)
{
	for(int i=0; i<m_vectFuMoDongConfigInfo.size();i++)
	{
		if(m_vectFuMoDongConfigInfo[i].m_Level==level)
		{
			return &m_vectFuMoDongConfigInfo[i];
		}
	}
	return 0;
}

//根据玩家等级获得伏魔洞对应层配置
const SFuMoDongCnfg* ConfigServer::GetFuMoDongCnfgByUserLv(UINT8 UserLevel)
{
	int index = 0;

	for ( int i = 0; i < m_vectFuMoDongConfigInfo.size(); ++i )
	{
		if ( m_vectFuMoDongConfigInfo[i].m_ActorLevel <= UserLevel ){
			index = i;
			continue;
		}

		if ( m_vectFuMoDongConfigInfo[i].m_ActorLevel > UserLevel ){
			break;
		}
	}

	return &m_vectFuMoDongConfigInfo[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool ConfigServer::OnLoadGoods(FileCSV & File,const char* szFileName)
{
	std::vector<SGoodsCnfg> vectGoods;
	if(File.GetTable(vectGoods)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vectGoods.size();i++)
	{
		std::hash_map<TGoodsID,SGoodsCnfg>::iterator it = m_mapGoodsCnfg.find(vectGoods[i].m_GoodsID);
		if(it != m_mapGoodsCnfg.end())
		{
			TRACE("<error> %s : %d line 重复的物品ID[%d]",__FUNCTION__,__LINE__,vectGoods[i].m_GoodsID);
		}
		m_mapGoodsCnfg[vectGoods[i].m_GoodsID] = vectGoods[i];
	}

	return true;
}

//装备
bool ConfigServer::OnLoadEquipment(FileCSV & File,const char* szFileName)
{
	std::vector<SGoodsCnfg> vectGoods;
	if(File.GetTable(vectGoods)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vectGoods.size();i++)
	{
		m_mapGoodsCnfg[vectGoods[i].m_GoodsID] = vectGoods[i];
	}


	return true;
}

//仙剑
bool ConfigServer::OnLoadGodSword(FileCSV & File,const char* szFileName)
{
	std::vector<SGoodsCnfg> vectGoods;
	if(File.GetTable(vectGoods)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vectGoods.size();i++)
	{
		m_mapGoodsCnfg[vectGoods[i].m_GoodsID] = vectGoods[i];
	}

	return true;
}

//法宝
bool ConfigServer::OnLoadTalisman(FileCSV & File,const char* szFileName)
{
	std::vector<SGoodsCnfg> vectGoods;
	if(File.GetTable(vectGoods)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vectGoods.size();i++)
	{
		m_mapGoodsCnfg[vectGoods[i].m_GoodsID] = vectGoods[i];
	}

	return true;
}

//物品配置数据
const SGoodsCnfg*  ConfigServer::GetGoodsCnfg(TGoodsID GoodsID)
{
	std::hash_map<TGoodsID,SGoodsCnfg>::iterator it = m_mapGoodsCnfg.find(GoodsID);

	if(it == m_mapGoodsCnfg.end())
	{
		return NULL;
	}
	return &(*it).second ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//物品合成
bool ConfigServer::OnLoadGoodsCompose(FileCSV & File,const char* szFileName)
{
	std::vector<SGoodsComposeCnfg> vectGoods;
	if(File.GetTable(vectGoods)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for(int i=0; i<vectGoods.size();i++)
	{
		m_mapGoodsComposeCnfg[vectGoods[i].m_GoodsID] = vectGoods[i];
	}

	return true;
}

//物品合成配置
const SGoodsComposeCnfg * ConfigServer::GetGoodsComposeCnfg(TGoodsID GoodsID) 
{
	std::hash_map<TGoodsID,SGoodsComposeCnfg>::iterator it= m_mapGoodsComposeCnfg.find(GoodsID);
	if(it == m_mapGoodsComposeCnfg.end())
	{
		return 0;
	}
	return &(*it).second;
}

//得到所有物品合成配置
const std::hash_map<TGoodsID, SGoodsComposeCnfg> * ConfigServer::GetAllGoodsComposeCnfg()
{
	return &m_mapGoodsComposeCnfg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//强化配置
bool ConfigServer::OnLoadEquipStronger(FileCSV & File,const char* szFileName)
{	
	std::vector<SEquipStrongerCnfg>  vectStrongCnfg;
	if(File.GetTable(vectStrongCnfg)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	for( int i = 0; i < vectStrongCnfg.size(); ++i)
	{
		UINT8 key = vectStrongCnfg[i].m_level * 10 + vectStrongCnfg[i].m_Type;
		m_mapStrongerCnfg[key] = vectStrongCnfg[i];
	}

	return true;
}

//强化配置
const SEquipStrongerCnfg * ConfigServer::GetEquipStrongerCnfg(UINT8 key)
{
	std::hash_map<UINT8/*十位数为强化到等级，个位数为强化类型*/,SEquipStrongerCnfg>::iterator iter = m_mapStrongerCnfg.find(key);

	if( iter != m_mapStrongerCnfg.end()){
		return &(iter->second);
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//宝石配置
bool ConfigServer::OnLoadGemCnfg(FileCSV & File,const char* szFileName)
{
	if(File.GetTable(m_vectGemCnfg)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}


	return true;
}

//宝石配置
const SGemCnfg *   ConfigServer::GetGemCnfg(UINT8  GemType)
{
	for(int i=0; i<m_vectGemCnfg.size();++i)
	{
		if(m_vectGemCnfg[i].m_GemType==GemType)
		{
			return &m_vectGemCnfg[i];
		}
	}

	return 0;
}

//招募角色配置
bool ConfigServer::OnLoadEmployeeData(FileCSV & File, const char* szFileName)
{
	std::vector<SEmployeeDataCnfg> vectEmployee;
	if(File.GetTable(vectEmployee) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	m_nFreeEmployRate  = 0;
	m_nMoneyEmployRate = 0;

	for(int i = 0; i < vectEmployee.size(); ++i)
	{
		m_mapEmployeeDataCnfg[vectEmployee[i].m_EmployeeID] = vectEmployee[i];

		m_nFreeEmployRate  += vectEmployee[i].m_FreeFlushRate;
		m_nMoneyEmployRate += vectEmployee[i].m_FlushRate;
	}

	return true;
}

//得到招募角色配置
const SEmployeeDataCnfg * ConfigServer::GetEmployeeDataCnfg(TEmployeeID EmployeeID)
{
	std::hash_map<TEmployeeID, SEmployeeDataCnfg>::iterator iter = m_mapEmployeeDataCnfg.find(EmployeeID);

	if(iter == m_mapEmployeeDataCnfg.end())
	{
		return 0;
	}
	return &(*iter).second;
}

//随机(每条有机率大小不同)获取一个招募角色数据
const SEmployeeDataCnfg * ConfigServer::RandGetEmployData(enFlushType FlushType)
{
	int nRandom = 0;

	if( 0 == m_nFreeEmployRate || 0 == m_nMoneyEmployRate){
		TRACE("<error> %s : %d 行 招募角色配置错误,刷新总概率为0,刷新方式 = %d", __FUNCTION__, __LINE__, FlushType);
		return 0;
	}

	if(enFlushType_Free == FlushType){
		nRandom = RandomService::GetRandom() % m_nFreeEmployRate;
	}else if(enFlushType_Money == FlushType){
		nRandom = RandomService::GetRandom() % m_nMoneyEmployRate;
	}

	int nTemp = 0;

	std::hash_map<TEmployeeID, SEmployeeDataCnfg>::iterator it = m_mapEmployeeDataCnfg.begin();

	for(; it != m_mapEmployeeDataCnfg.end(); ++it)
	{
		SEmployeeDataCnfg & EmployeeTemp = it->second;

		//免费刷新和付费刷新的几率不同
		if(enFlushType_Free == FlushType){
			nTemp += EmployeeTemp.m_FreeFlushRate;
		}else if(enFlushType_Money == FlushType){
			nTemp += EmployeeTemp.m_FlushRate;
		}

		if(nTemp > nRandom)
		{
			return &(it->second);
		}
	}

	return 0;
}

//随机获得指定资质的招募角色
const SEmployeeDataCnfg * ConfigServer::RandGetEmployApt(UINT32 Aptitude, enFlushType FlushType)
{
	UINT32 nTotalRand = 0;

	std::hash_map<TEmployeeID, SEmployeeDataCnfg>::iterator it = m_mapEmployeeDataCnfg.begin();

	for(; it != m_mapEmployeeDataCnfg.end(); ++it)
	{
		SEmployeeDataCnfg & EmployeeTemp = it->second;

		if( EmployeeTemp.m_Aptitude != Aptitude){
			continue;
		}

		if( FlushType == enFlushType_Free){
			nTotalRand += EmployeeTemp.m_FreeFlushRate;
		}else if( FlushType == enFlushType_Money){
			nTotalRand += EmployeeTemp.m_FlushRate;
		}
	}

	int nRandom = RandomService::GetRandom() % nTotalRand;

	int temp = 0;

	it = m_mapEmployeeDataCnfg.begin();

	for(; it != m_mapEmployeeDataCnfg.end(); ++it)
	{
		SEmployeeDataCnfg & EmployeeTemp = it->second;

		if( EmployeeTemp.m_Aptitude != Aptitude){
			continue;
		}

		if( FlushType == enFlushType_Free){
			temp += EmployeeTemp.m_FreeFlushRate;
		}else if( FlushType == enFlushType_Money){
			temp += EmployeeTemp.m_FlushRate;
		}

		if(temp > nRandom){
			return &(*it).second;
		}
	}

	return 0;
}

//法术书配置
bool ConfigServer::OnLoadMagicBook(FileCSV & File, const char* szFileName)
{
	std::vector<SMagicBookCnfg> vectMagicBook;

	if(File.GetTable(vectMagicBook) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	m_nFreeMagicBookRate  = 0;
	m_nMoneyMagicBookRate = 0;

	for(int i = 0; i < vectMagicBook.size(); ++i)
	{
		m_mapMagicBookCnfg[vectMagicBook[i].m_MagicBookID] = vectMagicBook[i];

		m_nFreeMagicBookRate  += vectMagicBook[i].m_FreeFlushRate;
		m_nMoneyMagicBookRate += vectMagicBook[i].m_FlushRate;
	}
	return true;
}

//得到法术书配置
const SMagicBookCnfg * ConfigServer::GetMagicBookCnfg(TMagicBookID MagicBookID)
{
	std::hash_map<TMagicBookID, SMagicBookCnfg>::iterator it = m_mapMagicBookCnfg.find(MagicBookID);

	if(it == m_mapMagicBookCnfg.end()){
		return 0;
	}

	return &(*it).second;
}

//随机(每条有机率大小不同)获取一个法术书数据
const SMagicBookCnfg * ConfigServer::RandGetMagicBookCnfg(enFlushType FlushType)
{
	int nRandom = 0;

	if( 0 == m_nFreeMagicBookRate || 0 == m_nMoneyMagicBookRate){
		TRACE("<error> %s : %d 行 法术书配置错误,刷新总概率为0,刷新方式 = %d", __FUNCTION__, __LINE__, FlushType);
		return 0;
	}

	if(enFlushType_Free == FlushType){
		nRandom = RandomService::GetRandom() % m_nFreeMagicBookRate;
	}else if(enFlushType_Money == FlushType){
		nRandom = RandomService::GetRandom() % m_nMoneyMagicBookRate;
	}

	int nTemp = 0;

	std::hash_map<TMagicBookID, SMagicBookCnfg>::iterator it = m_mapMagicBookCnfg.begin();

	for(; it != m_mapMagicBookCnfg.end(); ++it)
	{
		SMagicBookCnfg & MagicBookCnfg = it->second;

		//免费刷新和付费刷新的几率不同
		if(enFlushType_Free == FlushType){
			nTemp += MagicBookCnfg.m_FreeFlushRate;
		}else if(enFlushType_Money == FlushType){
			nTemp += MagicBookCnfg.m_FlushRate;
		}

		if(nTemp > nRandom)
		{
			return &(it->second);
		}
	}

	return 0;
}

//商城物品配置
bool ConfigServer::OnLoadShopMall(FileCSV & File, const char* szFileName)
{
	//std::vector<SShopMallCnfg> vectShopMallCnfg;

	//if(File.GetTable(vectShopMallCnfg) == false){
	//	TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
	//	return false;
	//}

	//for(int i = 0; i < vectShopMallCnfg.size(); ++i){

	//	SShopMallCnfg & ShopMallCnfg =  vectShopMallCnfg[i];

	//	for(int n = 0; n < ShopMallCnfg.m_vecKind.size(); ++n){
	//		INT32 lable = ShopMallCnfg.m_vecKind[n];
	//		if(lable>=enShopMall_Max)
	//		{
	//			TRACE("%s : %d Line 读取配置文件[%s] 类型标签值%d有误失败!", __FUNCTION__, __LINE__, szFileName,lable);
	//			continue;
	//		}

	//		MAP_MALL & Map = m_vecShopMallCnfgByLabel[lable];

	//		Map[ShopMallCnfg.m_GoodsID] = ShopMallCnfg;
	//	}
	//}

	return true;
}

//得到商城抒写商店的物品配置集合
//const std::hash_map<TGoodsID, SShopMallCnfg> * ConfigServer::GetShopMallCnfgByLable(enShopMallLabel label)
//{
//	if(label >= m_vecShopMallCnfgByLabel.size()){
//		return 0;
//	}
//	return &m_vecShopMallCnfgByLabel[label];
//}

//放进商场配置表
//void  ConfigServer::Push_ShopMallCnfg(const SShopMallCnfg & ShopMallCnfg)
//{
//	if ( ShopMallCnfg.m_Type >= enShopMall_Max){
//		
//		TRACE("<error> %s : %d Line 商场配置信息出错,类型 = %d", __FUNCTION__, __LINE__, ShopMallCnfg.m_Type);
//		return;
//	}
//
//	MAP_MALL & Map = m_vecShopMallCnfgByLabel[ShopMallCnfg.m_Type];
//
//	Map[ShopMallCnfg.m_GoodsID] = ShopMallCnfg;	
//}
//
////清除商城配置信息
//void  ConfigServer::ClearShopMallCnfg()
//{
//	m_vecShopMallCnfgByLabel.clear();
//}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//法宝品质对应的孕育物品
bool  ConfigServer::OnLoadTalismanQuality(FileCSV & File, const char* szFileName)
{
	std::vector<STalismanQualityCnfg> vectTalismanbQualityCnfg;

	if(File.GetTable(vectTalismanbQualityCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectTalismanbQualityCnfg.size(); ++i){

		STalismanQualityCnfg & QualityCnfg = vectTalismanbQualityCnfg[i];

		QualityCnfg.m_TotalProb = 0;

		for(int n=0; n<QualityCnfg.m_vectGoodsAndProb.size()/2; n++)
		{
			QualityCnfg.m_TotalProb += QualityCnfg.m_vectGoodsAndProb[n*2+1];
		}

		UINT32 Key = ((UINT32)QualityCnfg.m_QualityLevel<<16) | ((UINT32)QualityCnfg.m_TalismanWorldLevel << 8) | QualityCnfg.m_TalismanWorldType;

		m_mapTalismanQuality[Key] = QualityCnfg;

		if(0 == QualityCnfg.m_TotalProb)
		{
			TRACE("%s : %d Line 取配置文件[%s]品质[%d]法宝世界级别[%d]孕育物品总概率为零!", __FUNCTION__, __LINE__, szFileName,QualityCnfg.m_QualityLevel,QualityCnfg.m_TalismanWorldLevel);

		}


	}



	return true;
}

//根据法宝品质级别及法宝世界级别，随机获取一个孕育物品
TGoodsID  ConfigServer::GetGestateGoodsID(UINT8 QualityLevel,UINT8 TalismanWorldLevel,UINT8 TalismanWorldType)
{
	UINT32 Key = ((UINT32)QualityLevel<<16) | ((UINT32)TalismanWorldLevel << 8) | TalismanWorldType;

	MAP_QUALITY::iterator it = m_mapTalismanQuality.find(Key);

	if(it == m_mapTalismanQuality.end())
	{
		return INVALID_GOODS_ID;
	}

	STalismanQualityCnfg & QualityCnfg = (*it).second;

	if(0 == QualityCnfg.m_TotalProb)
	{
		return INVALID_GOODS_ID;
	}


	UINT32 nRandom = RandomService::GetRandom() % QualityCnfg.m_TotalProb;

	for(int i=0; i<QualityCnfg.m_vectGoodsAndProb.size()/2;++i)
	{
		if(nRandom<QualityCnfg.m_vectGoodsAndProb[i*2+1])
		{
			return QualityCnfg.m_vectGoodsAndProb[i*2];
		}

		nRandom -= QualityCnfg.m_vectGoodsAndProb[i*2+1];
	}

	return INVALID_GOODS_ID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//法宝品质等级划分
bool ConfigServer::OnLoadTalismanQualityLevel(FileCSV & File, const char* szFileName)
{
	if(File.GetTable(m_TalismanQualityLevel) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}


	return true;
}

//根据品质点获取品质等级
UINT8 ConfigServer::GetTalismanQualityLevel(INT32 QualityPoint) 
{
	UINT8 Level = 0;

	for(int i=0; i<m_TalismanQualityLevel.size();++i)
	{
		if(QualityPoint>m_TalismanQualityLevel[i].m_QualityPoint)
		{
			Level++;
		}
		else
		{
			break;
		}
	}

	return Level;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//效果
bool ConfigServer::OnLoadEffect(FileCSV & File, const char* szFileName)
{
	std::vector<SEffectCnfg> vectEffect;

	if(File.GetTable(vectEffect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectEffect.size();++i)
	{
		m_mapEffect[vectEffect[i].m_EffectID] = vectEffect[i];
	}

	return true;
}

//获得效果配置信息
const SEffectCnfg * ConfigServer::GetEffectCnfg(TEffectID  EffectID)
{
	std::hash_map<TEffectID,  SEffectCnfg>::iterator it = m_mapEffect.find(EffectID);

	if(it == m_mapEffect.end())
	{
		return NULL;
	}

	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//状态
bool ConfigServer::OnLoadStatus(FileCSV & File, const char* szFileName)
{
	std::vector<SStatusCnfg> vectStatus;

	if(File.GetTable(vectStatus) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectStatus.size();++i)
	{
		m_mapStatus[vectStatus[i].m_StatusID] = vectStatus[i];
	}

	return true;
}

//获得状态配置信息
const SStatusCnfg * ConfigServer::GetStatusCnfg(TStatusID   StatusID)
{

	MAP_STATUS::iterator it = m_mapStatus.find(StatusID);

	if(it == m_mapStatus.end())
	{
		return NULL;
	}

	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////
//状态组
bool ConfigServer::OnLoadStatusGroup(FileCSV & File, const char* szFileName)
{
	std::vector<SStatusGroupCnfg> vectStatusGroup;

	if(File.GetTable(vectStatusGroup) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;	
	}

	for( int i = 0; i < vectStatusGroup.size(); ++i)
	{
		m_mapStatusGroup[vectStatusGroup[i].m_StatusGroupID] = vectStatusGroup[i];
	}

	return true;
}


//获取状态组配置
const SStatusGroupCnfg * ConfigServer::GetStatusGroupCnfg(TStatusGroupID StatusGroupID)
{
	MAP_STATUSGROUP::iterator iter = m_mapStatusGroup.find(StatusGroupID);
	if( iter == m_mapStatusGroup.end()){
		return 0;
	}

	return &(iter->second);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法术信息
bool ConfigServer::OnLoadMagic(FileCSV & File, const char* szFileName)
{
	std::vector<SMagicCnfg> vectMagic;

	if(File.GetTable(vectMagic) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectMagic.size();++i)
	{
		m_mapMagicCnfg[vectMagic[i].m_MagicID] = vectMagic[i];
	}

	return true;

}

//获取法术配置信息
const SMagicCnfg * ConfigServer::GetMagicCnfg(TMagicID MagicID)
{
	std::hash_map<TMagicID,SMagicCnfg>::iterator it = m_mapMagicCnfg.find(MagicID);

	if(m_mapMagicCnfg.end() == it)
	{
		return 0;
	}

	return &(*it).second;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////

//法术等级信息
bool ConfigServer::OnLoadMagicLevel(FileCSV & File, const char* szFileName)
{
	std::vector<SMagicLevelCnfg> vectMagic;

	if(File.GetTable(vectMagic) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectMagic.size();++i)
	{
		UINT32 key = ((UINT32)vectMagic[i].m_MagicID << 16) | vectMagic[i].m_Level;

		m_mapMagicLevel[key] = vectMagic[i];
	}

	return true;
}

//获取法术等级配置信息
const SMagicLevelCnfg * ConfigServer::GetMagicLevelCnfg(TMagicID MagicID, UINT8 Level)
{

	UINT32 key = ((UINT32)MagicID << 16) | Level;
	MAP_MAGIC_LEVEL::iterator it = m_mapMagicLevel.find(key);

	if(m_mapMagicLevel.end() == it)
	{
		return 0;
	}

	return &(*it).second;
}

//获取该法术的所有等级信息
const std::vector<SMagicLevelCnfg *> ConfigServer::GetMagicAllLevelCnfg(TMagicID MagicID)
{
	std::vector<SMagicLevelCnfg *> vectLevelCnfg;

	UINT8 Level = 1;
	UINT32 key	= 1;

	MAP_MAGIC_LEVEL::iterator it;

	do{
		key = ((UINT32)MagicID << 16) | Level;

		it = m_mapMagicLevel.find(key);

		if( it == m_mapMagicLevel.end()){
			break;
		}else{
			vectLevelCnfg.push_back(&(it->second));
		}
	}
	while( ++Level);

	return vectLevelCnfg;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CmpLess(const SMonsterOutput & first,const SMonsterOutput & second)
{
	return first.m_nLineup < second.m_nLineup;
}

//地图上的怪物
bool ConfigServer::OnLoadMapMonster(FileCSV & File, const char* szFileName)
{
	std::vector<SMonsterOutput> vectMonster;

	if(File.GetTable(vectMonster) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectMonster.size();++i)
	{		
		m_mapMapMonster[vectMonster[i].m_MapID].push_back(vectMonster[i]);
	}

	//需要对每张地图上的怪按阵型排序
	for(MAP_MAP_MONSTER::iterator it =m_mapMapMonster.begin(); it !=m_mapMapMonster.end(); ++it )
	{
		std::vector<SMonsterOutput> & vect = (*it).second;
		std::sort(vect.begin(),vect.end(),CmpLess);
	}

	return true;
}

//获得单个地图怪物分布信息
const std::vector<SMonsterOutput> * ConfigServer::GetMapMonsterCnfg(TMapID mapid)
{
	MAP_MAP_MONSTER::iterator it = m_mapMapMonster.find(mapid);
	if(it == m_mapMapMonster.end())
	{
		return 0;
	}

	return &(*it).second;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//帮派配置信息
bool ConfigServer::OnLoadSyndicate(FileCSV & File, const char* szFileName)
{
	std::vector<SSyndicateCnfg> vectSyndicate;

	if(File.GetTable(vectSyndicate) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i= 0; i < vectSyndicate.size();++i)
	{
		m_mapSyndicate[vectSyndicate[i].m_Level] = vectSyndicate[i];
	}

	return true;
}

//获取帮派配置信息
const SSyndicateCnfg * ConfigServer::GetSyndicateCnfg(UINT8 Level)
{
	std::hash_map<UINT8,  SSyndicateCnfg>::iterator iter = m_mapSyndicate.find(Level);

	if(iter == m_mapSyndicate.end())
	{
		return 0;
	}

	return &(*iter).second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//副本配置信息
bool ConfigServer::OnLoadFuBen(FileCSV & File, const char* szFileName)
{
	std::vector<SFuBenCnfg> vectFuBen;

	if(File.GetTable(vectFuBen) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectFuBen.size();++i)
	{

		m_mapFuBen[vectFuBen[i].m_FuBenID] = vectFuBen[i];
	}

	return true;
}


//副本配置信息
const SFuBenCnfg * ConfigServer::GetFuBenCnfg(TFuBenID FuBenID)
{

	MAP_FUBEN::iterator it = m_mapFuBen.find(FuBenID);

	if(it == m_mapFuBen.end())
	{
		return 0;
	}

	return &(*it).second;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//仙剑副本配置
bool ConfigServer::OnLoadGoldSwordFuBen(FileCSV & File, const char* szFileName)
{
	std::vector<SGodSwordFuBenCnfg> vectFuBen;

	if(File.GetTable(vectFuBen) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectFuBen.size();++i)
	{
		UINT16 key = ((UINT16)vectFuBen[i].m_FuBenID << 8) | vectFuBen[i].m_FuBenLevel;

		m_mapGodSwordFuBen[key] = vectFuBen[i];
	}

	return true;
}

//获得仙剑副本
const SGodSwordFuBenCnfg * ConfigServer::GetGodSwordFuBenCnfg(TFuBenID FuBenID, UINT8 Level)
{
	UINT16 key = ((UINT16)FuBenID << 8) | Level;

	MAP_GODSWORD_FUBEN::iterator it = m_mapGodSwordFuBen.find(key);

	if(it == m_mapGodSwordFuBen.end())
	{
		return 0;
	}

	return &(*it).second;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//帮派物品配置信息
bool ConfigServer::OnLoadSynGoods(FileCSV & File, const char* szFileName)
{
	std::vector<SSynGoodsCnfg>  vectSynGoodsCnfg;

	if(File.GetTable(vectSynGoodsCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectSynGoodsCnfg.size(); ++i)
	{
		m_mapSyndicateGoods[vectSynGoodsCnfg[i].m_GoodsID] = vectSynGoodsCnfg[i];
	}
	return true;
}

//获取所有帮派物品配置信息
const std::hash_map<TGoodsID, SSynGoodsCnfg> * ConfigServer::GetAllSynGoodsCnfg()
{
	return &m_mapSyndicateGoods;
}

//帮派技能配置信息
bool ConfigServer::OnLoadSynMagic(FileCSV & File, const char* szFileName)
{
	std::vector<SSynMagicCnfg>  vectSynMagic;

	if( File.GetTable(vectSynMagic) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectSynMagic.size(); ++i)
	{
		std::hash_map<TSynMagicID, VECTSYNMAGIC>::iterator iter = m_mapSynMagic.find(vectSynMagic[i].m_SynMagicID);
		if( iter == m_mapSynMagic.end()){
			VECTSYNMAGIC vectMagic;
			vectMagic.push_back(vectSynMagic[i]);
			m_mapSynMagic[vectSynMagic[i].m_SynMagicID]  = vectMagic;
		}else{
			VECTSYNMAGIC & vectMagic = iter->second;
			vectMagic.push_back(vectSynMagic[i]);
		}
	}
	return true;
}

//获取所有帮派技能的配置信息
const std::vector<SSynMagicCnfg> ConfigServer::GetAllSynMagicCnfg()
{
	std::vector<SSynMagicCnfg> vectSynMagic;

	std::hash_map<TSynMagicID, VECTSYNMAGIC>::iterator iter = m_mapSynMagic.begin();

	for(; iter != m_mapSynMagic.end(); ++iter)
	{
		VECTSYNMAGIC & vectMagic = iter->second;

		for( int i = 0; i < vectMagic.size(); ++i)
		{
			vectSynMagic.push_back(vectMagic[i]);
		}
	}
	return vectSynMagic;
}

//获取帮派技能的配置信息
const std::vector<SSynMagicCnfg> * ConfigServer::GetSynMagicCnfg(TSynMagicID SynMagicID)
{
	std::hash_map<TSynMagicID, VECTSYNMAGIC>::iterator iter = m_mapSynMagic.find(SynMagicID);
	if( iter == m_mapSynMagic.end()){
		return 0;
	}

	return &(iter->second);
}

//获取帮派技能的配置信息
const SSynMagicCnfg * ConfigServer::GetSynMagicCnfg(TSynMagicID SynMagicID, UINT8 Level)
{
	std::hash_map<TSynMagicID, VECTSYNMAGIC>::iterator iter = m_mapSynMagic.find(SynMagicID);
	if( iter == m_mapSynMagic.end()){
		return 0;
	}

	VECTSYNMAGIC & vectMagic = iter->second;

	for( int i = 0; i < vectMagic.size(); ++i)
	{
		SSynMagicCnfg & MagicCnfg = vectMagic[i];

		if( MagicCnfg.m_SynMagicLevel == Level){
			return &MagicCnfg;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//物品使用配置信息
bool  ConfigServer::OnLoadGoodsUse(FileCSV & File, const char* szFileName)
{
	std::vector<SGoodsUseCnfg>  vectGoodsUseCnfg;

	if(File.GetTable(vectGoodsUseCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectGoodsUseCnfg.size(); ++i)
	{
		SGoodsUseCnfg & GoodsUseCnfg = vectGoodsUseCnfg[i];
		MAP_GOODSUSE::iterator it = m_mapGoodsUse.find(GoodsUseCnfg.m_GoodsID);
		if(it != m_mapGoodsUse.end())
		{
			TRACE("%s : %d Line 重复的物品ID[%d]!", __FUNCTION__, __LINE__, GoodsUseCnfg.m_GoodsID);
		}

		m_mapGoodsUse[GoodsUseCnfg.m_GoodsID] = GoodsUseCnfg;
	}

	return true;
}

//物品使用配置
const SGoodsUseCnfg * ConfigServer::GetGoodsUseCnfg(TGoodsID GoodsID)
{
	MAP_GOODSUSE::iterator it = m_mapGoodsUse.find(GoodsID);

	if(it == m_mapGoodsUse.end())
	{
		return 0;
	}

	return &(*it).second;
}


//剑冢配置
bool ConfigServer::OnLoadGodSwordShop(FileCSV & File, const char* szFileName)
{
	std::vector<SGodSwordCnfg>  vectGodSwordCnfg;

	if( File.GetTable(vectGodSwordCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	m_TotalFreeGodSwordRate	 = 0;
	m_TotalMoneyGodSwordRate = 0;

	for(int i = 0; i < vectGodSwordCnfg.size(); ++i)
	{
		m_mapGodSword[vectGodSwordCnfg[i].m_GodSwordID] = vectGodSwordCnfg[i];

		m_TotalFreeGodSwordRate += vectGodSwordCnfg[i].m_FreeFlushRate;
		m_TotalMoneyGodSwordRate += vectGodSwordCnfg[i].m_MoneyFlushRate;
	}
	return true;
}

//得到剑冢的配置信息
const SGodSwordCnfg * ConfigServer::GetGodSwordShopCnfg(TGoodsID GodSwordID)
{
	std::hash_map<TGoodsID, SGodSwordCnfg>::iterator  iter = m_mapGodSword.find(GodSwordID);

	if( iter == m_mapGodSword.end()){
		return 0;
	}

	return &(iter->second);
}

//随机获取一个仙剑数据
const SGodSwordCnfg * ConfigServer::RandGetGodSwordShopCnfg(enFlushType FlushType)
{
	int nRandom = 0;

	if( 0 == m_TotalFreeGodSwordRate || 0 == m_TotalMoneyGodSwordRate){
		TRACE("<error> %s : %d 行 仙剑配置错误,刷新总概率为0,刷新方式 = %d", __FUNCTION__, __LINE__, FlushType);
		return 0;
	}

	if( enFlushType_Free == FlushType){
		nRandom = RandomService::GetRandom() % m_TotalFreeGodSwordRate;
	}else if( enFlushType_Money == FlushType){
		nRandom = RandomService::GetRandom() % m_TotalMoneyGodSwordRate;
	}

	int nTemp = 0;

	std::hash_map<TGoodsID, SGodSwordCnfg>::iterator iter = m_mapGodSword.begin();

	for(; iter != m_mapGodSword.end(); ++iter)
	{
		SGodSwordCnfg & GodSwordCnfg = iter->second;

		//免费刷新和付费刷新的几率不同
		if(enFlushType_Free == FlushType){
			nTemp += GodSwordCnfg.m_FreeFlushRate;
		}else if(enFlushType_Money == FlushType){
			nTemp += GodSwordCnfg.m_MoneyFlushRate;
		}

		if(nTemp > nRandom)
		{
			return &(iter->second);
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//帮派保卫战配置
bool  ConfigServer::OnLoadSynCombat(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vectSynCombat) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//根据玩家等级获取合适的帮派保卫战模式
const SSynCombatCnfg *  ConfigServer::GetSynCombatCnfg(UINT8 ActorLevel) 
{
	for(int i=0; i<m_vectSynCombat.size();i++)
	{
		if( m_vectSynCombat[i].m_vectEnterLevel.size() < 2){
			continue;
		}

		if( ActorLevel >= m_vectSynCombat[i].m_vectEnterLevel[0] && ActorLevel <= m_vectSynCombat[i].m_vectEnterLevel[1]){
			return &m_vectSynCombat[i];
		}
	}

	return 0;
}

//根据模式获取配置信息
const SSynCombatCnfg * ConfigServer::GetSynCombatCnfgByMode(UINT8 Mode)
{
	if(Mode<1 || Mode>m_vectSynCombat.size())
	{
		return 0;
	}

	return &m_vectSynCombat[Mode-1];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//状态类型
bool ConfigServer::OnLoadStatusType(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vectStatusType) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}
	return true;
}
//状态类型参数
const SStatusTypeCnfg * ConfigServer::GetStatusTypeCnfg(UINT8  StatusType)
{
	for (VECT_STATUSTYPE::iterator it = m_vectStatusType.begin(); it != m_vectStatusType.end(); ++it)
	{
		SStatusTypeCnfg & TypeCnfg = *it;
		if(TypeCnfg.m_StatusType == StatusType)
		{
			return &TypeCnfg;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//角色等级
bool ConfigServer::OnLoadActorLevel(FileCSV & File, const char* szFileName)
{
	std::vector<SActorLevelCnfg> vectLevel;
	if( File.GetTable(vectLevel) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i=0; i<vectLevel.size();i++)
	{
		m_mapActorLevelCnfg[vectLevel[i].m_Level] = vectLevel[i];
	}

	return true;
}


//获得角色等级配置
const SActorLevelCnfg * ConfigServer::GetActorLevelCnfg(UINT8 Level)
{
	MAP_ACTORLEVEL::iterator it = m_mapActorLevelCnfg.find(Level);

	if(it == m_mapActorLevelCnfg.end())
	{
		return 0;
	}
	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//人物境界2提升
bool ConfigServer::OnLoadActorLayer(FileCSV & File, const char* szFileName)
{
	std::vector<SActorLayerCnfg>	 vectLayer;
	if( File.GetTable(vectLayer) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectLayer.size(); i++)
	{
		m_mapActorLayer[vectLayer[i].m_LayerLevel] = vectLayer[i];
	}

	return true;
}

//获得人物境界配置
const SActorLayerCnfg * ConfigServer::GetActorLayerCnfg(UINT8 Level)
{
	MAP_ACTORLAYER::iterator iter = m_mapActorLayer.find(Level);
	if( iter == m_mapActorLayer.end()){
		return 0;
	}

	return &(iter->second);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//掉落物品配置
bool ConfigServer::OnLoadDropGoods(FileCSV & File, const char* szFileName)
{
	std::vector<SDropGoods>		vectDropGoods;

	if( File.GetTable(vectDropGoods) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;		
	}

	for( int i = 0; i < vectDropGoods.size(); ++i)
	{
		SDropGoods & DropGoods = vectDropGoods[i];

		DropGoods.m_nTotalDropGoodsRand = 0;
		for( int k = 0; k < DropGoods.m_vectDropGoods.size()/3; k++)
		{
			DropGoods.m_nTotalDropGoodsRand += DropGoods.m_vectDropGoods[k*3+2];
		}

		m_mapDropGoods[DropGoods.m_DropID].push_back(DropGoods);
	}
	return true;
}

//获得掉落物品配置
const std::vector<SDropGoods> * ConfigServer::GetDropGoodsCnfg(UINT16 DropID)
{
	MAP_DROPGOODS::iterator iter = m_mapDropGoods.find(DropID);
	if( iter == m_mapDropGoods.end()){
		return 0;
	}

	return &(iter->second);
}

//获得副本主将的掉落ID, bOpenHardType为是否开启困难模式
UINT16	ConfigServer::GetMapBossDropID(TMapID MapID, bool bOpenHardType)
{
	const std::vector<SMonsterOutput> * pvectMonster = this->GetMapMonsterCnfg(MapID);

	if ( 0 == pvectMonster){

		return 0;
	}

	for ( int i = 0; i < (*pvectMonster).size(); ++i)
	{
		const SMonsterOutput & MonsterOutput = (*pvectMonster)[i];

		if ( MonsterOutput.m_MonsterType != enMonsterType_Boss){

			continue;
		}

		if ( bOpenHardType){

			return MonsterOutput.m_DropIDHard;
		}else{
		
			return MonsterOutput.m_DropID;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//修炼增加好友附加值
bool ConfigServer::OnLoadXiuLianFriendAdd(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vectXiuLianFriendAdd) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;		
	}

	return true;
}

//获得修炼时的好友附加值
const SXiuLianFriendAdd * ConfigServer::GetXiuLianFriendAddCnfg(INT32 nRelationNum)
{
	for( int i = 0; i < m_vectXiuLianFriendAdd.size(); ++i)
	{
		if( nRelationNum < m_vectXiuLianFriendAdd[i].m_RelationRand){
			return &m_vectXiuLianFriendAdd[i];
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//仙剑世界配置
bool ConfigServer::OnLoadGoldSwordWorld(FileCSV & File, const char* szFileName)
{
	std::vector<SGodSwordWorldCnfg>	 vectSwordWorld;
	if( File.GetTable(vectSwordWorld) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectSwordWorld.size(); i++)
	{
		m_mapSwordWorld[vectSwordWorld[i].m_SwordSecretID] = vectSwordWorld[i];
	}

	return true;
}

//仙剑世界配置
const SGodSwordWorldCnfg * ConfigServer::GetGoldSwordWorldCnfg(TSwordSecretID SwordSecretID)
{
	MAP_SWORDWORLD::iterator it = m_mapSwordWorld.find(SwordSecretID);

	if(it == m_mapSwordWorld.end())
	{
		return 0;
	}

	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//仙剑级别配置
bool ConfigServer::OnLoadGoldSwordLevel(FileCSV & File, const char* szFileName)
{
	std::vector<SGodSwordLevelCnfg>	 vectSwordWorld;
	if( File.GetTable(vectSwordWorld) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectSwordWorld.size(); i++)
	{
		m_mapSwordLevelCnfg[vectSwordWorld[i].m_Level] = vectSwordWorld[i];
	}

	return true;
}

//仙剑级别配置
const SGodSwordLevelCnfg * ConfigServer::GetGodSwordLevelCnfg(UINT8  Level)
{
	MAP_SWORDLEVEL::iterator it = m_mapSwordLevelCnfg.find(Level);

	if(it == m_mapSwordLevelCnfg.end())
	{
		return 0;
	}

	return &(*it).second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法宝世界
bool ConfigServer::OnLoadTalismanWorld(FileCSV & File, const char* szFileName)
{
	std::vector<STalismanWorldCnfg>	 vectTalismanWorld;
	if( File.GetTable(vectTalismanWorld) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectTalismanWorld.size(); i++)
	{
		m_mapTalismanWorld[vectTalismanWorld[i].m_TalismanWorldID] = vectTalismanWorld[i];
	}

	return true;
}
//法宝世界配置
const STalismanWorldCnfg * ConfigServer::GetTalismanWorldCnfg(TTalismanWorldID  TalismanWorldID)
{
	MAP_TALISMAN_WORLD::iterator it = m_mapTalismanWorld.find(TalismanWorldID);

	if( it == m_mapTalismanWorld.end())
	{
		return 0;
	}

	return &(*it).second;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//修炼类法宝世界
bool ConfigServer::OnLoadTalismanWorldXiuLian(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vectTalismanWorldXiuLian) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//修炼类法宝世界配置
const STalismanWorldXiuLianCnfg * ConfigServer::GetTalismanWorldXiuLianCnfg(TTalismanWorldID  TalismanWorldID)
{
	for(VECT_TALISMAN_WORLD_XIULIAN::iterator it = m_vectTalismanWorldXiuLian.begin(); it != m_vectTalismanWorldXiuLian.end(); ++it )
	{
		STalismanWorldXiuLianCnfg & Cnfg  = (*it);
		if(Cnfg.m_TalismanWorldID == TalismanWorldID)
		{
			return & Cnfg;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ConfigServer::OnLoadXiuLianGameConfig(FileConfig & File,const char* szFileName )
{
	if(File.Read(m_XiuLianGameParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}

//修炼游戏配置
const SXiuLianGameParam & ConfigServer::GetXiuLianGameParam()
{
	return m_XiuLianGameParam;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//修炼类法宝世界
bool ConfigServer::OnLoadTalismanWorldXiuLianAward(FileCSV & File, const char* szFileName)
{
	if(File.GetTable(m_vectXiuLianGameAward)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}


//修炼奖励
std::vector<SXiuLianGameAwardCnfg> & ConfigServer::GetXiuLianGameAwardCnfg()
{
	return m_vectXiuLianGameAward;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigServer::OnLoadPortal(FileCSV & File, const char* szFileName)
{
	std::vector<SPortalCnfg>	 vectPortal;
	if( File.GetTable(vectPortal) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectPortal.size(); i++)
	{
		m_mapPortal[vectPortal[i].m_PortalID] = vectPortal[i];
	}

	return true;
}

//传送门配置
const SPortalCnfg *  ConfigServer::GetPortalCnfg(TPortalID PortalID) 
{
	MAP_PORTAL ::iterator it = m_mapPortal.find(PortalID);

	if(it == m_mapPortal.end())
	{
		return 0;
	}

	return &(*it).second;
}


////////////////////////////////////////////////////////////////////////////
//冷却时间配置
bool ConfigServer::OnLoadCDTimeCnfg(FileCSV & File, const char* szFileName)
{
	std::vector<SCDTimeCnfg>	 vectCDTime;
	if( File.GetTable(vectCDTime) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectCDTime.size(); i++){
		m_mapCDTimeCnfg[vectCDTime[i].m_CDTime_ID] = vectCDTime[i].m_CDTime;
	}

	return true;
}

//获得冷却时间
UINT32 ConfigServer::GetCDTimeCnfg(TCDTimerID CDTime_ID)
{
	MAP_CDTIMECNFG::iterator iter = m_mapCDTimeCnfg.find(CDTime_ID);
	if( iter == m_mapCDTimeCnfg.end()){
		return 0;
	}

	return iter->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//奇遇配置
bool ConfigServer::OnLoadAdventureAwardCnfg(FileCSV & File,const char* szFileName)
{
	std::vector<SAdventureAwardCnfg> vectAdventureAward;

	if( File.GetTable(vectAdventureAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectAdventureAward.size(); ++i)
	{
		std::hash_map<TTalismanWorldID, MAP_ADVENTUREAWARD>::iterator iter = m_mapAdventureAwardCnfg.find(vectAdventureAward[i].m_TalismanWorldID);
		if( iter == m_mapAdventureAwardCnfg.end()){
			MAP_ADVENTUREAWARD mapAward;
			mapAward[vectAdventureAward[i].m_AdventureAwardID] = vectAdventureAward[i];
			m_mapAdventureAwardCnfg[vectAdventureAward[i].m_TalismanWorldID] = mapAward;
		}else{
			MAP_ADVENTUREAWARD & mapAward = iter->second;
			mapAward[vectAdventureAward[i].m_AdventureAwardID] = vectAdventureAward[i];
		}
	}

	return true;
}

//获得指定法宝世界可获得的奇遇
const std::hash_map<UINT16, SAdventureAwardCnfg> * ConfigServer::GetAdventureAwardCnfgVect(TTalismanWorldID TalismanWorldID)
{
	std::hash_map<TTalismanWorldID, MAP_ADVENTUREAWARD>::iterator iter = m_mapAdventureAwardCnfg.find(TalismanWorldID);
	if( iter == m_mapAdventureAwardCnfg.end()){
		return 0;
	}

	return &(iter->second);
	//std::vector<SAdventureAwardCnfg> vectAwardCnfg;

	//MAP_ADVENTUREAWARD & map_Award = iter->second;

	//MAP_ADVENTUREAWARD::iterator it = map_Award.begin();

	//for( ; it != map_Award.end(); ++it)
	//{
	//	vectAwardCnfg.push_back(it->second);
	//}

	//return &vectAwardCnfg;
}

//获得指定奇遇
const SAdventureAwardCnfg * ConfigServer::GetAdventureAwardCnfg(UINT16 AdventureAwardID)
{
	std::hash_map<TTalismanWorldID, MAP_ADVENTUREAWARD>::iterator iter = m_mapAdventureAwardCnfg.begin();
	for( ; iter != m_mapAdventureAwardCnfg.end(); ++iter)
	{
		MAP_ADVENTUREAWARD & mapAdventureAward = iter->second;

		MAP_ADVENTUREAWARD::iterator it = mapAdventureAward.find(AdventureAwardID);
		if( it != mapAdventureAward.end()){
			return &(it->second);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//GM命令
bool ConfigServer::OnLoadGMCmd(FileCSV & File,const char* szFileName)
{
	std::vector<SGMCmdCnfg> vectGMCmdCnfg;

	if( File.GetTable(vectGMCmdCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectGMCmdCnfg.size(); ++i)
	{
		m_mapGMCmdCnfg[vectGMCmdCnfg[i].m_strGMCmd] = vectGMCmdCnfg[i];
	}

	return true;
}

//获得GM命令
const SGMCmdCnfg *  ConfigServer::GetGMCmdCnfg(std::string strGMCmd)
{
	MAP_GMCMD::iterator iter = m_mapGMCmdCnfg.find(strGMCmd);
	if( iter == m_mapGMCmdCnfg.end()){
		return 0;
	}

	return &(iter->second);
}
//////////////////////////////////////////////////////////////////////////////////////////////

//夺宝奖励
bool ConfigServer::OnLoadDuoBaoAward(FileCSV & File,const char* szFileName)
{
	std::vector<SDuoBaoAwardCnfg> vectDuoBaoAwardCnfg;

	if( File.GetTable(vectDuoBaoAwardCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectDuoBaoAwardCnfg.size(); ++i)
	{
		UINT32 Key = ((UINT16)vectDuoBaoAwardCnfg[i].m_DuoBaoType << 16) | (UINT16)vectDuoBaoAwardCnfg[i].m_LevelRangeType;

		vectDuoBaoAwardCnfg[i].m_FirstTotalRandom = 0;
		vectDuoBaoAwardCnfg[i].m_SecondTotalRandom = 0;
		vectDuoBaoAwardCnfg[i].m_ThirdTotalRandom = 0;

		for( int k = 0; k + 1 < vectDuoBaoAwardCnfg[i].m_FirstAwardVect.size(); k = k + 2){
			vectDuoBaoAwardCnfg[i].m_FirstTotalRandom += vectDuoBaoAwardCnfg[i].m_FirstAwardVect[k + 1];
		}

		for( int n = 0; n + 1 < vectDuoBaoAwardCnfg[i].m_SecondAwardVect.size(); n = n + 2){
			vectDuoBaoAwardCnfg[i].m_SecondTotalRandom += vectDuoBaoAwardCnfg[i].m_SecondAwardVect[n + 1];
		}

		for( int c = 0; c + 1 < vectDuoBaoAwardCnfg[i].m_ThirdAwardVect.size(); c = c + 2){
			vectDuoBaoAwardCnfg[i].m_ThirdTotalRandom += vectDuoBaoAwardCnfg[i].m_ThirdAwardVect[c + 1];
		}

		m_mapDuoBaoAwardCnfg[Key] = vectDuoBaoAwardCnfg[i];
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//任务
bool ConfigServer::OnLoadTask(FileCSV & File,const char* szFileName)
{
	std::vector<STaskCnfg> vect;
	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	m_vectTaskList.resize(enTaskClass_Max);

	for(int i=0; i<vect.size();i++)
	{
		if(vect[i].m_TaskClass >= enTaskClass_Max)
		{
			TRACE("%s : %d Line 错误的任务分类 taskid=%d taskclass = %d!", __FUNCTION__, __LINE__,vect[i].m_TaskID , vect[i].m_TaskClass );
			continue;
		}
		m_vectTaskList[vect[i].m_TaskClass][vect[i].m_TaskID] = vect[i];
	}

	return true;
}


//获得任务列表
const  std::hash_map<TTaskID,STaskCnfg> *  ConfigServer::GetTaskList(UINT8 TaskClass) 
{
	if(TaskClass >= m_vectTaskList.size())
	{
		return 0;
	}
	return &m_vectTaskList[TaskClass];
}

const STaskCnfg * ConfigServer::GetTaskCnfg(TTaskID TaskID)
{
	for(int i=0; i<m_vectTaskList.size();i++)
	{
		MAP_TASK::iterator it = m_vectTaskList[i].find(TaskID);
		if(it !=  m_vectTaskList[i].end())
		{
			return &(*it).second;
		}
	}
	return 0;
}

//帮派物品任务概率配置表
bool ConfigServer::OnLoadSynGoodsTask(FileCSV & File,const char* szFileName)
{
	std::vector<SSynGoodsTask> vectSynGoodsTask;

	if( File.GetTable(vectSynGoodsTask) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	m_SynGoodsTaskTotalRandom = 0;

	for( int i = 0; i < vectSynGoodsTask.size(); ++i)
	{
		m_mapSynGoodsTask[vectSynGoodsTask[i].m_TaskID] = vectSynGoodsTask[i];

		m_SynGoodsTaskTotalRandom += vectSynGoodsTask[i].m_RandomNum;
	}

	return true;
}

//随机获取一个帮派物品任务
const SSynGoodsTask * ConfigServer::RandomGetSynGoodsTask()
{
	if( m_SynGoodsTaskTotalRandom < 0){
		TRACE("<error> %s : %d 行 配置信息出错!!", __FUNCTION__, __LINE__);
		return 0;
	}

	INT32 RandomNum = RandomService::GetRandom() % m_SynGoodsTaskTotalRandom;

	MAP_SYNGOODSTASK::iterator iter = m_mapSynGoodsTask.begin();

	INT32 nCount = 0;
	for(; iter != m_mapSynGoodsTask.end(); ++iter)
	{
		SSynGoodsTask & SynGoodsTask = iter->second;

		nCount += SynGoodsTask.m_RandomNum;

		if( nCount > RandomNum){
			return &SynGoodsTask;
		}
	}

	return 0;
}

//得到帮派物品任务
const SSynGoodsTask * ConfigServer::GetSynGoodsTask(TTaskID TaskID)
{
	MAP_SYNGOODSTASK::iterator iter = m_mapSynGoodsTask.find(TaskID);
	if( iter == m_mapSynGoodsTask.end()){
		return 0;
	}

	return  &(iter->second);
}

//帮派任务数量
UINT8	ConfigServer::GetSynGoodsTaskNum()
{
	return (UINT8)m_mapSynGoodsTask.size();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//清理土地类游戏配置
bool ConfigServer::OnLoadQingLiGame(FileCSV & File,const char* szFileName)
{
	std::vector<SQingLiGameConfig> vectQingLi;

	if( File.GetTable(vectQingLi) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectQingLi.size(); ++i)
	{
		m_mapQingLiGame[vectQingLi[i].m_TalismanWorldID] = vectQingLi[i];
	}
	return true;
}

//清理土地
bool ConfigServer::OnLoadQingLiGameConfig(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectQingLiAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;	
}

//清理类法宝世界游戏参数
bool ConfigServer::OnLoadQingLiGameConfig(FileConfig & File,const char* szFileName)
{
	if(File.Read(m_QingLiGameParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}

//清理土地法宝游戏
const SQingLiGameParam & ConfigServer::GetQingLiGameParam()
{
	return m_QingLiGameParam;
}

const  SQingLiGameConfig *	 ConfigServer::GetQingLiGameCnfg(TTalismanWorldID TalismanWorldID)
{
	MAP_QINGLIGAME::iterator iter = m_mapQingLiGame.begin();
	if( iter == m_mapQingLiGame.end()){
		return 0;
	}

	return &(iter->second);
}

//清理奖励
const std::vector<SQingLiGameAwardCnfg> & ConfigServer::GetQingLiAwardCnfg()
{
	return m_vectQingLiAward;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//题库
bool ConfigServer::OnLoadTiKuConfig(FileCSV & File,const char* szFileName)
{
	std::vector<STiMuData> vectTiKuConfig;
	if( File.GetTable(vectTiKuConfig) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectTiKuConfig.size(); ++i)
	{
		STiMuData TiMuData = vectTiKuConfig[i];

		MAP_TIMUTYPE::iterator iter = m_mapTiMuInfo.find(TiMuData.m_TiMuType);
		if( iter == m_mapTiMuInfo.end()){
			MAP_TIMUINFO mapTiMu; 
			mapTiMu[TiMuData.m_TiMuID] = TiMuData;
			m_mapTiMuInfo[TiMuData.m_TiMuType] = mapTiMu;
		}else{
			MAP_TIMUINFO & mapTiMu = iter->second;
			mapTiMu[TiMuData.m_TiMuID] = TiMuData;
		}
	}
	return true;	
}

//答题
bool ConfigServer::OnLoadDaTiGameConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SDaTiGameCnfg> vectDaTiConfig;
	if( File.GetTable(vectDaTiConfig) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectDaTiConfig.size(); ++i)
	{
		m_mapDaTaGameInfo[vectDaTiConfig[i].m_TalismanWorldID] = vectDaTiConfig[i];
	}
	return true;
}

//答题
bool ConfigServer::OnLoadDaTiParam(FileConfig & File,const char* pszFileName)
{
	if(File.Read(m_DaTiGameParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,pszFileName);
		return false;
	}
	return true;
}

//答题奖励
bool ConfigServer::OnLoadDaTiGameLevelAwardConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SDaTiGameLevelAward> vectDaTiLevelAward;
	if( File.GetTable(vectDaTiLevelAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectDaTiLevelAward.size(); ++i)
	{
		m_mapDaTiGameLevelAward[vectDaTiLevelAward[i].m_TalismanWorldID] = vectDaTiLevelAward[i];
	}

	return true;
}

//答题完成级别奖励
bool ConfigServer::OnLoadDaTiGameAwardConfig(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectDaTiGameAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}	
	return true;
}

//得到某类型的题目
const std::hash_map<UINT16/*TiMuID*/, STiMuData> * ConfigServer::GetTiMuListCnfg(UINT8 TiMuType)
{
	MAP_TIMUTYPE::iterator iter = m_mapTiMuInfo.find(TiMuType);
	if( iter == m_mapTiMuInfo.end()){
		return 0;
	}

	return &(iter->second);
}

//根据题目ID得到题目
const STiMuData * ConfigServer::GetTiMuCnfg(UINT16  TiMuID)
{
	for(MAP_TIMUTYPE::iterator iter = m_mapTiMuInfo.begin(); iter != m_mapTiMuInfo.end(); ++iter)
	{
		MAP_TIMUINFO mapTiMuInfo = iter->second;
		MAP_TIMUINFO::iterator it = mapTiMuInfo.find(TiMuID);
		if( it == mapTiMuInfo.end()){
			break;
		}

		return &(it->second);
	}

	return 0;
}

//得到答题配置
const SDaTiGameCnfg * ConfigServer::GetDaTiCnfg(TTalismanWorldID TalismanWorldID)
{
	MAP_DATAGAMECNFG::iterator iter = m_mapDaTaGameInfo.find(TalismanWorldID);
	if( iter == m_mapDaTaGameInfo.end()){
		return 0;
	}

	return &(iter->second);
}

//得到答题参数配置
const SDaTiGameParam & ConfigServer::GetDaTiParam()
{
	return m_DaTiGameParam;
}

//得到答题完成级别奖励
const std::vector<SDaTiGameAwardCnfg> & ConfigServer::GetDaTiGameAward()
{
	return m_vectDaTiGameAward;
}

//得到答题奖励
const SDaTiGameLevelAward * ConfigServer::GetDaTiGameLevelAward(TTalismanWorldID TalismanWorldID)
{
	MAP_DATIGAMELEVELAWARD::iterator iter = m_mapDaTiGameLevelAward.find(TalismanWorldID);
	if( iter == m_mapDaTiGameLevelAward.end()){
		return 0;
	}

	return &(iter->second);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//战斗地图上的怪物分布
bool ConfigServer::OnLoadCombatMapMonster(FileCSV & File,const char* szFileName)
{
	std::vector<SCombatMapMonster> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		m_mapCombatMonster[vect[i].m_CombatIndex] = vect[i];
	}

	return true;
}

//战斗地图怪物分布
const SCombatMapMonster * ConfigServer::GetCombatMapMonster(TMonsterID MonsterID)
{
	MAP_COMBAT_MONSTER::iterator it = m_mapCombatMonster.find(MonsterID);

	if(it == m_mapCombatMonster.end())
	{
		return 0;
	}
	return &(*it).second;
}

	//得到战斗地图主将ID
const TMonsterID ConfigServer::GetMainMonsterID(UINT32 CombatIndex)
{
	const SCombatMapMonster * pCombatMonster = this->GetCombatMapMonster(CombatIndex);

	if ( 0 == pCombatMonster){
		
		TRACE("<error> %s : %d Line 获取战斗地图怪物出错！！战斗索引=%d", __FUNCTION__, __LINE__, CombatIndex);
		return INVALID_MONSTER_ID;
	}

	//主将索引
	switch ( pCombatMonster->m_MainMonsterIndex)
	{
	case 1:
		{
			return pCombatMonster->m_MonsterID1;
		}
		break;
	case 2:
		{
			return pCombatMonster->m_MonsterID2;
		}
		break;
	case 3:
		{
			return pCombatMonster->m_MonsterID3;
		}
		break;
	case 4:
		{
			return pCombatMonster->m_MonsterID4;
		}
		break;
	case 5:
		{
			return pCombatMonster->m_MonsterID5;
		}
		break;
	case 6:
		{
			return pCombatMonster->m_MonsterID6;
		}
		break;
	case 7:
		{
			return pCombatMonster->m_MonsterID7;
		}
		break;
	case 8:
		{
			return pCombatMonster->m_MonsterID8;
		}
		break;
	case 9:
		{
			return pCombatMonster->m_MonsterID9;
		}
		break;
	}

	return INVALID_MONSTER_ID;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//怪物法术配置
bool ConfigServer::OnLoadMonsterMagicCnfg(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectMonsterMagic) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}


//怪物法术列表
std::vector<TMagicID> ConfigServer::GetMonsterMagicCnfg(UINT8 Level,INT32 num)
{
		std::vector<TMagicID> vect;

		if(num==0)
		{
			return vect;
		}

	std::vector<std::vector<SMonsterMagicCnfg> > vectMagicCnfg;

	//满足等级要求的法术
	for(int i=0 ; i< m_vectMonsterMagic.size(); i++)
	{
		SMonsterMagicCnfg & MagicCnfg = m_vectMonsterMagic[i];
		if(Level>= MagicCnfg.m_MinLevel && Level <= MagicCnfg.m_MaxLevel && num >= MagicCnfg.m_MinMagicNumLimit)
		{
			//按类型查找
			bool bFound = false;
			for(int j=0; j<vectMagicCnfg.size();j++)
			{
				std::vector<SMonsterMagicCnfg> & vect = vectMagicCnfg[j];
				SMonsterMagicCnfg & MagicCnfgOld = vect[0];
				if(MagicCnfgOld.m_MagicType == MagicCnfg.m_MagicType)
				{
					vect.push_back(MagicCnfg);
					bFound = true;
					break;
				}
			}

			if(bFound==false)
			{
				std::vector<SMonsterMagicCnfg>  vect;
				vect.push_back(MagicCnfg);
               vectMagicCnfg.push_back(vect);
			}
		}
	}



	for(; vect.size()<num && vectMagicCnfg.size()>0;)
	{
		INT32 Index = RandomService::GetRandom() % vectMagicCnfg.size();

		std::vector<SMonsterMagicCnfg> & vect2 = vectMagicCnfg[Index];

		for(;vect2.size()>0;)
		{
			INT32 Index2 = RandomService::GetRandom() % vect2.size();

		  SMonsterMagicCnfg & MagicCnfg = vect2[Index2];
		  TMagicID MagicID = MagicCnfg.m_MagicID;

		  //已选中的法术是否与该法术互斥
		  if(std::find_first_of(vect.begin(),vect.end(),MagicCnfg.m_vectMutexMagic.begin(),MagicCnfg.m_vectMutexMagic.end())==vect.end())
		  {
			vect.push_back(MagicID);
			break;
		  }

		  vect2.erase(vect2.begin()+Index2);
		}

		vectMagicCnfg.erase(vectMagicCnfg.begin()+Index);
	}
	

	return vect;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//寻宝
bool ConfigServer::OnLoadTalismanWorldXunBao(FileCSV & File,const char* szFileName)
{
	std::vector<SXunBaoGameConfig> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SXunBaoGameConfig & GameConfig = vect[i];

		for(int j=0; j< GameConfig.m_vectGoods.size()/3; j++)
		{
			GameConfig.m_TotalProbability += GameConfig.m_vectGoods[j*3+2];
		}

		m_mapTalismanWorldXunBao[GameConfig.m_TalismanWorldID] = GameConfig;
		
	}

	return true;

}

bool ConfigServer::OnLoadTalismanWorldXunBaoAward(FileCSV & File,const char* szFileName)
{

	if( File.GetTable(m_vectXunBaoGameAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//寻宝
bool ConfigServer::OnLoadXunBaoGameConfig(FileConfig & File,const char* szFileName)
{
	if(File.Read(m_XunBaoGameParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}

//寻宝法宝游戏
const SXunBaoGameParam & ConfigServer::GetXunBaoGameParam()
{
	return m_XunBaoGameParam;
}

const SXunBaoGameConfig * ConfigServer::GetXunBaoGameCnfg(TTalismanWorldID TalismanWorldID)
{
	MAP_TALISMAN_WORLD_XUNBAO::iterator it = m_mapTalismanWorldXunBao.find(TalismanWorldID);

	if(it == m_mapTalismanWorldXunBao.end())
	{
		return 0;
	}

	return &(*it).second;
}

//寻宝奖励
const std::vector<SXunBaoGameAwardCnfg> & ConfigServer::GetXunBaoGameAwardCnfg()
{
	return m_vectXunBaoGameAward;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//采集
bool ConfigServer::OnLoadTalismanWorldGather(FileCSV & File,const char* szFileName)
{
	std::vector<SGatherGameConfig> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SGatherGameConfig & GameConfig = vect[i];

		for(int j=0; j< GameConfig.m_vectGoods.size()/2; j++)
		{
			GameConfig.m_TotalProbability += GameConfig.m_vectGoods[j*2+1];
		}
		
		m_mapTalismanWorldGather[GameConfig.m_TalismanWorldID] = GameConfig;
		
	}

	return true;

}

bool ConfigServer::OnLoadTalismanWorldGatherAward(FileCSV & File,const char* szFileName)
{

	if( File.GetTable(m_vectGatherGameAward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//采集
bool ConfigServer::OnLoadGatherGameConfig(FileConfig & File,const char* szFileName)
{
	if(File.Read(m_GatherGameParam)==false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}
	return true;
}

//采集法宝游戏
const SGatherGameParam & ConfigServer::GetGatherGameParam()
{
	return m_GatherGameParam;
}

const SGatherGameConfig * ConfigServer::GetGatherGameCnfg(TTalismanWorldID TalismanWorldID)
{
	MAP_TALISMAN_WORLD_GATHER::iterator it = m_mapTalismanWorldGather.find(TalismanWorldID);

	if(it == m_mapTalismanWorldGather.end())
	{
		return 0;
	}

	return &(*it).second;
}

//寻宝奖励
const std::vector<SGatherGameAwardCnfg> & ConfigServer::GetGatherGameAwardCnfg()
{
	return m_vectGatherGameAward;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//加载成就
bool ConfigServer::OnLoadChengJiuConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SChengJiuCnfg> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SChengJiuCnfg & GameConfig = vect[i];
		
		m_mapChengJiu[GameConfig.m_ChengJiuID] = GameConfig;
		
	}

	return true;
}


//获得指定成就
const SChengJiuCnfg * ConfigServer::GetChengJiuCnfg(TChengJiuID ChengJiuID)
{
	MAP_CHENGJIU::iterator it = m_mapChengJiu.find(ChengJiuID);

	if(it == m_mapChengJiu.end())
	{
		return 0;
	}

	return &(*it).second;
}

	//获得所有成就配置信息
const std::hash_map<TChengJiuID,SChengJiuCnfg> & ConfigServer::GetAllChengJiuCnfg()
{
	return m_mapChengJiu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//加载签到奖励
bool ConfigServer::OnLoadSignInAwardConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SSignInAwardCnfg> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SSignInAwardCnfg & GameConfig = vect[i];
		
		m_mapSignInAward[GameConfig.m_AwardID] = GameConfig;
		
	}

	return true;
}

//获到奖励
const std::map<UINT8,SSignInAwardCnfg> & ConfigServer::GetAllSignInAwardCnfg()
{
	return m_mapSignInAward;
}

const SSignInAwardCnfg * ConfigServer::GetSignInAwardCnfg(UINT8 AwardID)
{
	MAP_SIGNIN_AWARD::iterator it = m_mapSignInAward.find(AwardID);

	if(it == m_mapSignInAward.end())
	{
		return 0;
	}

	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//加载活动
bool ConfigServer::OnLoadActivityConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SActivityCnfg> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SActivityCnfg & GameConfig = vect[i];
		
		m_mapActivity[GameConfig.m_ActivityID] = GameConfig;
		
	}

	return true;
}

//活动配置
const std::map< UINT16, SActivityCnfg> &  ConfigServer::GetAllActivityCnfg()
{
	return m_mapActivity;
}

const SActivityCnfg *  ConfigServer::GetActivityCnfg(UINT16 ActivityID) 
{
	MAP_ACTIVITY::iterator it = m_mapActivity.find(ActivityID);

	if(it == m_mapActivity.end())
	{
		return 0;
	}

	return &(*it).second;
}

void ConfigServer::Push_ActivityCnfg(const SActivityCnfg & Activity)
{
	m_mapActivity[Activity.m_ActivityID] = Activity;
}

void ConfigServer::DelActivityCnfg(UINT16 ActivityID)
{
	MAP_ACTIVITY::iterator it = m_mapActivity.find(ActivityID);

	if ( it == m_mapActivity.end() )
		return;

	m_mapActivity.erase(it);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//称号
bool	ConfigServer::OnLoadTitleConfig(FileCSV & File,const char* szFileName)
{
	std::vector<STitleCnfg>	vectTitleCnfg;

	if( File.GetTable(vectTitleCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectTitleCnfg.size(); ++i)
	{
		m_mapTitleCnfg[vectTitleCnfg[i].m_TitleID] = vectTitleCnfg[i];
		//m_mapTitle[vectTitleCnfg[i].m_TitleID] = vectTitleCnfg[i].m_TitleName;
	}

	return true;
}

//称号
/*const STitleCnfg * ConfigServer::GetTitleCnfg(TTitleID TitleID)
{
	MAP_TITLE::iterator iter = m_mapTitle.find(TitleID);

	if( iter == m_mapTitle.end()){
		return 0;
	}

	return &(iter->second);
}*/
const STitleCnfg * ConfigServer::GetTitleCnfg(TTitleID TitleID)
{
	std::hash_map<TTitleID, STitleCnfg>::iterator iter = m_mapTitleCnfg.find(TitleID);

	if(iter == m_mapTitleCnfg.end())
	{
		return 0;
	}
	return &(*iter).second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//在线奖励
bool ConfigServer::OnLoadOnlineAwardConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SOnlineAwardCnfg>	vectTitleCnfg;

	if( File.GetTable(vectTitleCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectTitleCnfg.size(); ++i)
	{
		SOnlineAwardCnfg & AwardCnfg = vectTitleCnfg[i];

		m_mapOnlineAward[AwardCnfg.m_AwardID] = AwardCnfg; 
		
	}

	return true;
}

//在线奖励
const SOnlineAwardCnfg * ConfigServer::GetOnlineAwardCnfg(UINT16 AwardID) 
{
	MAP_ONLINE_AWARD::iterator it = m_mapOnlineAward.find(AwardID);

	if(it == m_mapOnlineAward.end())
	{
		return 0;
	}

	return &(*it).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//套装
bool ConfigServer::OnLoadSuitConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SSuitCnfg>	vectTitleCnfg;

	if( File.GetTable(vectTitleCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectTitleCnfg.size(); ++i)
	{
		m_mapSuit[vectTitleCnfg[i].m_SuitID] = vectTitleCnfg[i];
	}

	return true;
}


//套装配置信息
const SSuitCnfg*  ConfigServer::GetSuitCnfg(UINT16 SuitID)
{
	MAP_SUIT::iterator it = m_mapSuit.find(SuitID);

	if(it == m_mapSuit.end())
	{
		return 0;
	}

	return &(*it).second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//帮派福利
bool ConfigServer::OnLoadWelfareConfig(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectSynWelfareCnfg) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//获取帮派福利
const SSynWelfareCnfg * ConfigServer::GetSynWelfareCnfg(UINT8/*enWelfare*/ WelfareType, UINT8 SynLevel)
{
	int index = -1;

	int nValue = 0;

	//获取该类型最大值
	for( int i = 0; i < m_vectSynWelfareCnfg.size(); ++i)
	{
		if( m_vectSynWelfareCnfg[i].m_WelfareType == WelfareType && m_vectSynWelfareCnfg[i].m_NeedSynLevel <= SynLevel){
			if( nValue < m_vectSynWelfareCnfg[i].m_AddValue){
				index = i;
			}
		}
	}

	if( index < 0){
		return 0;
	}

	return &m_vectSynWelfareCnfg[index];
}

//获取所有帮派福利
const std::vector<SSynWelfareCnfg> & ConfigServer::GetAllSynWelfareCnfg()
{
	return m_vectSynWelfareCnfg;
}

//////////////////////////////////////////////////////////////////////////////////////
//不可改名的角色名或者不能改成这名字
bool ConfigServer::OnLoadNotChangeName(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectNoChangeName) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//得到不可改名的名字
const std::vector<SNoChangeName> & ConfigServer::GetAllNotChangeNameCnfg()
{
	return m_vectNoChangeName;
}


//获得客户端文件版本
const std::string & ConfigServer::GetClientVersion()
{
	return m_strClientVersion;
}

//获得客户端所有配置文件
const std::vector<std::string> & ConfigServer::GetClientCnfgFile() 
{
	return m_vectClientCnfgFile;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

				//获得客户端资源文件版本
const std::string & ConfigServer::GetClientResVersion()
{
	return m_strClientResVersion;
}

	//获得客户端所有资源文件
const std::vector<std::string> & ConfigServer::GetClientResFile()
{
	return m_vectClientResFile;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

//挑战奖励
bool ConfigServer::OnLoadChallengeForwardConfig(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vectChallengeForward) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//得到奖励配置
const SChallengeForward * ConfigServer::GetChallengeForward(UINT8 LvGroup, UINT8 nRank)
{
	for( int i = 0; i < m_vectChallengeForward.size(); ++i)
	{
		if( LvGroup == m_vectChallengeForward[i].m_LvGroup && nRank == m_vectChallengeForward[i].m_Rank){
			return &m_vectChallengeForward[i];
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//强化传承
bool ConfigServer::OnLoadStrongInherit(FileCSV & File,const char* szFileName)
{
	if( File.GetTable(m_vecStrongInherit) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//随机获得强化传承等级
UINT8 ConfigServer::RandGetStrongInheritLeve(IEquipment * pEuipment, enGoodsCategory DesGoodsClass)
{
	INT32 nTotalRandom = 0;

	UINT8 nAddLevel = 0;
	
	IGodSword * pGodSword = (IGodSword *)pEuipment;
	if ( DesGoodsClass == enGoodsCategory_Talisman || DesGoodsClass == enGoodsCategory_GodSword){
		//法宝、仙剑等级从1开始，结果要加1
		nAddLevel = 1;
	}

	int Level = 0;

	if ( pEuipment->GetGoodsClass() == enGoodsCategory_Equip){
		
		pEuipment->GetPropNum(enGoodsProp_StarLevel, Level);
	}

	else if ( pEuipment->GetGoodsClass() == enGoodsCategory_Talisman)
	{
		
		pEuipment->GetPropNum(enGoodsProp_MagicLevel, Level);
		//法宝等级从1开始
		--Level;
	}

	else if ( pGodSword->GetGoodsClass() == enGoodsCategory_GodSword)
	{
		
		pGodSword->GetPropNum(enGoodsProp_SwordLevel, Level);
		//仙剑等级从1开始
		--Level;
	}

	else
	{

		return nAddLevel;
	}
	

	for( int i = 0; i < m_vecStrongInherit.size(); ++i)
	{
		SStrongInherit & Inherit = m_vecStrongInherit[i];

		if ( Inherit.m_Level > Level){
			
			break;
		}

		nTotalRandom += Inherit.m_Random;
	}

	INT32 nRandom = RandomService::GetRandom() % nTotalRandom;

	INT32 nTmp = 0;

	for( int i = 0; i < m_vecStrongInherit.size(); ++i)
	{
		SStrongInherit & Inherit = m_vecStrongInherit[i];

		if ( Inherit.m_Level > Level){
			
			break;
		}

		nTmp += Inherit.m_Random;

		if( nTmp >= nRandom){
			
			return Inherit.m_Level + nAddLevel;
		}
	}

	return nAddLevel;
}

///////////////////////////////////////////////////////////////////////////////////////////////

//自动刷新招募角色
bool ConfigServer::OnLoadAutoFlushEmployee(FileCSV & File,const char* szFileName)
{
	std::vector<SAutoFlushEmployee> vecAutoEmployee;

	if ( File.GetTable(vecAutoEmployee) == false){

		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for ( int i = 0; i < vecAutoEmployee.size(); ++i)
	{
		SAutoFlushEmployee & AutoEmploy = vecAutoEmployee[i];

		AutoEmploy.m_TotalRandom = 0;

		for ( int k = 0; k + 1 < AutoEmploy.m_MustAptitude.size(); k += 2)
		{
			AutoEmploy.m_TotalRandom += AutoEmploy.m_MustAptitude[k + 1];
		}

		m_mapAutoFlushEmployee[AutoEmploy.m_FlushNum] = AutoEmploy;
	}

	return true;
}

//得到自动刷新招募角色指定次数的信息
const SAutoFlushEmployee * ConfigServer::GetAutoFlushEmploy(UINT16 FlushNum)
{
	MAP_AUTOEMPLOYEE::iterator iter = m_mapAutoFlushEmployee.find(FlushNum);

	if ( iter == m_mapAutoFlushEmployee.end()){
		
		return 0;
	}

	return &(iter->second);
}

//随机获得一个必然出现的角色资质
INT32	ConfigServer::RandGetMushAptitude(UINT16 FlushNum)
{
	const SAutoFlushEmployee * pAutoEmploy = this->GetAutoFlushEmploy(FlushNum);
	
	if ( 0 == pAutoEmploy){
		
		return 0;
	}

	INT32 Random = RandomService::GetRandom() % pAutoEmploy->m_TotalRandom;

	for ( int i = 0; i + 1 < pAutoEmploy->m_MustAptitude.size(); i += 2)
	{
		if ( pAutoEmploy->m_MustAptitude[i + 1] > Random){
			
			return pAutoEmploy->m_MustAptitude[i];
		}
	}

	return 0;
}

//得到该挑战排名最多有多少挑战次数
INT32	ConfigServer::GetMaxChallengeNum(UINT32 nRank)
{
	const SGameServerConfigParam & ServerConfigParam = this->GetGameConfigParam().m_ServerConfigParam;

	for ( int i = 0; i + 1 < ServerConfigParam.m_Max_ChallengeNum.size(); i += 2)
	{
		if ( nRank <= ServerConfigParam.m_Max_ChallengeNum[i] || ServerConfigParam.m_Max_ChallengeNum[i] == -1){
			
			return ServerConfigParam.m_Max_ChallengeNum[i + 1];
		}
	}

	return this->GetGameConfigParam().m_MaxJoinChallenge;
}

//////////////////////////////////////////////////////////////////////////////////////

// fly add

//语言类型配置
bool ConfigServer::OnLoadLanguageType(FileCSV & File, const char* szFileName)
{
	std::vector<SLanguageTypeCnfg> vectLanguage;
	if(File.GetTable(vectLanguage) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectLanguage.size(); ++i)
	{
		m_mapLanguageTypeCnfg[vectLanguage[i].m_LanguageID] = vectLanguage[i];
	}

	return true;
}

//得到语言类型配置
const SLanguageTypeCnfg * ConfigServer::GetLanguageTypeCnfg(TLanguageID LanguageID)
{
	std::hash_map<TLanguageID, SLanguageTypeCnfg>::iterator iter = m_mapLanguageTypeCnfg.find(LanguageID);

	if(iter == m_mapLanguageTypeCnfg.end())
	{
		return 0;
	}
	return &(*iter).second;
}
////////////////////////////////////////////////////////////////////////////////////////////////

//Vip功能
bool ConfigServer::OnLoadVipConfig(FileCSV & File,const char* szFileName)
{
	std::vector<SVipConfig>  vectVipConfig;

	if ( File.GetTable(vectVipConfig) == false){

		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for ( int i = 0; i < vectVipConfig.size(); ++i)
	{
		SVipConfig & VipConfig = vectVipConfig[i];

		m_mapVipConfig[VipConfig.m_VipLevel] = VipConfig;
	}

	return true;
}

//得到VIP配置信息
const SVipConfig * ConfigServer::GetVipConfig(UINT8 vipLevel)
{
	MAP_VIPCONFIG::iterator iter = m_mapVipConfig.find(vipLevel);

	if ( iter == m_mapVipConfig.end() )
	{
		return 0;
	}

	return &(iter->second);
}


//得到充值额度可以升到的VIP级别
UINT8	ConfigServer::GetCanVipLevel(INT32 Recharge)
{
	if ( 0 > Recharge){
		
		return 0;
	}

	UINT8 vipLv = 0;

	for ( int i = 0; i + 1 < m_GameConfigParam.m_vecVipUpRecharge.size(); i += 2)
	{
		if ( Recharge >= m_GameConfigParam.m_vecVipUpRecharge[i + 1]){
			
			vipLv = m_GameConfigParam.m_vecVipUpRecharge[i];
			continue;
		}

		break;
	}

	return  vipLv;
}

////////////////////////////////////////////////////////////////////////////////////////////////


//得到服务器状态
UINT8	ConfigServer::GetServerStatus(INT32 UserNum)
{
	const SGameServerConfigParam & ServerParam = this->GetGameConfigParam().m_ServerConfigParam;

	for (int i = 0; i + 1 < ServerParam.m_ServerState.size(); ++i)
	{
		if (UserNum <= ServerParam.m_ServerState[i] || ServerParam.m_ServerState[i] == -1){
			
			return ServerParam.m_ServerState[i + 1];
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

//聚灵气配置
bool ConfigServer::OnLoadPolyNimbus(FileCSV & File, const char* szFileName)
{
	std::vector<SPolyNimbusCnfg> vectPolyNimbus;
	if(File.GetTable(vectPolyNimbus) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectPolyNimbus.size(); ++i)
	{
		m_mapPolyNimbusCnfg[vectPolyNimbus[i].m_OperationID] = vectPolyNimbus[i];
	}

	return true;
}

//得到聚灵气配置
const SPolyNimbusCnfg * ConfigServer::GetPolyNimbusCnfg(UINT16 OperationID)
{
	std::hash_map<UINT16, SPolyNimbusCnfg>::iterator iter = m_mapPolyNimbusCnfg.find(OperationID);

	if(iter == m_mapPolyNimbusCnfg.end())
	{
		return 0;
	}
	return &(*iter).second;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//玄天奖励
bool	ConfigServer::OnLoadXTForward(FileCSV & File,const char* szFileName)
{
	std::vector<SXuanTianForward> vecXTForward;

	if( File.GetTable(vecXTForward) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vecXTForward.size(); ++i)
	{
		m_mapXTForward[vecXTForward[i].m_Rank] = vecXTForward[i];
	}

	return true;
}

//得到玄天奖励
const SXuanTianForward * ConfigServer::GetXTForward(INT32	Rank)
{
	std::map<INT32,SXuanTianForward>::iterator iter = m_mapXTForward.find(Rank);

	if ( iter == m_mapXTForward.end() ){
		return 0;
	}

	return &(iter->second);
}

//得到帮战参数配置
void ConfigServer::GetSynCombatParam(INT32 SynCombatLevel, SSynCombatParam & SynCombatParam)
{
	const SGameServerConfigParam & ServerParam = this->GetGameConfigParam().m_ServerConfigParam;

	for ( int i = 0; i + 3 < ServerParam.m_SynCombatParam.size(); i += 4 )
	{
		int lv = ServerParam.m_SynCombatParam[i];

		if ( SynCombatLevel < ServerParam.m_SynCombatParam[i] || -1 == ServerParam.m_SynCombatParam[i] ){
			SynCombatParam.m_CreditUp = ServerParam.m_SynCombatParam[i + 1];
			SynCombatParam.m_WinParam = ServerParam.m_SynCombatParam[i + 2];
			SynCombatParam.m_FailParam = ServerParam.m_SynCombatParam[i + 3];
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////



//剑诀升级配置
bool ConfigServer::OnLoadGodSwordMagicLv(FileCSV & File, const char* szFileName)
{
	std::vector<SDamageLevelCnfg>  vectMagicLv;

	if( File.GetTable(vectMagicLv) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectMagicLv.size(); ++i)
	{
		std::hash_map<UINT16, VECTDAMAGELEVEL>::iterator iter = m_mapGodSwordMagicLv.find(vectMagicLv[i].m_MagicLevel);
		if( iter == m_mapGodSwordMagicLv.end()){
			VECTDAMAGELEVEL vectSwordMagic;
			vectSwordMagic.push_back(vectMagicLv[i]);
			m_mapGodSwordMagicLv[vectMagicLv[i].m_MagicLevel]  = vectSwordMagic;
		}else{
			VECTDAMAGELEVEL & vectSwordMagic = iter->second;
			vectSwordMagic.push_back(vectMagicLv[i]);
		}
	}
	return true;
}

//得到剑诀升级配置
const SDamageLevelCnfg * ConfigServer::GetMagicLvCnfg(UINT16 SwordMagicLevel, UINT8 SwordMagicType)
{
	std::hash_map<UINT16, VECTDAMAGELEVEL>::iterator iter = m_mapGodSwordMagicLv.find(SwordMagicLevel);
	if( iter == m_mapGodSwordMagicLv.end()){
		return 0;
	}

	VECTDAMAGELEVEL & vectSwordMagic = iter->second;

	for( int i = 0; i < vectSwordMagic.size(); ++i)
	{
		SDamageLevelCnfg & SwordMagicCnfg = vectSwordMagic[i];

		if(SwordMagicCnfg.m_MagicType == SwordMagicType){
			return &SwordMagicCnfg;
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////

//声望技能配置
bool ConfigServer::OnLoadCreditMagic(FileCSV & File, const char* szFileName)
{
	std::vector<SCreditMagicCnfg>  vectCreditMagic;

	if( File.GetTable(vectCreditMagic) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectCreditMagic.size(); ++i)
	{
		std::hash_map<TGoodsID, VECTCREDITMAGIC>::iterator iter = m_mapCreditMagic.find(vectCreditMagic[i].m_CreditMagicLevel);
		if( iter == m_mapCreditMagic.end()){
			VECTCREDITMAGIC vectCredit;
			vectCredit.push_back(vectCreditMagic[i]);
			m_mapCreditMagic[vectCreditMagic[i].m_CreditMagicLevel]  = vectCredit;
		}else{
			VECTCREDITMAGIC & vectCredit = iter->second;
			vectCredit.push_back(vectCreditMagic[i]);
		}
	}
	return true;
}

//得到声望技能配置
const SCreditMagicCnfg * ConfigServer::GetCreditMagicCnfg(UINT16 MagicLevel, UINT8 MagicType)
{
	std::hash_map<UINT16, VECTCREDITMAGIC>::iterator iter = m_mapCreditMagic.find(MagicLevel);
	if( iter == m_mapCreditMagic.end()){
		return 0;
	}

	VECTCREDITMAGIC & vectCredit = iter->second;

	for( int i = 0; i < vectCredit.size(); ++i)
	{
		SCreditMagicCnfg & MagicCnfg = vectCredit[i];

		if(MagicCnfg.m_CreditMagicType == MagicType){
			return &MagicCnfg;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

//法宝世界配置
bool	ConfigServer::OnLoadTalismanWorldInfo(FileCSV & File,const char* szFileName)
{
	std::vector<STalismanWorldInfo> vecTalismanWorld;

	if ( File.GetTable(vecTalismanWorld) == false )
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for ( int i = 0; i < vecTalismanWorld.size(); ++i )
	{
		STalismanWorldInfo & TWInfo = vecTalismanWorld[i];

		VECT_TALISMANWORLD & vecTW = m_mapTalismanWorldInfo[TWInfo.m_Level];

		vecTW.push_back(TWInfo);
	}

	return true;
}

//法宝世界其它配置
bool	ConfigServer::OnLoadTalismanWorldParam(FileCSV & File,const char* szFileName)
{
	if ( File.GetTable(m_vectTWParam) == false )
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//得到法宝世界配置
const STalismanWorldInfo * ConfigServer::GetTalismanWorldInfo(UINT8 level, UINT8 floor)
{
	std::map<UINT8,	VECT_TALISMANWORLD>::iterator iter = m_mapTalismanWorldInfo.find(level);

	if ( iter == m_mapTalismanWorldInfo.end() )
		return 0;

	VECT_TALISMANWORLD & vecTWInfo = iter->second;

	if ( vecTWInfo.size() < floor + 1 ){
		return 0;
	}

	return &vecTWInfo[floor];
}

//得到所有法宝世界配置
const std::map<UINT8,	std::vector<STalismanWorldInfo>> * ConfigServer::GetAllTalismanWorldInfo()
{
	return &m_mapTalismanWorldInfo;
}

//根据层数，获得法宝世界其它配置
const STalismanWorldParam *  ConfigServer::GetTalismanWorldParam(UINT8 Floor)
{
	for ( int i = 0; i < m_vectTWParam.size(); ++i )
	{
		STalismanWorldParam & Param = m_vectTWParam[i];

		if ( Floor < Param.m_Floor )
		{
			return &m_vectTWParam[i];
		} 
	}

	return 0;
}

//根据玩家等级，获得进入法宝世界的级别
INT8  ConfigServer::GetEnterTWLevel(UINT8 Level)
{
	const SGameServerConfigParam & ServerCnfg = this->GetGameConfigParam().m_ServerConfigParam;

	for ( int i = 0; i + 1 < ServerCnfg.m_TalismanWorldEnterLv.size(); i += 2 )
	{
		if ( Level >= ServerCnfg.m_TalismanWorldEnterLv[i] )
			return ServerCnfg.m_TalismanWorldEnterLv[i + 1];
	}

	return INVALID_TWLevel;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//装备制作配置
bool	ConfigServer::OnLoadEquipMake(FileCSV & File,const char* szFileName)
{
	std::vector<SEquipMakeCnfg> vectEquipMake;
	if( File.GetTable(vectEquipMake) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for(int i = 0; i < vectEquipMake.size(); i++)
	{
		m_mapEquipMake[vectEquipMake[i].m_GoodsID] = vectEquipMake[i];
	}

	return true;
}

//得到装备制作配置
const SEquipMakeCnfg * ConfigServer::GetEquipMakeCnfg(UINT16 GoodsID)
{
	std::map<UINT16,SEquipMakeCnfg>::iterator iter = m_mapEquipMake.find(GoodsID);

	if ( iter == m_mapEquipMake.end() ){
		return 0;
	}

	return &(iter->second);
}

//得到所有装备制作配置
const std::map<UINT16, SEquipMakeCnfg> * ConfigServer::GetAllEquipMakeCnfg()
{
	return &m_mapEquipMake;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//灵件升级配置信息
bool ConfigServer::OnLoadUpGhost(FileCSV & File, const char* szFileName)
{
	std::vector<SUpGhostCnfg>  vectUpGhost;

	if( File.GetTable(vectUpGhost) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vectUpGhost.size(); ++i)
	{
		std::hash_map<TGoodsID, VECTUPGHOST>::iterator iter = m_mapUpGhost.find(vectUpGhost[i].m_GhostID);
		if( iter == m_mapUpGhost.end()){
			VECTUPGHOST vectGhost;
			vectGhost.push_back(vectUpGhost[i]);
			m_mapUpGhost[vectUpGhost[i].m_GhostID]  = vectGhost;
		}else{
			VECTUPGHOST & vectGhost = iter->second;
			vectGhost.push_back(vectUpGhost[i]);
		}
	}
	return true;
}


//获取灵件升级的配置信息
const std::vector<SUpGhostCnfg> * ConfigServer::GetUpGhostCnfg(TGoodsID GhostID)
{
	std::hash_map<TGoodsID, VECTUPGHOST>::iterator iter = m_mapUpGhost.find(GhostID);
	if( iter == m_mapUpGhost.end()){
		return 0;
	}

	return &(iter->second);
}

//获取灵件升级的配置信息
const SUpGhostCnfg * ConfigServer::GetUpGhostCnfg(TGoodsID GhostID, UINT8 Level)
{
	std::hash_map<TGoodsID, VECTUPGHOST>::iterator iter = m_mapUpGhost.find(GhostID);
	if( iter == m_mapUpGhost.end()){
		return 0;
	}

	VECTUPGHOST & vectGhost = iter->second;

	for( int i = 0; i < vectGhost.size(); ++i)
	{
		SUpGhostCnfg & GhostCnfg = vectGhost[i];

		if( GhostCnfg.m_GhostLevel == Level){
			return &GhostCnfg;
		}
	}

	return 0;
}

//得到所有灵件升级配置
const std::hash_map<TGoodsID, std::vector<SUpGhostCnfg>> * ConfigServer::GetAllUpGhostCnfg()
{
	return &m_mapUpGhost;
}

/////////////////////////////////////////////////////////////////////////////////////////

//夺宝配置
bool	ConfigServer::OnLoadDuoBaoCnfg(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vecDuoBaoCnfg) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//获得夺宝配置
const SDuoBaoCnfg * ConfigServer::GetDuoBaoCnfg(UINT32 DuoBaoLevel)
{
	for ( int i = 0; i < m_vecDuoBaoCnfg.size(); ++i )
	{
		if ( DuoBaoLevel >= m_vecDuoBaoCnfg[i].m_MinDuoBaoLevel && ( DuoBaoLevel <= m_vecDuoBaoCnfg[i].m_MaxDuoBaoLevel || m_vecDuoBaoCnfg[i].m_MaxDuoBaoLevel == -1) )
		{
			return &m_vecDuoBaoCnfg[i];
		}
	}

	return 0;
}


//获得所有夺宝配置
const std::vector<SDuoBaoCnfg> * ConfigServer::GetAllDuoBaoCnfg()
{
	return &m_vecDuoBaoCnfg;
}

#define SUITFACADE_KEY(SuitID,FacadeID) (((INT32)SuitID << 16)|FacadeID)

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//套装外观
bool ConfigServer::OnLoadSuitFacadeCnfg(FileCSV & File, const char* szFileName)
{
	std::vector<SSuitFacadeInfo> vect;

	if( File.GetTable(vect) == false){
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	for( int i = 0; i < vect.size(); ++i)
	{
		SSuitFacadeInfo & Info = vect[i];

		m_mapSuitFacade[SUITFACADE_KEY(Info.m_SuitID,Info.m_DefaultFacadeID)] = Info.m_UpgradeFacadeID;
	}

	return true;
}


//获得升级外观
UINT16 ConfigServer::GetUpgradeFacadeID(INT16 SuitID,UINT16 DefaultFacadeID)
{
	MAP_FACADE::iterator it = m_mapSuitFacade.find(SUITFACADE_KEY(SuitID,DefaultFacadeID));
	if(it != m_mapSuitFacade.end())
	{
		return (*it).second;
	}
	return 0;
}

//得到玩家当前可招募角色个数
UINT8 ConfigServer::GetCanEmployNum(UINT8 Level)
{
	const SGameConfigParam & GameConfigParam = this->GetGameConfigParam();

	for ( int i = 0; i + 1 < GameConfigParam.m_CanEmployNum.size(); i += 2 )
	{
		if ( Level <= GameConfigParam.m_CanEmployNum[i] )
		{
			return GameConfigParam.m_CanEmployNum[i + 1];
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

//提升资质
bool	ConfigServer::OnLoadUpAptitudeCnfg(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vecUpAptitude) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;	
}

//根据玩家资质获取提升资质配置信息
const SUpAptitude * ConfigServer::GetUpAptitude(INT32 Aptitude)
{
	for ( int i = 0; i < m_vecUpAptitude.size(); ++i )
	{
		if ( Aptitude >= m_vecUpAptitude[i].m_MinAptitudePoint && Aptitude < m_vecUpAptitude[i].m_MaxAptitudePoint )
		{
			return &m_vecUpAptitude[i];
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

//帮战奖励
bool ConfigServer::OnLoadSynWarForwardCnfg(FileCSV & File, const char* szFileName)
{
	if( File.GetTable(m_vecSynWarForward) == false)
	{
		TRACE("%s : %d Line 读取配置文件[%s]失败!", __FUNCTION__, __LINE__, szFileName);
		return false;
	}

	return true;
}

//根据玩家的帮战等级获取帮战奖励
const SSynWarForward * ConfigServer::GetSynWarForward(INT32 SynWarLv)
{
	for ( int i = 0; i < m_vecSynWarForward.size(); ++i )
	{
		SSynWarForward & Forward = m_vecSynWarForward[i];

		if ( SynWarLv >=  Forward.m_MinSynWarLevel && ( SynWarLv <= Forward.m_MaxSynWarLevel || -1 == Forward.m_MaxSynWarLevel ) )
		{
			return & m_vecSynWarForward[i];
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//计算角色实际获得的经验值
INT32 ConfigServer::AcotorRealGetExp(INT16 OldLevel,INT32 OldExp,INT16 NewLevel,INT32 NewExp)
{

	INT32 LevelUpExp = NewExp - OldExp;

	if(OldLevel != NewLevel){
		
		for(int i = OldLevel; i < NewLevel; i++){
		
			const SActorLevelCnfg * pActorLevelCnfg = GetActorLevelCnfg(i + 1);

			if ( 0 == pActorLevelCnfg)
				continue;
			LevelUpExp += pActorLevelCnfg->m_NeedExp;

		}
		

	}

	return LevelUpExp;
	

}
