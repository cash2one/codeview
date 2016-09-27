
#ifndef __DB_PROTOCOL_H__
#define __DB_PROTOCOL_H__

#include "FieldDef.h"
#include "DSystem.h"
#include "ProtocolHeader.h"
#include "UniqueIDGenerator.h"
#include "GameSrvProtocol.h"
#include <map>

#pragma pack(push,1)

//DB消息
//登陆模块消息命令字
enum enDBCmd VC_PACKED_ONE 
{
	enDBCmd_DBInit = 0,                 //数据库代理服务器初始化
	enDBCmd_CenterDB            = 1,	//中心数据相关的操作开始

    enDBCmd_GetUserInfo        ,		//获取用户信息
	enDBCmd_InsertUserInfo        ,		//注册用户信息
	enDBCmd_GetKeywordInfo         ,    //获取关键词信息
	enDBCmd_ServerStart           ,     //服务器启动
	enDBCmd_ServerStop           ,     //服务器停止
	enDBCmd_UserEnterGame           ,     //用户进入游戏了
	enDBCmd_UserExitGame           ,     //用户退出游戏了
	enDBCmd_Get_SysMsg,					//获取系统消息
	enDBCmd_Get_ServerInfo ,            //获取服务器信息
	enDBCmd_Get_UserCrlPermission,		//得到玩家的GM命令权限
	enDBCmd_Get_DontTalk,				//禁言
	enDBCmd_Get_SealNo,					//封号
	enDBCmd_Get_AllDontTalk,			//得到所有禁言信息
	enDBCmd_Get_AllSealNo,				//得到所有封号信息
	enDBCmd_Get_GiveGoods,				//发放物品
	enDBCmd_Get_GiveResource,			//发放资源
	enDBCmd_Get_BackSysMail,			//得到后台系统邮件
	enDBCmd_Get_AllSysMsg,				//得到所有系统消息
	enDBCmd_Get_Version,				//得到版本的更新
	enDBCmd_Get_ServiceData,			//得到客服信息

	enDBCmd_Get_PlatformInfo,             //获取平台信息

	enDBCmd_Get_PayData,				//得到充值信息
	enDBCmd_RemoveToPayLog,				//数据移到充值历史记录中
	enDBCmd_Get_AllPayData,				//得到该玩家所有充值信息
	enDBCmd_Get_FirstOpenServerTime,	//得到第一次服务器开启时间
	enDBCmd_Save_FirstOpenServerTime,	//保存第一次服务器开启时间

	enDBCmd_Save_OnLineNum,				//保存在线玩家数量

	enDBCmd_Save_GodStoneLog,			//记录仙石日志

	enDBCmd_Get_NeedLogGoods,			//记录需要日志的物品
	enDBCmd_Save_GameGoodsLog,			//记录物品日志

	enDBCmd_ChangeTicket,               //兑换礼券
	enDBCmd_ChangeTicketFinish,         //兑换礼券完成

	enDBCmd_GetTotalRecharge,			//得到玩家在指定时间内的充值总和

	enDBCmd_GetMultipExpInfo,			//得到多倍经验信息

	enDBCmd_Insert_PointMsg,				//插入节点信息


    enDBCmd_GameDB                ,		//游戏数据库的操作开始
	enDBCmd_GetActorInfo ,              //获取角色信息
	enDBCmd_GetActorInfoByUID ,         //通过UID获取角色信息
	enDBCmd_UpdateActorInfo,            //更新角色信息
	enDBCmd_InsertActorInfo,            //插入角色信息
	enDBCmd_GetActorUIDByName1,			//根据角色名字获取角色UID值
	enDBCmd_GetActorUIDByName2,			//根据角色名字获取角色UID值(实现和上一样)
	enDBCmd_DeleteActor,				//删除角色

	enDBCmd_UpdateFriendInfo,			//更新好友度信息,因为好友度是双方的共同属性，所以也要修改另一方的数据
	enDBCmd_InsertFriendInfo,			//插入好友度信息,如果对方没加你为好友，则也在好友信息表中插入一条新数据
	enDBCmd_DeleteFriendInfo,			//删除好友度信息
	enDBCmd_GetFriendListInfo,			//获取玩家的所有好友度过信息

	enDBCmd_DeleteFriendEnventInfo,		//删除好友度改变事件信息
	enDBCmd_InsertFriendEnventInfo,		//插入好友度改变事件信息
	enDBCmd_GetFriendEnventListInfo,	//获取玩家的所有好友度改变事件的信息

	enDBCmd_InsertFriendMsgInfo,		//插入好友信息标签里的信息
	enDBCmd_DeleteFriendMsgInfo,		//删除好友信息标签里的信息
	enDBCmd_GetFriendMsgListInfo,		//获取此玩家的所有好友信息标签中的信息

	enDBCmd_InsertMailInfo,				//插入邮件,如果邮件数量超过300封，则删除时间最早的那封邮件
	enDBCmd_DeleteMailInfo,				//删除邮件
	enDBCmd_UpdateMailInfo,				//更新邮件数据到数据库
	enDBCmd_GetMailListInfo,			//获取此玩家的所有邮件信息
	
	enDBCmd_InsertTradeInfo,			//插入寄售物品新数据
	enDBCmd_GetTradeDataListInfo,		//加载所有交易物品的信息
	enDBCmd_DeleteTradeDataInfo,		//删除一条交易物品数据

	enDBCmd_InsertSyndicateInfo,		//插入帮派
	enDBCmd_InsertSynMemberInfo,		//插入帮派成员

	enDBCmd_LoadSyndicateListInfo,		//得到所有帮派信息
	enDBCmd_LoadSyndicateMemberListInfo,//得到所有帮派成员信息
	enDBCmd_LoadSyndicateApplyInfo,		//得到所有帮派加入申请者信息

	enDBCmd_InsertSynApplyInfo,			//插入帮派加入申请信息
	enDBCmd_DeleteSynApplyInfo,			//删除帮派加入申请信息
	enDBCmd_DeleteSynMemberInfo,		//删除帮派成员信息

	enDBCmd_UpdateSynMemberInfo,		//更新帮派成员信息
	enDBCmd_UpdateSyndicateInfo,		//更新帮派信息

	enDBCmd_GetBuildingInfo,			//获取资源产出型建筑数据
	enDBCmd_UpdateBuildingInfo,			//更新资源产出型建筑数据
	enDBCmd_GetBuildingRecordInfo,		//获取资源产出型建筑记录数据
	enDBCmd_UpdateBuildingRecordInfo,	//更新资源产出型建筑记录数据

	enDBCmd_InsertFuMoDongInfo,			//插入伏魔洞数据
	enDBCmd_GetFuMoDongInfo,			//获取伏魔洞数据
	enDBCmd_UpdateFuMoDongInfo,			//更新伏魔洞数据

	enDBCmd_InsertTrainingHallInfo,		//插入练功堂数据
	enDBCmd_GetTrainingHallInfo,		//获取练功堂数据
	enDBCmd_UpdateTrainingHallInfo,		//更新练功堂数据

	enDBCmd_InsertGatherGodHouseInfo,	//插入聚仙楼数据
	enDBCmd_GetGatherGodHouseInfo,		//获取聚仙楼数据
	enDBCmd_UpdateGetGatherGodHouseInfo,//更新聚仙楼数据

	enDBCmd_InsertXiuLianInfo,			//插入修炼数据
	enDBCmd_GetXiuLianInfo,				//得到修炼数据
	enDBCmd_UpdateXiuLianInfo,			//更新修炼数据
	enDBCmd_UpdateXiuLianRecord,        //更新修炼记录
	enDBCmd_DeleteXiuLianRecord,		//删除修炼记录

	enDBCmd_InsertFuBenInfo,			//插入副本数据
	enDBCmd_GetFuBenInfo,				//得到副本数据
	enDBCmd_UpdateFuBenInfo,			//更新副本数据
	enDBCmd_UpdateFuBenProgressInfo,	//更新副本进度数据

	enDBCmd_InsertGodSwordShopInfo,		//插入剑冢数据
	enDBCmd_GetGodSwordShopInfo,		//得到剑冢数据
	enDBCmd_UpdateGodSwordShopInfo,		//更新剑冢数据

	enDBCmd_InsertPacketInfo,			//插入背包数据
	enDBCmd_GetPacketInfo,				//得到背包数据
	enDBCmd_UpdatePacketInfo,			//更新背包数据

	enDBCmd_InsertEquipInfo,			//插入装备栏数据
	enDBCmd_GetEquipInfo,				//得到装备栏数据
	enDBCmd_UpdateEquipInfo,			//更新装备栏数据

	enDBCmd_InsertMagicPanelInfo,		//插入法术栏数据
	enDBCmd_GetMagicPanelInfo,			//得到法术栏数据
	enDBCmd_UpdateMagicPanelInfo,		//更新法术栏数据

	enDBCmd_InsertSynMagicInfo,			//插入帮派技能数据
	enDBCmd_GetSynMagicInfo,			//得到帮派技能数据
	enDBCmd_UpdateSynMagicInfo,			//更新帮派技能数据

	enDBCmd_InsertCombatInfo,			//插入战斗数据
	enDBCmd_GetCombatInfo,				//得到战斗数据
	enDBCmd_UpdateCombatInfo,			//更新战斗数据

	enDBCmd_InsertActorBasicDataInfo,	//插入玩家的基本数据
	enDBCmd_GetActorBasicDataInfo,		//获取玩家的基本数据

	enDBCmd_GetActorNameByUID,			//根据玩家的UID值获取玩家的名字
	enDBCmd_Update_ActorBasicDataInfo,	//更新玩家的基本数据

	enDBCmd_GetActorEmployInfo,			//得到玩家招募角色的数据集

	enDBCmd_GetActorBasicDataByUIDInfo,	//得到玩家的信息通过UID

	enDBCmd_GetStatusInfo,				//得到状态信息
	enDBCmd_UpdateStatusInfo,			//更新状态信息

	enDBCmd_ViewSynMemberListLevel,		//查看帮派列表时,得到帮众的等级

	enDBCmd_DouFaRandEnemyList,			//斗法随机获得敌人
	enDBCmd_InsertDouFaPartInfo,		//插入斗法数据
	enDBCmd_GetDouFaPartInfo,			//获取斗法数据
	enDBCmd_UpdateDouFaPartInfo,		//更新斗法数据

	enDBCmd_GetTaskPartInfo,			//获取任务PART与任务数据
	enDBCmd_InsertTaskPartInfo,			//插入任务PART数据
	enDBCmd_UpdateTaskPartInfo,			//更新任务PART数据
	enDBCmd_UpdateTaskInfo,				//更新任务

	enDBCmd_GetActorFacade,				//得到玩家外观

	enDBCmd_GetChengJiuInfo,			//得到成就数据
	enDBCmd_InsertChengJiuPartInfo,		//插入成就PART数据
	enDBCmd_UpdateChengJiuPartInfo,		//更新成就PART数据
	enDBCmd_UpdateChengJiuInfo,			//更新单个成就数据

	enDBCmd_GetActivityPartInfo,       //获得活动part数据
	enDBCmd_UpdateActivityPartInfo,    //更新活动part数据
	enDBCmd_UpdateActivityData,        //更新单个活动数据

	enDBCmd_UpdatePacketPacketPart,	   //更新背包PART数据
	enDBCmd_UpdatePacketPacketGoods,   //更新背包单个物品数据
	enDBCmd_DeletePacketPacketGoods,   //删除背包单个物品数据

	enDBCmd_SaveGoods,					//保存物品
	enDBCmd_DeleteGoods,				//删除物品

	enDBCmd_LoadUserRank,				//加载玩家排行

	enDBCmd_DeleteSyn,					//删除帮派

	enDBCmd_GetSectionMail,				//得到一个区间的邮件
	enDBCmd_DeleteOverMail,				//删除超出最大邮件数的邮件

	enDBCmd_GetCDTimer,					//得到冷却时间
	enDBCmd_UpdateCDTimer,				//更新冷却时间
	enDBCmd_DeleteCDTimer,				//删除冷却时间

	enDBCmd_GetUserDataByName,			//通过名字得到玩家基本信息

	enDBCmd_DeleteAllStatus,			//删除状态

	enDBCmd_GetMailNum,					//得到邮件数量

	enDBCmd_Update_UserRank,			//更新玩家排行
	enDBCmd_Get_UserRank,				//得到玩家排行
	enDBCmd_Get_MyRank,					//得到我的排行
	enDBCmd_Insert_UserRank,			//插入我的排行

	enDBCmd_Get_ChallengeLevelRank,		//得到等级区间挑战排行
	enDBCmd_Get_ChallengeMyEnemy,		//得到我可挑战的对手
	enDBCmd_Insert_ChallengeRank,		//插入到挑战排行
	enDBCmd_Change_ChallengeRank,		//更换挑战排行榜排名
	enDBCmd_Update_AllUserLevelChallenge,//周日晚更新挑战排行榜所有玩家等级
	enDBCmd_Check_CanChallenge,			//检测是否是自己可挑战的玩家
	enDBCmd_Get_ChallengeRank,			//得到总挑战排行

	enDBCmd_Get_MyChallengeRank,		//得到我的总挑战排行

	enDBCmd_Flush_QieCuoEnemy,			//刷新切磋的对手

	enDBCmd_Flush_DouFaEnemy,			//刷新斗法的对手

	enDBCmd_Insert_AllUserSysMail,		//给所有玩家写系统邮件

	enDBCmd_Insert_SysMailByUserID,		//通过UserID给玩家写邮件

	enDBCmd_OffLine_RemoveGoods,		//玩家不在线时的扣除物品

	enDBCmd_OffLine_DescRes,			//离线扣除钱灵石礼卷

	enDBCmd_Insert_AllSynMemberSysMail,	//给所有帮派成员写系统邮件

	enDBCmd_Delete_ResBuildRecord,		//删除资源建筑代收记录

	enDBCmd_Insert_ResBuildRecord,		//插入资源建筑代收记录

	enDBCmd_OpenServer_Init,			//开服数据库初始化

	enDBCmd_Get_ActivityCnfg,			//得到活动配置

	enDBCmd_Get_MyRank_Challenge,		//得到我的排行，只得到排行

	enDBCmd_Get_AllShopMallCnfg,		//得到所有商场配置信息

	enDBCmd_Update_ShopMallCnfg,		//更新商场配置信息

	enDBCmd_Get_OneGoodsShopCnfg,		//得到单个商场配置信息

	enDBCmd_Insert_ShopMallLog,			//记录商城日志

	enDBCmd_Save_XTBossInfo,			//记录玄天BOSS信息
	enDBCmd_Get_XTBossInfo,				//得到玄天BOSS信息
	enDBCmd_Delete_XTBossInfo,			//清除玄天BOSS信息
	enDBCmd_Save_XTBossDie,				//记录玄天BOSS死亡信息
	enDBCmd_Get_XTBossDie,				//得到玄天BOSS死亡信息
	enDBCmd_Change_XTBossDie,			//删除上一次玄天BOSS死亡信息，并把BOSS死亡信息转为上一次BOSS死亡信息
	enDBCmd_Update_XTDamage,			//更新玄天伤害
	enDBCmd_Get_XTDamage,				//得到玄天伤害
	enDBCmd_Change_XTLastDamage,		//删除上一次伤害，并把当前伤害转为上一次伤害

	enDBCmd_Save_TalismanWorld,			//记录法宝世界数据
	enDBCmd_Get_TalismanWorld,			//得到法宝世界数据
	enDBCmd_Delete_TalismanWorld,		//删除法宝世界数据

	enDBCmd_Get_TalismanPart,			//得到法宝世界PART数据
	enDBCmd_Update_TalismanPart,		//更新法宝世界PART数据

	
	enDBCmd_Get_SynWarKillHistory,		//得到帮战杀敌记录
	enDBCmd_Update_SynWarKillHistory,	//更新排长战杀敌记录
	enDBCmd_Get_SynWarInfo,				//得到帮战情况
	enDBCmd_Update_SynWarInfo,			//更新排长战情况
	enDBCmd_Get_SynWarDeclare,			//得到帮战宣战
	enDBCmd_Insert_SynWarDeclare,		//插入帮战宣战
	enDBCmd_Delete_SynWarDeclare,		//删除所有帮战宣战
	enDBCmd_Get_LastRecordTime,			//得到帮战最后记录时间


	enDBCmd_Get_SynPart,				//得到帮派PART数据
	enDBCmd_Update_SynPart,				//更新帮派PART数据

	enDBCmd_Delete_SynWarInfo,			//删除所有帮战信息

	enDBCmd_Update_DuoBaoRank,			//更新夺宝排行
	enDBCmd_Get_DuoBaoRank,				//得到夺宝排行
	enDBCmd_Get_MyDuoBaoRank,			//得到我的夺宝排行
	enDBCmd_Insert_DuoBaoRank,			//插入夺宝排行

	enDBCmd_Get_MyChallengeRecord,		//得到我的前n场挑战信息
	enDBCmd_Get_ChallengeRecordBuf,		//得到我的前n场战斗buf（一场）
	enDBCmd_Insert_MyChallengeRecord,	//插入到前n场挑战信息
	enDBCmd_Get_ChallengeLevelTotalNum,	//得到挑战区间总人数

	enDBCmd_Update_PreSynWarInfo,		//更新上一场帮战信息

	enDBCmd_GetRechargeForward,			//得到充值返利信息

	enDBCmd_GetRechargeForwardCnfg,		//得到充值返利配置信息


	enDBCmd_GetMonsterCnfg,				//得到怪物配置信息

	enDBCmd_GetActivityCnfg,			//获取单个活动配置

	enDBCmd_DeleteTask,					//删除任务

	enDBCmd_UpdateTask,					//插入任务
	enDBCmd_UpdateOccupationInfo,		//更新法宝世界占领时被击败的信息
	enDBCmd_GetOccupationInfo,			//得到法宝世界占领时被谁击败
	enDBCmd_UpdateOccupationGhostSoul,	//更新法宝世界占领被击败获得的灵魄

	enDBCmd_Update_LastOnlineTime,		//保存玩家上次上线时间

	enDBCmd_Get_FirstRechargeCnfg,		//得到首次充值配置

	enDBCmd_Get_IsGetFirstRechargeForward,	//检测是否可以给某次时间起的首次充值奖励
	
	enDBCmd_Max,
} PACKED_ONE;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//命令字与数据结构的映射
#define DB_ALL_CMD_TO_TYPE  \
    CMD_TO_TYPE(enDBCmd_GetUserInfo,             DB_Get_User_Info_Req)   \
	CMD_TO_TYPE(enDBCmd_InsertUserInfo,          DB_Insert_User_Info_Req)  \
	CMD_TO_TYPE(enDBCmd_GetKeywordInfo,          DB_Get_Keyword_Info_Req)  \
	CMD_TO_TYPE(enDBCmd_Get_ServerInfo,          DB_Get_Server_Info_Req)  \
	CMD_TO_TYPE(enDBCmd_ServerStart,             DB_GameServerStart_Req)  \
	CMD_TO_TYPE(enDBCmd_ServerStop,              DB_GameServerStop_Req)  \
	CMD_TO_TYPE(enDBCmd_UserEnterGame,           DB_UserEnterGame_Req)  \
	CMD_TO_TYPE(enDBCmd_UserExitGame,            DB_UserExitGame_Req)  \
	CMD_TO_TYPE(enDBCmd_Get_PlatformInfo,        DB_GetPlatformInfo_Req)\
	CMD_TO_TYPE(enDBCmd_ChangeTicket,            DB_ChangeTicket_Req) \
	CMD_TO_TYPE(enDBCmd_ChangeTicketFinish,      DB_ChangeTicketFinish_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorInfo,            SDB_Get_ActorBasicData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateActorInfo,         SDB_Update_ActorBasicData_Req) \
    CMD_TO_TYPE(enDBCmd_InsertActorInfo,         SDB_Insert_ActorBasicData_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorUIDByName1,		 SDB_Get_ActorUIDByName_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorUIDByName2,		 SDB_Get_ActorUIDByName_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteActor,			 SDB_Delete_Actor) \
	CMD_TO_TYPE(enDBCmd_UpdateFriendInfo,		 SDB_Update_FriendData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertFriendInfo,		 SDB_Insert_FriendData_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteFriendInfo,		 SDB_Delete_FriendData_Req) \
	CMD_TO_TYPE(enDBCmd_GetFriendListInfo,		 SDB_Get_FriendListData_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteFriendEnventInfo,	 SDB_Delete_FriendEnventData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertFriendEnventInfo,	 SDB_Insert_FriendEnventData_Req) \
	CMD_TO_TYPE(enDBCmd_GetFriendEnventListInfo, SDB_Get_FriendEnventListData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertFriendMsgInfo,	 SDB_Insert_FriendMsgData_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteFriendMsgInfo,	 SDB_Delete_FriendMsgData_Req) \
	CMD_TO_TYPE(enDBCmd_GetFriendMsgListInfo,	 SDB_Get_FriendMsgListData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertMailInfo,			 SDB_Insert_MailData_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteMailInfo,			 SDB_Delete_MailDate_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateMailInfo,			 SDB_Updata_MailData_Req) \
	CMD_TO_TYPE(enDBCmd_GetMailListInfo,		 SDB_Get_MailDataList_Req) \
	CMD_TO_TYPE(enDBCmd_InsertTradeInfo,		 SDB_Insert_TradeData_Req) \
	CMD_TO_TYPE(enDBCmd_GetTradeDataListInfo,	 SDB_Get_TradeDataList_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteTradeDataInfo,	 SDB_Delete_TradeData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertSyndicateInfo,	 SDB_Insert_Syndicate_Req) \
	CMD_TO_TYPE(enDBCmd_InsertSynMemberInfo,	 SDB_Insert_SyndicateMember_Req) \
	CMD_TO_TYPE(enDBCmd_LoadSyndicateListInfo,	 SDB_Get_SyndicateList_Req) \
	CMD_TO_TYPE(enDBCmd_LoadSyndicateMemberListInfo,	 SDB_Get_SyndicateMemberList_Req) \
	CMD_TO_TYPE(enDBCmd_LoadSyndicateApplyInfo,	 SDB_Get_SyndicateApplyList_Req) \
	CMD_TO_TYPE(enDBCmd_InsertSynApplyInfo,		 SDB_Insert_SyndicateApply_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteSynApplyInfo,		 SDB_Delete_SyndicateApply_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteSynMemberInfo,	 SDB_Delete_SyndicateMember_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateSynMemberInfo,	 SDB_Update_SyndicateMember_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateSyndicateInfo,	 SDB_Updata_Syndicate_Req) \
	CMD_TO_TYPE(enDBCmd_GetBuildingInfo,		 SDB_GetBuildingData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateBuildingInfo,		 SDB_Update_BuildingData_Req) \
	CMD_TO_TYPE(enDBCmd_GetBuildingRecordInfo,   SDB_Get_BuildingRecord_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateBuildingRecordInfo,   SDB_Update_BuildingRecord_Req) \
	CMD_TO_TYPE(enDBCmd_InsertFuMoDongInfo,		 SDB_Insert_FuMoDongData_Req) \
	CMD_TO_TYPE(enDBCmd_GetFuMoDongInfo,		 SDB_Get_FuMoDongData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateFuMoDongInfo,		 SDB_Update_FuMoDongData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertTrainingHallInfo,	 SDB_Insert_TrainingHallData_Req) \
	CMD_TO_TYPE(enDBCmd_GetTrainingHallInfo,	 SDB_Get_TrainingHallData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateTrainingHallInfo,	 SDB_Update_TrainingHallData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertGatherGodHouseInfo,SDB_Insert_GatherGodHouseData_Req) \
	CMD_TO_TYPE(enDBCmd_GetGatherGodHouseInfo,	 SDB_Get_GatherGodHouseData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateGetGatherGodHouseInfo,	 SDB_Update_GatherGodHouseData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertXiuLianInfo,		 SDB_Insert_XiuLianData_Req) \
	CMD_TO_TYPE(enDBCmd_GetXiuLianInfo,			 SDB_Get_XiuLianData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateXiuLianInfo,		 SDB_Update_XiuLianData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateXiuLianRecord,	 SDB_Update_XiuLianData_Record_Req)  \
	CMD_TO_TYPE(enDBCmd_DeleteXiuLianRecord,	 SDB_DeleteXiuLianRecord_Req)  \
	CMD_TO_TYPE(enDBCmd_InsertFuBenInfo,		 SDB_Insert_FuBenData_Req) \
	CMD_TO_TYPE(enDBCmd_GetFuBenInfo,			 SDB_Get_FuBenData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateFuBenInfo,		 SDB_Update_BasicFuBenData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateFuBenProgressInfo, SDB_Update_FuBenProgressData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertGodSwordShopInfo,  SDB_Insert_GodSwordShopData_Req) \
	CMD_TO_TYPE(enDBCmd_GetGodSwordShopInfo,	 SDB_Get_GodSwordShopData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateGodSwordShopInfo,  SDB_Update_GodSwordShopData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertPacketInfo,		 SDB_Insert_PacketData_Req) \
	CMD_TO_TYPE(enDBCmd_GetPacketInfo,			 SDB_Get_PacketData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdatePacketInfo,		 SDB_Update_PacketData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertEquipInfo,		 SDB_Insert_EquipPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_GetEquipInfo,			 SDB_Get_EquipPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateEquipInfo,		 SDB_Update_EquipPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertMagicPanelInfo,	 SDB_Insert_MagicPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_GetMagicPanelInfo,		 SDB_Get_MagicPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateMagicPanelInfo,	 SDB_Update_MagicPanelData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertSynMagicInfo,		 SDB_Insert_SynMagicData_Req) \
	CMD_TO_TYPE(enDBCmd_GetSynMagicInfo,		 SDB_Get_SynMagicData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateSynMagicInfo,		 SDB_Update_SynMagicData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertCombatInfo,		 SDB_Insert_CombatData_Req) \
	CMD_TO_TYPE(enDBCmd_GetCombatInfo,			 SDB_Get_CombatData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateCombatInfo,		 SDB_Update_CombatData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertActorBasicDataInfo,SDB_Insert_ActorBasicData_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorBasicDataInfo,	 SDB_Get_ActorBasicData_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorNameByUID,		 SDB_Get_ActorNameByUid_Req) \
	CMD_TO_TYPE(enDBCmd_Update_ActorBasicDataInfo,	SDB_Update_ActorBasicData_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorEmployInfo,		 SDB_Get_EmployeeData_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorBasicDataByUIDInfo,	SDB_Get_ActorBasicDataByUID_Req) \
	CMD_TO_TYPE(enDBCmd_GetStatusInfo,			 SDB_Get_Status_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateStatusInfo,		 SDB_Update_Status_Req) \
	CMD_TO_TYPE(enDBCmd_ViewSynMemberListLevel,	 SDB_Get_SynMemberListLevel_Req) \
	CMD_TO_TYPE(enDBCmd_DouFaRandEnemyList,		 SDB_GetDouFaEnemy_Req) \
	CMD_TO_TYPE(enDBCmd_GetDouFaPartInfo,		 SDB_Get_DouFaPartInfo_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateDouFaPartInfo,	 SDB_Update_DouFaPartInfo_Req) \
	CMD_TO_TYPE(enDBCmd_GetTaskPartInfo,		 SDB_Get_TaskPartInfo_Req) \
	CMD_TO_TYPE(enDBCmd_InsertTaskPartInfo,		 SDB_Insert_TaskPartInfo_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateTaskPartInfo,		 SDB_Update_TaskPartInfo_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateTaskInfo,			 SDB_Update_TaskInfo_Req) \
	CMD_TO_TYPE(enDBCmd_GetActorFacade,			 SDB_GetActorFacade_Req) \
	CMD_TO_TYPE(enDBCmd_GetChengJiuInfo,		 SDB_Get_ChengJiuData_Req) \
	CMD_TO_TYPE(enDBCmd_InsertChengJiuPartInfo,	 SDB_Insert_ChengJiuPart_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateChengJiuPartInfo,	 SDB_Update_ChengJiuPart_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateChengJiuInfo,		 SDB_Update_ChengJiu_Req) \
    CMD_TO_TYPE(enDBCmd_GetActivityPartInfo,	 SDB_Get_ActivityData_Req)  \
    CMD_TO_TYPE(enDBCmd_UpdateActivityPartInfo,	 SDB_UpdateActivityPart_Req)    \
    CMD_TO_TYPE(enDBCmd_UpdateActivityData,		 SDB_ActivityData_Req) \
	CMD_TO_TYPE(enDBCmd_UpdatePacketPacketPart,	 SDB_UpdatePacketPartReq) \
	CMD_TO_TYPE(enDBCmd_UpdatePacketPacketGoods, SDB_UpdateGoodsReq) \
	CMD_TO_TYPE(enDBCmd_DeletePacketPacketGoods, SDB_DeleteGoodsReq) \
	CMD_TO_TYPE(enDBCmd_SaveGoods,				 SDB_Save_GoodsReq) \
	CMD_TO_TYPE(enDBCmd_DeleteGoods,			 SDB_Delete_GoodsReq) \
	CMD_TO_TYPE(enDBCmd_LoadUserRank,			 SDB_Load_RankReq) \
	CMD_TO_TYPE(enDBCmd_DeleteSyn,				 SDB_Delete_Syndicate_Req) \
	CMD_TO_TYPE(enDBCmd_GetSectionMail,			 SDB_Get_SectionMailData_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteOverMail,			 SDB_Delete_OverMail_Req) \
	CMD_TO_TYPE(enDBCmd_GetCDTimer,				 SDB_Get_UserCDTimer_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateCDTimer,			 SDB_Update_UserCDTimer_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteCDTimer,			 SDB_Delete_UserCDTimer_Req) \
	CMD_TO_TYPE(enDBCmd_GetUserDataByName,		 SDB_Get_ActorBasicDataByName_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SysMsg,				 SDB_Get_SysMsg_Req) \
	CMD_TO_TYPE(enDBCmd_DeleteAllStatus,		 SDB_Delete_AllStatus_Req) \
	CMD_TO_TYPE(enDBCmd_GetMailNum,				 SDB_Get_MailNum_Req) \
	CMD_TO_TYPE(enDBCmd_Update_UserRank,		 SDB_Update_UserRank) \
	CMD_TO_TYPE(enDBCmd_Get_UserRank,			 SDB_Get_UserRank) \
	CMD_TO_TYPE(enDBCmd_Get_MyRank,				 SDB_Get_MyRank) \
	CMD_TO_TYPE(enDBCmd_Insert_UserRank,		 SDB_Insert_UserRank) \
	CMD_TO_TYPE(enDBCmd_Get_ChallengeLevelRank,	 SDB_Get_ChallengeLevelRank) \
	CMD_TO_TYPE(enDBCmd_Get_ChallengeMyEnemy,	 SDB_Get_ChallengeMyEnemy) \
	CMD_TO_TYPE(enDBCmd_Insert_ChallengeRank,	 SDB_Insert_ChallengeRank) \
	CMD_TO_TYPE(enDBCmd_Change_ChallengeRank,	 SDB_Change_ChallengeRank) \
	CMD_TO_TYPE(enDBCmd_Update_AllUserLevelChallenge, SDB_Update_AllUserLevelChallenge) \
	CMD_TO_TYPE(enDBCmd_Check_CanChallenge,		 SDB_Check_CanChallenge) \
	CMD_TO_TYPE(enDBCmd_Get_ChallengeRank,		 SDB_Get_ChallengeRank) \
	CMD_TO_TYPE(enDBCmd_Get_MyChallengeRank,	 SDB_Get_MyChallengeRank) \
	CMD_TO_TYPE(enDBCmd_Get_UserCrlPermission,	 SDB_Get_UserCrlPermission) \
	CMD_TO_TYPE(enDBCmd_Flush_QieCuoEnemy,		 SDB_Flush_QieCuoEnemy) \
	CMD_TO_TYPE(enDBCmd_Flush_DouFaEnemy,		 SDB_Flush_DouFaEnemy) \
	CMD_TO_TYPE(enDBCmd_Get_DontTalk,			SDB_DontTalk_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SealNo,				SDB_SealNo_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_AllUserSysMail,  SDB_WriteAllUserSysMail_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_SysMailByUserID,	SDB_Insert_SysMailByUserID) \
	CMD_TO_TYPE(enDBCmd_OffLine_RemoveGoods,	SDB_OffLine_RemoveGoods_Req) \
	CMD_TO_TYPE(enDBCmd_OffLine_DescRes,		SDB_OffLine_DescRes_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_AllSynMemberSysMail,SDB_WriteAllSynMemberSysMail_Req) \
	CMD_TO_TYPE(enDBCmd_Get_AllDontTalk,		SDB_AllDontTalk_Req) \
	CMD_TO_TYPE(enDBCmd_Get_AllSealNo,			SDB_GetAllSealNo_Req) \
	CMD_TO_TYPE(enDBCmd_Get_GiveGoods,			SDB_GiveGoods_Req) \
	CMD_TO_TYPE(enDBCmd_Get_GiveResource,		SDB_GiveResource_Req) \
	CMD_TO_TYPE(enDBCmd_Get_BackSysMail,		SDB_BackSysMail_Req) \
	CMD_TO_TYPE(enDBCmd_Get_AllSysMsg,			SDB_Get_AllSysMsg_Req) \
	CMD_TO_TYPE(enDBCmd_Get_Version,			SDB_Get_Version_Req) \
	CMD_TO_TYPE(enDBCmd_Get_ServiceData,		SDB_Get_ServiceData_Req) \
	CMD_TO_TYPE(enDBCmd_Get_PayData,			SDB_Get_PayData_Req) \
	CMD_TO_TYPE(enDBCmd_RemoveToPayLog,			SDB_RemoveToPayLog_Req) \
	CMD_TO_TYPE(enDBCmd_Get_AllPayData,			SDB_Get_AllPayData_Req) \
	CMD_TO_TYPE(enDBCmd_Delete_ResBuildRecord,	SDB_Delete_BuildingRecord_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_ResBuildRecord,	SDB_Insert_BuildingRecord_Req) \
	CMD_TO_TYPE(enDBCmd_Get_FirstOpenServerTime,SDB_Get_FirstOpenServerTimeReq) \
	CMD_TO_TYPE(enDBCmd_Save_FirstOpenServerTime, SDB_Save_FirstOpenServerTime_Req) \
	CMD_TO_TYPE(enDBCmd_OpenServer_Init,		SDB_OpenServer_Init_Req) \
	CMD_TO_TYPE(enDBCmd_Save_OnLineNum,			SDB_Save_OnlineInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Get_ActivityCnfg,		SDB_Get_ActivityCnfg_Req) \
	CMD_TO_TYPE(enDBCmd_Save_GodStoneLog,		SDB_Save_GodStoneLog_Req) \
	CMD_TO_TYPE(enDBCmd_Get_NeedLogGoods,		SDB_Get_NeedLogGoodsID_Req) \
	CMD_TO_TYPE(enDBCmd_Save_GameGoodsLog,		SDB_Save_GameGoodsLog_Req) \
	CMD_TO_TYPE(enDBCmd_Get_MyRank_Challenge,	SDB_Get_MyRank_Challenge_Req) \
	CMD_TO_TYPE(enDBCmd_Get_AllShopMallCnfg,	SDB_Get_ShopMallCnfg_Req) \
	CMD_TO_TYPE(enDBCmd_Update_ShopMallCnfg,	SDB_Update_ShopMallCnfg_Req) \
	CMD_TO_TYPE(enDBCmd_Get_OneGoodsShopCnfg,	SDB_Get_OneGoodsShopCnfg_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_ShopMallLog,		SDB_Insert_ShopMallLog_Req) \
	CMD_TO_TYPE(enDBCmd_Save_XTBossInfo,		SDB_Save_XTBossInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Get_XTBossInfo,			SDB_Get_XTBossInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Delete_XTBossInfo,		SDB_Delete_XTBossInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Save_XTBossDie,			SDB_Save_XTBossDie_Req) \
	CMD_TO_TYPE(enDBCmd_Get_XTBossDie,			SDB_Get_XTBossDie_Req) \
	CMD_TO_TYPE(enDBCmd_Change_XTBossDie,		SDB_Change_XTBossDie_Req) \
	CMD_TO_TYPE(enDBCmd_Update_XTDamage,		SDB_Update_XTDamage_Req) \
	CMD_TO_TYPE(enDBCmd_Get_XTDamage,			SDB_Get_XTDamage_Req) \
	CMD_TO_TYPE(enDBCmd_Change_XTLastDamage,	SDB_Change_XTLastDamage_Req) \
	CMD_TO_TYPE(enDBCmd_Save_TalismanWorld,		SDB_Save_TalismanWorld_Req) \
	CMD_TO_TYPE(enDBCmd_Get_TalismanWorld,		SDB_Get_TalismanWorld_Req) \
	CMD_TO_TYPE(enDBCmd_Delete_TalismanWorld,	SDB_Delete_TalismanWorld_Req) \
	CMD_TO_TYPE(enDBCmd_Get_TalismanPart,		SDB_Get_TalismanPart_Req) \
	CMD_TO_TYPE(enDBCmd_Update_TalismanPart,	SDB_Update_TalismanPart_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SynWarKillHistory,	SDB_Get_SynWarKillHistory_Req) \
	CMD_TO_TYPE(enDBCmd_Update_SynWarKillHistory,SDB_Update_SynWarKillHistory_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SynWarInfo,			SDB_Get_SynWarInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Update_SynWarInfo,		SDB_Update_SynWarInfo_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SynWarDeclare,		SDB_Get_SynWarDeclare_Req) \
	CMD_TO_TYPE(enDBCmd_Insert_SynWarDeclare,	SDB_Insert_SynWarDeclare_Req) \
	CMD_TO_TYPE(enDBCmd_Delete_SynWarDeclare,	SDB_Delete_SynWarDeclare_Req) \
	CMD_TO_TYPE(enDBCmd_Get_LastRecordTime,		SDB_Get_LastRecordTime_Req) \
	CMD_TO_TYPE(enDBCmd_Get_SynPart,			SDB_Get_SynPart_Req) \
	CMD_TO_TYPE(enDBCmd_Update_SynPart,			SDB_Update_SynPart_Req) \
	CMD_TO_TYPE(enDBCmd_Delete_SynWarInfo,		SDB_Delete_SynWarInfo_Req)\
	CMD_TO_TYPE(enDBCmd_Update_DuoBaoRank,		SDB_Update_DuoBaoRank) \
	CMD_TO_TYPE(enDBCmd_Get_DuoBaoRank,			SDB_Get_DuoBaoRank) \
	CMD_TO_TYPE(enDBCmd_Get_MyDuoBaoRank,		SDB_Get_MyDuoBaoRank) \
	CMD_TO_TYPE(enDBCmd_Insert_DuoBaoRank,		SDB_Insert_DuoBaoRank) \
	CMD_TO_TYPE(enDBCmd_Get_MyChallengeRecord,			SDB_Get_MyChallengeRecord) \
	CMD_TO_TYPE(enDBCmd_Get_ChallengeRecordBuf,			SDB_Get_ChallengeRecordBuf) \
	CMD_TO_TYPE(enDBCmd_Insert_MyChallengeRecord,		SDB_Insert_MyChallengeRecord) \
	CMD_TO_TYPE(enDBCmd_Get_ChallengeLevelTotalNum,		SDB_Get_ChallengeLevelTotalNum)\
	CMD_TO_TYPE(enDBCmd_Update_PreSynWarInfo,			SDB_Update_PreSynWarInfo) \
	CMD_TO_TYPE(enDBCmd_GetRechargeForward,				SDB_GetRechargeForward) \
	CMD_TO_TYPE(enDBCmd_GetRechargeForwardCnfg,			SDB_GetRechargeForwardCnfg) \
	CMD_TO_TYPE(enDBCmd_GetTotalRecharge,				SDB_GetTotalRechargeReq) \
	CMD_TO_TYPE(enDBCmd_GetMonsterCnfg,					SDB_GetMonsterCnfgReq) \
	CMD_TO_TYPE(enDBCmd_GetActivityCnfg,				SDB_GetActivityCnfgReq) \
	CMD_TO_TYPE(enDBCmd_DeleteTask,						SDB_DeleteTaskReq) \
	CMD_TO_TYPE(enDBCmd_UpdateTask,						SDB_UpdateTaskReq) \
	CMD_TO_TYPE(enDBCmd_GetMultipExpInfo,				SDB_GetMultipExpInfo_Req) \
	CMD_TO_TYPE(enDBCmd_UpdateOccupationInfo,			SDB_Update_Occupation_Info) \
	CMD_TO_TYPE(enDBCmd_GetOccupationInfo,				SDB_Get_OccupationInfo) \
	CMD_TO_TYPE(enDBCmd_UpdateOccupationGhostSoul,		SDB_Update_Occupation_GhostSoul) \
	CMD_TO_TYPE(enDBCmd_Insert_PointMsg,				SDB_Insert_PointMsg_Req) \
	CMD_TO_TYPE(enDBCmd_Update_LastOnlineTime,			SDB_Update_LastOnlineTime) \
	CMD_TO_TYPE(enDBCmd_Get_FirstRechargeCnfg,			SDB_Get_FirstRechargeCnfg) \
	CMD_TO_TYPE(enDBCmd_Get_IsGetFirstRechargeForward,	SDB_Get_IsGetFirstRechargeForward)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//数据库操作结果码
enum enDBRetCode 
{
	enDBRetCode_OK = 0, //成功
	enDBRetCode_ErrorCmd = -1, //错误的命令字
	enDBRetCode_ErrorParam = -2,  //参数有误
	enDBRetCode_DBError = -3,  //数据库调用出错
};

//DB通讯请求公用包头
struct DBReqPacketHeader : public AppPacketHeader
{
	DBReqPacketHeader()
	{
		MsgCategory = enMsgCategory_DB;
		m_UserID = 0;
		m_RequestSN = 0;
		this->m_length = sizeof(*this);
	}
	TUserID          m_UserID;
	UINT32           m_RequestSN;   //请求序列号
};


//DB通讯应答公用包头
struct DBRspPacketHeader : public AppPacketHeader
{
	DBRspPacketHeader()
	{
		MsgCategory = enMsgCategory_DB;
		m_UserID = 0;
		m_RequestSN = 0;
		m_RetCode = enDBRetCode_OK;
		this->m_length = sizeof(*this);
	}

	TUserID          m_UserID;
	UINT32           m_RequestSN;   //请求序列号
	INT32            m_RetCode; //调用结果
};

//存储过程输出参数
struct DB_OutParam
{
	FIELD_BEGIN();

	FIELD(INT32,retCode);  //结果码

	FIELD_END();
};







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum enChanel VC_PACKED_ONE
{
   enChanel_XunYou         = 0,   //讯游
   enChanel_XinLang        = 1,   //新浪

} PACKED_ONE;


//获取用户信息
struct DB_User_Info_Record
{
	FIELD_BEGIN();
	FIELD(INT32  ,retCode);
	FIELD(UINT32,UserID);
	FIELD(UINT8, IdentityStatus);
	FIELD(INT32,OnlineTime);
	FIELD_END();
};

//我的服务器
struct DB_My_ServerInfo
{
	FIELD_BEGIN();
	   FIELD(UINT16      ,m_ServerID);
	FIELD_END();
};

//获取用户信息
struct DB_Get_User_Info_Req : private DBReqPacketHeader
{
	DB_Get_User_Info_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetUserInfo");
	FIELD(TInt8Array<USER_NAME_LEN>, szUserName);	
    FIELD(TInt8Array<SHA256_LEN>, szPassword);		
	FIELD(TInt8Array<IP_LEN>  , szIp);  //用户IP地址

	FIELD(UINT8               , Chanel); //渠道
	FIELD(TInt8Array<ACCESS_SECRET_LEN>  , szAccessSecret);  //第三方平台的secret

	FIELD_END();


	typedef DB_User_Info_Record     TResultSet;  //结果集
	typedef DB_My_ServerInfo        TResultSet2;  //结果集
};

struct STestVarBinary
	{
		BIND_PROC_BEGIN("P_CDB_GetUserInfo2");
	    FIELD(VarBinary, szUserName);

    	FIELD_END();
	};
//OutParam字段的取字范围
enum enUserInfoRetCode
{
	enUserInfoRetCode_OK = 1,  //成功
	enUserInfoRetCode_ErrorDB = -1, //数据库错误
	enUserInfoRetCode_NoUser = -2, //用户不存在
	enUserInfoRetCode_ErrorPwd = -3, //密码错误
	enUserInfoRetCode_ExistUser = -4, //用户已存在
	enUserInfoRetCode_SealNo    = -5,  //该帐号已被封号
	enUserInfoRetCode_Define    = -6,  //服务器维护中
	enUserInfoRetCode_Max,         //未知
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//用户注册
struct DB_Insert_User_Info_Req : private DBReqPacketHeader
{
	DB_Insert_User_Info_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_InsertUserInfo");
	FIELD(TInt8Array<USER_NAME_LEN>, szUserName);	
	FIELD(TInt8Array<SHA256_LEN>, szPassword);	
	FIELD(TInt8Array<IP_LEN>  , szIp);  //用户IP地址
	FIELD(UINT8               , Chanel); //渠道
	FIELD_END();

	typedef DB_User_Info_Record  TResultSet;  //结果集
	typedef DB_My_ServerInfo  TResultSet2;  //结果集
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//关键词返回
struct DB_GetKeyword_Info_Rsp
{
	FIELD_BEGIN();
	FIELD(TInt8Array<20>        ,m_Keyword);
	FIELD_END();
};

//获得关键词
struct DB_Get_Keyword_Info_Req : private DBReqPacketHeader
{
	DB_Get_Keyword_Info_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetKeyWordInfo");

	FIELD_END();

	typedef DB_GetKeyword_Info_Rsp          TResultSet;  //结果集
	typedef ResultSetNull                   TResultSet2;  //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//获得系统信息
struct DB_GetServerInfo_Rsp
{
	DB_GetServerInfo_Rsp()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(TInt8Array<200>        ,m_szPublicNotic);
	FIELD(TInt8Array<1024>       ,m_szUpdateDesc);
	FIELD(TInt8Array<20>        ,m_szLastVersion);
	FIELD(TInt8Array<20>        ,m_szMinVersion);
	FIELD(TInt8Array<300>       ,m_szInstallPackageUrl);
	FIELD(TInt8Array<20>        ,m_szServiceTel); //客服电话
	FIELD(TInt8Array<50>        ,m_szServiceEmail); //客服邮箱
	FIELD(TInt8Array<20>        ,m_szServiceQQ);     //客服QQ

    FIELD(TInt8Array<30>        ,m_szThridPartIp);  //第三方平台IP
	FIELD(UINT16                ,m_ThridPartPort);  //第三方平台Port 
	FIELD(TInt8Array<33>        ,m_szThridPartAppKey);  //第三方平台分配的AppKey
	FIELD(TInt8Array<33>        ,m_szThridPartSecretKey);  //第三方平台SecretKey
	FIELD(TInt8Array<256>        ,m_szThridPartLoginUrl);  //第三方平台登陆接口
	FIELD(TInt8Array<256>        ,m_szThridPartRegisterUrl);  //第三方平台注册接口
	
	FIELD_END();
};

//安装包下载地址
struct DB_InstallPackageUrl
{
	DB_InstallPackageUrl()
	{
        MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(UINT32                ,m_PlatformID); //平台
	FIELD(TInt8Array<300>       ,m_szInstallPackageUrl); //安装包路径
	FIELD_END();
};


struct DB_Get_Server_Info_Req: private DBReqPacketHeader
{
	DB_Get_Server_Info_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetServerInfo");
	 FIELD(UINT16      ,m_ServerID);
	 FIELD(UINT16      ,m_PlatformID);
	FIELD_END();

	typedef DB_GetServerInfo_Rsp          TResultSet;  //结果集
	typedef DB_InstallPackageUrl           TResultSet2;  //结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//服务器信息
struct DB_Server_Info
{
	DB_Server_Info()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(TInt8Array<20>        ,m_szLastVersion);
	FIELD(TInt8Array<20>        ,m_szMinVersion);
	FIELD(TInt8Array<300>       ,m_szInstallPackageUrl);
	FIELD(TInt8Array<20>        ,m_szServerName);
	FIELD(TInt8Array<20>        ,m_szServiceTel); //客服电话
	FIELD(TInt8Array<50>        ,m_szServiceEmail); //客服邮箱
	FIELD(TInt8Array<20>        ,m_szServiceQQ);     //客服QQ
	FIELD(UINT32				,m_OpenServiceTime); //服务器开启时间(即第一次开服务器时间)
	FIELD(UINT8                 ,m_IsNewServer);     //是否是新服
	FIELD(float                 ,m_fMultipExp);      //多倍经验
	FIELD(UINT8                 ,m_MinMultipExpLv); //享受多倍经验最低玩家等级
	FIELD(UINT8                 ,m_MaxMultipExpLv); //享受多倍经验最高玩家等级
	FIELD(UINT32                ,m_MultipExpBeginTime);
	FIELD(UINT32                ,m_MultipExpEndTime);
	FIELD_END();
};

//服务器启动
struct DB_GameServerStart_Req  : private DBReqPacketHeader
{
	DB_GameServerStart_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_ServerStart");
    FIELD(UINT16     ,m_ServerID); //服务器ID
	FIELD_END();

	typedef DB_Server_Info          TResultSet;  //结果集
	typedef ResultSetNull          TResultSet2;  //结果集

};

//服务器正常关闭
struct DB_GameServerStop_Req  : private DBReqPacketHeader
{
	DB_GameServerStop_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_ServerStop");
    FIELD(UINT16     ,m_ServerID); //服务器ID
	FIELD_END();

	typedef ResultSetNull          TResultSet;  //结果集
	typedef ResultSetNull          TResultSet2;  //结果集

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DB_GetPlatformInfo_Rsp
{
	FIELD_BEGIN();
	FIELD(TInt8Array<30>        ,m_szThridPartIp);  //第三方平台IP
	FIELD(UINT16                ,m_ThridPartPort);  //第三方平台Port
	FIELD(TInt8Array<33>        ,m_szThridPartAppKey);  //第三方平台分配的AppKey
	FIELD(TInt8Array<33>        ,m_szThridPartSecretKey);  //第三方平台SecretKey
	FIELD(TInt8Array<33>        ,m_szMerchantKey);       //游戏厂商key
	FIELD(TInt8Array<33>        ,m_szDesKey);       //DES_KEY
	FIELD(INT32                ,m_MerchantID);       //游戏厂商ID
	FIELD(INT32                ,m_GameID);       //游戏ID
	FIELD(INT32                ,m_ServerID);       //游戏服务器ID
	FIELD_END();
};

//接口URL
struct DB_PlatformInterface
{
	FIELD_BEGIN();
	FIELD(UINT16                 ,m_InterfaceID);
	FIELD(TInt8Array<256>        ,m_szInterfaceUrl);       //游戏厂商key

	FIELD_END();
};

//获取平台信息
struct DB_GetPlatformInfo_Req : private DBReqPacketHeader
{
	DB_GetPlatformInfo_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetPlatformInfo");
    FIELD(UINT16      ,m_PlatformID);
	FIELD_END();

	typedef DB_GetPlatformInfo_Rsp    TResultSet;  //结果集
	typedef DB_PlatformInterface             TResultSet2;  //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//用户进游戏了
struct DB_UserEnterGame_Req  : private DBReqPacketHeader
{
	DB_UserEnterGame_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_UserEnterGame");
    FIELD(UINT16     ,m_ServerID); //服务器ID
	FIELD(UINT32     ,m_UserID); //用户ID
	FIELD(INT32      ,m_GodStone); //仙石
	FIELD(TInt8Array<IP_LEN>  , szIp);  //用户IP地址
	FIELD(UINT8      ,m_bFirstEnter); //是否初次进入
	FIELD(TInt8Array<THING_NAME_LEN>  , m_szActorName);  //角色名
	FIELD_END();

	typedef ResultSetNull          TResultSet;  //结果集
	typedef ResultSetNull          TResultSet2;  //结果集

};


//用户退出游戏了
struct DB_UserExitGame_Req  : private DBReqPacketHeader
{
	DB_UserExitGame_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_UserExitGame");
    FIELD(UINT16     ,m_ServerID); //服务器ID
	FIELD(UINT32     ,m_UserID); //用户ID
	FIELD(INT32      ,m_GodStone); //仙石
	FIELD(UINT8		 ,m_Level);		//等级
	FIELD_END();

	typedef ResultSetNull          TResultSet;  //结果集
	typedef ResultSetNull          TResultSet2;  //结果集

};


/////////////////////////////////////////////////////////////////
//礼券对应的物品信息
struct DB_TicketInfo
{
	FIELD_BEGIN();
	FIELD(TInt8Array<MAX_TICKET_NO_LEN>,        m_szTicketNo);
	FIELD(INT32,		m_MoneyNum);						//仙石数量
	FIELD(INT32,		m_StoneNum);						//灵石数量
	FIELD(INT32,		m_TicketNum);						//礼券数量
	FIELD(INT32,		m_PolyNimbusNum);					//聚灵气数量
	FIELD(UINT16                                ,m_GoodsID);     
	FIELD(UINT16                                ,m_Number); 
	FIELD(TInt8Array<DESCRIPT_LEN_50>,        m_szMailTitle);
	FIELD(TInt8Array<DESCRIPT_LEN_300>,        m_szMailContent);
	FIELD_END();
};

 //兑换礼券
//	enDBCmd_ChangeTicket, 
struct DB_ChangeTicket_Req  : private DBReqPacketHeader
{
	DB_ChangeTicket_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_ChangeTicket");
	FIELD(TInt8Array<MAX_TICKET_NO_LEN>,        m_szTicketNo);
    FIELD(UINT16     ,m_ServerID); //服务器ID
	FIELD(UINT32     ,m_UserID); //用户ID
	FIELD_END();

	typedef DB_OutParam            TResultSet;  //结果集
	typedef DB_TicketInfo          TResultSet2;  //结果集
};



 //兑换礼券完成
//	enDBCmd_ChangeTicketFinish,    
struct DB_ChangeTicketFinish_Req  : private DBReqPacketHeader
{
	DB_ChangeTicketFinish_Req()
	{
		this->m_length = SIZE_OF(*this);
	}

	BIND_PROC_BEGIN("P_CDB_ChangeTicketFinish");
	FIELD(TInt8Array<MAX_TICKET_NO_LEN>,        m_szTicketNo);
   
	FIELD_END();

	typedef ResultSetNull             TResultSet;  //结果集
	typedef ResultSetNull             TResultSet2;  //结果集
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//获得建筑数据
struct SDB_Get_BuildingData_Rsp
{
	SDB_Get_BuildingData_Rsp()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(UINT64,		m_Uid);							//建筑标识
	FIELD(UINT8,		m_BuildingType);				//建筑类型	
	FIELD(UINT32,		m_LastTakeResTime);				//上次领取资源的时间	
	FIELD(UINT64,		m_uidOwner);					//建筑所属的主人	
	FIELD(INT32,		m_OwnerNoTakeResNum);			//主人未领取的资源
	FIELD(UINT32,		m_LastCollectTime);		        //上代收取时间
	FIELD(UINT32,		m_TotalCollectResOfDay);	        //当天总共代收了的资源数
	FIELD(UINT8,		m_bAutoTakeRes);				//是否开启自动收取灵石

	FIELD_END();
};

//获取资源产出型建筑数据
struct SDB_GetBuildingData_Req : private DBReqPacketHeader
{
	SDB_GetBuildingData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetBuildingInfo");
	FIELD(UINT64,	Uid_User);					//玩家UID值

	FIELD_END();

	typedef DB_OutParam   		TResultSet;  //结果集
	typedef SDB_Get_BuildingData_Rsp	TResultSet2;  //结果集
};


//更新资源产出型建筑数据
struct SDB_Update_BuildingData_Req : private DBReqPacketHeader
{
	SDB_Update_BuildingData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_BuildingInfo");
	FIELD(UINT64,		m_Uid);							//建筑标识
	FIELD(UINT8,		m_BuildingType);				//建筑类型		
	FIELD(UINT32,		m_LastTakeResTime);				//上次领取资源的时间
	FIELD(UINT64,		m_uidOwner);					//建筑所属的主人
	FIELD(INT32,		m_OwnerNoTakeResNum);			//主人未领取的资源
	FIELD(UINT32,		m_LastCollectTime);		        //上次窃取时间
	FIELD(UINT32,		m_TotalCollectResOfDay);	    //当天总共窃取了的资源数
	FIELD(UINT8,		m_bAutoTakeRes);				//是否开启自动收取灵石


	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};



struct SDB_BuildingRecordNum
{
	FIELD_BEGIN();

	FIELD(INT32,	m_RecordNum);

	FIELD_END();
};

struct BuildingRecordData
{
	FIELD_BEGIN();

	FIELD(UINT8	,	m_BuildingType);
	FIELD(UINT32,		m_HappenTime);
	FIELD(TInt8Array<THING_NAME_LEN>,        m_szTargetName);
	FIELD(TInt8Array<THING_NAME_LEN>,		m_szHandleName);
	FIELD(TInt8Array<DESCRIPT_LEN_50>,        m_szContext);

	FIELD_END();
};

//获取资源产出型记录
struct SDB_Get_BuildingRecord_Req : private DBReqPacketHeader
{
	SDB_Get_BuildingRecord_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetBuildingRecordInfo");

	FIELD(UINT64,	m_UidUser);				//玩家的UID值

	FIELD_END();

	typedef  DB_OutParam				TResultSet;  //结果集
	typedef  BuildingRecordData			TResultSet2;  //结果集
};


//更新资源产出型记录
struct SDB_Update_BuildingRecord_Req : private DBReqPacketHeader
{
	SDB_Update_BuildingRecord_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_UpdateBuildRecord");
	FIELD(UINT64,		m_UidUser);						//玩家
//	FIELD(UINT8,		m_BuildingType);				//建筑类型
	FIELD(UINT32,		m_RecordNum);					//记录数量
	FIELD(VarBinary,	m_VarBinary);					//
    FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};

//删除资源建筑记录
struct SDB_Delete_BuildingRecord_Req : private DBReqPacketHeader
{
	SDB_Delete_BuildingRecord_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_ResBuildRecord");
	FIELD(UINT64,		m_UidUser);
	FIELD(UINT8,		m_BuildingType);
	FIELD(UINT32,		m_HappenTime);

	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};

//保存资源建筑记录
struct SDB_Insert_BuildingRecord_Req : private DBReqPacketHeader
{
	SDB_Insert_BuildingRecord_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_ResBuildRecord");
	FIELD(UINT64,		m_UidUser);						//玩家
	FIELD(UINT8	,		m_BuildingType);
	FIELD(UINT32,		m_HappenTime);
	FIELD(TInt8Array<THING_NAME_LEN>,			m_szTargetName);
	FIELD(TInt8Array<THING_NAME_LEN>,			m_szHandleName);
	FIELD(TInt8Array<DESCRIPT_LEN_50>,			m_szContext);

	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//伏魔洞数据
struct SDBFuMoDongData
{
	SDBFuMoDongData()
	{
		memset(this,0,sizeof(*this));
	}
	UINT32  m_EndOnHookTime;  //到期挂机时间，为零表示不在挂机状态
	INT32   m_AccelNumOfDay;          //当天加速次数
	UINT32  m_LastAccelTime;      //最后一次加速时间
	INT32   m_Level;                //伏魔洞的层次
	UINT32  m_LastGiveExpTime;   //最后一次给予挂机经验时间
};

//获得伏魔洞的数据
struct SDB_Get_FuMoDongData_Rsp
{
	SDB_Get_FuMoDongData_Rsp() : m_UidOwner(UID().ToUint64()),m_EndOnHookTime(0),m_AccelNumOfDay(0),m_LastAccelTime(0),m_Level(0),m_LastGiveExpTime(0),m_GiveExp(0)
	{
	}

	FIELD_BEGIN();
	FIELD(UINT64,	m_UidOwner);						//伏魔洞的主人UID值
	FIELD(UINT32,	m_EndOnHookTime);					//到期挂机时间，为零表示不在挂机状态
	FIELD(UINT32,	m_AccelNumOfDay);					//当天加速次数
	FIELD(UINT32,	m_LastAccelTime);					//最后一次加速时间
	FIELD(UINT32,	m_Level);							//伏魔洞的层次
	FIELD(UINT32,	m_LastGiveExpTime);					//最后一次给予挂机经验时间
	FIELD(UINT32,	m_GiveExp);					        //每一次挂机给予经验

	FIELD_END();
};

//获得伏魔洞的数据
struct SDB_Get_FuMoDongData_Req : private DBReqPacketHeader
{
	SDB_Get_FuMoDongData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetFuMoDongInfo");
	FIELD(UINT64,	Uid_User);

	typedef DB_OutParam					TResultSet;   //结果集
	typedef SDB_Get_FuMoDongData_Rsp	TResultSet2;  //结果集

	FIELD_END();
};
//插入伏魔洞数据
struct SDB_Insert_FuMoDongData_Req : private DBReqPacketHeader
{
	SDB_Insert_FuMoDongData_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertFuMoDongInfo");
	FIELD(UINT64,	Uid_User);						  //伏魔洞的主人UID值

	FIELD_END();

	typedef DB_OutParam					TResultSet;   //结果集
	typedef SDB_Get_FuMoDongData_Rsp	TResultSet2;  //结果集
};

//更新伏魔洞数据
struct SDB_Update_FuMoDongData_Req : private DBReqPacketHeader
{
	SDB_Update_FuMoDongData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_UpdateFuMoDongInfo");
	FIELD(UINT64,	Uid_User);							//伏魔洞的主人UID值
	FIELD(UINT32,	m_EndOnHookTime);					//到期挂机时间，为零表示不在挂机状态
	FIELD(UINT32,	m_AccelNumOfDay);					//当天加速次数
	FIELD(UINT32,	m_LastAccelTime);					//最后一次加速时间
	FIELD(UINT32,	m_Level);							//伏魔洞的层次
	FIELD(UINT32,	m_LastGiveExpTime);					//最后一次给予挂机经验时间
	FIELD(UINT32,	m_GiveExp);					        //每一次挂机给予经验

	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//练功堂数据
struct SDBTrainingHallData
{
	SDBTrainingHallData()
	{
		memset(this,0,sizeof(*this));
	}
	INT16  m_RemainNum;			//剩余练功次数
	UINT32 m_BeginTime;			//开始练功时间
	UINT32 m_LastFinishTime;	//最后一次完成练功时间
};

//获取练功堂数据
struct SDB_Get_TrainingHallData_Rsp
{
	SDB_Get_TrainingHallData_Rsp()
	{
		Uid_User = UID().ToUint64();
		OnHookNum = 0;
		RemainTime = 0;
		LastFinishTime = 0;
		TrainStatus = 0;
		VipFinishTrainNum = 0;
		LastVipFinishTrainTime = 0;
		TrainingTimeLong = 0;
		BeginXiWuTime = 0;
		uidActorXiWu = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT64,	Uid_User);				//练功堂的主人UID值
	FIELD(UINT16,	OnHookNum);				//已挂机次数
	FIELD(UINT32,	RemainTime);			//剩余时间
	FIELD(UINT32,	LastFinishTime);		//最后一次完成练功时间
	FIELD(UINT8,	TrainStatus);
	FIELD(UINT8,	VipFinishTrainNum);		//今天使用VIP立即完成次数
	FIELD(UINT32,	LastVipFinishTrainTime);//最后使用VIP立即完成时间
	FIELD(UINT32,	TrainingTimeLong);	//可练功总时长
	FIELD(UINT32,	BeginXiWuTime);		//开始习武的时间
	FIELD(UINT64,	uidActorXiWu);		//参加习武的角色

	FIELD_END();
};

//获取练功堂数据
struct SDB_Get_TrainingHallData_Req : private DBReqPacketHeader
{
	SDB_Get_TrainingHallData_Req()
	{
		Uid_User = UID().ToUint64();
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetTrainingHallInfo");
	FIELD(UINT64,	Uid_User);

	FIELD_END();

	typedef DB_OutParam						TResultSet;   //结果集
	typedef SDB_Get_TrainingHallData_Rsp	TResultSet2;  //结果集
};

//插入练功堂数据
struct SDB_Insert_TrainingHallData_Req : private DBReqPacketHeader
{
	SDB_Insert_TrainingHallData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_InsertTrainingHallInfo");
	FIELD(UINT64,	Uid_User);	//练功堂的主人UID值

	FIELD_END();

	typedef DB_OutParam						TResultSet;   //结果集
	typedef SDB_Get_TrainingHallData_Rsp	TResultSet2;  //结果集
};

//更新练功堂数据
struct SDB_Update_TrainingHallData_Req : private DBReqPacketHeader
{
	SDB_Update_TrainingHallData_Req() : OnHookNum(0), RemainTime(0), LastFinishTime(0)
	{
		Uid_User = UID().ToUint64();
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_UpdateTrainingHallInfo");
	FIELD(UINT64,	Uid_User);		 //练功堂的主人UID值
	FIELD(UINT16,	OnHookNum);		 //已挂机次数
	FIELD(UINT32,	RemainTime);	 //剩余时间
	FIELD(UINT32,	LastFinishTime); //最后一次完成练功时间
	FIELD(UINT8,	TrainStatus);	 //练功状态
	FIELD(UINT8,	VipFinishTrainNum);		//今天使用VIP立即完成次数
	FIELD(UINT32,	LastVipFinishTrainTime);//最后使用VIP立即完成时间
	FIELD(UINT32,	TrainingTimeLong);	//可练功总时长
	FIELD(UINT32,	BeginXiWuTime);		//开始习武的时间
	FIELD(UINT64,	uidActorXiWu);		//参加习武的角色
	
	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 装备特有的，需要存盘的属性
struct SDBEquipProp
{
	UINT8       m_Star;      //强化星级
	TGoodsID    m_GemGoodsID[MAX_INLAY_NUM];  //镶嵌的宝石
};

//仙剑特有的，需要存盘的数据
struct SDBGodSwordProp
{
	UINT32 m_Nimbus;     //灵气
	UINT8  m_SwordLevel; //剑气级别
	UINT8  m_SecretLevel;    //剑诀等级
	
	UINT8  m_FuBenLevel;     //所进入的副本层次
	UINT8  m_KillMonsterNum; //杀怪数量
	UINT32 m_LastEnterFuBenTime;  //最后一次进副本时间
};

//法宝有的，需要存盘的数据
struct SDBTalismanProp
{
	UINT16    m_QualityPoint;   //品质点
	TGoodsID  m_GestateGoodsID; //孕育的物品ID
	UINT32    m_BeginGestateTime;    //开始孕育时间
	//UINT32    m_LastEnterTime; //最后一次时入法宝世界时间
	TGoodsID  m_GhostGoodsID; //法宝上的灵件的物品ID
	UINT8	  m_GhostLevel;	//灵件的等级
	UINT8     m_ExtendData[1]; //扩展数据 
	UINT8     m_EnterNum;  //当天进入法宝世界的次数
	UINT8     m_MagicLevel :7 ; //法宝法术的等级
	UINT8     m_bUseSpawnProp :1; //是否使用了孕育道具
	
};

//灵件有的，需要存盘的数据
struct Ghost
{
	UINT8	m_GhostLevel;	//灵件星级
};

//普通物品有的属性
struct SDBCommonGoodsProp
{	
	
	//UINT8		enGoodsThreeType;
	union
	{
		Ghost		m_Ghost;		  //灵件有的属性
		UINT8       m_ExtendData[13]; //扩展数据 
	};
};

//物品
struct SDBGoodsData
{
	SDBGoodsData()
	{
		m_uidGoods = UID();
		m_GoodsID = INVALID_GOODS_ID;
		m_Location = 0;
	}

	INT16       m_Location;  //在装备栏中的位置
	UID         m_uidGoods;  //UID标识物品
	TGoodsID    m_GoodsID;   //GoodsID
	UINT32      m_CreateTime; //创建时间
	UINT8       m_Number;     //数量
	bool        m_Binded;    //是否已绑定
	union
	{
		SDBCommonGoodsProp   m_CommonProp;   //普通物品属性
		SDBEquipProp         m_EquipProp;    //装备数据
		SDBGodSwordProp      m_GodSwordProp; //仙剑数据
		SDBTalismanProp      m_TalismanProp;  //法宝属性
	};

};

//物品（数据库更新时用）
struct SDBGoodsData_Update
{
	SDBGoodsData_Update()
	{
		m_uidGoods = UID().ToUint64();
		m_GoodsID = INVALID_GOODS_ID;
		m_Location = 0;
		m_CreateTime = 0;
		m_Number = 0;
		m_Binded = false;
	}

	INT16       m_Location;  //在装备栏中的位置
	UINT64      m_uidGoods;  //UID标识物品
	TGoodsID    m_GoodsID;   //GoodsID
	UINT32      m_CreateTime; //创建时间
	UINT8       m_Number;     //数量
	bool        m_Binded;    //是否已绑定
	union
	{
		SDBCommonGoodsProp   m_CommonProp;   //普通物品属性
		SDBEquipProp      m_EquipProp;    //装备数据
		SDBGodSwordProp   m_GodSwordProp; //仙剑数据
		SDBTalismanProp   m_TalismanProp;  //法宝属性
	};

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//背包数据
struct SDBPacketData
{
	SDBPacketData() : m_pData(0), m_nLen(0)
	{
	}

	char * m_pData;
	int	   m_nLen;
};

//背包数据
struct SDB_Get_PacketData_Rsp
{
	SDB_Get_PacketData_Rsp() : Uid_User(UID().ToUint64()), Capacity(0)
	{
	}

	FIELD_BEGIN();
	FIELD(UINT64,	Uid_User);	//玩家ID
	FIELD(INT16,	Capacity);	//容量

	FIELD_END();
};

//背包物品数据
struct SDB_Get_PacketGoodsData_Rsp
{
	FIELD_BEGIN();
	FIELD(INT16      , m_Location);  //在装备栏中的位置
	FIELD(UINT64,	uidGoods);
	FIELD(TGoodsID,	GoodsID);
	FIELD(UINT32,	CreateTime);
	FIELD(UINT8,	Number);
	FIELD(bool,		Binded);
	FIELD(TBinArray<sizeof(SDBTalismanProp)>, GoodsData);	//背包里面物品的信息

	FIELD_END();
};

//得到背包数据
struct SDB_Get_PacketData_Req : private DBReqPacketHeader
{
	SDB_Get_PacketData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_PacketInfo");
	FIELD(UINT64,		Uid_User);

	typedef SDB_Get_PacketData_Rsp		TResultSet;		//结果集
	typedef SDB_Get_PacketGoodsData_Rsp TResultSet2;	//结果集

	FIELD_END();
};

//插入背包
struct SDB_Insert_PacketData_Req : private DBReqPacketHeader
{
	SDB_Insert_PacketData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_PacketInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(INT16,		Capacity);	//背包容量

	FIELD_END();

	typedef SDB_Get_PacketData_Rsp		TResultSet;		//结果集
	typedef SDB_Get_PacketGoodsData_Rsp TResultSet2;	//结果集

};
//更新背包数据
struct SDB_Update_PacketData_Req : private DBReqPacketHeader
{
	SDB_Update_PacketData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_PacketInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(INT16,		Capacity);				//容量
	FIELD(INT16,		GoodsNum);				//物品数量
	FIELD(VarBinary,	VarBin);				//可变长

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//装备栏数据
struct SDBEquipPanel
{
    SDBEquipPanel()
	{
		m_GoodsNum = 0;
		for(int i=0; i<sizeof(m_GoodsData)/sizeof(m_GoodsData[0]);++i)
		{
			m_GoodsData[i].m_uidGoods = UID();
		}

		MEM_ZERO(m_szFacade);
	}

	INT32         m_GoodsNum;

	SDBGoodsData  m_GoodsData[MAX_EQUIP_NUM];
	char          m_szFacade[MAX_FACADE_STR_LEN];
};


//装备物品数据
struct SDB_Get_EquipGoodsData_Rsp
{
	FIELD_BEGIN();
    FIELD(INT16      , m_Location);  //在装备栏中的位置
	FIELD(UINT64,	uidGoods);
	FIELD(TGoodsID,	GoodsID);
	FIELD(UINT32,	CreateTime);
	FIELD(UINT8,	Number);
	FIELD(bool,		Binded);
	FIELD(TBinArray<sizeof(SDBTalismanProp)>, GoodsData);	//背包里面物品的信息
	FIELD_END();
};

//装备的物品数量
struct SDB_Get_EquipGoodsNum_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64, Uid_User);		
	FIELD(TInt8Array<MAX_FACADE_STR_LEN>,	m_szFacade); //外观数据
	FIELD_END();
};
//得到装备栏数据
struct SDB_Get_EquipPanelData_Req : private DBReqPacketHeader
{
	SDB_Get_EquipPanelData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_EquipPanelInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef SDB_Get_EquipGoodsNum_Rsp		TResultSet;		//结果集
	typedef SDB_Get_EquipGoodsData_Rsp		TResultSet2;	//结果集
};

//插入装备栏数据
struct SDB_Insert_EquipPanelData_Req : private DBReqPacketHeader
{
	SDB_Insert_EquipPanelData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_EquipPanelInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef SDB_Get_EquipGoodsNum_Rsp		TResultSet;		//结果集
	typedef SDB_Get_EquipGoodsData_Rsp		TResultSet2;	//结果集
};

//更新装备栏数据
struct SDB_Update_EquipPanelData_Req : private DBReqPacketHeader
{
	SDB_Update_EquipPanelData_Req(){
		this->m_length = sizeof(*this);
		m_szFacade[0]=0;
	}
	BIND_PROC_BEGIN("P_GDB_Update_EquipPanelInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(TInt8Array<MAX_FACADE_STR_LEN>,	m_szFacade); //外观数据

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//法术栏

//法术信息
struct SDBMagicInfo
{
	TMagicID  m_MagicID; //法术ID
	UINT8     m_Level;   //等级
};

struct SDBMagicPanelData
{
	SDBMagicPanelData()
	{
		m_MagicInfoNum = 0;
		memset(m_EquipMagic,0,sizeof(m_EquipMagic));
	}

	TMagicID     m_EquipMagic[MAX_EQUIP_MAGIC_NUM]; //法术栏已装备的法术,为零表示该栏为空
	UINT16       m_MagicInfoNum;  //已学会的法术数量
	SDBMagicInfo m_MagicInfo[MAX_STUDY_MAGIC_NUM]; //后面跟法术信息
};

//得到法术栏信息（个数）
struct SDB_GetMagicPanelNum_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,		Uid_User);
	FIELD(UINT8,		MagicNum);				//已学会法术数量
	FIELD_END();
};

//得到法术信息
struct SDB_GetMagicInfo_Rsp
{
	SDB_GetMagicInfo_Rsp() : MagicID(INVALID_MAGIC_ID), Level(0), Position(0)
	{
	}

	FIELD_BEGIN();
	FIELD(TMagicID,		MagicID);		//法术
	FIELD(UINT8,		Level);			//等级
	FIELD(UINT8,		Position);		//为0时，该法术未装备，不为0时,装备在的法术栏位置等于Position - 1
	FIELD_END();
};

//得到法术栏信息
struct SDB_Get_MagicPanelData_Req : private DBReqPacketHeader
{
	SDB_Get_MagicPanelData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_MagicPanelInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef  SDB_GetMagicPanelNum_Rsp	TResultSet;  //结果集
	typedef	 SDB_GetMagicInfo_Rsp		TResultSet2;  //结果集
};

//插入法术栏信息
struct SDB_Insert_MagicPanelData_Req : private DBReqPacketHeader
{
	SDB_Insert_MagicPanelData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_MagicPanelInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef  SDB_GetMagicPanelNum_Rsp		TResultSet;  //结果集
	typedef	 SDB_GetMagicInfo_Rsp			TResultSet2;  //结果集
};

//更新法术用的数据结构
struct SDB_Update_MagicPanelData
{
	SDB_Update_MagicPanelData() : m_MagicInfoNum(0)
	{
	}

	UINT16				 m_MagicInfoNum;  //已学会的法术数量
	SDB_GetMagicInfo_Rsp m_MagicInfo[MAX_STUDY_MAGIC_NUM]; //后面跟法术信息
};


//更新法术栏信息
struct SDB_Update_MagicPanelData_Req : private DBReqPacketHeader
{
	SDB_Update_MagicPanelData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_MagicPanelInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(TBinArray<sizeof(SDB_Update_MagicPanelData)>, MagicPanelData);

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//帮派技能
struct SDBSynMagicData
{
	SDBSynMagicData() : m_SynMagicID(0), m_SynMagicLevel(0){}
	TSynMagicID	 m_SynMagicID;
	UINT8		 m_SynMagicLevel;
};

struct SDBSynMagicPanelData
{
	SDBSynMagicPanelData() : m_SynMagicNum(0)
	{
	}

	UINT8			 m_SynMagicNum;			//已经学会的帮派技能数量
	SDBSynMagicData	 m_SynMagicData[MAX_SYNMAGICNUM];
};

struct SDBSynMagicNumData
{
	FIELD_BEGIN();
	FIELD(UINT64,	Uid_User);
	FIELD(UINT8,	m_SynMagicNum);		//已学帮派技能个数	
	FIELD_END();
};

struct SDB_Get_SynMagicData_Rsp
{
	FIELD_BEGIN();
	FIELD(TSynMagicID,  m_SynMagicID);
	FIELD(UINT8,		m_SynMagicLevel);
	FIELD_END();
};

//得到帮派技能信息
struct SDB_Get_SynMagicData_Req : private DBReqPacketHeader
{
	SDB_Get_SynMagicData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_SynMagicInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef SDBSynMagicNumData					TResultSet;  //结果集
	typedef SDB_Get_SynMagicData_Rsp			TResultSet2; //结果集
};

//插入帮派技能信息
struct SDB_Insert_SynMagicData_Req : private DBReqPacketHeader
{
	SDB_Insert_SynMagicData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_SynMagicInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef SDBSynMagicNumData					TResultSet;  //结果集
	typedef SDB_Get_SynMagicData_Rsp			TResultSet2; //结果集
};

//更新帮派技能信息
struct SDB_Update_SynMagicData_Req : private DBReqPacketHeader
{
	SDB_Update_SynMagicData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_SynMagicInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(TBinArray<sizeof(SDBSynMagicPanelData)>, SynMagicData);

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//剑冢
struct SDBGodSwordData
{
	SDBGodSwordData(){
		memset(this, 0, sizeof(0));
	}

	UINT64		m_LastFlushTime;			//最后一次刷新角色时间

	TGoodsID	m_idGodSword[FLUSH_NUM];	//记录玩家刷新仙剑ID
};

//获得剑冢数据
struct SDB_Get_GodSwordShopData_Rsp
{
	SDB_Get_GodSwordShopData_Rsp() : Uid_User(UID().ToUint64()), LastFlushTime(0),IdGodSword1(0),IdGodSword2(0),IdGodSword3(0),IdGodSword4(0)
	{
	}

	FIELD_BEGIN();
	FIELD(UINT64,		Uid_User);
	FIELD(UINT64,		LastFlushTime);
	FIELD(TGoodsID,		IdGodSword1);
	FIELD(TGoodsID,		IdGodSword2);
	FIELD(TGoodsID,		IdGodSword3);
	FIELD(TGoodsID,		IdGodSword4);
	FIELD_END();
};

//获得剑冢数据
struct SDB_Get_GodSwordShopData_Req : private DBReqPacketHeader
{
	SDB_Get_GodSwordShopData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_GodSwordShopInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 SDB_Get_GodSwordShopData_Rsp		TResultSet2;  //结果集
};

//插入剑冢数据
struct SDB_Insert_GodSwordShopData_Req : private DBReqPacketHeader
{
	SDB_Insert_GodSwordShopData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_GodSwordShopInfo");
	FIELD(UINT64,		Uid_User);


	FIELD_END();
	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 SDB_Get_GodSwordShopData_Rsp			 TResultSet2;  //结果集
};

//更新剑冢数据
struct SDB_Update_GodSwordShopData_Req : private DBReqPacketHeader
{
	SDB_Update_GodSwordShopData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_GodSwordShopInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(UINT64,		LastFlushTime);
	FIELD(TGoodsID,		IdGodSword1);
	FIELD(TGoodsID,		IdGodSword2);
	FIELD(TGoodsID,		IdGodSword3);
	FIELD(TGoodsID,		IdGodSword4);

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//修炼数据
struct STwoXiuLianData
{
	STwoXiuLianData()
	{
		
		m_uidFriend = UID();
		m_uidFriendActor = UID();
		m_szFriendName[0]=0;
		m_AskSeq = 0;
		m_Hours = 0;
		m_MagicID = INVALID_MAGIC_ID;
		m_Mode = enXiuLianMode_Max;
		m_EndTime = 0;
		m_XiuLianState = enXiuLianState_Non;
		m_uidSource = UID();
		m_XiuLianType = enXiuLianType_Non;
		m_uidSourceActor = UID();
		m_szSourceName[0]=0;
		m_bUpdate = false;
		m_FriendTotalNimbus = 0;
		m_SourceNotTakeNimbus = 0;
		m_FriendNotTakeNimbus = 0;
		m_bStudyMagic = false;
		m_UnloadTime = 0;
		m_FriendLastGetNimbusTime = 0;
		m_SourceNimbusSpeed	= 0;
		m_FriendNimbusSpeed	= 0;
		m_SourceLayer		= 0;
		m_FriendLayer		= 0;
	}

	enXiuLianType    m_XiuLianType; //修炼类型
	enXiuLianState   m_XiuLianState; //状态
	UINT8            m_Hours;       //请求修练小时数
	enXiuLianMode    m_Mode;    //方式
	UINT32           m_lastGetNimbusTime;  //自己上次获得灵气时间
	INT32            m_TotalNimbus; //自己总共获得的灵气
	UINT32           m_EndTime;    //修炼结束时间
	UINT32           m_AskSeq;   //请求序列号	
	UID              m_uidSource; //发起者
	UID              m_uidSourceActor; //发起者在修炼的角色
	char             m_szSourceName[THING_NAME_LEN]; //发起者名称	
	UID              m_uidFriend;    //好友
	UID              m_uidFriendActor; //好友角色
	char             m_szFriendName[THING_NAME_LEN]; //好友名称	
	TMagicID         m_MagicID; //法术	
	UINT32           m_AskTime;    //发起请求时间

	INT32            m_FriendTotalNimbus;  //好友总共获得的灵气
	INT32            m_SourceNotTakeNimbus; //发起方未领取灵气
	INT32            m_FriendNotTakeNimbus; //好友未领取灵气

	bool             m_bStudyMagic;  //是否有法术需要学完

	bool             m_bUpdate;   //是否有更新

	UINT32           m_UnloadTime;  //卸载时间

	UINT32			 m_FriendLastGetNimbusTime;	//好友上次获得灵气时间

	INT32			 m_SourceNimbusSpeed;	//我的灵气速率
	INT32			 m_FriendNimbusSpeed;	//好友的灵气速率
	INT32			 m_SourceLayer;			//我的境界
	INT32			 m_FriendLayer;			//好友的境界

};

struct AloneXLData
{
	AloneXLData() : m_ActorUID(UID()), m_GetNimbus(0) {}

	UID		m_ActorUID;		//角色UID
	INT32	m_GetNimbus;	//目前获得的灵气
};

struct SDBXiuLianData
{
	SDBXiuLianData()
	{
		m_AloneXiuLianState = enXiuLianState_Non;
		m_Hours = 0;
		m_lastGetNimbusTime = 0;
		m_EndTime = 0;
		m_GetGodSwordNimbus = 0;

		m_TwoXiuLianState = enXiuLianState_Non;
		m_uidActor  = UID();
		m_uidFriend = UID();
		m_TwoXiuLianHours = 0;
		m_lastGetNimbusTime = 0;
		m_TwoTotalNimbus = 0;
		m_TwoEndTime = 0;
		m_FriendFacade = 0;

		m_MagicXLSeq = 0;

		memset(m_FriendName, 0, sizeof(m_FriendName));
		
	}
	//单修数据
	enXiuLianState   m_AloneXiuLianState;				//单修状态
	AloneXLData      m_AloneXLData[MAX_ALONE_XL_NUM];	//在单修的角色数据
	UINT8            m_Hours;							//请求单修修练小时数
	UINT32           m_lastGetNimbusTime;				//单修上次获得灵气时间
	UINT32           m_EndTime;							//单修修炼结束时间
	UINT32			 m_GetGodSwordNimbus;				//单修获得的仙剑灵气

	//双修数据
	enXiuLianState	 m_TwoXiuLianState;					//双修状态
	UID				 m_uidActor;						//在双修的角色
	UID				 m_uidFriend;						//一起双修的好友UID,如果是双修的话
	UINT8            m_TwoXiuLianHours;					//请求双修修练小时数
	UINT32           m_TwoLastGetNimbusTime;			//双修上次获得灵气时间
	INT32            m_TwoTotalNimbus;					//双修自己总共获得的灵气
	UINT32           m_TwoEndTime;						//双修修炼结束时间
	UINT32			 m_FriendFacade;					//好友外观
	char			 m_FriendName[THING_NAME_LEN];		//好友名字
	//
	UINT32           m_MagicXLSeq;                      //法术修练序列号	
};


//获得修炼数据
struct SDB_Get_XiuLianData_Rsp
{
	SDB_Get_XiuLianData_Rsp(){
		memset( this, 0, sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(UINT64,   Uid_User);							//主人的UID
	FIELD(UINT8,	m_AloneXiuLianState);				//单修状态
	FIELD(UINT8,	m_AloneHours);						//单修请求小时数
	FIELD(UINT32,	m_AloneLastGetNimbusTime);			//单修上次获得灵气时间
	FIELD(UINT32,	m_AloneEndTime);					//单修结束时间
	FIELD(UINT64,   m_AloneXLActor);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor2);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor3);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor4);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor5);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor6);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor7);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor8);					//在单修的角色
	FIELD(UINT32,	m_GetGodSwordNimbus);				//单修获得的仙剑灵气
	FIELD(INT32,	m_AloneXLGetNimbus);				//单修的角色1获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus2);				//单修的角色2获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus3);				//单修的角色3获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus4);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus5);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus6);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus7);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus8);				//单修的角色4获得灵气值
	FIELD(UINT8,	m_TwoXiuLianState);					//双修状态
	FIELD(UINT8,	m_TwoXLHours);						//双修请求小时数
	FIELD(UINT32,	m_TwoXLLastGetNimbusTime);			//双修上次获得灵气时间
	FIELD(INT32,	m_TwoXLTotalNimbus);				//双修总共获得的灵气
	FIELD(UINT32,	m_TwoXLEndTime);					//双修结束时间
	FIELD(UINT64,	m_TwoXLUidActor);					//在双修的角色
	FIELD(UINT64,	m_TwoXLUidFriend);					//在双修的好友UID
	FIELD(UINT32,	m_FriendFacade);					//一起双修的好友外观
	FIELD(TInt8Array<THING_NAME_LEN>,   m_FriendName);	//一起双修的好友名称
	FIELD(UINT32,	m_MagicXLSeq);						//法术修炼序列号
	FIELD_END();
};

//获得修炼数据记录
struct SDB_Get_XiuLianData_Record
{
	FIELD_BEGIN();
	FIELD(UINT8,	XiuLianType);						//修炼类型
	FIELD(UINT8,	XiuLianState);						//状态
	FIELD(UINT8,	Hours);								//请求修练小时数
	FIELD(UINT8,	Mode);								//方式
	FIELD(UINT32,	LastGetNimbusTime);					//上次获得灵气时间
	FIELD(INT32,	TotalNimbus);						//总共获得的灵气
	FIELD(UINT32,	EndTime);							//修炼结束时间
	FIELD(UINT32,	AskSeq);							//请求序列号
	FIELD(UINT64,	UidSource);							//发起者
	FIELD(UINT64,	UidSourceActor);					//发起者在修炼的角色
	FIELD(TInt8Array<THING_NAME_LEN>,   SourceName);	//发起者名称
	FIELD(UINT64,	UidFriend);							//好友
	FIELD(UINT64,	UidFriendActor);					//好友角色
	FIELD(TInt8Array<THING_NAME_LEN>,  FriendName);		//好友名称	
	FIELD(TMagicID,	MagicID);							//法术	
	FIELD(UINT32,	AskTime);							//发起请求时间

	FIELD(INT32,	m_FriendTotalNimbus);     //好友总共获得的灵气
	FIELD(INT32,	m_SourceNotTakeNimbus);   //发起方未领取灵气
	FIELD(INT32,	m_FriendNotTakeNimbus);   //好友未领取灵气

	FIELD(bool ,     m_bStudyMagic);          //是否有法术需要学会

	FIELD(UINT32,	m_FriendLastGetNimbusTime);	//好友上次获得灵气时间

	FIELD(INT32,	m_SourceNimbusSpeed);	  //发起方的灵气速率
	FIELD(INT32,	m_FriendNimbusSpeed);	  //好友的灵气速率
	FIELD(INT32,	m_SourceLayer);			  //发起方的境界
	FIELD(INT32,	m_FriendLayer);			  //好友的境界

	FIELD_END();
};

//获得修炼数据
struct SDB_Get_XiuLianData_Req : private DBReqPacketHeader
{
	SDB_Get_XiuLianData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_XiuLianInfo2");
	FIELD(UINT64,  Uid_User);							//主人的ID

	FIELD_END();

	typedef  SDB_Get_XiuLianData_Rsp			TResultSet;  //结果集
	typedef	 SDB_Get_XiuLianData_Record		    TResultSet2;  //结果集
};

//插入修炼数据
struct SDB_Insert_XiuLianData_Req : private DBReqPacketHeader
{
	SDB_Insert_XiuLianData_Req()
	{
		this->m_length = sizeof(*this);
			
		Uid_User = UID().ToUint64();
		m_AloneXLActor =  UID().ToUint64();
		m_AloneXLActor2 =  UID().ToUint64();
		m_AloneXLActor3 =  UID().ToUint64();
		m_AloneXLActor4 =  UID().ToUint64();
		m_TwoXLUidActor =  UID().ToUint64();
		m_TwoXLUidFriend =  UID().ToUint64();
	}
	BIND_PROC_BEGIN("P_GDB_Insert_XiuLianInfo");
	FIELD(UINT64,   Uid_User);							//主人的UID
	FIELD(UINT64,   m_AloneXLActor);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor2);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor3);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor4);					//在单修的角色
	FIELD(UINT64,	m_TwoXLUidActor);					//在双修的角色
	FIELD(UINT64,	m_TwoXLUidFriend);					//在双修的好友UID

	FIELD_END();

	typedef  SDB_Get_XiuLianData_Rsp			TResultSet;  //结果集
	typedef	 SDB_Get_XiuLianData_Record		    TResultSet2;  //结果集
};

//更新修炼数据
struct SDB_Update_XiuLianData_Req : private DBReqPacketHeader
{
	SDB_Update_XiuLianData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_XiuLianInfo");
	FIELD(UINT64,   Uid_User);							//主人的UID
	FIELD(UINT8,	m_AloneXiuLianState);				//单修状态
	FIELD(UINT8,	m_AloneHours);						//单修请求修练小时数
	FIELD(UINT32,	m_AloneLastGetNimbusTime);			//单修上次获得灵气时间
	FIELD(UINT32,	m_AloneEndTime);					//单修修炼结束时间
	FIELD(UINT64,   m_AloneXLActor);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor2);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor3);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor4);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor5);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor6);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor7);					//在单修的角色
	FIELD(UINT64,   m_AloneXLActor8);					//在单修的角色
	FIELD(INT32,	m_AloneXLGetNimbus);				//单修的角色1获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus2);				//单修的角色2获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus3);				//单修的角色3获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus4);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus5);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus6);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus7);				//单修的角色4获得灵气值
	FIELD(INT32,	m_AloneXLGetNimbus8);				//单修的角色4获得灵气值
	FIELD(UINT32,	m_GetGodSwordNimbus);				//单修获得的仙剑灵气
	FIELD(UINT8,	m_TwoXiuLianState);					//双修状态
	FIELD(UINT8,	m_TwoXLHours);						//双修请求小时数
	FIELD(UINT32,	m_TwoXLLastGetNimbusTime);			//双修上次获得灵气时间
	FIELD(INT32,	m_TwoXLTotalNimbus);				//双修总共获得的灵气
	FIELD(UINT32,	m_TwoXLEndTime);					//双修结束时间
	FIELD(UINT64,	m_TwoXLUidActor);					//在双修的角色
	FIELD(UINT64,	m_TwoXLUidFriend);					//在双修的好友UID
	FIELD(UINT32,	m_FriendFacade);					//一起双修的好友外观
	FIELD(TInt8Array<THING_NAME_LEN>,   m_FriendName);	//一起双修的好友名称
	FIELD(UINT32,	m_MagicXLSeq);						//法术修炼序列号


	FIELD_END();

	typedef  ResultSetNull						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};


//更新修炼记录
struct SDB_Update_XiuLianData_Record_Req : private DBReqPacketHeader
{
	SDB_Update_XiuLianData_Record_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_XiuLianRecord");
	FIELD(UINT8,	XiuLianType);						//修炼类型
	FIELD(UINT8,	XiuLianState);						//状态
	FIELD(UINT8,	Hours);								//请求修练小时数
	FIELD(UINT8,	Mode);								//方式
	FIELD(UINT32,	LastGetNimbusTime);					//上次获得灵气时间
	FIELD(INT32,	TotalNimbus);						//总共获得的灵气
	FIELD(UINT32,	EndTime);							//修炼结束时间
	FIELD(UINT32,	AskSeq);							//请求序列号
	FIELD(UINT64,	UidSource);							//发起者
	FIELD(UINT64,	UidSourceActor);					//发起者在修炼的角色
	FIELD(TInt8Array<THING_NAME_LEN>,   SourceName);	//发起者名称
	FIELD(UINT64,	UidFriend);							//好友
	FIELD(UINT64,	UidFriendActor);					//好友角色
	FIELD(TInt8Array<THING_NAME_LEN>,  FriendName);		//好友名称	
	FIELD(TMagicID,	MagicID);							//法术	
	FIELD(UINT32,	AskTime);							//发起请求时间

	FIELD(INT32,	m_FriendTotalNimbus);     //好友总共获得的灵气
	FIELD(INT32,	m_SourceNotTakeNimbus);   //发起方未领取灵气
	FIELD(INT32,	m_FriendNotTakeNimbus);   //好友未领取灵气

	FIELD(bool ,     m_bStudyMagic);          //是否有法术需要学会
	FIELD(UINT32,	m_FriendLastGetNimbusTime);	//好友上次获得灵气时间

	FIELD(INT32,	m_SourceNimbusSpeed);	  //发起方的灵气速率
	FIELD(INT32,	m_FriendNimbusSpeed);	  //好友的灵气速率
	FIELD(INT32,	m_SourceLayer);			  //发起方的境界
	FIELD(INT32,	m_FriendLayer);			  //好友的境界

	FIELD_END();

	typedef  ResultSetNull						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};

//删除修炼记录
struct SDB_DeleteXiuLianRecord_Req : private DBReqPacketHeader
{
	SDB_DeleteXiuLianRecord_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Delete_XiuLianRecord");
	FIELD(UINT32,		AskSeq);		//请求序列号

	FIELD_END();

	typedef  ResultSetNull						TResultSet;  //结果集
	typedef	 ResultSetNull						TResultSet2;  //结果集
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//聚仙楼数据
struct SDBGatherGodHouseData
{
	SDBGatherGodHouseData()
	{
		memset(this, 0, sizeof(*this));
	}
	UINT32 m_LastFlushEmployTime;	//最后一次刷新角色时间

	TEmployeeID m_idEmployee[4];	//记录玩家可以招募角色的角色ID,4个

	UINT32 m_LastFlushMagicTime;	//最后一次刷新法术书时间

	TMagicBookID m_idMagicBook[4];	//记录玩家可以购买法术书的ID,4个
};

//获取聚仙楼数据
struct SDB_Get_GatherGodHouseData_Rsp
{
	SDB_Get_GatherGodHouseData_Rsp(){
		memset(this, 0, sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(UINT64,		Uid_User);				//聚仙楼主人ID
	FIELD(UINT32,		LastFlushEmployTime);	//最后一次刷新角色时间
	FIELD(TEmployeeID,	IdEmployee1);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee2);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee3);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee4);			//记录玩家可以招募角色的角色ID
	FIELD(UINT32,		LastFlushMagicTime);	//最后一次刷新法术书时间
	FIELD(TMagicBookID,	IdMagicBook1);			//记录玩家可以购买法术书的ID		
	FIELD(TMagicBookID,	IdMagicBook2);			//记录玩家可以购买法术书的ID
	FIELD(TMagicBookID,	IdMagicBook3);			//记录玩家可以购买法术书的ID
	FIELD(TMagicBookID,	IdMagicBook4);			//记录玩家可以购买法术书的ID
	FIELD_END();
};

//获取聚仙楼数据
struct SDB_Get_GatherGodHouseData_Req : private DBReqPacketHeader
{
	SDB_Get_GatherGodHouseData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_GatherGodHouseInfo");
	FIELD(UINT64,		Uid_User);	//聚仙楼主人的ID

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 SDB_Get_GatherGodHouseData_Rsp     TResultSet2;  //结果集
};

//插入聚仙楼数据
struct SDB_Insert_GatherGodHouseData_Req : private DBReqPacketHeader
{
	SDB_Insert_GatherGodHouseData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_GatherGodHouseInfo");
	FIELD(UINT64,		Uid_User);	//聚仙楼主人的ID

	FIELD_END();

	typedef  DB_OutParam						TResultSet;  //结果集
	typedef	 SDB_Get_GatherGodHouseData_Rsp     TResultSet2;  //结果集
};

//更新聚仙楼数据
struct SDB_Update_GatherGodHouseData_Req : private DBReqPacketHeader
{
	SDB_Update_GatherGodHouseData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Update_GatherGodHouseInfo");
	FIELD(UINT64,		Uid_User);				//聚仙楼主人ID
	FIELD(UINT32,		LastFlushEmployTime);	//最后一次刷新角色时间
	FIELD(TEmployeeID,	IdEmployee1);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee2);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee3);			//记录玩家可以招募角色的角色ID
	FIELD(TEmployeeID,	IdEmployee4);			//记录玩家可以招募角色的角色ID
	FIELD(UINT32,		LastFlushMagicTime);	//最后一次刷新法术书时间
	FIELD(TMagicBookID,	IdMagicBook1);			//记录玩家可以购买法术书的ID		
	FIELD(TMagicBookID,	IdMagicBook2);			//记录玩家可以购买法术书的ID  
	FIELD(TMagicBookID,	IdMagicBook3);			//记录玩家可以购买法术书的ID
	FIELD(TMagicBookID,	IdMagicBook4);			//记录玩家可以购买法术书的ID

	FIELD_END();

	
	typedef ResultSetNull					 TResultSet;  //结果集
	typedef ResultSetNull                TResultSet2;  //结果集
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//战斗数据
struct SDBCombatData
{
	SDBCombatData()
	{
		for(int i = 0; i < 	MAX_LINEUP_POS_NUM; ++i)
		{
			m_uidLineup[i] = UID();
		}
	}

	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //阵型
};

struct SDB_Get_CombatData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,	Uid_User);
	FIELD(UINT64,	uidLineup1);
	FIELD(UINT64,	uidLineup2);
	FIELD(UINT64,	uidLineup3);
	FIELD(UINT64,	uidLineup4);
	FIELD(UINT64,	uidLineup5);
	FIELD(UINT64,	uidLineup6);
	FIELD(UINT64,	uidLineup7);
	FIELD(UINT64,	uidLineup8);
	FIELD(UINT64,	uidLineup9);
	FIELD_END();
};

//获取战斗数据
struct SDB_Get_CombatData_Req : private DBReqPacketHeader
{
	SDB_Get_CombatData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_CombatInfo");
	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef DB_OutParam				TResultSet;			//结果集
	typedef SDB_Get_CombatData_Rsp	TResultSet2;		//结果集
};

//插入战斗数据
struct SDB_Insert_CombatData_Req : private DBReqPacketHeader
{
	SDB_Insert_CombatData_Req(){
		this->m_length = sizeof(*this);
		uidLineup1 = UID().ToUint64();
		uidLineup2 = UID().ToUint64();
		uidLineup3 = UID().ToUint64();
	}

	BIND_PROC_BEGIN("P_GDB_insert_CombatInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(UINT64,		uidLineup1);
	FIELD(UINT64,		uidLineup2);
	FIELD(UINT64,		uidLineup3);


	FIELD_END();
	typedef DB_OutParam				TResultSet;			//结果集
	typedef SDB_Get_CombatData_Rsp	TResultSet2;		//结果集
};

//更新战斗数据
struct SDB_Update_CombatData_Req : private DBReqPacketHeader
{
	SDB_Update_CombatData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_CombatInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(UINT64,		uidLineup1);
	FIELD(UINT64,		uidLineup2);
	FIELD(UINT64,		uidLineup3);
	FIELD(UINT64,		uidLineup4);
	FIELD(UINT64,		uidLineup5);
	FIELD(UINT64,		uidLineup6);
	FIELD(UINT64,		uidLineup7);
	FIELD(UINT64,		uidLineup8);
	FIELD(UINT64,		uidLineup9);

	FIELD_END();

	typedef ResultSetNull			TResultSet;		//结果集
	typedef ResultSetNull           TResultSet2;	//结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//副本Part存盘数据
struct SDBFuBenData
{
	SDBFuBenData()
	{
		memset(this,0,sizeof(*this));
	}
	UINT32            m_LastFreeEnterFuBenTime;  //最后一次免费进入且记录副本进度时间
	UINT32			  m_LastStoneEnterFuBenTime; //最后一次灵石进入且记录副本进度时间
	UINT32			  m_LastSynWelfareEnterFuBenTime; //最后一次帮派福利进入且记录副本进度时间
	UINT16            m_FreeEnterFuBenNum;       //今天免费进入次数
	UINT16			  m_SynWelfareEnterFuBenNum; //帮派福利进入次数
	UINT16			  m_CostStoneEnterFuBenNum;	 //支付灵石进入次数
	UINT8             m_FuBenNum;                //已开启的副本数量
	UINT32			  m_LastEnterFuBenGodSword;	 //最后一次进入剑印世界时间
	UINT16			  m_EnterSynFuBenNum;		 //进入帮派副本次数
	UINT32			  m_LastEnterSynFuBenTime;	 //最后一次进入帮派副本时间
	UINT16			  m_EnterGodSwordWorldNum;	 //今天进入仙剑副本次数
	UINT32			  m_LastVipEnterFuBenTime;	 //最后一次VIP进入副本时间
	UINT16			  m_VipEnterFuBenNum;		 //今天VIP进入次数
	TBitArray<MAX_FUBEN_ID> m_FinishedFuBen;   //通过的副本
    SFuBenProgress    m_FuBenProgress[MAX_OPEN_FUBEN_NUM];         //副本进度
	TBitArray<MAX_FUBEN_ID> m_HardFinishedFuBen; //通过的困难副本
};



//获得副本的基本数据
struct SDB_Get_BasicFuBenData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,		Uid_User);					//玩家ID
	FIELD(UINT32,		LastFreeEnterFuBenTime);	//最后一次免费进入且记录副本进度时间
	FIELD(UINT32,		LastStoneEnterFuBenTime);	//最后一次付灵石进入且记录副本进度时间
	FIELD(UINT32,		LastSynWelfareEnterFuBenTime);//最后一次帮派福利进入且记录副本进度时间
	FIELD(UINT16,		FreeEnterFuBenNum);			//今天免费进入次数
	FIELD(UINT16,		SynWelfareEnterFuBenNum);   //帮派福利进入次数
	FIELD(UINT16,	    CostStoneEnterFuBenNum);	//支付灵石进入次数
	FIELD(UINT8,		FuBenNum);					//已开启的副本数量
	FIELD(UINT32,		LastEnterFuBenGodSword);	//最后一次进入剑印世界时间
	FIELD(UINT16,		EnterSynFuBenNum);			//进入帮派副本次数
	FIELD(UINT32,		LastEnterSynFuBenTime);		//最后一次进入帮派副本时间
	FIELD(UINT16,		m_EnterGodSwordWorldNum);	//今天进入仙剑副本次数
	FIELD(UINT32,		LastVipEnterFuBenTime);		//最后一次VIP进入副本时间
	FIELD(UINT16,		VipEnterFuBenNum);			//今天VIP进入次数
	FIELD(TUint8Array<(MAX_FUBEN_ID+7)/8>,	m_FinishedFuBen);   //通过的副本
	FIELD(TUint8Array<(MAX_FUBEN_ID+7)/8>,	m_HardFinishedFuBen);   //通过的困难副本

	FIELD_END();
};

//获得副本进度的数据
struct SDB_Get_FuBenProgressData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,	Uid_User);			//玩家的ID
	FIELD(TFuBenID, FuBenID);			//副本ID
	FIELD(UINT8,    Level);				//级别
	FIELD(UINT8,    KillMonsterNum);	//杀怪数量
	FIELD_END();
};

//获得副本数据
struct SDB_Get_FuBenData_Req : private DBReqPacketHeader
{
	SDB_Get_FuBenData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_FuBenInfo");
	FIELD(UINT64,		Uid_User);


	FIELD_END();
	typedef SDB_Get_BasicFuBenData_Rsp        TResultSet;  //结果集
	typedef SDB_Get_FuBenProgressData_Rsp     TResultSet2;  //结果集
};

//插入副本数据
struct SDB_Insert_FuBenData_Req : private DBReqPacketHeader
{
	SDB_Insert_FuBenData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_FuBenInfo");

	FIELD(UINT64,		Uid_User);

	FIELD_END();

	typedef SDB_Get_BasicFuBenData_Rsp        TResultSet;  //结果集

	typedef SDB_Get_FuBenProgressData_Rsp     TResultSet2;  //结果集
};

//更新副本基本数据
struct SDB_Update_BasicFuBenData_Req : private DBReqPacketHeader
{
	SDB_Update_BasicFuBenData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_Update_BasicFuBenInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(UINT32,		LastFreeEnterFuBenTime);	//最后一次免费进入且记录副本进度时间
	FIELD(UINT32,		LastStoneEnterFuBenTime);	//最后一次付灵石进入且记录副本进度时间
	FIELD(UINT32,		LastSynWelfareEnterFuBenTime); //最后一次帮派福利进入且记录副本进度时间
	FIELD(UINT16,		FreeEnterFuBenNum);			//今天免费进入次数
	FIELD(UINT16,		SynWelfareFuBenNum);		//帮派福利进入次数
	FIELD(UINT16,		CostStoneEnterFuBenNum);	//支付灵石进入次数
	FIELD(UINT8,		FuBenNum);					//已开启的副本数量
	FIELD(UINT32,		LastEnterFuBenGodSword);	//最后一次进入剑印世界时间
	FIELD(UINT16,		EnterSynFuBenNum);			//进入帮派副本次数
	FIELD(UINT32,		LastEnterSynFuBenTime);		//最后一次进入帮派副本时间
	FIELD(UINT16,		EnterGodSwordWorldNum);		//今天进入仙剑副本次数
	FIELD(UINT32,		LastVipEnterFuBenTime);		//最后一次VIP进入副本时间
	FIELD(UINT16,		VipEnterFuBenNum);			//今天VIP进入次数
	FIELD(TUint8Array<(MAX_FUBEN_ID+7)/8>,	m_FinishedFuBen);	//通过的副本记录
	FIELD(TUint8Array<(MAX_FUBEN_ID+7)/8>,	m_HardFinishedFuBen);	//通过的困难副本记录

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull           TResultSet2;	//结果集
};

//更新副本进度的数据
struct SDB_Update_FuBenProgressData_Req : private DBReqPacketHeader
{
	SDB_Update_FuBenProgressData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_Update_FuBenProgressInfo");
	FIELD(UINT64,		Uid_User);
	FIELD(TFuBenID,		FuBenID);			//副本ID
	FIELD(UINT8,		Level);				//级别
	FIELD(UINT8,		KillMonsterNum);	//杀怪数量

	FIELD_END();

	typedef ResultSetNull					TResultSet;		//结果集
	typedef ResultSetNull               TResultSet2;	//结果集
};

////////////////////////////////////////////////////////////////////////////////////////////

//法宝part存盘数据
//进入法宝世界记录
struct STalismanWorldRecord
{
	TTalismanWorldID  m_TalismanWorldID;  //法宝世界ID
	UINT8             m_EnterNumOfDay;   //当天进入次数
	UINT32            m_LastEnterTime;   //最后进入时间
};

struct SDBTalismanData
{
	UINT16                  m_RecordNum; //记录数量
	STalismanWorldRecord    m_Records[50];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//获取角色数据应答
struct SDB_Get_ActorBasicData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT32,	UserID);					//玩家ID
	FIELD(TInt8Array<THING_NAME_LEN>,   Name);	//玩家名字
	FIELD(UINT8,    Level);						//等级
	FIELD(int,      Spirit);					//灵力
	FIELD(int,      Shield);					//护盾
	FIELD(int,      Avoid);						//身法
	FIELD(int,      ActorExp);					//经验
	FIELD(int,      ActorLayer);				//层次，境界
	FIELD(int,      ActorNimbus);				//灵气
	FIELD(int,      ActorAptitude);				//资质
	FIELD(UINT8,    ActorSex);					//性别
	FIELD(UINT64,   uid);						//uid
	FIELD(int,      ActorMoney);				//游戏币
	FIELD(int,      ActorTicket);				//礼券
	FIELD(int,      ActorStone);				//灵石
	FIELD(UINT16,    ActorFacade);				//外观,取值由客户端定义
	FIELD(int,      ActorBloodUp);				//气血上限
	FIELD(UINT8,    Dir);						//方向
	FIELD(UINT16,   ptX);						//X坐标
	FIELD(UINT16,   ptY);						//Y坐标
	FIELD(UINT64,   uidMaster);					//主人，如为INVALID_UID 表示本身是主角
	FIELD(UINT16,	CityID);					//城市编号
	FIELD(INT32,	Honor);						//荣誉
	FIELD(INT32,	Credit);					//声望
	FIELD(INT32,	ActorNimbusSpeed);			//灵气速率
	FIELD(INT32,	GodSwordNimbus);			//仙剑灵气
	FIELD(TUint8Array<(enUseFlag_Max+7)/8>,	m_UseFlag);	//给各模块使用的标志
	FIELD(UINT8,	VipLevel);					//Vip等级
	FIELD(INT32,	Recharge);					//历史充值数量
	FIELD(INT32,	ActorPolyNimbus);			//聚灵气
	FIELD(UINT8,	GoldDamageLv);				//金剑诀伤害等级
	FIELD(UINT8,	WoodDamageLv);				//木剑诀伤害等级
	FIELD(UINT8,	WaterDamageLv);				//水剑诀伤害等级
	FIELD(UINT8,	FireDamageLv);				//火剑诀伤害等级
	FIELD(UINT8,	SoilDamageLv);				//土剑诀伤害等级
	FIELD(INT32,	SynCombatLevel);			//帮战等级
	FIELD(UINT8,	CritLv);					//爆击等级
	FIELD(UINT8,	TenacityLv);				//坚韧等级
	FIELD(UINT8,	HitLv);						//命中等级
	FIELD(UINT8,	DodgeLv);					//回避等级
	FIELD(UINT8,	MagicCDLv);					//法术回复等级
	FIELD(INT32,	Crit);						//爆击
	FIELD(INT32,	Tenacity);					//坚韧
	FIELD(INT32,	Hit);						//命中
	FIELD(INT32,	Dodge);						//回避
	FIELD(INT32,	MagicCD);					//法术回复
	FIELD(INT32,	GhostSoul);					//灵魄
	FIELD(INT32,	DuoBaoLevel);				//夺宝等级
	FIELD(UINT32,	CombatAbility);				//战斗力
	FIELD(INT32,	GoldDamage);				//金剑诀伤害
	FIELD(INT32,	WoodDamage);				//木剑诀伤害
	FIELD(INT32,	WaterDamage);				//水剑诀伤害
	FIELD(INT32,	FireDamage);				//火剑诀伤害
	FIELD(INT32,	SoilDamage);				//土剑诀伤害
	FIELD(UINT32,	LastOnlineTime);			//上次上线时间

	FIELD_END();
};




//获角色数据请求
struct SDB_Get_ActorBasicData_Req : private DBReqPacketHeader
{
	SDB_Get_ActorBasicData_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetActorBasicInfo");
	FIELD(TUserID, UserID);


	FIELD_END();
	typedef  DB_OutParam					TResultSet;  //结果集
	typedef  SDB_Get_ActorBasicData_Rsp     TResultSet2;  //结果集
};

//通过UID获角色数据请求
struct SDB_Get_ActorBasicDataByUID_Req : private DBReqPacketHeader
{
	SDB_Get_ActorBasicDataByUID_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetActorBasicDataByUIDInfo");
	FIELD(UINT64, Uid_User);


	FIELD_END();
	typedef  DB_OutParam					TResultSet;  //结果集
	typedef  SDB_Get_ActorBasicData_Rsp     TResultSet2;  //结果集
};

//通过名字获取玩家信息
struct SDB_Get_ActorBasicDataByName_Req : private DBReqPacketHeader
{
	SDB_Get_ActorBasicDataByName_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetActorBasicDataByName");
	FIELD(TInt8Array<THING_NAME_LEN>,   Name);	//玩家名字


	FIELD_END();
	typedef  DB_OutParam					TResultSet;  //结果集
	typedef  SDB_Get_ActorBasicData_Rsp     TResultSet2;  //结果集
};

//插入角色的基本数据
struct SDB_Insert_ActorBasicData_Req : private DBReqPacketHeader
{
	SDB_Insert_ActorBasicData_Req(){
		this->m_length = sizeof(*this);
	}


	BIND_PROC_BEGIN("P_GDB_Insert_ActorBasicInfo");
	FIELD(TUserID,	UserID);					//玩家ID
	FIELD(TInt8Array<THING_NAME_LEN>,   Name);	//玩家名字
	FIELD(UINT8,    Level);						//等级
	FIELD(int,      Spirit);					//灵力
	FIELD(int,      Shield);					//护盾
	FIELD(int,      BloodUp);						//气血上限
	FIELD(int,      Avoid);						//身法
	FIELD(int,      ActorExp);					//经验
	FIELD(int,      ActorLayer);				//层次，境界
	FIELD(int,      ActorNimbus);				//灵气
	FIELD(int,      ActorAptitude);				//资质
	FIELD(UINT8,    ActorSex);					//性别
	FIELD(UINT64,   uid);						//uid
	FIELD(UINT16,    ActorFacade);				//外观,取值由客户端定义
	FIELD(UINT64,   uidMaster);					//主人，如为INVALID_UID 表示本身是主角
	FIELD(UINT16,	CityID);					//城市编号
	FIELD(UINT16,	ActorNimbusSpeed);			//灵气速率

	FIELD_END();

	
	typedef  DB_OutParam                   TResultSet;  //结果集
	typedef SDB_Get_ActorBasicData_Rsp     TResultSet2;  //结果集
};


struct SDB_Get_ActorUIDByName_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,		Uid_Actor);
	FIELD_END();
};

//根据角色名字获取角色ID
struct SDB_Get_ActorUIDByName_Req : private DBReqPacketHeader
{
	SDB_Get_ActorUIDByName_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetActorIDByNameInfo");
	FIELD(TInt8Array<THING_NAME_LEN>, ActorName); 

	FIELD_END();

	typedef  DB_OutParam                   TResultSet;  //结果集
	typedef	 SDB_Get_ActorUIDByName_Rsp   TResultSet2;  //结果集

};

//删除角色
struct SDB_Delete_Actor : private DBReqPacketHeader
{
	SDB_Delete_Actor()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Delete_Actor");
	FIELD(UINT64,		m_ActorUID);

	FIELD_END();

	typedef ResultSetNull				TResultSet;   //结果集
	typedef ResultSetNull				TResultSet2;  //结果集
};

//根据玩家的UID值获取玩家的名字
struct SDB_Get_ActorNameByUid_Rsp
{
	FIELD_BEGIN();
	FIELD(TInt8Array<THING_NAME_LEN>, UserName);	//玩家名字
	FIELD_END();
};

//根据玩家的UID值获取玩家的名字
struct SDB_Get_ActorNameByUid_Req : private DBReqPacketHeader
{
	SDB_Get_ActorNameByUid_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetActorNameByUid");
	FIELD(UINT64,		Uid_User);			//玩家的UID值

	FIELD_END();

	typedef  DB_OutParam					TResultSet;  //结果集
	typedef	 SDB_Get_ActorNameByUid_Rsp		TResultSet2;  //结果集
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//更新角色数据


struct SDB_Update_ActorBasicData_Req : private DBReqPacketHeader
{
	SDB_Update_ActorBasicData_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_UpdateActorInfo2");
	FIELD(UINT32,	UserID);					//玩家ID
	FIELD(TInt8Array<THING_NAME_LEN>,   Name);	//玩家名字
	FIELD(UINT8,    Level);						//等级
	FIELD(int,      Spirit);					//灵力
	FIELD(int,      Shield);					//护盾
	FIELD(int,      Avoid);						//身法
	FIELD(int,      ActorExp);					//经验
	FIELD(int,      ActorLayer);				//层次，境界
	FIELD(int,      ActorNimbus);				//灵气
	FIELD(int,      ActorAptitude);				//资质
	FIELD(UINT8,    ActorSex);					//性别
	FIELD(UINT64,   uid);						//uid
	FIELD(int,      ActorMoney);				//游戏币
	FIELD(int,      ActorTicket);				//礼券
	FIELD(int,      ActorStone);				//灵石
	FIELD(UINT16,    ActorFacade);				//外观,取值由客户端定义
	FIELD(int,      ActorBloodUp);				//气血上限
	FIELD(UINT8,    Dir);						//方向
	FIELD(UINT16,   ptX);						//X坐标
	FIELD(UINT16,   ptY);						//Y坐标
	FIELD(UINT64,   uidMaster);					//主人，如为INVALID_UID 表示本身是主角
	FIELD(UINT16,	CityID);					//城市编号
	FIELD(INT32,	Honor);						//荣誉
	FIELD(INT32,	Credit);					//声望
	FIELD(INT32,	ActorNimbusSpeed);			//灵气速率
    FIELD(INT32,	GodSwordNimbus);			//仙剑灵气
	FIELD(TUint8Array<(enUseFlag_Max+7)/8>,	m_UseFlag);	//给各模块使用的标志
	FIELD(UINT8,	VipLevel);					//Vip等级
	FIELD(INT32,	Recharge);					//历史充值数量
	FIELD(INT32,	ActorPolyNimbus);			//聚灵气
	FIELD(UINT8,	GoldDamageLv);				//金剑诀伤害等级
	FIELD(UINT8,	WoodDamageLv);				//木剑诀伤害等级
	FIELD(UINT8,	WaterDamageLv);				//水剑诀伤害等级
	FIELD(UINT8,	FireDamageLv);				//火剑诀伤害等级
	FIELD(UINT8,	SoilDamageLv);				//土剑诀伤害等级	
	FIELD(INT32,	SynCombatLevel);			//帮战等级
	FIELD(UINT8,	CritLv);					//爆击等级
	FIELD(UINT8,	TenacityLv);				//坚韧等级
	FIELD(UINT8,	HitLv);						//命中等级
	FIELD(UINT8,	DodgeLv);					//回避等级
	FIELD(UINT8,	MagicCDLv);					//法术回复等级
	FIELD(INT32,	Crit);						//爆击
	FIELD(INT32,	Tenacity);					//坚韧
	FIELD(INT32,	Hit);						//命中
	FIELD(INT32,	Dodge);						//回避
	FIELD(INT32,	MagicCD);					//法术回复
	FIELD(INT32,	GhostSoul);					//灵魄
	FIELD(INT32,	DuoBaoLevel);				//夺宝等级
	FIELD(UINT32,	CombatAbility);				//战斗力
	FIELD(INT32,	GoldDamage);				//金剑诀伤害
	FIELD(INT32,	WoodDamage);				//木剑诀伤害
	FIELD(INT32,	WaterDamage);				//水剑诀伤害
	FIELD(INT32,	FireDamage);				//火剑诀伤害
	FIELD(INT32,	SoilDamage);				//土剑诀伤害
	

	FIELD_END();

	typedef ResultSetNull   TResultSet;		//结果集
	typedef ResultSetNull   TResultSet2;		//结果集
};

//应答
struct SDB_Update_ActorBasicData_Rsp
{
	DB_OutParam OutParam;
};
/////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
//招募角色

//招募角色的数量
struct SDB_GetEmployeeNum_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT8,	EmployeeNum);	//招募角色的数量
	FIELD_END();
};

//获取玩家的银幕角色UID值
struct SDB_Get_EmployeeData_Req : private DBReqPacketHeader
{
	SDB_Get_EmployeeData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetEmployeeInfo");
	FIELD(UINT64,	Uid_User);					//玩家UID值

	FIELD_END();

	typedef SDB_GetEmployeeNum_Rsp			TResultSet;  //结果集
	typedef SDB_Get_ActorBasicData_Rsp		TResultSet2;  //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SDB_Get_CreateActor_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,   Uid_Actor);
	FIELD_END();
};
//在这个回调函数创建角色
struct SDB_CreateActor_Req : private DBReqPacketHeader
{
	SDB_CreateActor_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_CreateActor");
	FIELD(UINT64,	Uid_Actor);							//角色UID值

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_CreateActor_Rsp		TResultSet2;    //结果集
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//获取此玩家的所有好友应答
struct SDB_Get_FriendListData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64, uid_Friend);							//好友的UID值
	FIELD(UINT32, RelationNum);							//好友的好友度
	FIELD(UINT8,  Flag);								//0为单方好友，1为双向好友
	FIELD(TInt8Array<THING_NAME_LEN>, FriendName);		//好友名字
	FIELD(UINT8,  FriendLevel);							//好友等级
	FIELD(UINT8,  FriendSex);							//好友性别
	FIELD(TInt8Array<THING_NAME_LEN>, SynName);			//好友帮派名字
	FIELD(bool,	  bOneCity);							//是否同城
	FIELD(UINT16, FriendFacade);						//好友外观
	FIELD(TTitleID, TitleID);							//好友ID /
	FIELD(UINT8,	vipLevel);							//vip等级
	FIELD(INT32,	DuoBaoLevel);						//夺宝等级
	FIELD(INT32,	SynWarLevel);						//帮战等级
	FIELD(UINT32,	CombatAbility);						//战斗力
	FIELD_END();

};

//获取此玩家的所有好友
struct SDB_Get_FriendListData_Req :  private DBReqPacketHeader
{
	SDB_Get_FriendListData_Req()
	{
          this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetFriendListDataInfo");
	FIELD(UINT64,	uid_User);


	FIELD_END();
	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_FriendListData_Rsp	TResultSet2;  //结果集
};

//更新好友度数据，返回好友度数值
struct SDB_Update_FriendData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT32, RelationNum);
	FIELD_END();
};

//更新好友数据
struct SDB_Update_FriendData_Req :  private DBReqPacketHeader
{
	SDB_Update_FriendData_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_UpdateFriendDataInfo");
	FIELD(UINT64,	uid_User);
	FIELD(UINT64,	uid_Friend);
	FIELD(UINT32,	RelationNum);
	//FIELD(UINT32,	CombatAbility);					//战斗力

	FIELD_END();
	typedef SDB_Update_FriendData_Rsp	TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2;  //结果集
};

//插入好友度数据时，返回好友度值(因为双方好友度值得一样，所以得检测另一方的)
struct SDB_Insert_FriendData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,		uid_Friend);				//好友的UID值
	FIELD(UINT32,		RelationNum);				//返回的好友度值
	FIELD(TInt8Array<ACTOR_NAME_LEN>, FriendName);	//好友名字
	FIELD(UINT8,		FriendLevel);				//好友等级
	FIELD(UINT8,		FriendSex);					//好友性别
	FIELD(TInt8Array<ACTOR_NAME_LEN>, SynName);		//好友帮派名称
	FIELD(UINT8,		Flag);						//0为单向好友，1为双向好友
	FIELD(bool,			bOneCity);					//是否同城
	FIELD(UINT16,		FriendFacade);				//好友外观
	FIELD(TTitleID,		FriendTitleID);				//好友称号ID /
	FIELD(UINT8,		vipLevel);					//vip等级
	FIELD(INT32,	DuoBaoLevel);					//夺宝等级
	FIELD(INT32,	SynWarLevel);					//帮战等级
	FIELD(UINT32,	CombatAbility);					//战斗力
	FIELD_END();
};

//插入好友度数据
struct SDB_Insert_FriendData_Req : private DBReqPacketHeader
{
	SDB_Insert_FriendData_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertFriendDataInfo");
	FIELD(UINT64,	uid_User);						//加FriendID玩家为好友的玩家ID
	FIELD(UINT64,	uid_Friend);					//被UserID玩家加为好友的玩家ID
	FIELD(UINT32,	RelationNum);					//好友度
	
	FIELD_END();

	TRESULT_SET(DB_OutParam,SDB_Insert_FriendData_Rsp);   //结果集
};

//删除好友度数据
struct SDB_Delete_FriendData_Req : private DBReqPacketHeader
{
	SDB_Delete_FriendData_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_DeleteFriendDataInfo");
	FIELD(UINT64,	UID_User);
	FIELD(UINT64,	UID_Friend);

	FIELD_END();

	typedef  ResultSetNull	TResultSet;  //结果集
	typedef	 ResultSetNull	TResultSet2;  //结果集
};

//获好友度改变事件数据应答
struct SDB_Get_FriendEnventData_Rsp
{
	FIELD_BEGIN();
	FIELD(TUserID,	UserID);			//加FriendID玩家为好友的玩家ID
	FIELD(TUserID,	FriendID);			//被UserID玩家加为好友的玩家ID
	FIELD(UINT32,	AddRelateionNum);	//好友度
	FIELD(TInt8Array<DESCRIPT_LEN_100>, Descript);	//事件信息描述
	FIELD(INT32,	Time);				//时间
	FIELD_END();
};


//删除好友度改变事件数据(此数据只保存1天)
struct SDB_Delete_FriendEnventData_Req :  private DBReqPacketHeader
{
	SDB_Delete_FriendEnventData_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_DeleteFriendEnventDataInfo");
	FIELD(UINT64,	Time);

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

//获好友度改变事件数据请求
struct SDB_Get_FriendEventData_Req : private DBReqPacketHeader
{
	SDB_Get_FriendEventData_Req()
	{
          this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetFriendEnventDataInfo");
	FIELD(TUserID,	UserID);
	FIELD(TUserID,	FriendID);

	FIELD_END();

	typedef  DB_OutParam					 TResultSet;  //结果集
	typedef	 SDB_Get_FriendEnventData_Rsp	 TResultSet2;  //结果集
};

//插入好友度改变事件数据
struct SDB_Insert_FriendEnventData_Req : private DBReqPacketHeader
{
	SDB_Insert_FriendEnventData_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertFriendEnventDataInfo");
	FIELD(UINT64,	uid_User);
	FIELD(UINT64,	uid_Friend);
	FIELD(UINT32,	AddRelationNum);
	FIELD(TInt8Array<DESCRIPT_LEN_100>, Descript);
	FIELD(UINT64,	Time);

	FIELD_END();

	TRESULT_SET(ResultSetNull,ResultSetNull);   //结果集
};

//获取此玩家的所有好友度改变事件应答
struct SDB_Get_FriendEnventListData_Rsp
{
	FIELD_BEGIN();		
	FIELD(UINT64, uid_Friend);						//好友的UID值
	FIELD(TInt8Array<DESCRIPT_LEN_100>, Descript);	//事件信息描述
	FIELD(UINT64, Time);							//事件发生的时间
	FIELD_END();
};

//获取此玩家的所有好友度改变事件
struct SDB_Get_FriendEnventListData_Req :  private DBReqPacketHeader
{
	SDB_Get_FriendEnventListData_Req()
	{
          this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetFriendEventListDataInfo");
	FIELD(UINT64,	uid_User);
	FIELD(UINT64,	time);

	FIELD_END();

	typedef DB_OutParam							TResultSet;  //结果集
	typedef SDB_Get_FriendEnventListData_Rsp	TResultSet2; //结果集
};

//获取此玩家的所有好友信息标签中的信息应答
struct SDB_Get_FrientMsgListData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64, uid_SrcUser);								//加你为好友，但你没加对方为好友的玩家ID
	FIELD(TInt8Array<ACTOR_NAME_LEN>, SrcUserName);			//好友名字
	FIELD(UINT8,		SrcUserLevel);						//好友等级
	FIELD(UINT8,		SrcUserSex);						//好友性别
	FIELD(TInt8Array<ACTOR_NAME_LEN>, SrcUserSynName);		//好友帮派名称
	FIELD(bool,			bOneCity);							//是否同城
	FIELD(UINT16,		SrcUserFacade);						//外观
	FIELD(TTitleID,		TitleID);							//称号ID /
	FIELD(INT32,		DuoBaoLevel);						//夺宝等级
	FIELD(INT32,		SynWarLevel);						//帮战等级
	FIELD(UINT32,		CombatAbility);						//战斗力
	FIELD_END();
};

//获取此玩家的所有好友信息标签中的信息
struct SDB_Get_FriendMsgListData_Req : private DBReqPacketHeader
{
	SDB_Get_FriendMsgListData_Req()
	{
          this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetFriendMsgListDataInfo");
	FIELD(UINT64,	uid_User);

	FIELD_END();

	typedef DB_OutParam						TResultSet;  //结果集
	typedef SDB_Get_FrientMsgListData_Rsp	TResultSet2;  //结果集
};

//插入好友信息标签中的信息的请求(没用的了)
struct SDB_Insert_FriendMsgData_Req : private DBReqPacketHeader
{
	SDB_Insert_FriendMsgData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_InsertFriendMsgDataInfo");
	FIELD(UINT64,	uid_User);			//玩家UID的值
	FIELD(UINT64,	uid_SrcUser);		//相关的玩家

	FIELD_END();

	typedef DB_OutParam		TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

//删除(没用的了)
struct SDB_Delete_FriendMsgData_Req : private DBReqPacketHeader
{
	SDB_Delete_FriendMsgData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_DeleteFriendMsgDataInfo");
	FIELD(UINT64,	uid_User);
	FIELD(UINT64,	uid_SrcUser);

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//邮件

//邮件数据库操作结果码
enum enDBMailRetCode 
{
	enDBMailRetCode_OK			= 0,  //成功
	enDBMailRetCode_NoUser		= 2,  //不存在此用户
	enDBMailRetCode_WriteError	= 3,  //邮件入库失败

};

//插入新邮件后，获得数据库返回的邮件ID
struct SDB_Insert_MailData_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT32,		ID_Mail);
	FIELD(UINT64,		uid_User);						//玩家UID值
	FIELD_END();
};

//插入新邮件
struct SDB_Insert_MailData_Req : private DBReqPacketHeader
{
	SDB_Insert_MailData_Req() : uid_User(UID().ToUint64()), uid_SendUser(UID().ToUint64()), Mail_Type(0), Stone(0), Money(0), Ticket(0), SynID(INVALID_SYN_ID), Time(0)
	{
		this->m_length = sizeof(*this);

		Name_SendUser[0] = '\0';
		ThemeText[0] = '\0';
		ContentText[0] = '\0';
		memset(&GoodsData, 0, sizeof(GoodsData));
	}
	BIND_PROC_BEGIN("P_GDB_Insert_MailData_Req");
	FIELD(UINT64,		uid_User);						//玩家UID值
	FIELD(UINT64,		uid_SendUser);					//发件人UID值
	FIELD(TInt8Array<THING_NAME_LEN>, Name_SendUser);	//发件人名字
	FIELD(UINT8,		Mail_Type);						//邮件类型
	FIELD(UINT32,		Stone);							//发送的灵石
	FIELD(UINT32,		Money);							//发送的仙石
	FIELD(UINT32,		Ticket);						//发送的礼卷
	FIELD(UINT32,		PolyNimbus);					//聚灵气
	FIELD(TSynID,		SynID);							//邀请的帮派ID
	FIELD(TInt8Array<DESCRIPT_LEN_50>, ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>, ContentText);	//文本内容
	FIELD(UINT64,		Time);							//时间
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据

	FIELD_END();

	typedef DB_OutParam				TResultSet;  //结果集
	typedef SDB_Insert_MailData_Rsp	TResultSet2;  //结果集
};

//用UserID给玩家插入系统邮件
struct SDB_Insert_SysMailByUserID : private DBReqPacketHeader
{
	SDB_Insert_SysMailByUserID(){
		this->m_length = sizeof(*this);
		uid_SendUser = UID().ToUint64();
		Mail_Type    = enMailType_System;
		Stone		 = 0;
		Money		 = 0;
		Ticket		 = 0;
		strncpy((char *)Name_SendUser, "系统", sizeof(Name_SendUser));
		memset(&GoodsData, 0, sizeof(GoodsData));
		PolyNimbus   = 0;
	}

	BIND_PROC_BEGIN("P_GDB_Insert_SysMailByUserID");
	FIELD(TUserID,		UserID);						//玩家UID值
	FIELD(UINT64,		uid_SendUser);					//发件人UID值
	FIELD(TInt8Array<THING_NAME_LEN>, Name_SendUser);	//发件人名字
	FIELD(UINT8,		Mail_Type);						//邮件类型
	FIELD(UINT32,		Stone);							//发送的灵石
	FIELD(UINT32,		Money);							//发送的仙石
	FIELD(UINT32,		Ticket);						//发送的礼卷
	FIELD(UINT32,		PolyNimbus);					//聚灵气
	FIELD(TInt8Array<DESCRIPT_LEN_50>, ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>, ContentText);	//文本内容
	FIELD(UINT64,		Time);							//时间
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

//删除邮件
struct SDB_Delete_MailDate_Req : private DBReqPacketHeader
{
	SDB_Delete_MailDate_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Delete_MailData_Req");
	FIELD(TMailID,			MailID);

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

//更新邮件数据到数据库
struct SDB_Updata_MailData_Req : private DBReqPacketHeader
{
	SDB_Updata_MailData_Req()
	{
		this->m_length = sizeof(*this);
		memset(&GoodsData, 0, sizeof(GoodsData));
	}
	BIND_PROC_BEGIN("P_GDB_Update_MailData_Req");
	FIELD(TMailID,		MailID);						//邮件ID
	FIELD(UINT32,		Stone);							//发送的灵石
	FIELD(UINT32,		Money);							//发送的仙石
	FIELD(UINT32,		Ticket);						//发送的礼卷
	FIELD(UINT32,		PolyNimbus);				    //聚灵气
	FIELD(TSynID,		SynID);							//帮派ID
	FIELD(UINT8,		bRead);							//是否已读
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

//获取玩家的所有邮件信息应答
struct SDB_Get_MailDataList_Rsp
{
	SDB_Get_MailDataList_Rsp(){
		memset(this, 0, sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(TMailID,			MailID);					//邮件ID
	FIELD(UINT64,			uid_SendUser);				//发件人UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	Name_SendUser);	//发件人名字
	FIELD(UINT8,			Mail_Type);						//邮件类型
	FIELD(UINT32,			Stone);						//发送的灵石
	FIELD(UINT32,			Money);						//发送的仙石
	FIELD(UINT32,			Ticket);					//发送的礼卷
	FIELD(UINT32,			PolyNimbus);				//聚灵气
	FIELD(TSynID,			SynID);						//邀请的帮派ID
	FIELD(TInt8Array<DESCRIPT_LEN_50>,	ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>, ContentText);	//文本内容
	FIELD(UINT64,			Time);						//时间
	FIELD(UINT8,			bRead);						//是否已读
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据
	

	FIELD_END();
};

//获取玩家的所有邮件信息
struct SDB_Get_MailDataList_Req : private DBReqPacketHeader
{
	SDB_Get_MailDataList_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_MailDataList_Req");
	FIELD(UINT64,		uid_User);						//玩家UID值

	FIELD_END();

	typedef  DB_OutParam				TResultSet;  //结果集
	typedef  SDB_Get_MailDataList_Rsp   TResultSet2; //结果集2
};

////加载指定区间的邮件邮件
struct SDB_Get_SectionMailData_Req : private DBReqPacketHeader
{
	SDB_Get_SectionMailData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SectionMailData");
	FIELD(UINT64,		uid_User);
	FIELD(INT16,		nBegin);
	FIELD(INT16,		nNum);

	FIELD_END();

	typedef  DB_OutParam				TResultSet;  //结果集
	typedef  SDB_Get_MailDataList_Rsp   TResultSet2; //结果集2
};

//删除超出的邮件
struct SDB_Delete_OverMail_Req : private DBReqPacketHeader
{
	SDB_Delete_OverMail_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_OverMail");
	FIELD(UINT64,		uid_User);

	FIELD_END();

	typedef ResultSetNull	TResultSet;  //结果集
	typedef ResultSetNull	TResultSet2;  //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//交易

//插入一条交易数据
struct SDB_Insert_TradeData_Req : private DBReqPacketHeader
{
	SDB_Insert_TradeData_Req()
	{
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_TradeData_Req");
	FIELD(UINT64,		uid_Goods);						//交易物品的UID值
	FIELD(UINT64,		uid_User);						//玩家的UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	szSellerName);	//卖家的名字
	FIELD(UINT32,		Price);							//出售价格
	FIELD(UINT8,		TradeLabel);					//物品所在标签
	FIELD(UINT8,		SubClass);						//子类别
	FIELD(UINT64,		Time);							//时间
	FIELD(TBinArray<sizeof(SDBGoodsData)>, GoodsData);	//创建物品所需的物品数据

	FIELD_END();

	typedef ResultSetNull	TResultSet;		//结果集
	typedef ResultSetNull	TResultSet2;	//结果集
};

//加载所有交易物品的信息
struct SDB_Get_TradeDataList_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,	uid_Goods);							//交易物品的UID值
	FIELD(UINT64,	uid_User);							//好友的UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	szSellerName);	//卖家的名字
	FIELD(UINT32,	Price);								//好友的好友度GoodsCategory
	FIELD(UINT8,	GoodsCategory);						//物品类别
	FIELD(UINT8,	SubClass);							//子类别
	FIELD(UINT64,	Time);								//时间
	FIELD(TBinArray<sizeof(SDBGoodsData)>, GoodsData);	//创建物品所需的物品数据

	FIELD_END();

};

//加载所有交易物品的信息
struct SDB_Get_TradeDataList_Req :  private DBReqPacketHeader
{
	SDB_Get_TradeDataList_Req()
	{
          this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_GetTradeDataListInfo");
	FIELD(INT32,    nValue);		//随便加的一个值

	FIELD_END();

	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_TradeDataList_Rsp	TResultSet2;  //结果集
};

//删除交易物品
struct SDB_Delete_TradeData_Req : private DBReqPacketHeader
{
	SDB_Delete_TradeData_Req()
	{
          this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_DeleteTradeDataInfo");
	FIELD(UINT64,			uid_Seller);			 //卖家的UID值
	FIELD(UINT64,			uid_Goods);				 //交易物品的UID值

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//帮派

//插入新帮派时返回的帮派ID
struct SDB_Get_SynID_Rsp
{
	FIELD_BEGIN();
	FIELD(TSynID,		SynID);	

	FIELD_END();
};

//插入新帮派
struct SDB_Insert_Syndicate_Req : private DBReqPacketHeader
{
	SDB_Insert_Syndicate_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_InsertSyndicateInfo");
	FIELD(TInt8Array<THING_NAME_LEN>,	szSynName);			//帮派名称
	FIELD(UINT64,						uid_Leader);		//帮主UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	szLeaderName);		//帮主名字

	FIELD_END();

	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_SynID_Rsp			TResultSet2; //结果集
};

//插入帮派成员数据
struct SDB_Insert_SyndicateMember_Req : private DBReqPacketHeader
{
	SDB_Insert_SyndicateMember_Req(){
		this->m_length = sizeof(*this);
		Position = enumSynPosition_General;
		Contribution = 0;
	}
	BIND_PROC_BEGIN("P_GDB_InsertSyndicateMemberInfo");
	FIELD(UINT64,						uid_User);			//玩家的UID值
	FIELD(TSynID,						SynID);				//玩家加入的帮派ID
	FIELD(TInt8Array<THING_NAME_LEN>,	UserName);			//玩家的名字
	FIELD(UINT8,						UserLevel);			//玩家的等级
	FIELD(UINT8,						Position);			//玩家在帮派的职位
	FIELD(UINT32,						Contribution);		//玩家的帮派贡献值
	FIELD(UINT8,						VipLevel);			//vip等级
	FIELD(UINT32,						LastOnlineTime);	//最后在线时间

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

struct SDB_Get_SyndicateRsp
{
	FIELD_BEGIN();
	FIELD(TSynID,						SynID);				//帮派ID
	FIELD(TInt8Array<THING_NAME_LEN>,	SynName);			//帮派名字
	FIELD(UINT64,						Uid_Leader);		//帮主UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	LeaderName);		//帮主名字
	FIELD(UINT8,						Level);				//帮派等级
	FIELD(UINT32,						Exp);				//帮派经验
	FIELD(UINT16,						MemberNum);			//帮派成员数量
	FIELD(UINT32,						SynWarScore);		//帮战积分
	FIELD(INT32,						SynWarAbility);		//帮战实力
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	szSynMsg);			//帮派公告
	FIELD(UINT8,						bWin);				//上一场帮战是否胜利
	FIELD(TInt8Array<THING_NAME_LEN>,	PreEnemySynName);	//上一场帮战敌对帮派名
	FIELD(UINT32,						SynWarTotalScore);  //帮战总积分

	FIELD_END();
};
//加载帮派信息
struct SDB_Get_SyndicateList_Req : private DBReqPacketHeader
{
	SDB_Get_SyndicateList_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_SyndicateListInfo");

	FIELD_END();

	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_SyndicateRsp		TResultSet2; //结果集
};

struct SDB_Get_SyndicateMember_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,						Uid_Member);	//帮派成员UID值
	FIELD(TSynID,						SynID);			//帮派ID
	FIELD(TInt8Array<THING_NAME_LEN>,	MemberName);	//帮派成员名字
	FIELD(UINT8,						MemberLevel);	//帮派成员等级
	FIELD(UINT8,						Position);		//在帮派的职位
	FIELD(UINT32,						Contribution);	//贡献值
	FIELD(UINT8,						VipLevel);		//vip等级
	FIELD(INT32,						CombatAbility);	//战斗力
	FIELD(UINT16,						ActorFacade);	//外观
	FIELD(INT32,						Score);			//帮派功勋	
	FIELD(INT32,						SynWarLv);		//帮战等级
	FIELD(UINT32,						LastOnlineTime);//最后在线时间

	FIELD_END();
};
//加载帮派成员信息
struct SDB_Get_SyndicateMemberList_Req : private DBReqPacketHeader
{
	SDB_Get_SyndicateMemberList_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_SyndicateMemberListInfo");

	FIELD_END();

	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_SyndicateMember_Rsp	TResultSet2; //结果集
};

struct SDB_Get_SyndicateApply_Rsp
{
	FIELD_BEGIN();
	FIELD(TSynID,						SynID);
	FIELD(UINT64,						Uid_ApplyUser);		//帮派加入申请者的UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	ApplyUserName);		//帮派加入申请者名字
	FIELD(UINT8,						ApplyUserLevel);	//帮派加入申请者等级
	FIELD(UINT8,						ApplyUserLayer);	//帮派加入申请者的境界
	FIELD(UINT32,						LastOnlineTime);	//最后在线时间

	FIELD_END();
};
//加载帮派加入申请信息
struct SDB_Get_SyndicateApplyList_Req : private DBReqPacketHeader
{
	SDB_Get_SyndicateApplyList_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Get_SyndicateApplyListInfo");

	FIELD_END();

	typedef DB_OutParam					TResultSet;  //结果集
	typedef SDB_Get_SyndicateApply_Rsp	TResultSet2; //结果集
};

//插入帮派加入申请信息
struct SDB_Insert_SyndicateApply_Req : private DBReqPacketHeader
{
	SDB_Insert_SyndicateApply_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_InsertSyndicateApplyInfo");
	FIELD(TSynID,						SynID);				//帮派ID
	FIELD(UINT64,						Uid_ApplyUser);		//申请加入者的UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	ApplyUserName);		//申请加入者的名字
	FIELD(UINT8,						ApplyUserLevel);	//申请加入者的等级
	FIELD(UINT8,						ApplyUserLayer);	//申请加入者的境界
	FIELD(UINT32,						LastOnlineTime);	//最后在线时间

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

//删除帮派加入申请信息
struct SDB_Delete_SyndicateApply_Req : private DBReqPacketHeader
{
	SDB_Delete_SyndicateApply_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_DeleteSyndicateApplyInfo");
	FIELD(TSynID,						SynID);				//帮派ID
	FIELD(UINT64,						uid_ApplyUser);		//申请者的UID值

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

//删除帮派成员信息
struct SDB_Delete_SyndicateMember_Req : private DBReqPacketHeader
{
	SDB_Delete_SyndicateMember_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_DeleteSyndicateMember");
	FIELD(TSynID,						SynID);				//帮派ID
	FIELD(UINT64,						uid_QuitUser);		//申请者的UID值

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集	
};

//更新帮派成员信息
struct SDB_Update_SyndicateMember_Req : private DBReqPacketHeader
{
	SDB_Update_SyndicateMember_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_UpdataSyndicateMember");
	FIELD(UINT64,						Uid_Member);		//帮派成员UID值
	FIELD(UINT8,						Position);			//在帮派的职位
	FIELD(UINT32,						Contribution);		//贡献值
	FIELD(INT32,						CombatAbility;)		//战斗力
	FIELD(INT32,						Score);				//帮派功勋	
	FIELD(UINT32,						LastOnlineTime);	//最后在线时间

	FIELD_END();


	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

//更新帮派信息
struct SDB_Updata_Syndicate_Req : private DBReqPacketHeader
{
	SDB_Updata_Syndicate_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_UpdataSyndicate");
	FIELD(TSynID,						SynID);							//帮派ID
	FIELD(TInt8Array<THING_NAME_LEN>,	SynName);						//帮派名字
	FIELD(UINT64,						Uid_Leader);					//帮主UID值
	FIELD(TInt8Array<THING_NAME_LEN>,	LeaderName);					//帮主名字
	FIELD(UINT8,						Level);							//帮派等级
	FIELD(UINT32,						Exp);							//帮派经验
	FIELD(UINT16,						MemberNum);						//帮派成员数量
	FIELD(UINT32,						SynWarScore);					//帮战积分
	FIELD(INT32,						SynWarAbility);					//帮战实力
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	szSynMsg);						//帮派公告
	FIELD(UINT32,						SynWarTotalScore);				//帮战总积分

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

//删除帮派信息
struct SDB_Delete_Syndicate_Req : private DBReqPacketHeader
{
	SDB_Delete_Syndicate_Req(){
		this->m_length = sizeof(*this);
	}	

	BIND_PROC_BEGIN("P_GDB_Delete_Syndicate");
	FIELD(TSynID,						SynID);			//帮派ID

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//测试
struct SDB_Insert_Bigint_Req : private DBReqPacketHeader
{
	SDB_Insert_Bigint_Req(){
		this->m_length = sizeof(*this);
	}
	BIND_PROC_BEGIN("P_GDB_Insert_bigint_test");
	FIELD(UINT64,						Bigint);

	typedef ResultSetNull					TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

struct STestStruct2
{
	BIND_PROC_BEGIN("P_TestProc");
	FIELD(UINT16,                          m_GoodsNum);
	FIELD(TBinArray<sizeof(SDBGoodsData)*100>, m_GoodsData);
	FIELD_END();

	TRESULT_SET(DB_OutParam,ResultSetNull);
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//状态
//更新
struct SDB_Update_Status_Req : private DBReqPacketHeader
{
	SDB_Update_Status_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_StatusInfo");
	FIELD(UINT64,				m_uidUser);
	FIELD(TStatusID,			m_StatusID);
	FIELD(UINT32,				m_EndStatusTime);
	FIELD(UINT64,				m_UidCreator);		//源角色UID,即是谁发起或者创建的这个状态
	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

struct SDB_Get_StatusNum_Rsp
{
	SDB_Get_StatusNum_Rsp() : m_uidUser(UID().ToUint64()), m_StatusNum(0)
	{
	}

	FIELD_BEGIN();
	FIELD(UINT64,				m_uidUser);
	FIELD(INT32,				m_StatusNum);
	FIELD_END();
};

struct SDB_Get_StatusData_Rsp
{
	FIELD_BEGIN();
	FIELD(TStatusID,			m_StatusID);
	FIELD(UINT32,				m_EndStatusTime);
	FIELD(UINT64,				m_UidCreator);		//源角色UID,即是谁发起或者创建的这个状态
	FIELD_END();
};
//获取
struct SDB_Get_Status_Req : private DBReqPacketHeader
{
	SDB_Get_Status_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_StatusInfo");
	FIELD(UINT64,				m_uidUser);
	FIELD_END();

	typedef SDB_Get_StatusNum_Rsp				TResultSet;  //结果集
	typedef SDB_Get_StatusData_Rsp				TResultSet2; //结果集
};

//删除状态
struct SDB_Delete_AllStatus_Req : private DBReqPacketHeader
{
	SDB_Delete_AllStatus_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_AllStatus");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//查看帮派列表时,得到帮众的等级

struct SDB_Get_SynMemberListNum_Rsp
{
	FIELD_BEGIN();
	FIELD(INT32,			m_Num);
	FIELD_END();
};

struct SDB_Get_SynMemberListLevel_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,			m_uidUser);
	FIELD(UINT8,			m_Level);

	FIELD_END();
};

struct SDB_Get_SynMemberListLevel_Req : private DBReqPacketHeader
{
	SDB_Get_SynMemberListLevel_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetSynMemberLevelInfo");
	FIELD(UINT16,			m_SynID);		//帮派等级

	FIELD_END();

	typedef SDB_Get_SynMemberListNum_Rsp		TResultSet;  //结果集
	typedef SDB_Get_SynMemberListLevel_Rsp		TResultSet2; //结果集
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//斗法

struct SDB_GetRandEnemyNum_Rsp
{
	FIELD_BEGIN();
	FIELD(int,				m_Num);	//数据库返回的敌人数量
	FIELD_END();
};

struct SDB_GetRandEnemyUID_Rsp
{
	FIELD_BEGIN();
	FIELD(UINT64,			m_UidEnemy);
	FIELD_END();
};

//获取符合规则的3个玩家
struct SDB_GetDouFaEnemy_Req : private DBReqPacketHeader
{
	SDB_GetDouFaEnemy_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetDouFaRand5Enemy");
	FIELD(UINT64,			m_UserUID);

	FIELD_END();

	typedef SDB_GetRandEnemyNum_Rsp		TResultSet;  //结果集
	typedef SDB_GetRandEnemyUID_Rsp		TResultSet2; //结果集
};

struct DB_DouFaEnemyInfo
{
	DB_DouFaEnemyInfo(){
		memset(this, 0, sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(UINT64,	m_uidEnemy);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_EnemyName);	//名字
	FIELD(UINT16,	m_Facade);
	FIELD(UINT8,	m_bFinish);
	FIELD(UINT8,	m_bWin);
	FIELD(UINT8,	m_VipLevel);

	FIELD_END();
};

struct DB_QieCuoEnemyInfo
{
	DB_QieCuoEnemyInfo(){
		memset(this, 0, sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(UINT64,	m_uidEnemy);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_EnemyName);	//名字
	FIELD(UINT8,	m_Level);					//等级
	FIELD(UINT32,	m_Layer);					//境界
	FIELD(TTitleID,	m_TitleID);					//称号ID
	FIELD(UINT8,	m_bFinish);
	FIELD(UINT8,	m_bWin);
	FIELD(UINT8,	m_VipLevel);

	FIELD_END();
};

//斗法的挑战和切磋对手数据
struct DB_DouFaQieCuoEnemyData
{
	DB_DouFaQieCuoEnemyData(){
		memset(this, 0, sizeof(*this));
	}

	DB_DouFaEnemyInfo	m_DouFaEnemy[DOUFA_ENEMY_NUM];
	DB_QieCuoEnemyInfo	m_QieCuoEnemy[QIECUO_ENEMY_NUM];

};


struct SDB_Get_DouFaPartInfo_Rsp
{
	SDB_Get_DouFaPartInfo_Rsp()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(UINT64,			m_UserUID);
	FIELD(INT32,			m_GetHonorToday);
	FIELD(UINT32,			m_LastGetHonorTime);			//最后一次获得荣誉的时间
	FIELD(UINT16,			m_JoinChallengeNum);			//今日参加的挑战次数
	FIELD(UINT32,			m_LastJoinChallengeTime);		//最后一次参加挑战的时间
	FIELD(UINT16,			m_MaxChallengeNumToday);		//今日最多可参加的挑战次数
	FIELD(UINT8,			m_LastFLushDouFaUpLevel);		//上次抽取斗法玩家的等级上限
	FIELD(TBinArray<sizeof(DB_DouFaQieCuoEnemyData)>, m_DouFaQieCuoEnemy);	//斗法的挑战和切磋对手数据
	FIELD(UINT32,			m_CreditUp);					//本周声望上限
	FIELD(UINT32,			m_GetCredit);					//本周获得声望
	FIELD(UINT32,			m_LastChangeCreditUp);			//最后一次修改声望上限的时间

	FIELD_END();
};


//获取斗法PART信息
struct SDB_Get_DouFaPartInfo_Req : private DBReqPacketHeader
{
	SDB_Get_DouFaPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetDouFaInfo");
	FIELD(UINT64,			m_UserUID);

	FIELD_END();

	typedef DB_OutParam						TResultSet;  //结果集
	typedef SDB_Get_DouFaPartInfo_Rsp		TResultSet2; //结果集
};

/*
//插入斗法PART信息
struct SDB_Insert_DouFaPartInfo_Req : private DBReqPacketHeader
{
	SDB_Insert_DouFaPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertDouFaInfo");
	FIELD(UINT64,			m_UserUID);

	FIELD_END();

	typedef DB_OutParam						TResultSet;  //结果集
	typedef SDB_Get_DouFaPartInfo_Rsp		TResultSet2; //结果集
};
*/

//更新斗法信息
struct SDB_Update_DouFaPartInfo_Req : private DBReqPacketHeader
{
	SDB_Update_DouFaPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_UpdateDouFaInfo");
	FIELD(UINT64,			m_UserUID);
	FIELD(INT32,			m_GetHonorToday);				//今天获得的荣誉值
	FIELD(UINT32,			m_LastGetHonorTime);			//最后一次获得荣誉的时间
	FIELD(UINT16,			m_JoinChallengeNum);			//今日参加的挑战次数
	FIELD(UINT32,			m_LastJoinChallengeTime);		//最后一次参加挑战的时间
	FIELD(UINT16,			m_MaxChallengeNumToday);		//今日最多可参加的挑战次数
	FIELD(UINT8,			m_LastFLushDouFaUpLevel);		//上次抽取斗法玩家的等级上限
	FIELD(TBinArray<sizeof(DB_DouFaQieCuoEnemyData)>, m_DouFaQieCuoEnemy);	//斗法的挑战和切磋对手数据
	FIELD(UINT32,			m_CreditUp);					//本周声望上限
	FIELD(UINT32,			m_GetCredit);					//本周获得声望
	FIELD(UINT32,			m_LastChangeCreditUp);			//最后一次修改声望上限的时间

	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩家身上不保存的任务
struct DB_NoSaveTask
{
	FIELD_BEGIN();
	FIELD(TTaskID,		m_TaskID);
	FIELD_END();
};

struct DB_TaskPartData
{
	DB_TaskPartData(){
		memset(this, 0, sizeof(*this));
	}
	FIELD_BEGIN();
	FIELD(TUint8Array<(MAX_TASK_ID+7)/8>     ,m_TaskHistory);   //所做过的任务
	FIELD(UINT32                         ,m_LastUpdateTime); //最后更新任务时间
	FIELD(UINT8							 ,m_NoSaveNum);		//玩家身上不保存的任务数量
	FIELD(UINT8							 ,m_TaskStatus)		//任务主界面显示状态
	FIELD(TBinArray<sizeof(DB_NoSaveTask) * MAX_TASKNUM>, m_NoSaveTask);	//不保存任务数据的任务

	FIELD_END();
};

//单个任务的数据
struct DB_TaskData
{
	FIELD_BEGIN();
	FIELD(TTaskID                        ,m_TaskID);   //任务ID
	FIELD(UINT32                         ,m_FinishTime);	//任务完成时间
	FIELD(UINT64                         ,m_TaskCount); //任务计数

	FIELD_END();
};

struct DB_TaskNum
{
	FIELD_BEGIN();
	FIELD(INT32                        ,m_TaskNum);   //任务数量

	FIELD_END();
};

//获取任务PART及玩家任务信息
struct SDB_Get_TaskPartInfo_Req : private DBReqPacketHeader
{
	SDB_Get_TaskPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_TaskInfo");
	FIELD(UINT64,			m_UserUID);

	FIELD_END();

	typedef DB_TaskPartData						TResultSet;  //结果集
	typedef DB_TaskData		                    TResultSet2; //结果集
};

//插入任务PART
struct SDB_Insert_TaskPartInfo_Req : private DBReqPacketHeader
{
	SDB_Insert_TaskPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_TaskInfo");
	FIELD(UINT64,			m_UserUID);

	FIELD_END();

	typedef DB_TaskPartData						TResultSet;  //结果集
	typedef DB_TaskData		                    TResultSet2; //结果集
};

//更新任务PART
struct SDB_Update_TaskPartInfo_Req : private DBReqPacketHeader
{
	SDB_Update_TaskPartInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_TaskPartInfo");
	FIELD(UINT64,						m_UserUID);
	FIELD(TUint8Array<(MAX_TASK_ID+7)/8>,	m_TaskHistory);	//所做过的任务
	FIELD(UINT32,						m_LastUpdateTime);	//最后更新任务时间
	FIELD(UINT8,						m_NotSaveNum);		//玩家身上不保存的任务数量
	FIELD(UINT8,						m_TaskStatus)		//任务主界面显示状态
	FIELD(VarBinary,					m_VarBinary);		//下面保存不用保存的任务ID

	FIELD_END();

	typedef DB_TaskPartData						TResultSet;  //结果集
	typedef DB_TaskData		                    TResultSet2; //结果集	
};

//保存任务数据
struct SDB_Update_TaskInfo_Req : private DBReqPacketHeader
{
	SDB_Update_TaskInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_TaskInfo");
	FIELD(UINT64,			m_UserUID);
	FIELD(UINT8,			m_TaskNum);
	FIELD(VarBinary,		m_VarBinary);	//
	FIELD_END();

	typedef ResultSetNull				TResultSet;  //结果集
	typedef ResultSetNull				TResultSet2; //结果集
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DB_GetActorFacade
{
	FIELD_BEGIN();
	FIELD(UINT64,		m_ActorUID);
	FIELD(UINT32,		m_ActorFacade);
	FIELD_END();
};

//得到玩家外观
struct SDB_GetActorFacade_Req : private DBReqPacketHeader
{
	SDB_GetActorFacade_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_GetActorFacade");
	FIELD(UINT64,			m_ActorUID);
	FIELD_END();

	typedef DB_OutParam				TResultSet;  //结果集
	typedef DB_GetActorFacade		TResultSet2; //结果集
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//成就

//成就PART数据
struct DB_ChengJiuPart
{
	DB_ChengJiuPart() : m_uidUser(UID().ToUint64()), m_ChengJiuPoint(0), m_GetChengJiuNum(0), m_ActiveTitleID(0)
	{
	}

	FIELD_BEGIN();
	FIELD(UINT64,		m_uidUser);
	FIELD(INT32,		m_ChengJiuPoint);	//获得的成就点
	FIELD(INT32,		m_GetChengJiuNum);	//已获得成就数量
	FIELD(UINT8,		m_ActiveTitleID);	//当前启用的称号ID

	FIELD_END();
};

//单个成就数据
struct DB_ChengJiu
{
	FIELD_BEGIN();
	FIELD(TChengJiuID,  m_ChengJiuID);	//成就ID
	FIELD(UINT16,		m_CurCount);	//当前计数
	FIELD(UINT32,		m_FinishTime);	//达成时间

	FIELD_END();
};

//获得成就数据
struct SDB_Get_ChengJiuData_Req : private DBReqPacketHeader
{
	SDB_Get_ChengJiuData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChengJiuData");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef DB_ChengJiuPart			TResultSet;  //结果集
	typedef DB_ChengJiu				TResultSet2; //结果集
};

//创建角色时插入成就数据
struct SDB_Insert_ChengJiuPart_Req : private DBReqPacketHeader
{
	SDB_Insert_ChengJiuPart_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_ChengJiuPart");
	FIELD(INT64,		m_uidUser);

	FIELD_END();

	typedef DB_ChengJiuPart			TResultSet;  //结果集
	typedef DB_ChengJiu				TResultSet2; //结果集
};

//更新成就PART
struct SDB_Update_ChengJiuPart_Req : private DBReqPacketHeader
{
	SDB_Update_ChengJiuPart_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_ChengJiuPart");
	FIELD(UINT64,		m_uidUser);
	FIELD(INT32,		m_ChengJiuPoint);
	FIELD(INT32,		m_GetChengJiuNum);
	FIELD(UINT8,		m_ActiveTitleID);

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//更新单个成就
struct SDB_Update_ChengJiu_Req : private DBReqPacketHeader
{
	SDB_Update_ChengJiu_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_ChengJiu");
	FIELD(UINT64,		m_uidUser);
	FIELD(TChengJiuID,	m_ChengJiuID);
	FIELD(UINT16,		m_CurCount);
	FIELD(UINT32,		m_FinishTime);

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SDB_GetActivityPart_Rsp
{
	SDB_GetActivityPart_Rsp()
	{
		m_SignInNumOfMonth = 0;
		m_LastSignInTime = 0;
		m_OnLineAwardID = 0;
		m_LastOnLineAwardRestTime = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT8        ,m_SignInNumOfMonth); //月签数
	FIELD(UINT32       ,m_LastSignInTime);   //最后一次签到
	FIELD(TBinArray<(MAX_DAILY_AWARD_NUM+7)/8>  ,m_AwardHistory); //领奖记录
	FIELD(UINT16       ,m_OnLineAwardID);    //在线奖励
	FIELD(UINT32       ,m_LastOnLineAwardRestTime); //在线奖励，最后复位时间
	FIELD(UINT8        ,m_bCanTakeOnlineAward);  //是否可以领取在线奖励
	FIELD(UINT8        ,m_OffLineNum); //连接下线次数
	FIELD(TInt8Array<50> ,m_szGuideContext); //新手指引上下文
//	FIELD(INT16        ,m_NewPlayerGuideIndex);

	FIELD_END();
};

struct SDB_ActivityData
{
	SDB_ActivityData()
	{
		MEM_ZERO(this);
	}
	FIELD_BEGIN();
	FIELD(UINT16        ,m_ActivityID); //活动ID
	FIELD(UINT8        ,m_bFinished); //是否已完成
	FIELD(UINT8        ,m_bTakeAward); //是否已领奖
	FIELD(UINT32        ,m_ActivityProgress); //进度
	FIELD_END();
};


//获得活动数据
struct SDB_Get_ActivityData_Req : private DBReqPacketHeader
{
	SDB_Get_ActivityData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ActivityData");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef SDB_GetActivityPart_Rsp			TResultSet;  //结果集
	typedef SDB_ActivityData				TResultSet2; //结果集
};


//更新活动part数据

struct SDB_UpdateActivityPart_Req: private DBReqPacketHeader
{
	SDB_UpdateActivityPart_Req(){
		this->m_length = sizeof(*this);
	}
	
	BIND_PROC_BEGIN("P_GDB_Update_ActivityPart");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT8        ,m_SignInNumOfMonth); //月签数
	FIELD(UINT32       ,m_LastSignInTime);   //最后一次签到
	FIELD(TBinArray<(MAX_DAILY_AWARD_NUM+7)/8>  ,m_AwardHistory); //领奖记录
	FIELD(UINT16       ,m_OnLineAwardID);    //在线奖励
	FIELD(UINT32       ,m_LastOnLineAwardRestTime); //在线奖励，最后复位时间
	FIELD(UINT8        ,m_bCanTakeOnlineAward);  //是否可以领取在线奖励
	FIELD(UINT8        ,m_OffLineNum); //连接下线次数
	//FIELD(INT16        ,m_NewPlayerGuideIndex);
	FIELD(TInt8Array<50> ,m_szGuideContext); //新手指引上下文

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//更新单个活动数据
struct SDB_ActivityData_Req: private DBReqPacketHeader
{
	SDB_ActivityData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_ActivityData");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT16        ,m_ActivityID); //活动ID
	FIELD(UINT8        ,m_bFinished); //是否已完成
	FIELD(UINT8        ,m_bTakeAward); //是否已领奖
	FIELD(UINT32        ,m_ActivityProgress); //进度
	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//保存背包PART数据
struct SDB_UpdatePacketPartReq : private DBReqPacketHeader
{
	SDB_UpdatePacketPartReq(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_PacketPart");
	FIELD(UINT64,	m_uidUser);
	FIELD(INT16,	m_Capacity);				//容量
	FIELD(INT16,	m_GoodsNum);				//物品数量

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//保存单个物品
struct SDB_UpdateGoodsReq : private DBReqPacketHeader
{
	SDB_UpdateGoodsReq(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_PacketGoods");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT64,		m_uidGoods);	//UID标识物品
	FIELD(TGoodsID,		mGoodsID);		//GoodsID
	FIELD(UINT32,		m_CreateTime);	//创建时间
	FIELD(UINT8,		m_Number);		//数量
	FIELD(bool,			m_Binded);		//是否已绑定
	FIELD(TBinArray<sizeof(SDBTalismanProp)>, GoodsData);	//物品信息

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//删除背包单个物品
struct SDB_DeleteGoodsReq : private DBReqPacketHeader
{
	SDB_DeleteGoodsReq(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_DeletePacketGoods");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT64,		m_uidGoods);	//UID标识物品

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//更新任务PART数据
struct SDB_Update_TaskPartReq : private DBReqPacketHeader
{
	SDB_Update_TaskPartReq(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_TaskPart");
	FIELD(UINT64,		m_UidUser);

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//保存
struct SDB_Save_GoodsReq : private DBReqPacketHeader
{
	SDB_Save_GoodsReq() : m_uidUser(UID().ToUint64()), m_uidGoods(UID().ToUint64()), m_GoodsID(0), m_CreateTime(0), m_Number(0), m_Binded(false), m_Location(0), m_Pos(0)
	{
		this->m_length = sizeof(*this);
		memset(&GoodsData, 0, sizeof(GoodsData));
	}
	
	BIND_PROC_BEGIN("P_GDB_Update_Goods");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT64,		m_uidGoods);	//UID标识物品
	FIELD(TGoodsID,		m_GoodsID);		//GoodsID
	FIELD(UINT32,		m_CreateTime);	//创建时间
	FIELD(UINT8,		m_Number);		//数量
	FIELD(bool,			m_Binded);		//是否已绑定
	FIELD(UINT8,		m_Location);	//装备栏的位置
	FIELD(UINT8,		m_Pos);			//物品所在位置:0为背包,1为装备栏
	FIELD(TBinArray<sizeof(SDBTalismanProp)>, GoodsData);	//物品信息

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//删除物品
struct SDB_Delete_GoodsReq : private DBReqPacketHeader
{
	SDB_Delete_GoodsReq() : m_uidUser(UID().ToUint64()), m_uidGoods(UID().ToUint64()), m_Pos(0)
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_Goods");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT64,		m_uidGoods);
	FIELD(UINT8,		m_Pos);	//0为背包,1为装备
	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//排行榜

struct SDB_UserRankActorInfo
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_uidUser);
	FIELD(UINT8,	m_Level);						//玩家等级
	FIELD(UINT32,	m_Exp);							//玩家经验
	FIELD(INT32,	m_Credit);						//玩家声望
	FIELD(UINT8,	m_Layer);						//玩家等级
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);	//玩家名字
	
	FIELD_END();
};

//加载玩家排行榜
struct SDB_Load_RankReq : private DBReqPacketHeader
{
	SDB_Load_RankReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_UserRand");

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_UserRankActorInfo	TResultSet2;  //结果集
};

///////////////////////////////////////////////////////////////////
//冷却

struct SDB_CDTimerData
{
	FIELD_BEGIN();
	FIELD(TCDTimerID,	m_CDTimerID);
	FIELD(UINT32,		m_EndTime);

	FIELD_END();
};

struct SDB_CDTimerActor
{
	FIELD_BEGIN();
	FIELD(UINT64,		m_uidActor);

	FIELD_END();
};

//获取玩家的冷却时间
struct SDB_Get_UserCDTimer_Req : private DBReqPacketHeader
{
	SDB_Get_UserCDTimer_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_CDTimer");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef SDB_CDTimerActor		TResultSet;   //结果集
	typedef SDB_CDTimerData			TResultSet2;  //结果集
};

//更新冷却时间
struct SDB_Update_UserCDTimer_Req : private DBReqPacketHeader
{
	SDB_Update_UserCDTimer_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_CDTimer");
	FIELD(UINT64,		m_uidUser);
	FIELD(TCDTimerID,   m_CDTimerID);
	FIELD(UINT32,		m_EndTime);

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//删除冷却时间
struct SDB_Delete_UserCDTimer_Req : private DBReqPacketHeader
{
	SDB_Delete_UserCDTimer_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_CDTimer");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef ResultSetNull			TResultSet;  //结果集
	typedef ResultSetNull			TResultSet2; //结果集
};

//////////////////////////////////////////////////////////////////////////////

struct SDB_SysMsgInfo
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_SysMsgID);
	FIELD(UINT8,		m_MsgType);						//类型:0聊天框,1主界面
	FIELD(UINT32,		m_BeginTime);					//开始时间
	FIELD(UINT32,		m_EndTime);						//结束时间
	FIELD(UINT32,		IntervalTime);					//发送间隔时间，单位：秒(聊天框用到)
	FIELD(TInt8Array<DESCRIPT_LEN_500>,  MsgContent);	//消息内容
	FIELD(UINT8,		m_State);						//0:启用 1:暂停
	FIELD_END();
};

//获取系统消息
struct SDB_Get_SysMsg_Req : private DBReqPacketHeader
{
	SDB_Get_SysMsg_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SysMsg");
	FIELD(UINT32,		m_SysMsgID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_SysMsgInfo			TResultSet2;  //结果集
};
//////////////////////////////////////////////////////////////////////////////////////////////

struct SDB_MailNum
{
	FIELD_BEGIN();
	FIELD(UINT16,	m_MailNum);

	FIELD_END();
};

//得到邮件数量
struct SDB_Get_MailNum_Req : private DBReqPacketHeader
{
	SDB_Get_MailNum_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MailNum");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef SDB_MailNum				TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//插入到玩家排行表
struct SDB_Insert_UserRank : private DBReqPacketHeader
{
	SDB_Insert_UserRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertUserRank");
	FIELD(UINT64,	m_uidUser);

	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

struct  SDB_UserRankData
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_Rank);
	FIELD(UINT64,	m_uidUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);
	FIELD(UINT8,	m_Level);
	FIELD(UINT32,	m_Exp);
	FIELD(UINT32,	m_Layer);
	FIELD(UINT8,	m_vipLevel);

	FIELD_END();
};

//得到我的排行
struct SDB_Get_MyRank : private DBReqPacketHeader
{
	SDB_Get_MyRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MyRank");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT16,		m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_UserRankData		TResultSet2;	//结果集
};

//得到排行
struct SDB_Get_UserRank : private DBReqPacketHeader
{
	SDB_Get_UserRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_RankSection");
	FIELD(UINT32,		m_BeginRank);
	FIELD(UINT16,		m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_UserRankData		TResultSet2;	//结果集
};

//更新排行
struct SDB_Update_UserRank : private DBReqPacketHeader
{
	SDB_Update_UserRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_UserRank");
	
	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//插入到夺宝排行表
struct SDB_Insert_DuoBaoRank : private DBReqPacketHeader
{
	SDB_Insert_DuoBaoRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_InsertDuobaoRank");
	FIELD(UINT64,	m_uidUser);

	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

struct  SDB_DuoBaoRankData
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_Rank);
	FIELD(UINT64,	m_uidUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);
	FIELD(UINT8,	m_Level);
	FIELD(UINT32,	m_CombatAbility);
	FIELD(INT32,	m_DuoBaoLevel);
	FIELD(UINT8,	m_VipLevel);

	FIELD_END();
};

//得到我的夺宝排行
struct SDB_Get_MyDuoBaoRank : private DBReqPacketHeader
{
	SDB_Get_MyDuoBaoRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MyDuoBaoRank");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT16,		m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_UserRankData		TResultSet2;	//结果集
};

//得到夺宝排行
struct SDB_Get_DuoBaoRank : private DBReqPacketHeader
{
	SDB_Get_DuoBaoRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_DuoBaoRank");
	FIELD(UINT32,		m_BeginRank);
	FIELD(UINT16,		m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_UserRankData		TResultSet2;	//结果集
};

//更新夺宝排行
struct SDB_Update_DuoBaoRank : private DBReqPacketHeader
{
	SDB_Update_DuoBaoRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_DuoBaoRank");
	
	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

//////////////////////////////////////////////////////////////////////////////////////////////////

struct SDB_ChallengeRankInfo
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_Rank);
	FIELD(UINT64,	m_uidUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);
	FIELD(UINT8,	m_Level);
	FIELD(UINT32,	m_Layer);
	FIELD(UINT8,	m_TitleID);
	FIELD(UINT16,	m_ChallengeNum);
	FIELD(UINT8,	m_VipLevel);
	FIELD(INT32,	m_DuoBaoLevel);

	FIELD_END();
};

//得到等级区间挑战排行
struct SDB_Get_ChallengeLevelRank : private DBReqPacketHeader
{
	SDB_Get_ChallengeLevelRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChallengeLevelRank");
	FIELD(UINT8,	m_BeginLevel);
	FIELD(UINT8,	m_EndLevel);
	FIELD(INT32,	m_BeginPos);
	FIELD(INT32,	m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef SDB_ChallengeRankInfo	TResultSet2;	//结果集
};


struct SDB_ChallengeLevelTotalNum
{
	FIELD_BEGIN();

	FIELD(INT32,	m_TotalNum);

	FIELD_END();
};

//得到等级区间挑战排行总人数
struct SDB_Get_ChallengeLevelTotalNum : private DBReqPacketHeader
{
	SDB_Get_ChallengeLevelTotalNum(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChallengeLevelTotalNum");
	FIELD(UINT8,	m_BeginLevel);
	FIELD(UINT8,	m_EndLevel);

	FIELD_END();

	typedef SDB_ChallengeLevelTotalNum				TResultSet;	//结果集
	typedef ResultSetNull							TResultSet2;   //结果集
	
};

struct SDB_MyChallengeRank
{
	FIELD_BEGIN();
	FIELD(INT32,   m_RetCode);
	FIELD(INT32,   m_MyRank);
	FIELD(INT32,   m_MyRankLevel);

	FIELD_END();
};

//得到我可挑战的对手ChallengeMyEnemy
struct SDB_Get_ChallengeMyEnemy : private DBReqPacketHeader
{
	SDB_Get_ChallengeMyEnemy(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChallengeMyEnemy");
	FIELD(UINT64,	m_UidUser);
	FIELD(INT32,	m_Num);

	FIELD_END();

	typedef SDB_MyChallengeRank		TResultSet;   //结果集
	typedef SDB_ChallengeRankInfo	TResultSet2;	//结果集
};



struct SDB_MyRank
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_Rank);

	FIELD_END();
};

//插入到挑战排行
struct SDB_Insert_ChallengeRank : private DBReqPacketHeader
{
	SDB_Insert_ChallengeRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_ChallengeRank");
	FIELD(UINT64,	m_UidUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_MyRank					TResultSet2;	//结果集
};

//更换挑战排行榜排名
struct SDB_Change_ChallengeRank : private DBReqPacketHeader
{
	SDB_Change_ChallengeRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Change_ChallengeRank");
	FIELD(UINT64,	m_UidUser1);
	FIELD(UINT64,	m_UidUser2);

	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

//周日晚更新挑战排行榜所有玩家等级
struct SDB_Update_AllUserLevelChallenge : private DBReqPacketHeader
{
	SDB_Update_AllUserLevelChallenge(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_AllUserLevelChallenge");

	FIELD_END();

	typedef ResultSetNull			TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

struct SDB_Check_CanChallenge : private DBReqPacketHeader
{
	SDB_Check_CanChallenge(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Check_CanChallenge");
	FIELD(UINT64,	m_uidMe);
	FIELD(UINT64,	m_uidEnemy);
	FIELD(INT32,	m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;   //结果集
	typedef ResultSetNull			TResultSet2;  //结果集
};

//得到总挑战排行
struct SDB_Get_ChallengeRank : private DBReqPacketHeader
{
	SDB_Get_ChallengeRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChallenteRank");
	FIELD(INT32,	m_BeginPos);
	FIELD(INT32,	m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_ChallengeRankInfo	TResultSet2;	//结果集
};

//得到我的挑战排行
struct SDB_Get_MyChallengeRank : private DBReqPacketHeader
{
	SDB_Get_MyChallengeRank(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MyChallengeRank");
	FIELD(UINT64,	m_uidUser);
	FIELD(INT16,	m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_ChallengeRankInfo	TResultSet2;	//结果集
};


////////////////////////////////////////////////////////////////////////////////////////

//我的前n场挑战信息
struct SDB_MyChallengeRecordInfo : private DBReqPacketHeader
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_uidUser);
	FIELD(UINT64,	m_uidEnemy);
	FIELD(UINT32,	m_Rank);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_EnemyName);	
	FIELD(UINT8,	m_EnemyLevel);
	FIELD(bool,		m_bWin);
	FIELD(bool,		m_bActive);
	FIELD(UINT8,		m_VipLevel);
	FIELD(UINT16,	m_EnemyFacade);

	FIELD_END();

};

//得到我的前n场挑战信息
struct SDB_Get_MyChallengeRecord : private DBReqPacketHeader
{
	SDB_Get_MyChallengeRecord(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MyChallengeRecord");
	FIELD(UINT64,	m_uidUser);
	FIELD(UINT32,	m_Num);

	FIELD_END();

	typedef DB_OutParam		TResultSet;   //结果集
	typedef SDB_MyChallengeRecordInfo	TResultSet2;	//结果集
};


//我的前n场战斗buf(一场)
struct SDB_ChallengeRecordBuf : private DBReqPacketHeader
{
	FIELD_BEGIN();
	FIELD(int,		m_BufLen);
	FIELD(TBinArray<sizeof(OBuffer4k)>, m_RecordBuf);	//战斗buf
	
	FIELD_END();

};

//得到我的前n场战斗buf（一场）
struct SDB_Get_ChallengeRecordBuf : private DBReqPacketHeader
{
	SDB_Get_ChallengeRecordBuf(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ChallengeRecordBuf");
	FIELD(UINT32,	m_Rank);

	FIELD_END();

	typedef DB_OutParam		TResultSet;   //结果集
	typedef SDB_ChallengeRecordBuf	TResultSet2;	//结果集
};

struct SDB_NewRecord
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_Rank);

	FIELD_END();
};

//插入到前n场挑战信息
struct SDB_Insert_MyChallengeRecord : private DBReqPacketHeader
{
	SDB_Insert_MyChallengeRecord(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_MyChallengeRecord");
	FIELD(UINT64,	m_UidUser);
	FIELD(UINT64,	m_UidEnemy);
	FIELD(bool,		m_bWin);
	FIELD(bool,		m_bActive);
	FIELD(int,		m_BufLen);
	FIELD(VarBinary,	m_RecordBuf);				//可变长
	
	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_NewRecord					TResultSet2;	//结果集
};

//////////////////////////////////////////////////////////////////////////////////////


struct SDB_Permission
{
	FIELD_BEGIN();
	FIELD(TInt8Array<DESCRIPT_LEN_100>,	m_Permission);

	FIELD_END();
};

//得到玩家的GM命令权限
struct SDB_Get_UserCrlPermission : private DBReqPacketHeader
{
	SDB_Get_UserCrlPermission(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetUserPermission");
	FIELD(TUserID,	m_ActorID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_Permission			TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////

struct SDB_QieCuoEnemy
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_UidUser);							//玩家的UID
	FIELD(TInt8Array<THING_NAME_LEN>,	m_Name);		//名字
	FIELD(UINT8,	m_Level);							//等级
	FIELD(UINT32,	m_Layer);							//境界
	FIELD(TTitleID,	m_TitleID);							//称号ID
	FIELD(UINT8,	m_vipLevel);						//vip等级

	FIELD_END();
};

//刷新切磋的对手
struct SDB_Flush_QieCuoEnemy : private DBReqPacketHeader
{
	SDB_Flush_QieCuoEnemy(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Flush_QieCuoEnemy");
	FIELD(UINT64,			m_UidUser);
	FIELD(UINT8,			m_BeginLevel);
	FIELD(UINT8,			m_EndLevel);
	FIELD(INT16,			m_Num);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_QieCuoEnemy			TResultSet2;	//结果集
};

struct SDB_DouFaEnemy
{
	FIELD_BEGIN();
	FIELD(UINT64,						m_UidEnemy);		//对手1UID
	FIELD(TInt8Array<THING_NAME_LEN>,	m_NameEnemy);		//对手1名字
	FIELD(UINT16,						m_Facade);			//对手1外观
	FIELD(UINT8,						m_VipLevel);		//对手1vip等级

	FIELD_END();
};

//刷新斗法的对手
struct SDB_Flush_DouFaEnemy : private DBReqPacketHeader
{
	SDB_Flush_DouFaEnemy(){
		this->m_length = sizeof(*this);
		m_uidEnemy1 = 0;
		m_uidEnemy2 = 0;
		m_uidEnemy3 = 0;
	}

	BIND_PROC_BEGIN("P_GDB_Flush_DouFaEnemy");
	FIELD(UINT64,			m_UidUser);
	FIELD(UINT8,			m_LimitUpLv);		//抽取玩家的等级上限
	FIELD(UINT64,			m_uidEnemy1);
	FIELD(UINT64,			m_uidEnemy2);
	FIELD(UINT64,			m_uidEnemy3);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_DouFaEnemy			TResultSet2;	//结果集	
};

//////////////////////////////////////////////////////////////////////////////////////
//禁言
struct SDB_DontTalk
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_ID);
	FIELD(TUserID,		m_UserID);
	FIELD(UINT32,		m_BeginTime);
	FIELD(UINT32,		m_EndTime);
	FIELD(UINT32,		m_RemoveTime);

	FIELD_END();
};

//禁言
struct SDB_DontTalk_Req : private DBReqPacketHeader
{
	SDB_DontTalk_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_DontTalk");
	FIELD(UINT32,		m_ID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_DontTalk			TResultSet2;	//结果集	
};

//得到所有禁言
struct SDB_AllDontTalk_Req : private DBReqPacketHeader
{
	SDB_AllDontTalk_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetAllDontTalk");
	FIELD(UINT16,		m_ServerID);	//服务器ID

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_DontTalk			TResultSet2;	//结果集
};

//封号
struct SDB_SealNo
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_ID);
	FIELD(TUserID,		m_UserID);
	FIELD(UINT32,		m_BeginTime);
	FIELD(UINT32,		m_EndTime);
	FIELD(UINT32,		m_RemoveTime);

	FIELD_END();
};

//封号
struct SDB_SealNo_Req : private DBReqPacketHeader
{
	SDB_SealNo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetSealNo");
	FIELD(UINT32,		m_ID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_DontTalk			TResultSet2;	//结果集
};

//得到所有封号
struct SDB_GetAllSealNo_Req : private DBReqPacketHeader
{
	SDB_GetAllSealNo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GetAllSealNo");
	FIELD(UINT16,		m_ServerID);	//服务器ID

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_DontTalk			TResultSet2;	//结果集
};

struct SDB_GiveThingMail
{
	FIELD_BEGIN();
	FIELD(INT32,		m_RetCode);
	FIELD(UINT32,		m_MailID);
	FIELD(UINT8,		m_TargetType);						//目标类型:0所有用户,1指定玩家
	FIELD(TInt8Array<DESCRIPT_LEN_1024>,	TargetList);	//目标列表，逗号分隔
	FIELD(TInt8Array<DESCRIPT_LEN_50>,		m_ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>,		m_ContentText);		//文本内容

	FIELD_END();
};

struct SDB_GiveGoods
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_ID);			
	FIELD(UINT8,		m_OperateType);						//操作类型:0增加,1扣除
	FIELD(TGoodsID,		m_GoodsID);							//物品ID
	FIELD(UINT16,		m_GoodsNum);						//物品数量

	FIELD_END();
};

//发放物品
struct SDB_GiveGoods_Req : private DBReqPacketHeader
{
	SDB_GiveGoods_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GiveGoods");
	FIELD(UINT32,		m_GiveGoodsMailID);	//和发放物品相关联的系统邮件ID
	FIELD(UINT16,		m_ServerID);

	FIELD_END();

	typedef SDB_GiveThingMail				TResultSet;		//结果集
	typedef SDB_GiveGoods					TResultSet2;	//结果集
};

struct SDB_GiveResource
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_ID);
	FIELD(UINT8,		m_OperateType);						//操作类型:0增加,1扣除
	FIELD(INT32,		m_MoneyNum);						//仙石数量
	FIELD(INT32,		m_StoneNum);						//灵石数量
	FIELD(INT32,		m_TicketNum);						//礼券数量
	FIELD(INT32,		m_PolyNimbusNum);					//聚灵气数量

	FIELD_END();
};

//发放资源
struct SDB_GiveResource_Req : private DBReqPacketHeader
{
	SDB_GiveResource_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_GiveResource");
	FIELD(UINT32,		m_GiveResourceMailID);	//和发放资源相关联的系统邮件ID
	FIELD(UINT16,		m_ServerID);

	FIELD_END();

	typedef SDB_GiveThingMail		TResultSet;		//结果集
	typedef SDB_GiveResource		TResultSet2;	//结果集
};

struct SDB_BackSysMail
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_ID);
	FIELD(UINT8,	m_TargetType);
	FIELD(TInt8Array<DESCRIPT_LEN_1024>,	m_TargetList);
	FIELD(TInt8Array<DESCRIPT_LEN_50>,		m_ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>,		m_ContentText);		//文本内容

	FIELD_END();
};

//后台邮件
struct SDB_BackSysMail_Req : private DBReqPacketHeader
{
	SDB_BackSysMail_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_BackSysMail");
	FIELD(UINT32,		m_ID);
	FIELD(UINT16,		m_ServerID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_BackSysMail			TResultSet2;	//结果集
};

//给所有玩家写系统邮件
struct SDB_WriteAllUserSysMail_Req : private DBReqPacketHeader
{
	SDB_WriteAllUserSysMail_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_WriteAllUserSysMail");
	FIELD(UINT64,		uid_SendUser);					//发件人UID值
	FIELD(TInt8Array<THING_NAME_LEN>, Name_SendUser);	//发件人名字
	FIELD(UINT8,		Mail_Type);						//邮件类型
	FIELD(UINT32,		Stone);							//发送的灵石
	FIELD(UINT32,		Money);							//发送的仙石
	FIELD(UINT32,		Ticket);						//发送的礼卷
	FIELD(UINT32,		PolyNimbus);					//聚灵气
	FIELD(TInt8Array<DESCRIPT_LEN_50>, ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>, ContentText);	//文本内容
	FIELD(UINT64,		Time);							//时间
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据	

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//玩家不在线时的扣除物品
struct SDB_OffLine_RemoveGoods_Req : private DBReqPacketHeader
{
	SDB_OffLine_RemoveGoods_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_OffLine_RemoveGoods");
	FIELD(TUserID,		m_UserID);
	FIELD(TGoodsID,		m_GoodsID);
	FIELD(UINT16,		m_GoodsNum);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//玩家不在线时的扣除钱灵石礼卷
struct SDB_OffLine_DescRes_Req : private DBReqPacketHeader
{
	SDB_OffLine_DescRes_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_OffLine_DeleteRes");
	FIELD(TUserID,		m_UserID);
	FIELD(INT32,		m_Money);
	FIELD(INT32,		m_Stone);
	FIELD(INT32,		m_Ticket);
	FIELD(INT32,		m_PolyNimbus);	//聚灵气

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//给所有帮派成员写系统邮件
struct SDB_WriteAllSynMemberSysMail_Req : private DBReqPacketHeader
{
	SDB_WriteAllSynMemberSysMail_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_WriteAllSynMemberSysMail");
	FIELD(UINT64,		uid_SendUser);					//发件人UID值
	FIELD(TInt8Array<THING_NAME_LEN>, Name_SendUser);	//发件人名字
	FIELD(UINT8,		Mail_Type);						//邮件类型
	FIELD(UINT32,		Stone);							//发送的灵石
	FIELD(UINT32,		Money);							//发送的仙石
	FIELD(UINT32,		Ticket);						//发送的礼卷
	FIELD(TInt8Array<DESCRIPT_LEN_50>, ThemeText);		//主题
	FIELD(TInt8Array<DESCRIPT_LEN_300>, ContentText);	//文本内容
	FIELD(UINT64,		Time);							//时间
	FIELD(TBinArray<sizeof(SDBGoodsData) * MAX_MAIL_GOODS_NUM>, GoodsData);	//创建物品所需的物品数据	

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//得到所有系统消息
struct SDB_Get_AllSysMsg_Req : private DBReqPacketHeader
{
	SDB_Get_AllSysMsg_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_AllSysMsg");
	FIELD(UINT16,		m_ServerID);	//服务器ID

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_SysMsgInfo				TResultSet2;	//结果集
};

struct SDB_Get_Version
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_ID);
	FIELD(TInt8Array<DESCRIPT_LEN_20>,	m_LastVersion);
	FIELD(TInt8Array<DESCRIPT_LEN_20>,	m_MinVersion);
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	m_InstallPackageUrl);

	FIELD_END();
};

//得到版本的更新
struct SDB_Get_Version_Req : private DBReqPacketHeader
{
	SDB_Get_Version_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_Version");
	FIELD(UINT32,		m_ID);	//版本表ID

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_Version				TResultSet2;	//结果集
};

struct SDB_Get_ServiceData
{
	FIELD_BEGIN();
	FIELD(TInt8Array<20>        ,m_szServiceTel); //客服电话
	FIELD(TInt8Array<50>        ,m_szServiceEmail); //客服邮箱
	FIELD(TInt8Array<20>        ,m_szServiceQQ);     //客服QQ

	FIELD_END();
};

//得到客服信息
struct SDB_Get_ServiceData_Req : private DBReqPacketHeader
{
	SDB_Get_ServiceData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_ServiceData");
	FIELD(UINT32,		m_ID);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_ServiceData			TResultSet2;	//结果集
};

//////////////////////////////////////////////////////////////////////////////////////
//支付
struct SDB_Get_PayData
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_ID);
	FIELD(INT32,		m_Money);	//支付金额，以角为单位
	FIELD(INT32,		m_GodStone);
	FIELD(TUserID,		m_UserID);	//玩家ID

	FIELD_END();
};

struct SDB_Get_PayData_Req : private DBReqPacketHeader
{
	SDB_Get_PayData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_PayData");
	FIELD(UINT32,		m_ID);
	FIELD(UINT16,		m_ServerID);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_PayData				TResultSet2;	//结果集
};

//移到支付历史
struct SDB_RemoveToPayLog_Req : private DBReqPacketHeader
{
	SDB_RemoveToPayLog_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_RemoveToPayLog");
	FIELD(UINT32,		m_ID);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};


struct SDB_Get_AllPayData_Req : private DBReqPacketHeader
{
	SDB_Get_AllPayData_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_AllPayData");
	FIELD(TUserID,		m_UserID);
	FIELD(UINT16,		m_ServerID);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_PayData				TResultSet2;	//结果集
};

struct FirstOpenServerTime
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_time);

	FIELD_END();
};

//得到第一次打开服务器时间
struct SDB_Get_FirstOpenServerTimeReq : private DBReqPacketHeader
{
	SDB_Get_FirstOpenServerTimeReq(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_FirstOpenServerTime");
	FIELD(UINT16      ,m_ServerID);
	FIELD(UINT16      ,m_PlatformID);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef FirstOpenServerTime			TResultSet2;	//结果集
};

//
struct SDB_Save_FirstOpenServerTime_Req : private DBReqPacketHeader
{
	SDB_Save_FirstOpenServerTime_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Save_FirstOpenServerTime");
	FIELD(UINT16      ,m_ServerID);
	FIELD(UINT16      ,m_PlatformID);
	FIELD(UINT32	  ,m_Time);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef FirstOpenServerTime			TResultSet2;	//结果集
};

//开服数据库初始化
struct SDB_OpenServer_Init_Req : private DBReqPacketHeader
{
	SDB_OpenServer_Init_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_OpenServer_Init");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};
//////////////////////////////////////////////////////////////////////////////////////

//保存在线玩家数量
struct SDB_Save_OnlineInfo_Req : private DBReqPacketHeader
{
	SDB_Save_OnlineInfo_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Save_OnlineInfo");
	FIELD(UINT16		,m_ServerID);
	FIELD(UINT32		,m_OnlineNum);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////
//得到活动配置

struct SDB_ActivityCnfg
{
	SDB_ActivityCnfg(){
		
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(UINT16								,m_ActivityID);		//活动ID
	FIELD(TInt8Array<THING_NAME_LEN>			,m_szName) ;		//名称
	FIELD(UINT8									,m_ActivityType);	//活动类型
	FIELD(TInt8Array<DESCRIPT_LEN_600>			,m_szRole) ;		// 活动规则
	FIELD(UINT32								,m_BeginTime) ;		//活动开始时间
	FIELD(UINT32								,m_EndTime) ;		//活动结束时间
	FIELD(UINT16								,m_ResID);			//资源ID
	FIELD(UINT8									,m_bFinished);		//初始状态
	FIELD(UINT16								,m_EventID) ;		//事件
	FIELD(TInt8Array<DESCRIPT_LEN_100>			,m_vectParam);		//参数
	FIELD(UINT16								,m_AttainNum) ;		//目标次数
	FIELD(TInt8Array<DESCRIPT_LEN_600>			,m_szAwardDesc) ;	// 活动奖励描述
	FIELD(UINT16								,m_Ticket);			//礼券
	FIELD(UINT16								,m_GodStone);		//仙石
	FIELD(TInt8Array<DESCRIPT_LEN_300>			,m_vectGoods);		//物品ID和数量
	FIELD(TInt8Array<DESCRIPT_LEN_50>			,m_szMailSubject);	//邮件主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>			,m_szMailContent);	//邮件内容

//	FIELD(TInt8Array<DESCRIPT_LEN_100>			,m_szResFileUrl);	//资源文件

	FIELD(INT16                                 ,m_Order); //排序
	
	FIELD_END();
};

struct SDB_Get_ActivityCnfg_Req : private DBReqPacketHeader
{
	SDB_Get_ActivityCnfg_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ActivityCnfg");
	
	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_ActivityCnfg			TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////
//类型
enum enGodStoneType VC_PACKED_ONE
{
	enGodStoneType_Get = 0,		//获得仙石
	enGodStoneType_Use,			//消耗仙石
}PACKED_ONE;

//渠道
enum enGodStoneChanel VC_PACKED_ONE
{
	enGodStoneChanel_Recharge = 0,	//充值
	enGodStoneChanel_Game,			//游戏
	enGodStoneChanel_Manage,		//网管
	enGodStoneChanel_Mail,			//邮件
}PACKED_ONE;


//仙石消耗日志
struct SDB_Save_GodStoneLog_Req : private DBReqPacketHeader
{
	SDB_Save_GodStoneLog_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Save_GodStoneLog");
	FIELD(TUserID,		m_UserID);
	FIELD(UINT8,		m_Type);						//类型:0获得,1消费
	FIELD(UINT8,		m_Chanel);						//渠道:0充值,1游戏,2网管
	FIELD(UINT16,		m_ServerID);					//
	FIELD(INT32,		m_Vargodstone);					//变动仙石
	FIELD(INT32,		m_Aftergodstone);				//变动后仙石
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	m_Description);	//说明

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};


//////////////////////////////////////////////////////////////////////////////////////

//得到需要日志记录的物品
struct SDB_NeedLogGoodsID
{
	FIELD_BEGIN();
	FIELD(TGoodsID, m_GoodsID);
	
	FIELD_END();
};

struct SDB_Get_NeedLogGoodsID_Req : private DBReqPacketHeader
{
	SDB_Get_NeedLogGoodsID_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_NeedLogGoodsID");
	
	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_NeedLogGoodsID			TResultSet2;	//结果集
};

//类型
enum enGameGoodsType VC_PACKED_ONE
{
	enGameGoodsType_Add = 0,	//获得物品
	enGameGoodsType_Use,		//失去物品
}PACKED_ONE;

//渠道
enum enGameGoodsChanel VC_PACKED_ONE
{
	enGameGoodsChanel_Buy = 0,	//购买
	enGameGoodsChanel_Drop,		//掉落
	enGameGoodsChanel_Mail,		//邮件
	enGameGoodsChanel_Use,		//使用物品
	enGameGoodsChanel_DiuQi,	//丢弃
	enGameGoodsChanel_Sell,		//出售
	enGameGoodsChanel_Trade,	//交易
	enGameGoodsChanel_Compose,	//合成
	enGameGoodsChanel_Stronger,	//强化
	enGameGoodsChanel_SystemDes,//系统扣除
	enGameGoodsChanel_SystemGet,//系统直接给予
	enGameGoodsChanel_Task,		//任务
	enGameGoodsChanel_TalismanSpawn,	//法宝孕育物
	enGameGoodsChanel_TalismanWorld,	//法定世界
	enGameGoodsChanel_Activity,			//活动模块
	enGameGoodsChanel_GMCmd,			//GM命令
	enGameGoodsChanel_Other,	//游戏其它
}PACKED_ONE;

//记录物品日志
struct SDB_Save_GameGoodsLog_Req : private DBReqPacketHeader
{
	SDB_Save_GameGoodsLog_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Save_GameGoodsLog");
	FIELD(TUserID,		m_UserID);
	FIELD(UINT8,		m_Type);						//类型 0:获得 1:失去
	FIELD(UINT8,		m_Chanel);						//渠道 0:购买 1:掉落 2:邮件 3:使用 4:丢弃 5:出售 6:寄售 7:合成 8:强化 9:系统扣除物品
	FIELD(TGoodsID,		m_GoodsID);
	FIELD(UINT64,		m_GoodsUID);
	FIELD(UINT16,		m_GoodsNum);
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	m_Description);	//说明

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//得到我的排行，只得到我的排行
struct SDB_Get_MyRank_Challenge_Req : private DBReqPacketHeader
{
	SDB_Get_MyRank_Challenge_Req(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_MyRank_Challenge");
	FIELD(UINT64,		m_uidUser);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_MyRank					TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////
//得到商场配置信息

struct SDB_Get_ShopMallCnfg
{
	FIELD_BEGIN();
	
	FIELD(UINT32,		m_ID);
	FIELD(TGoodsID,		m_GoodsID);
	FIELD(UINT16,		m_GoodsNum);
	FIELD(UINT8,		m_Type);
	FIELD(UINT8,		m_MoneyType);
	FIELD(INT32,		m_MoneyNum);
	FIELD(UINT8,		m_Binded);
	FIELD(UINT32,		m_StartTime);
	FIELD(UINT32,		m_EndTime);
	FIELD(INT32,		m_LeftFen);
	
	FIELD_END();
};

struct SDB_Get_ShopMallCnfg_Req : private DBReqPacketHeader
{
	SDB_Get_ShopMallCnfg_Req(){
		
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ShopMallCnfg");
	
	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_ShopMallCnfg		TResultSet2;	//结果集
};

//更新商城配置
struct SDB_Update_ShopMallCnfg_Req : private DBReqPacketHeader
{
	SDB_Update_ShopMallCnfg_Req(){
		
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_ShopMallCnfg");
	FIELD(UINT32,		m_ID);
	FIELD(INT32,		m_LeftFen);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//得到单个物品配置信息
struct SDB_Get_OneGoodsShopCnfg_Req : private DBReqPacketHeader
{
	SDB_Get_OneGoodsShopCnfg_Req(){
		
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_OneGoodsShopCnfg");
	FIELD(UINT32,		m_ID);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_Get_ShopMallCnfg		TResultSet2;	//结果集
};

//记录商城日志
struct SDB_Insert_ShopMallLog_Req : private DBReqPacketHeader
{
	SDB_Insert_ShopMallLog_Req(){
		
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_ShopMallLog");
	FIELD(TUserID,		m_UserID);
	FIELD(UINT32,		m_ShopMallID);
	FIELD(UINT32,		m_BuyNum);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集

};
/////////////////////////////////////////////////////////////////////////////////////////

//记录法宝世界数据
struct SDB_Save_TalismanWorld_Req : private DBReqPacketHeader
{
	SDB_Save_TalismanWorld_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Save_TalismanWorld");

	FIELD(UINT8,						m_TLevel);			//法宝世界级别
	FIELD(UINT8,						m_TFloor);			//法宝世界层数
	FIELD(UINT8,						m_Pos);				//怪物位置
	FIELD(UINT64,						m_uidOccupater);	//占领者UID
	FIELD(TInt8Array<THING_NAME_LEN>,	m_NameOccupater);	//占领者名字
	FIELD(UINT16,						m_Facade);			//占领者资源
	FIELD(UINT32,						m_BeginOccupTime);	//开始占领时间
	
	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_TalismanWorld
{
	FIELD_BEGIN();

	FIELD(UINT8,						m_TLevel);			//法宝世界级别
	FIELD(UINT8,						m_TFloor);			//法宝世界层数
	FIELD(UINT8,						m_Pos);				//怪物位置
	FIELD(UINT64,						m_uidOccupater);	//占领者UID
	FIELD(TInt8Array<THING_NAME_LEN>,	m_NameOccupater);	//占领者名字
	FIELD(UINT16,						m_Facade);			//占领者资源
	FIELD(UINT32,						m_BeginOccupTime);	//开始占领时间

	FIELD_END();
};
//得到法宝世界数据
struct SDB_Get_TalismanWorld_Req : private DBReqPacketHeader
{
	SDB_Get_TalismanWorld_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_TalismanWorld");

	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_TalismanWorld				TResultSet2;	//结果集
};

//删除法宝世界占领
struct SDB_Delete_TalismanWorld_Req : private DBReqPacketHeader
{
	SDB_Delete_TalismanWorld_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_TalismanWorld");

	FIELD(UINT8,		m_Level);
	FIELD(UINT8,		m_Floor);
	FIELD(UINT8,		m_Pos);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};


//更新占领被击败时获得的灵魄
struct SDB_Update_Occupation_GhostSoul : private DBReqPacketHeader
{
	SDB_Update_Occupation_GhostSoul(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_Occupation_GhostSoul");
	FIELD(UINT64,	m_UidUser);
	FIELD(UINT32,	m_GhostSoul);
	
	FIELD_END();

	typedef ResultSetNull					TResultSet;		//结果集
	typedef ResultSetNull					TResultSet2;	//结果集
};


//更新占领时被击败的信息
struct SDB_Update_Occupation_Info : private DBReqPacketHeader
{
	SDB_Update_Occupation_Info(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_Occupation_Info");
	FIELD(UINT64,	m_UidUser);
	FIELD(UINT64,	m_UidEnemy);
	FIELD(int,		m_BufLen);
	FIELD(VarBinary,	m_RecordBuf);				//可变长
	
	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_NewRecord					TResultSet2;	//结果集
};

//法宝世界占领时被击败的信息
struct SDB_OccupationInfo
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_Rank);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_EnemyName);	
	FIELD(UINT8,		m_VipLevel);
	FIELD(UINT32,	m_GhostSoul);

	FIELD_END();

};

//得到占领时被击败的信息
struct SDB_Get_OccupationInfo : private DBReqPacketHeader
{
	SDB_Get_OccupationInfo(){
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_OccupationInfo");
	FIELD(UINT64,	m_uidUser);

	FIELD_END();

	typedef DB_OutParam		TResultSet;   //结果集
	typedef SDB_OccupationInfo	TResultSet2;	//结果集
};
///////////////////////////////////////////////////////////////////////////////////////

//记录玄天BOSS信息
struct SDB_Save_XTBossInfo_Req : private DBReqPacketHeader
{
	SDB_Save_XTBossInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Save_XTBossInfo");
	FIELD(UINT64,		m_UIDBoss);
	FIELD(UINT16,		m_MonsterID);
	FIELD(INT32,		m_CurBlood);
	FIELD(UINT32,		m_RecordTime);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_XTBossInfo
{
	FIELD_BEGIN();

	FIELD(UINT64,		m_UIDBoss);
	FIELD(UINT16,		m_MonsterID);
	FIELD(INT32,		m_CurBlood);
	FIELD(UINT32,		m_RecordTime);

	FIELD_END();
};

//得到玄天BOSS信息
struct SDB_Get_XTBossInfo_Req : private DBReqPacketHeader
{
	SDB_Get_XTBossInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_XTBossInfo");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_XTBossInfo				TResultSet2;	//结果集
};

//清除玄天BOSS信息
struct SDB_Delete_XTBossInfo_Req : private DBReqPacketHeader
{
	SDB_Delete_XTBossInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_XTBossInfo");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//记录玄天BOSS死亡信息
struct SDB_Save_XTBossDie_Req : private DBReqPacketHeader
{
	SDB_Save_XTBossDie_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Save_XTBossDie");

	FIELD(UINT64,						m_UIDBoss);
	FIELD(UINT64,						m_UIDUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_UserName);	//玩家名字
	FIELD(TInt8Array<THING_NAME_LEN>,	m_BossName);	//BOSS名字
	FIELD(INT32,						m_TotalDamage);	//总伤害
	FIELD(INT32,						m_CurDamage);	//当次伤害
	FIELD(INT32,						m_AttackNum);	//攻击次数
	FIELD(UINT8,						m_UserLevel);	//玩家等级

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_XTBossDie
{
	FIELD_BEGIN();

	FIELD(UINT64,						m_UIDBoss);		//无效时表示是上一次玄天
	FIELD(UINT64,						m_UIDUser);
	FIELD(TInt8Array<THING_NAME_LEN>,	m_UserName);	//玩家名字
	FIELD(TInt8Array<THING_NAME_LEN>,	m_BossName);	//BOSS名字
	FIELD(INT32,						m_TotalDamage);	//总伤害
	FIELD(INT32,						m_CurDamage);	//当次伤害
	FIELD(INT32,						m_AttackNum);	//攻击次数
	FIELD(UINT8,						m_UserLevel);	//玩家等级

	FIELD_END();
};

//得到玄天BOSS死亡信息
struct SDB_Get_XTBossDie_Req : private DBReqPacketHeader
{
	SDB_Get_XTBossDie_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_XTBossDie");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_XTBossDie				TResultSet2;	//结果集
};

//删除上一次玄天BOSS死亡信息，并把BOSS死亡信息转为上一次BOSS死亡信息
struct SDB_Change_XTBossDie_Req : private DBReqPacketHeader
{
	SDB_Change_XTBossDie_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Change_XTBossDie");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//更新玄天伤害
struct SDB_Update_XTDamage_Req : private DBReqPacketHeader
{
	SDB_Update_XTDamage_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_XTDamage");

	FIELD(UINT64,						m_UIDUser);
	FIELD(UINT8,						m_Level);		//等级
	FIELD(TInt8Array<THING_NAME_LEN>,	m_UserName);	//玩家名字
	FIELD(INT32,						m_Damage);		//伤害
	FIELD(INT32,						m_AttackNum);	//攻击次数
	FIELD(UINT8,						m_bOver);		//是否结束

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_XTDamage
{
	FIELD_BEGIN();

	FIELD(UINT64,						m_UIDUser);
	FIELD(UINT8,						m_Level);		//等级
	FIELD(TInt8Array<THING_NAME_LEN>,	m_UserName);	//玩家名字
	FIELD(INT32,						m_Damage);		//伤害
	FIELD(INT32,						m_AttackNum);	//攻击次数
	FIELD(UINT8,						m_IsCurrent);	//0：当前玄天  1：上一次玄天

	FIELD_END();
};

//得到玄天伤害
struct SDB_Get_XTDamage_Req : private DBReqPacketHeader
{
	SDB_Get_XTDamage_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_XTDamage");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_XTDamage				TResultSet2;	//结果集
};

//删除上一次伤害，并把当前伤害转为上一次伤害
struct SDB_Change_XTLastDamage_Req : private DBReqPacketHeader
{
	SDB_Change_XTLastDamage_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Change_XTLastDamage");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////

struct SDB_TalismanPart
{
	SDB_TalismanPart()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();

	FIELD(UINT64,		m_UIDUser);
	FIELD(UINT32,		m_GetGhostSoul);
	FIELD(UINT8,		m_EnterTWNumToday);
	FIELD(UINT32,		m_LastEnterTWTime);
	FIELD(UINT8,		m_VipEnterTWNumToday);
	FIELD(UINT32,		m_LastVipEnterTWTime);
	FIELD(UINT8,		m_Level);
	FIELD(UINT8,		m_Floor);
	FIELD(UINT64,		m_uidMonster);
	FIELD(UINT32,		m_OccupationTime);

	FIELD_END();
};

//得到法宝世界PART数据
struct  SDB_Get_TalismanPart_Req : private DBReqPacketHeader
{
	SDB_Get_TalismanPart_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_TalismanPart");

	FIELD(UINT64,		m_UIDUser);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_TalismanPart			TResultSet2;	//结果集
};

//更新法宝世界PART数据
struct SDB_Update_TalismanPart_Req : private DBReqPacketHeader
{
	SDB_Update_TalismanPart_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_TalismanPart");

	FIELD(UINT64,		m_UIDUser);
	FIELD(UINT32,		m_GetGhostSoul);
	FIELD(UINT8,		m_EnterTWNumToday);
	FIELD(UINT32,		m_LastEnterTWTime);
	FIELD(UINT8,		m_VipEnterTWNumToday);
	FIELD(UINT32,		m_LastVipEnterTWTime);
	FIELD(UINT8,		m_TLevel);
	FIELD(UINT8,		m_TFloor);
	FIELD(UINT64,		m_uidMonster);
	FIELD(UINT32,		m_OccupationTime);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////
//帮战

struct SDB_SynWarKillHistroy
{
	FIELD_BEGIN();

	FIELD(UINT64,								m_uidUser);
	FIELD(TUint8Array<(MAX_SYNMEMBER_NUM+7)/8>,	m_KillHistroy);   //杀敌记录

	FIELD_END();
};

//得到帮战杀敌记录
struct SDB_Get_SynWarKillHistory_Req : private DBReqPacketHeader
{
	SDB_Get_SynWarKillHistory_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SynWarKillHistory");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_SynWarKillHistroy		TResultSet2;	//结果集
};

//更新排长战杀敌记录
struct SDB_Update_SynWarKillHistory_Req : private DBReqPacketHeader
{
	SDB_Update_SynWarKillHistory_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_SynWarKillHistory");

	FIELD(UINT64,								m_uidUser);
	FIELD(TUint8Array<(MAX_SYNMEMBER_NUM+7)/8>,	m_KillHistroy);   //杀敌记录

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_SynWarInfo
{
	FIELD_BEGIN();

	FIELD(TSynID,		m_SynID);
	FIELD(TSynID,		m_EnemySynID);
	FIELD(UINT8,		m_bWin);
	FIELD(TBinArray<(sizeof(UID) * MAX_SYNMEMBER_NUM)>, m_MemberPos);	//帮派成员的分布

	FIELD_END();
};

//得到帮战情况
struct SDB_Get_SynWarInfo_Req : private DBReqPacketHeader
{
	SDB_Get_SynWarInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SynWarInfo");


	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_SynWarInfo				TResultSet2;	//结果集
};

//更新帮战情况
struct SDB_Update_SynWarInfo_Req : private DBReqPacketHeader
{
	SDB_Update_SynWarInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_SynWarInfo");

	FIELD(TSynID,		m_SynID);
	FIELD(TSynID,		m_EnemySynID);
	FIELD(UINT8,		m_bWin);
	FIELD(TBinArray<(sizeof(UID) * MAX_SYNMEMBER_NUM)>, m_MemberPos);	//帮派成员的分布

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_SynWarDeclare
{
	FIELD_BEGIN();

	FIELD(TSynID,	m_SynID);
	FIELD(TSynID,	m_DeclareSynID);

	FIELD_END();
};

//得到帮战宣战
struct SDB_Get_SynWarDeclare_Req : private DBReqPacketHeader
{
	SDB_Get_SynWarDeclare_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SynWarDeclare");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_SynWarDeclare			TResultSet2;	//结果集
};

//插入帮战宣战
struct SDB_Insert_SynWarDeclare_Req : private DBReqPacketHeader
{
	SDB_Insert_SynWarDeclare_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Insert_SynWarDeclare");

	FIELD(TSynID,		m_SynID);
	FIELD(TSynID,		m_DeclareSynID);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//删除所有帮战宣战
struct SDB_Delete_SynWarDeclare_Req : private DBReqPacketHeader
{
	SDB_Delete_SynWarDeclare_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_SynWarDeclare");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_LastRecordTime
{
	FIELD_BEGIN();

	FIELD(UINT32,		m_LastRecordTime);

	FIELD_END();
};

//得到帮战最后记录时间
struct SDB_Get_LastRecordTime_Req : private DBReqPacketHeader
{
	SDB_Get_LastRecordTime_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_LastRecordTime");

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_LastRecordTime			TResultSet2;	//结果集
};

//删除所有帮战信息
struct SDB_Delete_SynWarInfo_Req : private DBReqPacketHeader
{
	SDB_Delete_SynWarInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Delete_SynWarInfo");

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};
///////////////////////////////////////////////////////////////////////////////////////

//帮派PART

struct SDB_SynPart
{
	SDB_SynPart()
	{
		MEM_ZERO(this);
	}
	FIELD_BEGIN();

	FIELD(UINT64,	m_uidUser);
	FIELD(UINT16,	m_JoinCombatNumToday);		//今天参加帮战战斗的次数
	FIELD(UINT32,	m_LastJoinSynWarTime);		//最后一次参加帮战的时间
	FIELD(UINT16,	m_MaxJoinCombatNumToday);	//今天总共可参加的帮战次数
	FIELD(UINT32,	m_LastSetMaxCombatNumTime);	//最后一次设置最多可参加帮战次数的时间
	FIELD(INT32,	m_GetCreditWeek);			//本周获得的声望数
	FIELD(UINT32,	m_LastGetCreditTime);		//最后一次获得声望时间

	FIELD_END();
};

//得到帮派PART数据
struct SDB_Get_SynPart_Req : private DBReqPacketHeader
{
	SDB_Get_SynPart_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_SynPart");

	FIELD(UINT64,	m_uidUser);

	FIELD_END();

	typedef DB_OutParam					TResultSet;		//结果集
	typedef SDB_SynPart					TResultSet2;	//结果集
};

//更新帮派PART数据
struct SDB_Update_SynPart_Req : private DBReqPacketHeader
{
	SDB_Update_SynPart_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_SynPart");

	FIELD(UINT64,	m_uidUser);
	FIELD(UINT16,	m_JoinCombatNumToday);		//今天参加帮战战斗的次数
	FIELD(UINT32,	m_LastJoinSynWarTime);		//最后一次参加帮战的时间
	FIELD(UINT16,	m_MaxJoinCombatNumToday);	//今天总共可参加的帮战次数
	FIELD(UINT32,	m_LastSetMaxCombatNumTime);	//最后一次设置最多可参加帮战次数的时间
	FIELD(INT32,	m_GetCreditWeek);			//本周获得的声望数
	FIELD(UINT32,	m_LastGetCreditTime);		//最后一次获得声望时间

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};
///////////////////////////////////////////////////////////////////////////////////////


//更新上一场帮战信息
struct SDB_Update_PreSynWarInfo : private DBReqPacketHeader
{
	SDB_Update_PreSynWarInfo()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_PreSynWarInfo");

	FIELD(TSynID,						m_SynID);			//帮派ID
	FIELD(UINT8,				m_bWin);			//胜负
	FIELD(TInt8Array<THING_NAME_LEN>,	SynName);			//敌对帮派名字

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

///////////////////////////////////////////////////////////////////////////////////////

struct SDB_RechareForward
{
	SDB_RechareForward()
	{
		MEM_ZERO(this);
	}
	FIELD_BEGIN();

	FIELD(INT32,	m_MoneyNum);							//金额数
	FIELD(UINT8,	m_CalType);								//计算的充值方式(1:单次充值，2:累计充值)
	FIELD(UINT8,	m_ForwardType);							//奖励方式(1:固定额度,2:一定比例)
	FIELD(INT32,	m_GodStone);							//奖励的仙石数或者比例
	FIELD(INT32,	m_Ticket);								//奖励的礼卷数或者比例
	FIELD(INT32,	m_Stone);								//奖励的灵石数或者比例
	FIELD(INT32,	m_PolyNimbus);							//奖励的聚灵气或者比例
	FIELD(TInt8Array<DESCRIPT_LEN_300>,	m_Goods);			//物品(ID,数量,...)

	FIELD_END();
};

//得到充值返利信息
struct SDB_GetRechargeForward : private DBReqPacketHeader
{
	SDB_GetRechargeForward()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_RechargeForward");


	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_RechareForward		TResultSet2;	//结果集
};

struct SDB_RechargeForwardCnfg
{
	FIELD_BEGIN();

	FIELD(UINT32,	m_BeginTime);							//开始时间
	FIELD(UINT32,	m_EndTime);								//结束时间
	FIELD(TInt8Array<DESCRIPT_LEN_50>,	m_szMailTheme);		//邮件主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>,	m_szMailContent);	//邮件正文

	FIELD_END();
};

//得到充值返利配置信息
struct SDB_GetRechargeForwardCnfg : private DBReqPacketHeader
{
	SDB_GetRechargeForwardCnfg()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_RechargeForwardCnfg");

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_RechargeForwardCnfg	TResultSet2;	//结果集
};

struct SDB_TotalRecharge
{
	SDB_TotalRecharge()
	{
		m_TotalRecharge = 0;
	}

	FIELD_BEGIN();

	FIELD(UINT32,	m_TotalRecharge);

	FIELD_END();
};

//得到玩家在指定时间内的充值总和
struct SDB_GetTotalRechargeReq : private DBReqPacketHeader
{
	SDB_GetTotalRechargeReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_TotalRecharge");

	FIELD(UINT32,	m_UserID);
	FIELD(UINT32,	m_BeginTime);
	FIELD(UINT32,	m_EndTime);
	FIELD(UINT16,	m_ServerID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_TotalRecharge		TResultSet2;	//结果集
};

//怪物配置数据
struct SDB_MonsterCnfg
{
	FIELD_BEGIN();

	FIELD(TMonsterID,                 m_MonsterID);					//怪物ID
	FIELD(TInt8Array<THING_NAME_LEN> ,m_szName);					//名称
	FIELD(UINT8,                      m_Level);						//怪物等级级别
    FIELD(UINT32                     ,m_Spirit);					//灵力
	FIELD(UINT32                     ,m_Shield);					//护盾
	FIELD(UINT32  ,                   m_Blood);						//气血
	FIELD(UINT32  ,                   m_Avoid);						//身法
	FIELD(UINT32                      ,m_MagicValue);				//法术值
	FIELD(UINT32                      ,m_Swordkee);					//剑气	
	FIELD(UINT16                      ,m_Facade);					//外观
	FIELD(UINT8                      ,m_MagicNum);					//法术技能数量
	FIELD(UINT32                      ,m_MonsterNameLangID);		//怪物名称语言ID
	FIELD(INT32						 ,Crit);						//爆击值
	FIELD(INT32						 ,Tenacity);					//坚韧值
	FIELD(INT32						 ,Hit);							//命中值
	FIELD(INT32						 ,Dodge);						//回避值
	FIELD(INT32						 ,MagicCD);						//法术回复值

	FIELD_END();
};

//得到怪物配置信息
struct SDB_GetMonsterCnfgReq : private DBReqPacketHeader
{
	SDB_GetMonsterCnfgReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_MonsterCnfg");

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_MonsterCnfg			TResultSet2;	//结果集
};


//获取单个活动配置
struct SDB_GetActivityCnfgReq : private DBReqPacketHeader
{
	SDB_GetActivityCnfgReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_ActivityCnfgReq");

	FIELD(UINT16,	ActivityID);

	FIELD_END();

	typedef DB_OutParam				TResultSet;		//结果集
	typedef SDB_ActivityCnfg		TResultSet2;	//结果集
};

//删除任务
struct SDB_DeleteTaskReq : private DBReqPacketHeader
{
	SDB_DeleteTaskReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_DeleteTask");
	FIELD(UINT64,		m_uidUser);
	FIELD(TTaskID,		m_TaskID);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

//插入任务
struct SDB_UpdateTaskReq : private DBReqPacketHeader
{
	SDB_UpdateTaskReq()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_UpdateTask");
	FIELD(UINT64,		m_UserUID);
	FIELD(TTaskID,		m_TaskID);
	FIELD(UINT32,		m_FinishTime);
	FIELD(UINT64,		m_CurCount);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_MultipExpInfo
{
	FIELD_BEGIN();

	FIELD(float                 ,m_fMultipExp);      //多倍经验
	FIELD(UINT8                 ,m_MinMultipExpLv); //享受多倍经验最低玩家等级
	FIELD(UINT8                 ,m_MaxMultipExpLv); //享受多倍经验最高玩家等级
	FIELD(UINT32                ,m_MultipExpBeginTime);
	FIELD(UINT32                ,m_MultipExpEndTime);

	FIELD_END();
};

//得到多倍经验信息
struct SDB_GetMultipExpInfo_Req : private DBReqPacketHeader
{
	SDB_GetMultipExpInfo_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Get_MultipExpInfo");
	FIELD(TServerID,		m_ServerID);

	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_MultipExpInfo				TResultSet2;	//结果集
};

struct SDB_BackVid
{
	FIELD_BEGIN();

	FIELD(UINT32,	m_Vid);

	FIELD_END();
};

//插入节点信息
struct SDB_Insert_PointMsg_Req : private DBReqPacketHeader
{
	SDB_Insert_PointMsg_Req()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_CDB_Insert_PointInfo");
	FIELD(UINT32,		m_Vid);		//虚拟ID
	FIELD(TUserID,		m_UserID);	//玩家ID
	FIELD(UINT8,		m_Operator);//操作

	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_BackVid						TResultSet2;	//结果集
};

//保存玩家上次上线时间
struct SDB_Update_LastOnlineTime : private DBReqPacketHeader
{
	SDB_Update_LastOnlineTime()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Update_LastOnlineTime");
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT32,		m_LastOnlineTime);

	FIELD_END();

	typedef ResultSetNull				TResultSet;		//结果集
	typedef ResultSetNull				TResultSet2;	//结果集
};

struct SDB_FirstRechargeCnfg
{
	FIELD_BEGIN();

	FIELD(UINT8,		m_RateTicket);
	FIELD(UINT8,		m_RateGodStone);
	FIELD(INT32,		m_MinRecharge);						//奖励的最小充值金额
	FIELD(TInt8Array<DESCRIPT_LEN_50>,	m_szMailTheme);		//邮件主题
	FIELD(TInt8Array<DESCRIPT_LEN_600>,	m_szMailContent);	//邮件正文

	FIELD_END();
};

//得到首次充值配置
struct SDB_Get_FirstRechargeCnfg : private DBReqPacketHeader
{
	SDB_Get_FirstRechargeCnfg()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_FirstRechargeCnfg");
	
	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_FirstRechargeCnfg			TResultSet2;	//结果集
};


struct SDB_FirstRechargeRet
{
	FIELD_BEGIN();

	FIELD(UINT64,		m_uidUser);

	FIELD_END();
};

//检测是否可以给某次时间起的首次充值奖励
struct SDB_Get_IsGetFirstRechargeForward : private DBReqPacketHeader
{
	SDB_Get_IsGetFirstRechargeForward()
	{
		this->m_length = sizeof(*this);
	}

	BIND_PROC_BEGIN("P_GDB_Get_IsGetFirstRechargeForward");	
	FIELD(TUserID,		m_UserID);
	FIELD(UINT64,		m_uidUser);
	FIELD(UINT8,		m_level);

	FIELD_END();

	typedef DB_OutParam						TResultSet;		//结果集
	typedef SDB_FirstRechargeRet			TResultSet2;	//结果集
};
///////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif
