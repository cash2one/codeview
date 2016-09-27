
#ifndef __CONFIGSERVER_CONFIGSERVER_H__
#define __CONFIGSERVER_CONFIGSERVER_H__

#include "IConfigServer.h"
#include "FileConfig.h"
#include "FileCsv.h"
#include <hash_map>


class ConfigServer : public IConfigServer
{
	typedef std::hash_map<TMonsterID,   SMonsterCnfg> MAP_MONSTER;

public:
	ConfigServer();
	virtual ~ConfigServer();

	virtual bool Create(); 

public:
	virtual void Release();
	
	//杂项配置
	virtual const SGameConfigParam & GetGameConfigParam() ;

	//获得地图所有配置信息
	virtual std::vector<SMapConfigInfo> & GetAllMapConfigInfo();

		//获得单个地图配置信息
	virtual const SMapConfigInfo * GetMapConfigInfo(TMapID mapid) ;

		//获得单个地图怪物分布信息
	virtual const std::vector<SMonsterOutput> * GetMapMonsterCnfg(TMapID mapid); 

		//创建角色数据配置
	virtual const SCreateActorCnfg & GetCreateActorCnfg();

		//获得怪物配置数据
	virtual const SMonsterCnfg* GetMonsterCnfg(TMonsterID MonsterID);

	//增加怪物配置
	virtual void Push_MonsterCnfg(const SMonsterCnfg & MonsterCnfg);

	//获得伏魔洞配置数据
	virtual const SFuMoDongCnfg* GetFuMoDongCnfg(UINT8 level);

	//根据玩家等级获得伏魔洞对应层配置
	virtual const SFuMoDongCnfg* GetFuMoDongCnfgByUserLv(UINT8 UserLevel);

		//物品配置数据
	virtual const SGoodsCnfg* GetGoodsCnfg(TGoodsID GoodsID);

		//物品合成配置
	virtual const SGoodsComposeCnfg * GetGoodsComposeCnfg(TGoodsID GoodsID);

	//获取物品合成全部配置
	virtual const std::hash_map<TGoodsID, SGoodsComposeCnfg> * GetAllGoodsComposeCnfg();

	//强化配置 key:十位数为强化到等级，个位数为强化类型
	virtual const SEquipStrongerCnfg * GetEquipStrongerCnfg(UINT8 key);

	//宝石配置
	virtual const SGemCnfg *   GetGemCnfg(UINT8  GemType);
	
	//招募角色配置
	virtual const SEmployeeDataCnfg * GetEmployeeDataCnfg(TEmployeeID EmployeeID);

	//随机(每条有机率大小不同)获取一个招募角色数据
	virtual const SEmployeeDataCnfg * RandGetEmployData(enFlushType FlushType);

	//随机获得指定资质的招募角色
	virtual const SEmployeeDataCnfg * RandGetEmployApt(UINT32 Aptitude, enFlushType FlushType);

	//得到法术书配置
	virtual const SMagicBookCnfg * GetMagicBookCnfg(TMagicBookID MagicBookID);

	//随机(每条有机率大小不同)获取一个法术书数据
	virtual const SMagicBookCnfg * RandGetMagicBookCnfg(enFlushType FlushType);

	//得到商城抒写商店的物品配置集合
//	virtual const std::hash_map<TGoodsID, SShopMallCnfg> * GetShopMallCnfgByLable(enShopMallLabel lable);

	//放进商场配置表
//	virtual void  Push_ShopMallCnfg(const SShopMallCnfg & ShopMallCnfg);

	//清除商城配置信息
//	virtual void  ClearShopMallCnfg();

		//根据法宝品质级别及法宝世界级别，随机获取一个孕育物品
	virtual TGoodsID GetGestateGoodsID(UINT8 QualityLevel,UINT8 TalismanWorldLevel,UINT8 TalismanWorldType);

		//根据品质点获取品质等级
	virtual UINT8 GetTalismanQualityLevel(INT32 QualityPoint) ;

		//获得效果配置信息
	virtual const SEffectCnfg * GetEffectCnfg(TEffectID  EffectID);

	//获得状态配置信息
	virtual const SStatusCnfg * GetStatusCnfg(TStatusID   StatusID);

		//获取法术配置信息
	virtual const SMagicCnfg * GetMagicCnfg(TMagicID MagicID);

	//获取法术等级配置信息
	virtual const SMagicLevelCnfg * GetMagicLevelCnfg(TMagicID MagicID, UINT8 Level);

	//获取该法术的所有等级信息
	virtual const std::vector<SMagicLevelCnfg *> GetMagicAllLevelCnfg(TMagicID MagicID);

	//获取帮派配置信息
	virtual const SSyndicateCnfg * GetSyndicateCnfg(UINT8 Level);

		//副本配置信息
	virtual const SFuBenCnfg * GetFuBenCnfg(TFuBenID FuBenID); 

	//获取所有帮派物品配置信息
	virtual const std::hash_map<TGoodsID, SSynGoodsCnfg> * GetAllSynGoodsCnfg();

	//获取所有帮派技能的配置信息
	virtual const std::vector<SSynMagicCnfg> GetAllSynMagicCnfg();

	//获取帮派技能的配置信息
	virtual const std::vector<SSynMagicCnfg> * GetSynMagicCnfg(TSynMagicID SynMagicID);

	//获取帮派技能的配置信息
	virtual const SSynMagicCnfg * GetSynMagicCnfg(TSynMagicID SynMagicID, UINT8 Level);

		//物品使用配置
	virtual const SGoodsUseCnfg * GetGoodsUseCnfg(TGoodsID GoodsID);

	//得到剑冢的配置信息
	virtual const SGodSwordCnfg * GetGodSwordShopCnfg(TGoodsID GodSwordID);

	//随机获取一个仙剑数据
	virtual const SGodSwordCnfg * RandGetGodSwordShopCnfg(enFlushType FlushType);

		//根据玩家等级获取合适的帮派保卫战模式
	virtual const SSynCombatCnfg * GetSynCombatCnfg(UINT8 ActorLevel); 

			//根据模式获取配置信息
	virtual const SSynCombatCnfg * GetSynCombatCnfgByMode(UINT8 Mode);

		//状态类型参数
	virtual const SStatusTypeCnfg * GetStatusTypeCnfg(UINT8  StatusType);

	//获取状态组配置
	virtual const SStatusGroupCnfg * GetStatusGroupCnfg(TStatusGroupID StatusGroupID);

		//获得角色等级配置
	virtual const SActorLevelCnfg * GetActorLevelCnfg(UINT8 Level); 

	//获得人物境界配置
	virtual const SActorLayerCnfg * GetActorLayerCnfg(UINT8 Level);

	//获得掉落物品配置
	virtual const std::vector<SDropGoods> * GetDropGoodsCnfg(UINT16 DropID);

	//获得副本主将的掉落ID, bOpenHardType为是否开启困难模式
	virtual UINT16	GetMapBossDropID(TMapID MapID, bool bOpenHardType = false);

	//获得修炼时的好友附加值
	virtual const SXiuLianFriendAdd * GetXiuLianFriendAddCnfg(INT32 nRelationNum);

	//仙剑世界配置
	virtual const SGodSwordWorldCnfg * GetGoldSwordWorldCnfg(TSwordSecretID SwordSecretID);
		//仙剑级别配置
	virtual const SGodSwordLevelCnfg * GetGodSwordLevelCnfg(UINT8  Level); 

		//获得仙剑副本
	virtual const SGodSwordFuBenCnfg * GetGodSwordFuBenCnfg(TFuBenID FuBenID, UINT8 Level);

		//法宝世界配置
	virtual const STalismanWorldCnfg * GetTalismanWorldCnfg(TTalismanWorldID  TalismanWorldID);

	//修炼类法宝世界配置
	virtual const STalismanWorldXiuLianCnfg * GetTalismanWorldXiuLianCnfg(TTalismanWorldID  TalismanWorldID);

	//修炼游戏配置
	virtual const SXiuLianGameParam & GetXiuLianGameParam();

		//修炼奖励
	virtual std::vector<SXiuLianGameAwardCnfg> & GetXiuLianGameAwardCnfg();

		//传送门配置
	virtual const SPortalCnfg * GetPortalCnfg(TPortalID PortalID) ;

		//套装配置信息
	virtual const SSuitCnfg*  GetSuitCnfg(UINT16 SuitID);

	//获得客户端文件版本
	virtual const std::string & GetClientVersion();

	//获得客户端所有配置文件
	virtual const std::vector<std::string> & GetClientCnfgFile(); 

				//获得客户端资源文件版本
	virtual const std::string & GetClientResVersion();

	//获得客户端所有资源文件
	virtual const std::vector<std::string> & GetClientResFile();

public:
	//获得冷却时间
	virtual UINT32 GetCDTimeCnfg(TCDTimerID CDTime_ID);

	//获得指定法宝世界可获得的奇遇
	virtual const std::hash_map<UINT16, SAdventureAwardCnfg> * GetAdventureAwardCnfgVect(TTalismanWorldID TalismanWorldID);

	//获得指定奇遇
	virtual const SAdventureAwardCnfg * GetAdventureAwardCnfg(UINT16 AdventureAwardID);

	//获得GM命令
	virtual const SGMCmdCnfg * GetGMCmdCnfg(std::string strGMCmd);

		//获得任务列表
	virtual const std::hash_map<TTaskID,STaskCnfg> *  GetTaskList(UINT8 TaskClass); 

	virtual const STaskCnfg * GetTaskCnfg(TTaskID TaskID); 

		//战斗地图怪物分布
	virtual const SCombatMapMonster * GetCombatMapMonster(TMonsterID MonsterID);

		//得到战斗地图主将ID
	virtual const TMonsterID GetMainMonsterID(UINT32 CombatIndex);

	//随机获取一个帮派物品任务
	virtual const SSynGoodsTask * RandomGetSynGoodsTask();

	//得到帮派物品任务
	virtual const SSynGoodsTask * GetSynGoodsTask(TTaskID TaskID);

	//帮派物品任务数量
	virtual UINT8	GetSynGoodsTaskNum();

	  	//怪物法术列表
	virtual std::vector<TMagicID> GetMonsterMagicCnfg(UINT8 Level,INT32 num);

	//清理土地法宝游戏
	virtual const SQingLiGameParam & GetQingLiGameParam();

	virtual const SQingLiGameConfig *	 GetQingLiGameCnfg(TTalismanWorldID TalismanWorldID);

	//清理奖励
	virtual const std::vector<SQingLiGameAwardCnfg> & GetQingLiAwardCnfg();

	//寻宝法宝游戏
	virtual const SXunBaoGameParam & GetXunBaoGameParam();

	virtual const SXunBaoGameConfig *	 GetXunBaoGameCnfg(TTalismanWorldID TalismanWorldID);

	//寻宝奖励
	virtual const std::vector<SXunBaoGameAwardCnfg> & GetXunBaoGameAwardCnfg();

	//得到某类型的题目
	virtual const std::hash_map<UINT16/*TiMuID*/, STiMuData> * GetTiMuListCnfg(UINT8 TiMuType);

	//根据题目ID得到题目
	virtual const STiMuData * GetTiMuCnfg(UINT16  TiMuID);

	//得到答题配置
	virtual const SDaTiGameCnfg * GetDaTiCnfg(TTalismanWorldID);

	//得到答题参数配置
	virtual const SDaTiGameParam & GetDaTiParam();

	//得到答题完成级别奖励
	virtual const std::vector<SDaTiGameAwardCnfg> & GetDaTiGameAward();

	//得到答题奖励
	virtual const SDaTiGameLevelAward * GetDaTiGameLevelAward(TTalismanWorldID TalismanWorldID);

				//采集法宝游戏
	virtual const SGatherGameParam & GetGatherGameParam();

	virtual const SGatherGameConfig *	 GetGatherGameCnfg(TTalismanWorldID TalismanWorldID);

	//采集奖励
	virtual const std::vector<SGatherGameAwardCnfg> & GetGatherGameAwardCnfg();

		//获得指定成就
	virtual const SChengJiuCnfg * GetChengJiuCnfg(TChengJiuID ChengJiuID);

	//获得所有成就配置信息
	virtual const std::hash_map<TChengJiuID,SChengJiuCnfg> & GetAllChengJiuCnfg();

		//获到奖励
	virtual const std::map<UINT8,SSignInAwardCnfg> & GetAllSignInAwardCnfg();

	virtual const SSignInAwardCnfg * GetSignInAwardCnfg(UINT8 AwardID);

	//活动配置
	virtual const std::map< UINT16, SActivityCnfg> & GetAllActivityCnfg();

	virtual const SActivityCnfg * GetActivityCnfg(UINT16 ActivityID) ;

	virtual void Push_ActivityCnfg(const SActivityCnfg & Activity);

	virtual void DelActivityCnfg(UINT16 ActivityID);

		//在线奖励
	virtual const SOnlineAwardCnfg * GetOnlineAwardCnfg(UINT16 AwardID) ;

	//获取帮派福利
	virtual const SSynWelfareCnfg * GetSynWelfareCnfg(UINT8/*enWelfare*/ WelfareType, UINT8 SynLevel);

	//获取所有帮派福利
	virtual const std::vector<SSynWelfareCnfg> & GetAllSynWelfareCnfg();

	//得到不可改名的名字
	virtual const std::vector<SNoChangeName> & GetAllNotChangeNameCnfg();

	//得到奖励配置
	virtual const SChallengeForward * GetChallengeForward(UINT8 LvGroup, UINT8 nRank);

	//随机获得强化传承等级
	virtual UINT8 ConfigServer::RandGetStrongInheritLeve(IEquipment * pEuipment, enGoodsCategory DesGoodsClass);

	//得到自动刷新招募角色指定次数的信息
	virtual const SAutoFlushEmployee * GetAutoFlushEmploy(UINT16 FlushNum);

	//随机获得一个必然出现的角色资质
	virtual INT32	RandGetMushAptitude(UINT16 FlushNum);

	//得到该挑战排名最多有多少挑战次数
	virtual INT32	GetMaxChallengeNum(UINT32 nRank);

	//得到语言类型配置  fly add 
	virtual const SLanguageTypeCnfg * GetLanguageTypeCnfg(TLanguageID LanguageID);

	//得到VIP配置信息
	virtual const SVipConfig * GetVipConfig(UINT8 vipLevel);

	//得到充值额度可以升到的VIP级别
	virtual UINT8	GetCanVipLevel(INT32 Recharge);

	//得到服务器状态
	virtual UINT8	GetServerStatus(INT32 UserNum);

	//得到聚灵气配置    
	virtual const SPolyNimbusCnfg * GetPolyNimbusCnfg(UINT16 OperationID);

	//得到玄天奖励
	virtual const SXuanTianForward * GetXTForward(INT32	Rank);

	//得到帮战参数配置
	virtual void GetSynCombatParam(INT32 SynCombatLevel, SSynCombatParam & SynCombatParam);

	//得到剑诀升级配置
	virtual const SDamageLevelCnfg * GetMagicLvCnfg(UINT16 SwordMagicLevel, UINT8 SwordMagicType);

	//得到声望技能配置
	virtual const SCreditMagicCnfg * GetCreditMagicCnfg(UINT16 CreditMagicLevel,UINT8 MagicType);

	//得到法宝世界配置
	virtual const STalismanWorldInfo * GetTalismanWorldInfo(UINT8 level, UINT8 floor);

	//得到所有法宝世界配置
	virtual const std::map<UINT8,	std::vector<STalismanWorldInfo>> * GetAllTalismanWorldInfo();

	//根据玩家等级，获得法宝世界其它配置
	virtual const STalismanWorldParam *  GetTalismanWorldParam(UINT8 Floor);

	//根据玩家等级，获得进入法宝世界的级别
	virtual INT8  GetEnterTWLevel(UINT8 Level);

	//得到装备制作配置
	virtual const SEquipMakeCnfg * GetEquipMakeCnfg(UINT16 GoodsID);

	//获取装备制作全部配置
	virtual const std::map<UINT16, SEquipMakeCnfg> * GetAllEquipMakeCnfg();

	//获取灵件升级的配置信息
	virtual const std::vector<SUpGhostCnfg> * GetUpGhostCnfg(TGoodsID GhostID);

	//获取灵件升级的配置信息
	virtual const SUpGhostCnfg * GetUpGhostCnfg(TGoodsID GhostID, UINT8 Level);

	//获取灵件升级全部配置
	virtual const std::hash_map<UINT16, std::vector<SUpGhostCnfg>> * GetAllUpGhostCnfg();

	//获得夺宝配置
	virtual const SDuoBaoCnfg * GetDuoBaoCnfg(UINT32 DuoBaoLevel);

	//获得所有夺宝配置
	virtual const std::vector<SDuoBaoCnfg> * GetAllDuoBaoCnfg();

		//获得升级外观
	virtual UINT16 GetUpgradeFacadeID(INT16 SuitID,UINT16 DefaultFacadeID);

	//得到玩家当前可招募角色个数
	virtual UINT8 GetCanEmployNum(UINT8 Level);

	//根据玩家资质获取提升资质配置信息
	virtual const SUpAptitude * GetUpAptitude(INT32 Aptitude);

	//根据玩家的帮战等级获取帮战奖励
	virtual const SSynWarForward * GetSynWarForward(INT32 SynWarLv);

	//计算角色实际获得的经验值
	virtual INT32 AcotorRealGetExp(INT16 OldLevel,INT32 OldExp,INT16 NewLevel,INT32 NewExp);

public:

	//称号
	virtual const STitleCnfg * GetTitleCnfg(TTitleID TitleID);
	//virtual const std::string * GetTitleName(TTitleID TitleID);
public:
	//加载地图信息
	bool OnLoadMap(FileCSV & File,const char* szFileName);

	//创建角色
	bool OnLoadCreateActor(FileCSV & File,const char* szFileName);

	//怪物配置
	bool OnLoadMonster(FileCSV & File,const char* szFileName);

	//伏魔洞
	bool OnLoadFuMoDong(FileCSV & File,const char* szFileName);

	//物品
	bool OnLoadGoods(FileCSV & File,const char* szFileName);

	//装备
	bool OnLoadEquipment(FileCSV & File,const char* szFileName);

	//仙剑
	bool OnLoadGodSword(FileCSV & File,const char* szFileName);

	//法宝
	bool OnLoadTalisman(FileCSV & File,const char* szFileName);

	//物品合成
	bool OnLoadGoodsCompose(FileCSV & File,const char* szFileName);

	//强化配置
	bool OnLoadEquipStronger(FileCSV & File,const char* szFileName);

	//宝石配置
	bool OnLoadGemCnfg(FileCSV & File,const char* szFileName);

	//招募角色配置
	bool OnLoadEmployeeData(FileCSV & File, const char* szFileName);

	//法术书配置
	bool OnLoadMagicBook(FileCSV & File, const char* szFileName);

	//商城物品配置
	bool OnLoadShopMall(FileCSV & File, const char* szFileName);

	//法宝品质对应的孕育物品
	bool OnLoadTalismanQuality(FileCSV & File, const char* szFileName);

	//法宝品质等级划分
	bool OnLoadTalismanQualityLevel(FileCSV & File, const char* szFileName);

	//效果
	bool ConfigServer::OnLoadEffect(FileCSV & File, const char* szFileName);

	//状态
	bool ConfigServer::OnLoadStatus(FileCSV & File, const char* szFileName);

	//状态组
	bool ConfigServer::OnLoadStatusGroup(FileCSV & File, const char* szFileName);

	//法术信息
	bool ConfigServer::OnLoadMagic(FileCSV & File, const char* szFileName);

	//法术等级信息
	bool ConfigServer::OnLoadMagicLevel(FileCSV & File, const char* szFileName);

	//地图上的怪物
	bool ConfigServer::OnLoadMapMonster(FileCSV & File, const char* szFileName);

	//帮派配置信息
	bool OnLoadSyndicate(FileCSV & File, const char* szFileName);

	//副本配置信息
	bool OnLoadFuBen(FileCSV & File, const char* szFileName);

	//帮派物品配置信息
	bool OnLoadSynGoods(FileCSV & File, const char* szFileName);

	//帮派技能配置信息
	bool OnLoadSynMagic(FileCSV & File, const char* szFileName);

		//帮派保卫战配置
	bool OnLoadSynCombat(FileCSV & File, const char* szFileName);

		//物品使用配置信息
	bool OnLoadGoodsUse(FileCSV & File, const char* szFileName);

	//剑冢配置
	bool OnLoadGodSwordShop(FileCSV & File, const char* szFileName);

	//状态类型
	bool OnLoadStatusType(FileCSV & File, const char* szFileName);

	//角色等级
	bool ConfigServer::OnLoadActorLevel(FileCSV & File, const char* szFileName);

	//人物境界配置
	bool ConfigServer::OnLoadActorLayer(FileCSV & File, const char* szFileName);

	//掉落物品配置
	bool ConfigServer::OnLoadDropGoods(FileCSV & File, const char* szFileName);

	//获得修炼时的好友附加值
	bool ConfigServer::OnLoadXiuLianFriendAdd(FileCSV & File, const char* szFileName);

	//仙剑世界配置
	bool ConfigServer::OnLoadGoldSwordWorld(FileCSV & File, const char* szFileName);

	//仙剑级别配置
	bool ConfigServer::OnLoadGoldSwordLevel(FileCSV & File, const char* szFileName);

	//仙剑副本配置
	bool ConfigServer::OnLoadGoldSwordFuBen(FileCSV & File, const char* szFileName);

	//法宝世界
	bool ConfigServer::OnLoadTalismanWorld(FileCSV & File, const char* szFileName);

	//修炼类法宝世界
	bool ConfigServer::OnLoadTalismanWorldXiuLian(FileCSV & File, const char* szFileName);

	//修炼类法宝世界
	bool ConfigServer::OnLoadTalismanWorldXiuLianAward(FileCSV & File, const char* szFileName);


		//传送门
	bool ConfigServer::OnLoadPortal(FileCSV & File, const char* szFileName);


	//冷却时间配置
	bool ConfigServer::OnLoadCDTimeCnfg(FileCSV & File, const char* szFileName);

	//奇遇配置
	bool ConfigServer::OnLoadAdventureAwardCnfg(FileCSV & File,const char* szFileName);

	//GM命令
	bool ConfigServer::OnLoadGMCmd(FileCSV & File,const char* szFileName);

	//夺宝奖励
	bool ConfigServer::OnLoadDuoBaoAward(FileCSV & File,const char* szFileName);

	   //任务
   bool ConfigServer::OnLoadTask(FileCSV & File,const char* szFileName);

   //战斗地图上的怪物分布
  bool ConfigServer::OnLoadCombatMapMonster(FileCSV & File,const char* szFileName);

  //怪物法术配置
   bool ConfigServer::OnLoadMonsterMagicCnfg(FileCSV & File,const char* szFileName);

   //寻宝
   	bool ConfigServer::OnLoadTalismanWorldXunBao(FileCSV & File,const char* szFileName);

	bool ConfigServer::OnLoadTalismanWorldXunBaoAward(FileCSV & File,const char* szFileName);

	   //采集
   	bool ConfigServer::OnLoadTalismanWorldGather(FileCSV & File,const char* szFileName);

	bool ConfigServer::OnLoadTalismanWorldGatherAward(FileCSV & File,const char* szFileName);

	//加载成就
	bool ConfigServer::OnLoadChengJiuConfig(FileCSV & File,const char* szFileName);

	//加载签到奖励
	bool ConfigServer::OnLoadSignInAwardConfig(FileCSV & File,const char* szFileName);

	//加载活动
	bool ConfigServer::OnLoadActivityConfig(FileCSV & File,const char* szFileName);

	//在线奖励
	bool OnLoadOnlineAwardConfig(FileCSV & File,const char* szFileName);

	//套装
	bool ConfigServer::OnLoadSuitConfig(FileCSV & File,const char* szFileName);

	//不可改名的角色名或者不能改成这名字
	bool ConfigServer::OnLoadNotChangeName(FileCSV & File,const char* szFileName);



public:
   //帮派物品任务概率配置表
   bool ConfigServer::OnLoadSynGoodsTask(FileCSV & File,const char* szFileName);

   //清理土地类游戏配置
   bool ConfigServer::OnLoadQingLiGame(FileCSV & File,const char* szFileName);

   //清理土地
   bool ConfigServer::OnLoadQingLiGameConfig(FileCSV & File,const char* szFileName);

   //题库
   bool ConfigServer::OnLoadTiKuConfig(FileCSV & File,const char* szFileName);

   //答题
   bool ConfigServer::OnLoadDaTiGameConfig(FileCSV & File,const char* szFileName);

   //答题奖励
   bool ConfigServer::OnLoadDaTiGameLevelAwardConfig(FileCSV & File,const char* szFileName);

   //答题完成级别奖励
   bool ConfigServer::OnLoadDaTiGameAwardConfig(FileCSV & File,const char* szFileName);

   //称号
   bool	ConfigServer::OnLoadTitleConfig(FileCSV & File,const char* szFileName);

   //帮派福利
   bool ConfigServer::OnLoadWelfareConfig(FileCSV & File,const char* szFileName);

	//挑战奖励
   bool ConfigServer::OnLoadChallengeForwardConfig(FileCSV & File,const char* szFileName);

   //强化传承
   bool ConfigServer::OnLoadStrongInherit(FileCSV & File,const char* szFileName);

   //自动刷新招募角色
   bool ConfigServer::OnLoadAutoFlushEmployee(FileCSV & File,const char* szFileName);

   //Vip功能
   bool ConfigServer::OnLoadVipConfig(FileCSV & File,const char* szFileName);

public:
	//杂项游戏参数
   bool OnLoadGameConfig(FileConfig & File,const char* );

   	//修炼类法宝世界游戏参数
   bool OnLoadXiuLianGameConfig(FileConfig & File,const char* );

   //清理类法宝世界游戏参数
   bool OnLoadQingLiGameConfig(FileConfig & File,const char* );

   //寻宝
   bool ConfigServer::OnLoadXunBaoGameConfig(FileConfig & File,const char* );

   //答题参数配置
   bool ConfigServer::OnLoadDaTiParam(FileConfig & File,const char* );

    //采集
   bool ConfigServer::OnLoadGatherGameConfig(FileConfig & File,const char* );

   //服务器端杂项游戏参数
   bool ConfigServer::OnLoadServerGameConfig(FileConfig & File,const char* );

   //游戏语言类型配置   fly add
   bool ConfigServer::OnLoadLanguageType(FileCSV & File,const char* szFileName);

   //聚灵气配置    
   bool ConfigServer::OnLoadPolyNimbus(FileCSV & File,const char* szFileName);

   //玄天奖励
   bool	ConfigServer::OnLoadXTForward(FileCSV & File,const char* szFileName);

   //剑诀升级配置
   bool	ConfigServer::OnLoadGodSwordMagicLv(FileCSV & File,const char* szFileName);

   //声望技能配置
   bool	ConfigServer::OnLoadCreditMagic(FileCSV & File,const char* szFileName);

   //法宝世界配置
   bool	ConfigServer::OnLoadTalismanWorldInfo(FileCSV & File,const char* szFileName);

   //法宝世界其它配置
   bool ConfigServer::OnLoadTalismanWorldParam(FileCSV & File,const char* szFileName);

   //装备制作配置
   bool	ConfigServer::OnLoadEquipMake(FileCSV & File,const char* szFileName);

   //灵件升级配置信息
   bool OnLoadUpGhost(FileCSV & File, const char* szFileName);

   //夺宝配置
   bool	OnLoadDuoBaoCnfg(FileCSV & File, const char* szFileName);

   //套装外观
   bool ConfigServer::OnLoadSuitFacadeCnfg(FileCSV & File, const char* szFileName);

   //提升资质
   bool	ConfigServer::OnLoadUpAptitudeCnfg(FileCSV & File, const char* szFileName);

	//帮战奖励
   bool ConfigServer::OnLoadSynWarForwardCnfg(FileCSV & File, const char* szFileName);

private:
	SGameConfigParam             m_GameConfigParam;  //杂项配置
	std::vector<SMapConfigInfo>  m_vectMapConfigInfo;  //地图配置
	SCreateActorCnfg             m_CreateActorCnfg;    //创建解色配置
	MAP_MONSTER                  m_MapMonster;        //怪物配置
	std::vector<SFuMoDongCnfg>  m_vectFuMoDongConfigInfo;  //伏魔洞配置

	std::hash_map<TGoodsID,SGoodsCnfg> m_mapGoodsCnfg;  //物品配置

	std::hash_map<TGoodsID,SGoodsComposeCnfg> m_mapGoodsComposeCnfg;		//物品合成

	std::hash_map<UINT8/*十位数为强化到等级，个位数为强化类型*/,SEquipStrongerCnfg>    m_mapStrongerCnfg;	//装备强化配置

	std::vector<SGemCnfg>                    m_vectGemCnfg;					//宝石配置

	std::hash_map<TEmployeeID, SEmployeeDataCnfg>	m_mapEmployeeDataCnfg;  //招募角色配置

	std::hash_map<TMagicBookID, SMagicBookCnfg>		m_mapMagicBookCnfg;		//法术书配置

	int  m_nFreeEmployRate;													//招募角色免费刷新的几率总和
	int  m_nMoneyEmployRate;												//招募角色付费刷新的几率总和
	int  m_nFreeMagicBookRate;												//法术书免费刷新的几率总和
	int  m_nMoneyMagicBookRate;												//法术书付费刷新的几率总和

	//typedef std::hash_map<TGoodsID, SShopMallCnfg>  MAP_MALL;

	//std::vector<MAP_MALL>	 m_vecShopMallCnfgByLabel;	//商城购买物品配置

	typedef std::hash_map<UINT32,STalismanQualityCnfg> MAP_QUALITY;  //
	MAP_QUALITY          m_mapTalismanQuality;  //法宝品质对应的孕育物品

	std::vector<STalismanQualityLevel>  m_TalismanQualityLevel; //法宝品质等级划分

	std::hash_map<TEffectID,  SEffectCnfg>    m_mapEffect;   //效果

	typedef std::hash_map<TStatusID,  SStatusCnfg>  MAP_STATUS;
	MAP_STATUS   m_mapStatus;   //状态

	typedef std::hash_map<TStatusGroupID, SStatusGroupCnfg> MAP_STATUSGROUP;

	MAP_STATUSGROUP		m_mapStatusGroup;	//状态组

	std::hash_map<TMagicID,SMagicCnfg>   m_mapMagicCnfg;

	typedef std::hash_map<UINT32,SMagicLevelCnfg> MAP_MAGIC_LEVEL;

	MAP_MAGIC_LEVEL   m_mapMagicLevel;

	typedef std::hash_map<TMapID,std::vector<SMonsterOutput> > MAP_MAP_MONSTER;

	MAP_MAP_MONSTER  m_mapMapMonster;

	std::hash_map<UINT8,  SSyndicateCnfg>	m_mapSyndicate;	//帮派配置

	typedef std::hash_map<UINT16,SFuBenCnfg> MAP_FUBEN;

	MAP_FUBEN               m_mapFuBen;     //副本配置信息

	typedef std::hash_map<UINT16,SGodSwordFuBenCnfg> MAP_GODSWORD_FUBEN;

	MAP_GODSWORD_FUBEN               m_mapGodSwordFuBen;     //仙剑副本配置信息
	         

	std::hash_map<TGoodsID, SSynGoodsCnfg>	m_mapSyndicateGoods; //帮派物品配置

	typedef std::vector<SSynMagicCnfg>	VECTSYNMAGIC;

	std::hash_map<TSynMagicID, VECTSYNMAGIC>		m_mapSynMagic;	//帮派技能配置

	typedef std::hash_map<TGoodsID, SGoodsUseCnfg>  MAP_GOODSUSE;
	MAP_GOODSUSE                      m_mapGoodsUse;  //物品使用配置

	std::hash_map<TGoodsID, SGodSwordCnfg>		m_mapGodSword;	//剑冢配置

	UINT32	m_TotalFreeGodSwordRate;							//剑冢免费机率总和

	UINT32	m_TotalMoneyGodSwordRate;							//剑冢付费机率总和

	typedef std::vector<SSynCombatCnfg>      VECT_SYNCOMBAT;
	VECT_SYNCOMBAT       m_vectSynCombat;        //帮派保卫战配置信息

	typedef std::vector<SStatusTypeCnfg>     VECT_STATUSTYPE; //状态类型配置信息
	VECT_STATUSTYPE        m_vectStatusType;

	typedef std::hash_map<UINT8,SActorLevelCnfg>   MAP_ACTORLEVEL;

	MAP_ACTORLEVEL         m_mapActorLevelCnfg;

	typedef std::hash_map<UINT8,SActorLayerCnfg>   MAP_ACTORLAYER;		//人物境界提升配置文件

	MAP_ACTORLAYER		   m_mapActorLayer;

	typedef std::hash_map<UINT16/*DropID*/,std::vector<SDropGoods> >	 MAP_DROPGOODS;

	MAP_DROPGOODS		   m_mapDropGoods;				//掉落物品的配置

	typedef std::hash_map<TSwordSecretID,SGodSwordWorldCnfg>  MAP_SWORDWORLD;
	typedef std::vector<SXiuLianFriendAdd>				 VECT_XIULIANFRIENDADD;

	VECT_XIULIANFRIENDADD  m_vectXiuLianFriendAdd;

	typedef std::hash_map<TSwordSecretID,SGodSwordWorldCnfg>  MAP_SWORDWORLD;

	MAP_SWORDWORLD        m_mapSwordWorld;     //仙剑世界

	typedef std::hash_map<UINT8,SGodSwordLevelCnfg>  MAP_SWORDLEVEL;

	MAP_SWORDLEVEL       m_mapSwordLevelCnfg;

	typedef std::hash_map<TTalismanWorldID,STalismanWorldCnfg> MAP_TALISMAN_WORLD;  //法宝世界

	MAP_TALISMAN_WORLD  m_mapTalismanWorld;


	typedef std::vector<STalismanWorldXiuLianCnfg>      VECT_TALISMAN_WORLD_XIULIAN;  //修炼类法宝世界
	VECT_TALISMAN_WORLD_XIULIAN       m_vectTalismanWorldXiuLian;

	SXiuLianGameParam                m_XiuLianGameParam;  //修炼类法宝世界游戏配置

	std::vector<SXiuLianGameAwardCnfg> m_vectXiuLianGameAward;  //修炼类法宝世界游戏奖励


	typedef std::hash_map<TPortalID ,SPortalCnfg> MAP_PORTAL;  //传送门

	MAP_PORTAL         m_mapPortal;

	typedef std::hash_map<TCDTimerID, UINT32/*CD时间(秒)*/>	 MAP_CDTIMECNFG;

	MAP_CDTIMECNFG		 m_mapCDTimeCnfg;							//CD时间配置

	typedef std::hash_map<UINT16, SAdventureAwardCnfg>		MAP_ADVENTUREAWARD;

	std::hash_map<TTalismanWorldID, MAP_ADVENTUREAWARD>		m_mapAdventureAwardCnfg;	//法宝奇遇

	typedef std::hash_map<std::string, SGMCmdCnfg>	MAP_GMCMD;

	MAP_GMCMD			m_mapGMCmdCnfg;							//GM命令

	typedef std::hash_map<UINT16, SDuoBaoAwardCnfg>		MAP_DUOBAOAWARD;

	MAP_DUOBAOAWARD		m_mapDuoBaoAwardCnfg;					//夺宝奖励

	typedef std::hash_map<TTaskID,STaskCnfg>  MAP_TASK;
	std::vector<MAP_TASK>       m_vectTaskList; //任务列表

	typedef std::hash_map<TMonsterID,SCombatMapMonster>   MAP_COMBAT_MONSTER; //战斗地图上的怪

	MAP_COMBAT_MONSTER     m_mapCombatMonster;

	typedef std::hash_map<TTaskID, SSynGoodsTask> MAP_SYNGOODSTASK;
	MAP_SYNGOODSTASK			m_mapSynGoodsTask;	//帮派物品任务概率配置表

	INT32 m_SynGoodsTaskTotalRandom;	//概率总和

	typedef std::vector<SMonsterMagicCnfg> VECT_MONSTER_MAGIC;
	VECT_MONSTER_MAGIC                        m_vectMonsterMagic;

	SQingLiGameParam				 m_QingLiGameParam;	 //清理类法宝世界游戏配置

	std::vector<SQingLiGameAwardCnfg> m_vectQingLiAward;	//清理类法宝世界游戏奖励配置

	std::vector<SXunBaoGameAwardCnfg> m_vectXunBaoGameAward;  //寻宝类法宝世界游戏奖励

	typedef std::hash_map<TTalismanWorldID,SQingLiGameConfig> MAP_QINGLIGAME;
	MAP_QINGLIGAME				m_mapQingLiGame;

	typedef std::hash_map<TTalismanWorldID,SXunBaoGameConfig>      MAP_TALISMAN_WORLD_XUNBAO;  //寻宝类法宝世界
	MAP_TALISMAN_WORLD_XUNBAO                          m_mapTalismanWorldXunBao;

	SXunBaoGameParam                m_XunBaoGameParam;  //寻宝类法宝世界游戏配置

	typedef std::hash_map<UINT16/*TiMuID*/, STiMuData>		MAP_TIMUINFO;

	typedef std::hash_map<UINT8/*TiMuType*/ , MAP_TIMUINFO> MAP_TIMUTYPE;

	MAP_TIMUTYPE	m_mapTiMuInfo;	//题库

	typedef std::hash_map<TTalismanWorldID, SDaTiGameCnfg>  MAP_DATAGAMECNFG;

	MAP_DATAGAMECNFG m_mapDaTaGameInfo;	//答题配置信息

	SDaTiGameParam	m_DaTiGameParam;	//答题游戏配置

	typedef std::hash_map<TTalismanWorldID, SDaTiGameLevelAward> MAP_DATIGAMELEVELAWARD;

	MAP_DATIGAMELEVELAWARD m_mapDaTiGameLevelAward;		//答题奖励

	std::vector<SDaTiGameAwardCnfg> m_vectDaTiGameAward;	//答题完成级别奖励

	typedef std::hash_map<TTalismanWorldID,SGatherGameConfig>      MAP_TALISMAN_WORLD_GATHER;  //采集类法宝世界
	MAP_TALISMAN_WORLD_GATHER                          m_mapTalismanWorldGather;

	SGatherGameParam                m_GatherGameParam;  //采集类法宝世界游戏配置

	std::vector<SGatherGameAwardCnfg> m_vectGatherGameAward;  //采集类法宝世界游戏奖励

	typedef std::hash_map<TChengJiuID,SChengJiuCnfg> MAP_CHENGJIU;

	MAP_CHENGJIU  m_mapChengJiu;


	typedef std::map<UINT8,SSignInAwardCnfg> MAP_SIGNIN_AWARD;

	MAP_SIGNIN_AWARD   m_mapSignInAward;  //签奖励

	typedef std::map< UINT16, SActivityCnfg> MAP_ACTIVITY; //活动配置

	MAP_ACTIVITY  m_mapActivity;

	typedef std::hash_map<TTitleID, std::string>	MAP_TITLE;

	//MAP_TITLE	  m_mapTitle;		//称号
	std::hash_map<TTitleID, STitleCnfg>	m_mapTitleCnfg;

	typedef std::hash_map<UINT16,SOnlineAwardCnfg> MAP_ONLINE_AWARD;

	MAP_ONLINE_AWARD  m_mapOnlineAward;

	//套装
	typedef std::hash_map<UINT16,SSuitCnfg>  MAP_SUIT;
	MAP_SUIT      m_mapSuit;  //套装

	typedef std::vector<SSynWelfareCnfg> VECT_SYNWELFARECNFG;

	VECT_SYNWELFARECNFG m_vectSynWelfareCnfg;	//帮派福利

	//不可改名的角色名或者不能改成这名字
	std::vector<SNoChangeName>		m_vectNoChangeName;

	//客户端配置文件版本md5
	std::string    m_strClientVersion;

	std::vector<std::string> m_vectClientCnfgFile;

		//客户端资源文件版本md5
	std::string    m_strClientResVersion;

	std::vector<std::string> m_vectClientResFile;

	//挑战奖励
	std::vector<SChallengeForward>	m_vectChallengeForward;

	//强化传承
	std::vector<SStrongInherit>		m_vecStrongInherit;

	typedef std::map<UINT16/*自动刷新次数*/, SAutoFlushEmployee>	MAP_AUTOEMPLOYEE;

	MAP_AUTOEMPLOYEE				m_mapAutoFlushEmployee;

	std::hash_map<TLanguageID, SLanguageTypeCnfg>	m_mapLanguageTypeCnfg;  //语言类型配置		fly add

	//VIP功能
	typedef std::map<UINT8/*vip等级*/,	SVipConfig>					MAP_VIPCONFIG;

	MAP_VIPCONFIG					m_mapVipConfig;

	std::hash_map<UINT16, SPolyNimbusCnfg>	m_mapPolyNimbusCnfg;  //聚灵气配置

	std::map<INT32,SXuanTianForward>		m_mapXTForward;			//玄天奖励

	typedef std::vector<SCreditMagicCnfg>	VECTCREDITMAGIC;
	std::hash_map<TGoodsID, VECTCREDITMAGIC>		m_mapCreditMagic;	//声望技能配置


	typedef std::vector<SDamageLevelCnfg>	VECTDAMAGELEVEL;
	std::hash_map<UINT16, VECTDAMAGELEVEL>		m_mapGodSwordMagicLv;	//剑诀升级配置



	std::map<UINT16, SEquipMakeCnfg>	m_mapEquipMake;        //装备制作配置

	typedef std::vector<STalismanWorldInfo>	VECT_TALISMANWORLD;

	std::map<UINT8,	VECT_TALISMANWORLD>		m_mapTalismanWorldInfo;	//法宝世界地图配置

	std::vector<STalismanWorldParam>	m_vectTWParam;			//法宝世界其它参数配置

	typedef std::vector<SUpGhostCnfg>	VECTUPGHOST;

	std::hash_map<TGoodsID, VECTUPGHOST>		m_mapUpGhost;	//灵件升级配置

	std::vector<SDuoBaoCnfg>					m_vecDuoBaoCnfg;	//守宝配置

	typedef std::hash_map<INT32,UINT16>   MAP_FACADE;

	MAP_FACADE     m_mapSuitFacade;  //套装外观

	std::vector<SUpAptitude>					m_vecUpAptitude;		//提升资质

	std::vector<SSynWarForward>					m_vecSynWarForward;		//帮战奖励
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef bool (ConfigServer::* OnLoadCsvFunc)(FileCSV & File,const char* );

//.csv文件
static const char *  g_ConfigCsvFile[] =
{
	//客户端文件
	"./Config/client/Map.csv",
	//"./Config/client/ShopMall.csv",
	"./Config/client/FuBen.csv",	
	"./Config/client/ActorLevel.csv",	
	"./Config/client/ActorLayer.csv",
	"./Config/client/GodSwordLevel.csv",
	"./Config/client/TalismanWorld.csv",
	
	//服务器端的文件	
	"./Config/MapMonster.csv",
	"./Config/CreateActor.csv",
	"./Config/Monster.csv",
	"./Config/FuMoDong.csv",
	"./Config/Goods.csv",
	"./Config/Equipment.csv",
	"./Config/GodSword.csv",
	"./Config/Talisman.csv",
	"./Config/clientcommon/GoodsCompose.csv",
	"./Config/EquipStronger.csv",
	//"./Config/GemCnfg.csv",
	"./Config/EmployeeData.csv",
	"./Config/MagicBook.csv",	
	"./Config/TalismanQuality.csv",
	"./Config/TalismanQualityLevel.csv",
	"./Config/Effect.csv",
	"./Config/Status.csv",
	"./Config/Magic.csv",
	"./Config/MagicLevel.csv",
	"./Config/Syndicate.csv",	
	"./Config/SyndicateGoods.csv",
	"./Config/SyndicateMagic.csv",	
	"./Config/GodSwordShop.csv",
	"./Config/GoodsUse.csv",	
	"./Config/SyndicateCombat.csv",	
	"./Config/StatusType.csv",	
	"./Config/DropGoods.csv",
	"./Config/XiuLianFriendAdd.csv",
	"./Config/GodSwordWorld.csv",	
	"./Config/GodSwordFuBen.csv",
	"./Config/TalismanWorldXiuLian.csv",
	"./Config/TalismanWorldXiuLianAward.csv",
	"./Config/Portal.csv",
	"./Config/CDTime.csv",
	"./Config/TalismanWorldAdventureAward.csv",
	"./Config/GMCmd.csv",
	"./Config/StatusGroup.csv",
	"./Config/Task.csv",
	"./Config/CombatMapMonster.csv",
	"./Config/SynGoodsTask.csv",
	"./Config/MonsterMagic.csv",
	"./Config/TalismanWorldQingLi.csv",
	"./Config/TalismanWorldXunBao.csv",
	"./Config/TalismanWorldXunBaoAward.csv",
	"./Config/TalismanWorldGather.csv",
	"./Config/TalismanWorldGatherAward.csv",
	"./Config/TalismanWorldQingLiAward.csv",
	"./Config/ChengJiu.csv",
	"./Config/SignInAward.csv",
//	"./Config/Activity.csv",
	"./Config/Title.csv",
	"./Config/OnlineAward.csv",
	"./Config/Suit.csv",
	"./Config/SynWelfare.csv",
	"./Config/NotChangeName.csv",
	"./Config/ChallengeForward.csv",
	"./Config/StrongInherit.csv",
	"./Config/AutoFlushEmployee.csv",
	"./Config/LanguageType.csv",
	"./Config/clientcommon/Vip.csv",
	"./Config/PolyNimbus.csv",
	"./Config/XuanTianForward.csv",
	"./Config/clientcommon/CreditMagic.csv",
	"./Config/clientcommon/EquipMake.csv",
	"./Config/clientcommon/GodSwordMagicLv.csv",
	"./Config/TalismanWorldInfo.csv",
	"./Config/clientcommon/TalismanWorldParam.csv",
	"./Config/clientcommon/GhostLevel.csv",
	"./Config/DuoBao.csv",
	"./Config/clientcommon/SuitFacade.csv",
	"./Config/UpAptitude.csv",
	"./Config/SynWarForward.csv",
};

//.csv文件加载处理
static OnLoadCsvFunc g_OnLoadCsvFunc[] =
{
	//客户端的文件
	&ConfigServer::OnLoadMap,
	//&ConfigServer::OnLoadShopMall,
	&ConfigServer::OnLoadFuBen,
	&ConfigServer::OnLoadActorLevel,
	&ConfigServer::OnLoadActorLayer,
	&ConfigServer::OnLoadGoldSwordLevel,
	&ConfigServer::OnLoadTalismanWorld,

    //服务器端文件
	&ConfigServer::OnLoadMapMonster,
	&ConfigServer::OnLoadCreateActor,
	&ConfigServer::OnLoadMonster,
	&ConfigServer::OnLoadFuMoDong,
	&ConfigServer::OnLoadGoods,
	&ConfigServer::OnLoadEquipment,
	&ConfigServer::OnLoadGodSword,
	&ConfigServer::OnLoadTalisman,
	&ConfigServer::OnLoadGoodsCompose,
	&ConfigServer::OnLoadEquipStronger,
	//&ConfigServer::OnLoadGemCnfg,
	&ConfigServer::OnLoadEmployeeData,
	&ConfigServer::OnLoadMagicBook,

	&ConfigServer::OnLoadTalismanQuality,
	&ConfigServer::OnLoadTalismanQualityLevel,
	&ConfigServer::OnLoadEffect,
	&ConfigServer::OnLoadStatus,
	&ConfigServer::OnLoadMagic,
	&ConfigServer::OnLoadMagicLevel,
	&ConfigServer::OnLoadSyndicate,
	&ConfigServer::OnLoadSynGoods,
	&ConfigServer::OnLoadSynMagic,
	&ConfigServer::OnLoadGodSwordShop,
	&ConfigServer::OnLoadGoodsUse,
	&ConfigServer::OnLoadSynCombat,
	&ConfigServer::OnLoadStatusType,	
	&ConfigServer::OnLoadDropGoods,
	&ConfigServer::OnLoadXiuLianFriendAdd,
	&ConfigServer::OnLoadGoldSwordWorld,
	&ConfigServer::OnLoadGoldSwordFuBen,
	&ConfigServer::OnLoadTalismanWorldXiuLian,
	&ConfigServer::OnLoadTalismanWorldXiuLianAward,
	&ConfigServer::OnLoadPortal,
	&ConfigServer::OnLoadCDTimeCnfg,
	&ConfigServer::OnLoadAdventureAwardCnfg,
	&ConfigServer::OnLoadGMCmd,
	&ConfigServer::OnLoadStatusGroup,
	&ConfigServer::OnLoadTask,
	&ConfigServer::OnLoadCombatMapMonster,

	&ConfigServer::OnLoadSynGoodsTask,
	&ConfigServer::OnLoadMonsterMagicCnfg,
	&ConfigServer::OnLoadQingLiGame,
	&ConfigServer::OnLoadTalismanWorldXunBao,
	&ConfigServer::OnLoadTalismanWorldXunBaoAward,
	&ConfigServer::OnLoadTalismanWorldGather,
	&ConfigServer::OnLoadTalismanWorldGatherAward,
	&ConfigServer::OnLoadQingLiGameConfig,
	&ConfigServer::OnLoadChengJiuConfig,
	&ConfigServer::OnLoadSignInAwardConfig,
//	&ConfigServer::OnLoadActivityConfig,
	&ConfigServer::OnLoadTitleConfig,
	&ConfigServer::OnLoadOnlineAwardConfig,
	&ConfigServer::OnLoadSuitConfig,
	&ConfigServer::OnLoadWelfareConfig,
	&ConfigServer::OnLoadNotChangeName, 
	&ConfigServer::OnLoadChallengeForwardConfig,
	&ConfigServer::OnLoadStrongInherit,
	&ConfigServer::OnLoadAutoFlushEmployee,
	&ConfigServer::OnLoadLanguageType,
	&ConfigServer::OnLoadVipConfig,
	&ConfigServer::OnLoadPolyNimbus,
	&ConfigServer::OnLoadXTForward,
	&ConfigServer::OnLoadCreditMagic,
	&ConfigServer::OnLoadEquipMake,
	&ConfigServer::OnLoadGodSwordMagicLv,
	&ConfigServer::OnLoadTalismanWorldInfo,
	&ConfigServer::OnLoadTalismanWorldParam,
	&ConfigServer::OnLoadUpGhost,
	&ConfigServer::OnLoadDuoBaoCnfg,
	&ConfigServer::OnLoadSuitFacadeCnfg,
	&ConfigServer::OnLoadUpAptitudeCnfg,
	&ConfigServer::OnLoadSynWarForwardCnfg,
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
typedef bool (ConfigServer::* OnLoadIniFunc)(FileConfig & File,const char* );

//.ini文件
static const char *  g_ConfigIniFile[] =
{
	//客户端也用到的文件
	"./Config/client/GameConfig.ini",

	//仅服务器用到的文件
	"./Config/TalismanWorldXiuLian.ini",
	"./Config/TalismanWorldQingLi.ini",
	"./Config/TalismanWorldXunBao.ini",
	"./Config/TalismanWorldGather.ini",
	"./Config/ServerConfigParam.ini",

};

//.int文件加载处理
static OnLoadIniFunc g_OnLoadIniFunc[] =
{
	&ConfigServer::OnLoadGameConfig,
	&ConfigServer::OnLoadXiuLianGameConfig,
	&ConfigServer::OnLoadQingLiGameConfig,
	&ConfigServer::OnLoadXunBaoGameConfig,
	&ConfigServer::OnLoadGatherGameConfig,
	&ConfigServer::OnLoadServerGameConfig,
};

#endif

