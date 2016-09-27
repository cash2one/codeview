

#ifndef __CONFIGSERVER_ICONFIGSERVER_H__
#define __CONFIGSERVER_ICONFIGSERVER_H__

#include "BclHeader.h"
#include "FieldDef.h"
#include "DSystem.h"
#include <string>
#include <vector>
#include <hash_map>
#include <map>
#include "GameSrvProtocol.h"
#include "IEquipment.h"

#ifndef BUILD_CONFIGSERVER_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"ConfigServer.lib")
#endif
#endif


//服务器端杂项数据配置
struct SGameServerConfigParam
{
	FIELD_BEGIN();
	FIELD(float                        ,m_AvoidDamageFactor1); //物理免伤公式系数C
	FIELD(float                        ,m_AvoidDamageFactor2); //物理免伤公式系数D
	FIELD(float                        ,m_MagicAvoidDamageFactor1); //法术免伤公式系数C
	FIELD(float                        ,m_MagicAvoidDamageFactor2); //法术免伤公式系数D
	FIELD(INT32                        ,m_TrainingExpFactor);//练功堂打怪获得经验系数
	FIELD(TGoodsID					,m_GaoJiExpGoodsID);	///高级经验丹ID
	FIELD(INT32						,m_GaoJiExpAddExpNum);	///高级经验丹增加多少经验
	FIELD(UINT16					,m_MaxBackGaoJiNum);	///最多返回多少高级经验丹
	FIELD(INT32						,m_RemoveGemStoneNum);	///宝石摘除消耗的灵石数
	FIELD(INT32						,m_MoneyInheritMoneyNum); ///支付仙石传承需要多少仙石
	FIELD(INT32						,m_StoneInheritStoneNum); ///支付灵石传承需要多少灵石
	FIELD(UINT8						,m_EquipInheritNoRandLevel);	  ///装备稳定强化等级的开始等级
	FIELD(UINT8						,m_TalismanInheritNoRandLevel);	  ///法宝稳定强化等级的开始等级
	FIELD(std::vector<INT32>		,m_Max_ChallengeNum);			  ///挑战排名对应的挑战次数
	FIELD(UINT8						,m_LevelCantUseGaoJiExp);			///多少级以后的角色不能使用高级经验丹
	FIELD(TGoodsID					,m_SuperGaoJiExpGoodsID);		///超高级经验丹ID
	FIELD(INT32						,m_SuperGaoJiExpAddExpNum);	///超高级经验丹增加多少经验
	FIELD(INT32						,m_CombatExpParam);				///战斗获得经验参数
	FIELD(std::vector<INT32>		,m_ServerState);				///服务器状态(人数,状态... (0:良好 1:一般 2:爆满 3:不能连接))
	FIELD(TMapID					,m_XTBossMap);					///玄天BOSS所在的地图
	FIELD(std::vector<UINT16>		,m_XuanTianBoss);				///玄天BOSS的ID和战斗地图
	FIELD(TCDTimerID				,m_XuanTianAttackCD);			///玄天攻击CD
	FIELD(TCDTimerID				,m_MoneyXuanTianAttackCD);		///使用仙石后的玄天攻击CD
	FIELD(INT16                     ,m_CombatReadyTime);        ///战斗准备时间,倒计时，单位:秒
	FIELD(INT16                     ,m_PhysicsAttackCDTime);   ///物理攻击间隔时间,单位:秒
	FIELD(INT32						,m_XTGetPolyNimbusParam);		///玄天获得聚灵气参数
	FIELD(INT32						,m_MaxXTGetPolyNimbusNoRank);	///玄天10名以后最多获得的聚灵气
	FIELD(UINT16					,m_ShowNumActorXTDamage);		///显示多少名以内的玄天伤害排名
	FIELD(INT32						,m_MoneyInheritPolyNimbusNum);	///支付仙石传承需要多少聚灵气
	FIELD(INT32						,m_OrdinaryInheritPolyNimbusNum); ///普通传承需要多少聚灵气
	FIELD(UINT8						,m_JoinSynCombatLvLow);			///参加帮战需要的玩家最低等级
	FIELD(UINT8						,m_JoinSynCombatUserNumLow);	///一个帮派参加帮战的最少人数下限
	FIELD(std::vector<INT32>		,m_SynCombatParam);				///帮战参数(帮战等级,声望上限,胜利帮战等级系数,失败帮战等级系数,帮战等级,...)
	FIELD(INT32						,m_SynWarAbilityParam);			///计算帮战实力的参数
	FIELD(TMapID					,m_SynWarMapID);				///帮战地图ID
	FIELD(TMapID					,m_SynWarCombatMapID);			///帮战战斗地图ID
	FIELD(UINT16					,m_MaxSynWarCombatNum);			///帮战最大可参加战斗次数

	FIELD(INT32						,m_WinSynWarGetCredit);			///帮战胜利获得的声望数
	FIELD(INT32						,m_WinSynWarGetSynWarLv);		///帮战胜利获得的帮战等级数
	FIELD(INT32						,m_WinSynWarCombatCredit);		///帮战战斗胜利获得声望数
	FIELD(INT32						,m_FailSynWarCombatCredit);		///帮战战斗失败获得声望数
	FIELD(INT32						,m_TalismanWorldGhostSoul);		///法宝世界战斗获得灵魄数

	FIELD(UINT8						,m_PairNumCanOtherLevel);	///夺宝配对多少次后可跨级配对

	FIELD(UINT32					,m_FuMoDongGetExpTimeSpace);	///伏魔洞挂机多久给一次经验

	FIELD(UINT32					,m_FuMoDongCombatGetExp);		///伏魔洞打怪获得经验

	FIELD(std::vector<UINT8>			,m_DuoBaoLevelGroup);			///夺宝的等级组(最低等级，最高等级，最低等级，...)

	FIELD(UINT32					,m_DuoBaoPairSpace);			///夺宝多久配对一次

	FIELD(TMapID					,m_DuoBaoCombatMapID);			///夺宝战地图

	FIELD(INT32					,m_PhysicsAttackFactor);	        ///物理攻击系数(单位:千份之一)

	FIELD(INT32					,m_MagicAttackFactor);           ///法术攻击系数(单位:千份之一)

	FIELD(INT32					,m_MagicHitFactor);           ///法术命中系数(单位:千份之一)	

	FIELD(INT32                 ,m_PhysicsHitFactor);   ///物理命中系数(单位:千份之一)	

	FIELD(std::vector<INT16>    ,m_vectAptitudeFacade);   //资质外观  [ 资质(%),套装ID(负数)]

	FIELD(UINT8					,m_OpenTeamFuBenUserLevel);	///多少级开放组队副本

	FIELD(UINT32				,m_OccupationTimeSpaceGetSoul); ///法宝世界占领后多久获取灵魄

	FIELD(std::vector<UINT8>	,m_TeamLeaderLineupPos);		///组队默认阵形，队长角色默认阵形位置

	FIELD(std::vector<UINT8>	,m_TeamMemberLineupPos);		///组队默认阵形，成员角色默认阵形位置

	FIELD(INT32					,m_SynCombatAbilityParam);		///帮派战力计算系数

	FIELD(TCDTimerID			,m_ZhaoMuSynMemberCDTimer);		///帮派招募冷却ID

	FIELD(std::vector<UINT8>	,m_TalismanWorldEnterLv);		///法宝世界进入级别对应玩家等级(最小玩家等级,法宝世界级别,...)

	FIELD(INT32					,m_MinGetScoreSynWar);			///帮战胜利最少获得积分数
	FIELD(INT32					,m_MaxGetScoreSynWar);			///帮战胜利最大获得积分数
	FIELD(INT32					,m_SynWarWinScore);				///获得多少积分即可判帮战胜利

	FIELD(INT32					,m_NoticeXuanTianStart);		///广播玄天开启的语言ID

	FIELD(INT32					,m_NoticeSynWarStart);			///广播帮战开启的语言ID
	FIELD(INT32					,m_NoticeDuoBaoStart);			///广播夺宝开启的语言ID

	FIELD(UINT32				,m_SaveXuanTianTimeSpace);		///多久保存一次玄天BOSS信息


	FIELD(INT16                 ,m_AutoMagicAttackDelay);     ///自动法术攻击最大延时,单位:秒

	FIELD(UINT32				,m_DecTraingTime);				///连续上线时间每增加1天，则减少的练功时间
	FIELD(UINT32				,m_AddTraingTime );				///连续上线时间每间隔1天，则增加的练功时间
	FIELD(UINT32				,m_MinTrainingTime);			///练功最小时间值


	FIELD(INT32					,m_XiWuExpParam);				///习武经验系数
	FIELD(INT32					,m_CancelDecExpRand);			///取消习武扣除经验的百分比参数
	

	FIELD_END();
};


//杂项数据配置
struct SGameConfigParam
{
	FIELD_BEGIN();
	FIELD(INT32                    ,m_SaveToDBInterval);  //角色数据定时存盘间隔
	FIELD(INT32                    ,m_OfflineInterval);  //角色下线间隔
	FIELD(INT32                    ,m_BuildingResOutputInterval );  ///建筑资产出时间间隔(单位:秒)
	FIELD(INT32                    ,m_BuildingOutputResNum      ); ///建筑每次产出资源数量
	FIELD(INT32                    ,m_BuildingGuardTime);            ///每次守护时长
	FIELD(INT32                    ,m_BuildingResFilchMax);       //////窃取建筑产出资源上限
	FIELD(INT32                    ,m_BuildingResFilchRate);       //窃取资源可获得资源分成比例(单位:%)
	FIELD(INT32                    ,m_BuildingResFilchedRate);       //被窃取资源可获得资源分成比例(单位:%)
	FIELD(INT32                    ,m_FuMoDongOnHookTimeSpace);        //伏魔洞每次挂机时长
	FIELD(INT16                    ,m_PacketInitCapacity);             //背包初始容量	
	FIELD(INT16                    ,m_PacketExtendSize);               //背包每次扩展容量
	FIELD(std::vector<INT32>       ,m_vectExtendCharge);          //背包每次扩展的费用
	FIELD(UINT8                    ,m_EquipMaxStarLevel);          //装备最大星级
	FIELD(UINT8					   ,m_TalismanMaxStarLevel);	//法宝最大星级
	FIELD(INT16			           ,m_CDTimeFreeFlush);			//聚仙楼免费刷新时间(秒)
	FIELD(INT16			           ,m_FriendInitRelationNum);		//加好友时的初始好友值
	FIELD(INT16			           ,m_MaxFriendNum);				//最多有几个好友
	FIELD(INT16		               ,m_MaxFriendMsgNum);			//最多有几条好友信息(此信息为好友的信息标签中的信息)
	FIELD(INT32			           ,m_FriendEnventRecordTime );	//好友度改变事件记录保存时间(单位秒,目前是一天)
	FIELD(INT16			           ,m_AddRelationHelpStone);			//帮对方收取灵石，增加多少好友度
	FIELD(INT16			           ,m_AddRelationGuardOneHour);		//守护对方1小时(满1小时才算),增加多少好友度
	FIELD(INT16			           ,m_AddRelationXiuLianOneHour);		//一起修炼1小时(满1小时才算),增加多少好友度
	FIELD(INT16			           ,m_AddRelationXiuLianMagic);		//修炼成功一个法术,增加多少好友度
	FIELD(INT16			           ,m_AddRelationJoinFaBaoComplete);	//进入对方法宝世界游玩并达成任意一个完成条件,增加多少好友度
	FIELD(INT16			           ,m_AddRelationStealStone);			//窃取好友灵石,增加多少好友度
	FIELD(INT16			           ,m_AddRelationCancelBeGuard);		//取消被守护,增加多少好友度
	FIELD(INT16			           ,m_AddRelationCancelGuard);			//取消守护,增加多少好友度
	FIELD(INT16			           ,m_MaxMailNum);						//玩家邮件最多保存数量
	FIELD(INT16			           ,m_MaxSellGoodsNum);				//最多可以寄售的物品个数(帮派福利还可以增加5个)
	FIELD(INT16		               ,m_SpendStoneSellGoods);			//寄售一件物品需要花费的灵石数量
	FIELD(INT32			           ,m_MaxSellTradeGoodsTime);			//寄售物品的最大时长(秒)
	FIELD(INT16                    ,m_MaxFreeEnterFuBenNum);           //免费进入副本最大次数
	FIELD(INT16					   ,m_MaxCostStoneEnterFuBenNum);	   ///支付灵石进入最大次数
	FIELD(INT16                    ,m_MaxCostEnterFuBenNum);           ///支付仙石进入最大次数
	FIELD(INT16					   ,m_EnterFuBenCostStone);			   ///进入副本的灵石费用
	FIELD(INT16                    ,m_EnterFuBenCost);                 //进入副本的费用
	FIELD(std::vector<INT16>       ,m_vectHitScopeParam);              //命中范围参数
	FIELD(std::vector<INT16>       ,m_PhysicsKnockingParamAndProb);    //物理爆击参数及几率,单位千分之
	FIELD(std::vector<INT16>       ,m_MagicKnockingParamAndProb);      //法术爆击参数及几率,单位千分之
	FIELD(std::vector<INT16>       ,m_vectMagicDamageScopeParam);      //法术伤害范围参数单位(千分之)
	FIELD(INT16                    ,m_SameWuXinParam);  //仙剑五行和法术五行属性相同时的法术五行参数单位(千分之)
	FIELD(INT16                    ,m_DifferentWuXinParam);  //仙剑五行和法术五行属性不相同时的法术五行参数单位(千分之)
	FIELD(UINT16				   ,m_CDFreeFlushTime);				   //剑冢免费刷新的CD时间
	FIELD(UINT16				   ,m_SpendMoneyNumMoneyFlush);		   //剑冢付费刷新一次多少仙石
	FIELD(UINT16				   ,m_SpendMoneyFlush);				   //聚仙楼付费刷新一次多少仙石
	FIELD(UINT16                   ,m_MaxEnterSynCombatNum);           //每天挑战帮派保卫战的最大次数

	FIELD(TGoodsID                 ,m_gidMagicXLPropID);                  //修炼法术需要的道具
	FIELD(INT32                    ,m_MagicXLTime);                       //法术修炼时间 (单位:小时)

	FIELD(INT16                    ,m_MaxGroupCureTargetNum);             ///群体治疗最大目标数
	FIELD(INT16                    ,m_MaxStatusGroupAttackTargetNum);           ///带状态的群体攻击法术最大目标数
	FIELD(INT16                    ,m_MaxNoStatusGroupAttackTargetNum);           ///不带状态的/群体攻击法术最大目标数
	FIELD(INT16                    ,m_MaxGroupAttackNpcTargetNum);           ///群体攻击NPC法术最大目标数

	FIELD(TMapID                   ,m_MainMapID);             //主城地图ID
	FIELD(TMapID                   ,m_HouShanMapID);          //后山
	FIELD(TMapID                   ,m_FuMoDongMapID);          //伏摩洞

	FIELD(INT16                   ,m_MaxSpawnTalismanNum);          ///可孕育法宝的最大数量

	FIELD(INT16                   ,m_SpawnTalismanTime);            ///法宝孕育时长(单位：小时)

	FIELD(std::vector<TGoodsID>   ,m_vectUpgradeTalismanQualityProp);  ///提升法宝品质点需要的道具

	FIELD(UINT16				   ,m_SynCombatBeginTime);				  //帮派保卫战开启时间(百千位数为小时，个十位数为分钟)
	FIELD(UINT16				   ,m_SynCombatCloseTime);				  //帮派保卫战关闭时间(百千位数为小时，个十位数为分钟)

	FIELD(INT32                   ,m_MaxGodSwordNimbus );       ///仙剑灵气最大值

	FIELD(UINT16				   ,m_UpdateMinuteNum);				//修炼每几分钟更新一次

	FIELD(INT32					   ,m_MagicXiuLianNeedRelation);	//好友法术教学所需好友度值设定

	FIELD(UINT16				   ,m_BackUnloadXiuLianRecordTime); //修炼记录数据延时几秒卸载
	FIELD(INT16                    ,m_TamlismanGameReadyCountDown); ///法宝世界游戏准备阶段倒计时长度,单位:秒
	FIELD(UINT8					   ,m_MinLevelCreateSyn);			///创建帮派的最低等级要求
	FIELD(UINT8					   ,m_MinLevelJoinSyn);				///最低加入帮派等级

	FIELD(UINT16				   ,m_MaxBuyGoodsNum);				///商城一次最大购买物品数量

	FIELD(UINT32				   ,m_BuildRecordSaveTime);			///资源建筑记录保存多久(秒)

	FIELD(TCDTimerID			   ,m_WorldTalkCDTimerID);			///世界聊天的冷却ID

	FIELD(TCDTimerID			   ,m_SynTalkCDTimerID);			///帮派聊天的冷却ID

	FIELD(TCDTimerID			   ,m_WorldViewGoodsTimerID);		///世界展示物品的冷却ID

	FIELD(INT32					   ,m_MaxGetHonorDouFaToday);		///斗法每日获得荣誉值上限

	FIELD(INT32					   ,m_DouFaWinGetCredit);			///斗法赢一场获得声望值

	FIELD(INT32					   ,m_DouFaFailLostCredit);			///斗法输一场失去声望值

	FIELD(INT32					   ,m_DouFaWinGetHonor);			///斗法战斗一轮胜利得到的荣誉值

	FIELD(UINT8					   ,m_DouFaRandEnemyNum);			///斗法随机敌人个数

	FIELD(UINT8					   ,m_NeedUserNumShanDian);			///山巅之战需要人数
	FIELD(UINT8					   ,m_NeedUserNumHuangJiao);		///荒郊之战需要人数
	FIELD(UINT8					   ,m_NeedUserNumMiShi);			///密室之战需要人数

	FIELD(UINT8					   ,m_SynGoodsTaskNum);				///帮派物品任务每天抽取个数

	FIELD(TMapID				   ,m_DouFaMapID);					///斗法战斗地图ID

	FIELD(TMapID				   ,m_QiCuoMapID);					///切磋战斗地图ID

	FIELD(INT16					   ,m_MaxAptitude);					///资质上限
	FIELD(INT16					   ,m_MaxNenLiPuTong);				///能力为普通的资质最大
	FIELD(INT16					   ,m_MaxNenLiLiangHao);			///能力为良好的资质最大
	FIELD(INT16					   ,m_MaxNenLiYouZhi);				///能力为优质的资质最大
	FIELD(UINT16				   ,m_StoneFlushMagicBook);			///聚仙楼付费刷新法术书一次多少灵石
	FIELD(std::vector<INT16>       ,m_vectEquipStrongPropRate);      ///装备强化1~5级时，增加的基础属性的比例(单位:百分之)
	FIELD(UINT16                   ,m_AttackFloatLowLimit); //攻击力浮动下限,单位:千分之
	FIELD(UINT16                   ,m_AttackFloatHigLimit); //攻击力浮动上限,单位:千分之
	FIELD(UINT16                   ,m_MaxTrainingNum); //每次挂机最大练功次数
	FIELD(UINT16                   ,m_TrainingTimeSpace);   //每次练功时长，单位:秒
	FIELD(INT16                    ,m_MaxOnhookNumOfDay); ///每天最大免费挂机次数,（不包括帮派福利增加的挂机次数）
	FIELD(std::vector<UINT16>	   ,m_PeiYuanDanRand);	  ///培元丹使用成功的概率(资质1,资质2,概率....  表示从资质1到资质2的概率)
	FIELD(INT16					   ,m_TrainingFinishNowPrice); ///练功立即完成需要的仙石或礼卷数
	FIELD(UINT32				   ,m_RankFlushTimeNum);  ///排行榜多久更新一次(秒)
	FIELD(UINT16				   ,m_CanJieSanMemberNum); ///多少人以下才允许解散帮派
	FIELD(UINT8					   ,m_Capacity_TipPopUpBox); ///背包容量少于多少时，战斗后弹框提示
	FIELD(INT16                    ,m_OffLineNumOnlineAwardReset ); ///连接下线多少次后，在线领奖失效
	FIELD(TMapID					,m_VisitFriendMap);		///拜访好友地图
	FIELD(std::string				,m_KFMailAddress);		///客服邮件发件箱地址
	FIELD(std::string				,m_KFMailPassword);		///客服邮件发件箱密码
	FIELD(std::string				,m_KFMailServer);		///发件箱的服务器
	FIELD(std::string				,m_KFMailDesAddress);	///客服邮件收件箱地址
	FIELD(UINT32					,m_KFMailCDTimerID);	///发客服邮件冷却ID
	FIELD(UINT8						,m_MaxJoinChallenge);	///每天参加挑战上限
	FIELD(UINT8						,m_FlushMyEnemyNum);	///挑战每次刷新多名自己可挑战玩家
	FIELD(TMapID					,m_ChallengeCombatMap);	///挑战的战斗地图
	FIELD(INT32						,m_ChallengeWinGetHonorNum);///挑战胜利获得多少荣誉
	FIELD(UINT8                     ,m_WallowSwitch);		///防沉迷开关,0关闭,1打开
	FIELD(UINT8						,m_QieCuoEnemyNum);		///切磋获取的对象数量
	FIELD(TCDTimerID				,m_QieCuoCDTimerID);	///切磋的冷却ID
	FIELD(UINT8						,m_ChallengeRankGetForward); ///挑战从第几名开始可获得奖励
	FIELD(INT16						,m_AloneXLNimbusParam);	///独自修炼灵气获取参数
	FIELD(INT16						,m_AloneXLGodSwordNimbusParam);///独自修炼仙剑灵气获取参数
	FIELD(INT16						,m_TwoXLNimbusParam);	///双修灵气获取参数
	FIELD(INT32						,m_NumMoney_OneYuan);	///一元钱等于多少仙石
	FIELD(UINT32					,m_ClearWallow_OfflineTimeNum); ///下线多久，防沉迷清0
	FIELD(std::vector<UINT8>		,m_AutoFlushChooseNum);			///自动刷新可选择次数
	FIELD(std::vector<INT32>		,m_AutoFlushGodStone);			///自动刷新费用
	FIELD(std::vector<UINT16>		,m_AutoFlushChooseAptitude);	///自动刷新可选择资质
	FIELD(UINT16					,m_MaxEnterGodSwordWorldNum);	  ///仙剑世界每天可进入次数
	FIELD(std::vector<INT32>		,m_vecVipUpRecharge);			///VIP升级对应充值金额表
	FIELD(UINT8						,m_LvNumCommonAndHardFB);		  ///多少级开始的副本有区分普通和困难模式
	FIELD(INT16						,m_UnEmployeeBackExpRate); ///解雇招募角色最多返还多少经验(百分比)
	FIELD(UINT8						,m_MaxAccelNum);			///每天最大挂机加速次数

	FIELD(std::vector<UINT8>		,m_XuanTianTime);			///玄天开启结束时间(开启小时，开启分钟，结束小时，结束分钟，开启小时……)

	FIELD(std::vector<UINT8>		,m_XuanTianLevelGroup);		///玄天伤害等级组

	FIELD(std::vector<UINT8>		,m_SynCombatTime);			///帮战开启结束时间(开启小时，开启分钟，结束小时，结束分钟)

	FIELD(std::vector<UINT8>		,m_SynCombatDeclareTime);	///帮战开始结束宣战时间

	FIELD(INT32						,m_BuyCombatNumGodStone);	///购买战斗次数的仙石数

	FIELD(UINT16					,m_BuyCanAddCombatNum);		///一次可购买多少战斗次数

	FIELD(std::vector<UINT16>		,m_SynWarPosPoint);			///帮战的坐标

	FIELD(UINT32					,m_OccupationTimeNum);		///法宝世界占领时长

	FIELD(UINT8						,m_MaxEnterTWNum);			///每天最多可进入法宝世界次数

	FIELD(UINT32					,m_DuoBaoReadyTimeNum);		///夺宝战斗准备时间

	FIELD(std::vector<UINT32>		,m_DuoBaoTime);				///夺宝开始时间和持续时间(开启小时，开启分钟，持续时间)

	FIELD(UINT32					,m_XuanTianBuyCDMoney);		///玄天多少仙石可以缩短CD时长

	FIELD(INT16						,m_PolyNimbusAddGhost);		///法宝附灵需消耗的聚灵气

	FIELD(std::vector<UINT8>		,m_CanEmployNum);			///可以招募角色的个数

	FIELD(UINT8				,m_ChallengeRankGetForward_B);		///百分之几之内的玩家可获得B类奖励(百分比)

	FIELD(UINT8				,m_EnterMinLevel);					///玄天最低进入等级

	FIELD(UINT8						,m_ChallengeRecordNum);		///挑战回放最多可回放几场

	FIELD(UINT32					,m_DuoBaoRunTimeNum);			///逃跑惩罚时长

	FIELD(INT16                     ,m_CombatNullMagicCDTime);  ///战斗空法术CD时间 ,单位:秒

	FIELD(INT32						,m_AccelKilNeedMoney);		///伏魔洞加速挂机消耗仙石数

	FIELD(INT32						,m_BuyChallengeNumGodStone);	///购买挑战战斗次数的仙石数

	FIELD(UINT16					,m_BuyCanAddChellengeNum);		///一次可购买多少挑战战斗次数

	FIELD(UINT32					,m_TotalXiWuTime);				///习武总时间
	FIELD(UINT32					,m_XiWuNeedStoneNum);			///习武消耗的灵石数

	FIELD(UINT8						,m_FirstRechargeID);			///首次充值活动ID

	FIELD(UINT32					,m_MaxTrainingTime);			///练功最大时间值
 
	FIELD_END();

	//仅服务器端用到的系数
	SGameServerConfigParam           m_ServerConfigParam;

};




//绑定方式
enum enMonsterType VC_PACKED_ONE
{
	enMonsterType_Xiao    = 0 , //不绑定
	enMonsterType_Main    = 1,  //主将
	enMonsterType_Boss  = 2,  //boss
}PACKED_ONE;


//怪物分布
struct SMonsterOutput
{
	SMonsterOutput()
	{
		memset(this,0,sizeof(*this));
	}

	FIELD_BEGIN();
	FIELD(TMapID             ,m_MapID);
	FIELD(TMonsterID         ,m_MonsterID);
	FIELD(UINT16             ,m_PointX);
	FIELD(UINT16             ,m_PointY);
	FIELD(UINT8              ,m_nDir);
	FIELD(UINT8              ,m_nLineup);
	FIELD(UINT16             ,m_DropID);  //普通掉落
	FIELD(UINT16			 ,m_DropIDHard);	//困难掉落
	FIELD(UINT16			 ,m_DropIDTeam);	//组队掉落
	FIELD(UINT8              ,m_MonsterType); //怪物类型enMonsterType
	FIELD(UINT32             ,m_CombatIndex); //普通战怪索引
	FIELD(UINT32			 ,m_CombatIndexHard);//困难战怪索引
	FIELD(UINT32			 ,m_CombatIndexTeam);	//组队战斗索引
	FIELD(UINT8				 ,m_bCanReplace);		//是否可被替换

	FIELD_END();
};

//战斗地图怪物分布
struct SCombatMapMonster
{	
	SCombatMapMonster()
	{
		MEM_ZERO(this);
	}

	FIELD_BEGIN();
	FIELD(UINT32             ,m_CombatIndex);     //索引
	FIELD(TMapID             ,m_CombatMapID);     //战斗地图
	FIELD(UINT8					,m_MainMonsterIndex);//主将索引
	FIELD(TMonsterID             ,m_MonsterID1);      //小怪1
	FIELD(TMonsterID             ,m_MonsterID2);      //小怪2
	FIELD(TMonsterID              ,m_MonsterID3);       //小怪3
	FIELD(TMonsterID              ,m_MonsterID4);      //小怪4
	FIELD(TMonsterID             ,m_MonsterID5);     //小怪5
	FIELD(TMonsterID             ,m_MonsterID6);     //小怪6
	FIELD(TMonsterID              ,m_MonsterID7);      //小怪7
	FIELD(TMonsterID             ,m_MonsterID8);     //小怪8
	FIELD(TMonsterID             ,m_MonsterID9);     //小怪9

	FIELD_END();
};

//地图配置信息
struct SMapConfigInfo
{
	SMapConfigInfo()
	{
		m_MapID = 0;
		m_keepliveTime = 0;
		m_DropID = 0;
		m_DropIDHard = 0;
	}

	FIELD_BEGIN();
	FIELD(TMapID               ,m_MapID);
	FIELD(std::string          ,m_strMapFileName);
	FIELD(INT32                ,m_keepliveTime);
	FIELD(std::vector<UINT8>    ,m_vectBuildingType);
	FIELD(UINT16               ,m_DropID); //掉落物品及几率 
	FIELD(UINT16				,m_DropIDHard); //困难掉落
	FIELD(std::vector<TPortalID>  ,m_vectPortal); //传送门分布
	FIELD(UINT16               ,m_ResID); //资源ID
	FIELD(UINT16               ,m_UIType); //UI类型
	FIELD(INT32               ,m_MapLanguageID);//地图语言ID	//fly add

	FIELD_END();

};

//角色初始数据
struct SCreateActorCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8   ,m_Level);       //等级
	FIELD(UINT16  ,m_Exp);         //经验
	FIELD(UINT8   ,m_Layer);       //境界
	FIELD(UINT16  ,m_Nimbus);      //灵气
	FIELD(UINT16  ,m_Spirit);      //灵力
	FIELD(UINT16  ,m_Shield);      //护盾
	FIELD(UINT16  ,m_Avoid);       //身法
	FIELD(UINT16  ,m_Blood);       //气血
	FIELD(UINT16  ,m_Aptitude);    //资质
	FIELD(UINT16  ,m_Ability); //能力
	FIELD(UINT16  ,m_NimbusSpeed);	//灵气速率
	
	FIELD(INT32						 ,m_Crit);						//爆击
	FIELD(INT32						 ,m_Tenacity);					//坚韧
	FIELD(INT32						 ,m_Hit);						//命中
	FIELD(INT32						 ,m_Dodge);						//回避
	FIELD(INT32						 ,m_MagicCD);					//法术回复

	FIELD(std::vector<TGoodsID>  ,m_vectEquip);  //装备
	FIELD(std::vector<TGoodsID>  ,m_vectPacket);  //背包物品及数量

	FIELD(INT32						 ,m_GoldDamage);					//金剑诀伤害
	FIELD(INT32						 ,m_WoodDamage);					//木剑诀伤害
	FIELD(INT32						 ,m_WaterDamage);					//水剑诀伤害
	FIELD(INT32						 ,m_FireDamage);					//火剑诀伤害
	FIELD(INT32						 ,m_SoilDamage);					//土剑诀伤害

	SCreateActorCnfg()
	{
       MEM_ZERO(this);
	}
	

	FIELD_END();
};


//怪物配置数据
struct SMonsterCnfg
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

	SMonsterCnfg()
	{
       MEM_ZERO(this);
	}


	FIELD_END();
};


//伏魔洞数据配置
struct SFuMoDongCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8,                 m_Level); //层次
	FIELD(UINT8,                 m_ActorLevel); //需要角色达到的等级
	FIELD(TMonsterID             ,m_MonsterID);  //怪物
	FIELD(TMapID                 ,m_CombatIndex); //战斗索引
	FIELD(UINT16                 ,m_DropID);      //掉落ID
	FIELD(INT32                 ,m_LanguageID);      //地图名称语言ID

	
	SFuMoDongCnfg()
	{
       MEM_ZERO(this);
	}


	FIELD_END();
};

//绑定方式
enum enBindType VC_PACKED_ONE
{
	enBindType_Non    = 0 , //不绑定
	enBindType_Get    = 1,  //获取绑定
	enBindType_Equip  = 2,  //装备绑定
}PACKED_ONE;


//物品配置
struct SGoodsCnfg
{
	SGoodsCnfg()
	{
		m_GoodsID = 0;
		m_szName[0] = 0;
		m_GoodsClass = 0;
		m_SubClass = 0;
		m_ThirdClass = 0;
		m_UsedLevel = 0;		
		m_SellPrice = 0;
		m_GoodsLevel = 0;
		m_PileNum = 0;
		m_Quality = 0;
		m_BindType = 0;
		m_LifeTime = 0;
		m_SpiritOrMagic = 0;
		m_ShieldOrWuXing = 0;
		m_BloodOrSwordkee = 0;
		m_AvoidOrSwordLvMax = 0;
		m_SuitIDOrSwordSecretID = 0;
		m_bIsGod = 0;
	}

	FIELD_BEGIN();
	FIELD(TGoodsID                      ,m_GoodsID);  //GoodsID
	FIELD(TInt8Array<THING_NAME_LEN>    ,m_szName);   //名称 
	FIELD(std::string                   ,m_strDesc);  //描述
	FIELD(UINT8                         ,m_GoodsClass); //一级分类
	FIELD(UINT8                         ,m_SubClass);   //二级分类
	FIELD(UINT8                         ,m_ThirdClass);   //三级分类
	FIELD(UINT8                         ,m_UsedLevel);  //使用等级
	FIELD(INT32                         ,m_SellPrice);  //售价
	FIELD(UINT8                         ,m_GoodsLevel); //物品等级
	FIELD(UINT8                         ,m_PileNum);    //叠加数量上限
	FIELD(UINT8                         ,m_Quality);    //品质
	FIELD(UINT8                         ,m_BindType);   //绑定类型
	FIELD(INT32                         ,m_LifeTime);   //生存时间
	FIELD(INT32                         ,m_SpiritOrMagic);    //普通物品:是否可以多个使用,装备:灵力,法宝:法术ID,仙剑:法术伤害
	FIELD(INT32                         ,m_ShieldOrWuXing);    //护盾或五行,普通物品为是否具有指向性
	FIELD(INT32                         ,m_BloodOrSwordkee);    //气血或剑气,对于宝石：表示宝石属性ID,对于灵件表示增加的属性值
	FIELD(INT32                         ,m_AvoidOrSwordLvMax);    //身法或剑气级别上限,对于宝石：表示宝石属性值
	FIELD(INT16                         ,m_SuitIDOrSwordSecretID);   //套装ID或剑诀或法宝世界ID
	FIELD(INT16							,m_ResID);					 //资源ID
	FIELD(INT32							,m_GoodsLangID);			//物品语言ID		fly add
	FIELD(INT32							,m_GoodsDescLangID);  //物品描述语言ID
	FIELD(UINT8							,m_bIsGod);				//是否仙级(0:不是，1:是)

	FIELD_END();
};

//套装配置信息
struct SSuitCnfg
{
	FIELD_BEGIN();
	FIELD(UINT16            , m_SuitID); //套装ID
	FIELD(UINT8             ,m_SuitPropID1); //套装属性ID1
	FIELD(UINT16            , m_SuitPropValue1); //套装属性值1
	FIELD(UINT8            , m_SuitPropID2); //套装属性ID2
	FIELD(UINT16            , m_SuitPropValue2); //套装属性值2
	FIELD(UINT8            ,m_SuitTotalNum); //全套装备数

	FIELD_END();

};


//物品合成
struct SGoodsComposeCnfg
{
	SGoodsComposeCnfg()
	{
		memset(this,0,sizeof(*this));
	}
	FIELD_BEGIN();
	FIELD(TGoodsID             ,m_GoodsID);      //GoodsID
	FIELD(INT32                ,m_Charge);       //费用
	FIELD(std::vector<int>         ,m_vectMaterial);           //原料及数量


	FIELD_END();
};


//装备强化
struct SEquipStrongerCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8                ,m_level);        //强化等级
	FIELD(UINT8				   ,m_Type);		//强化类型  0为装备，1为法宝，2为仙剑
    FIELD(UINT8                ,m_HoleNum);     //拥有孔的数量
	FIELD(TGoodsID             ,m_gidMaterial);  //原料
	FIELD(INT16                ,m_MaterialNum);  //原料数量
	FIELD(INT16                ,m_SuccessRate);  //成功率(单位:千分之)
	FIELD(INT16                ,m_AddEquipPropRate); //增加装备属性比例(单位:千分之)
	FIELD(TGoodsID             ,m_gidExtender);   //增加剂
	FIELD(INT16                ,m_AddSuccessRate); //增加剂增加成功比例(单位:千分之)

	SEquipStrongerCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//宝石属性
struct SGemCnfg
{
	SGemCnfg()
	{
		m_GemType = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT8               ,m_GemType);   //宝石类型
	FIELD(std::vector<INT16>  ,m_vectAddEquipProp);//可增加装备属性

	FIELD_END();
};

//招募角色配置
struct SEmployeeDataCnfg
{
	FIELD_BEGIN();
	FIELD(TEmployeeID					,m_EmployeeID);		//角色ID
	FIELD(TInt8Array<THING_NAME_LEN>    ,m_szName);			//名称
	FIELD(UINT16						,m_FreeFlushRate);  //免费刷新几率
	FIELD(UINT16						,m_FlushRate);		//付费刷新几率
	FIELD(UINT16						,m_Exp);			//经验
	FIELD(UINT8							,m_Level);			//等级
	FIELD(UINT16						,m_Nimbus);			//灵气
	FIELD(UINT8							,m_Layer);			//境界
	FIELD(UINT16						,m_Spirit);			//灵力
	FIELD(UINT16						,m_Shield);			//护盾
	FIELD(UINT16						,m_BloodUp);		//气血上限
	FIELD(UINT16						,m_Avoid);			//身法
	FIELD(UINT16						,m_Aptitude);		//资质	是浮点数
	FIELD(INT16							,m_ResID);			//资源
	FIELD(UINT16						,m_Price);			//价格
	FIELD(UINT8							,m_Sex);			//性别
	FIELD(UINT8							,m_NenLi);			//能力
	FIELD(UINT16						,m_NimbusSpeed);	//灵气速率
	FIELD(INT16							,m_NameLanguageID);			//名字语言ID	fly add
	FIELD(UINT32						,m_Crit);			//爆击
	FIELD(UINT32						,m_Tenacity);		//坚韧
	FIELD(UINT32						,m_Hit);			//命中
	FIELD(UINT32						,m_Dodge);			//回避
	FIELD(UINT32						,m_MagicCD);		//法术回复
	FIELD(UINT32						,m_GoldDamage);		//金剑诀伤害
	FIELD(UINT32						,m_WoodDamage);		//木剑诀伤害
	FIELD(UINT32						,m_WaterDamage);	//水剑诀伤害
	FIELD(UINT32						,m_FireDamage);		//火剑诀伤害
	FIELD(UINT32						,m_SoilDamage);		//土剑诀伤害

	SEmployeeDataCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//五行
enum enWuXing VC_PACKED_ONE
{
	enWuXing_Jing    = 0 , //金
	enWuXing_Mu    , //木
	enWuXing_Shui  ,  //水
	enWuXing_Huo ,    //火
	enWuXing_Tu,        //土
	enWuXing_Max,
}PACKED_ONE;

//法术书配置
struct SMagicBookCnfg
{
	SMagicBookCnfg()
	{
		m_MagicBookID	= 0;
		m_szName[0]		= 0;
		m_FreeFlushRate = 0;
		m_FlushRate		= 0;
		m_Price = 0;
	}

	FIELD_BEGIN();
	FIELD(TMagicBookID					,m_MagicBookID);	//法术书ID
	FIELD(TInt8Array<THING_NAME_LEN>	,m_szName);			//名称
	FIELD(UINT16						,m_FreeFlushRate);	//免费刷新几率
	FIELD(UINT16						,m_FlushRate);		//付费刷新几率
	FIELD(INT32							,m_Price);			//灵石数量

	FIELD_END();
};

//商城物品配置
//struct SShopMallCnfg
//{
//	SShopMallCnfg()
//	{
//		m_GoodsID = 0;
//		m_Amount = 0;
//		m_nMoney = 0;
//		m_nTicket = 0;
//		m_nStone = 0;
//		m_nHonor = 0;
//		m_bBinded = 0;
//		m_Type = 0;
//		m_EndTime = 0;
//	}
//
//	FIELD_BEGIN();
//	FIELD(TGoodsID						,m_GoodsID);		//物品ID
//	FIELD(UINT8							,m_Amount);			//数量
//	FIELD(INT32							,m_nMoney);			//仙石数量
//	FIELD(INT32							,m_nTicket);		//礼券数量
//	FIELD(INT32							,m_nStone);			//灵石数量
//	FIELD(INT32							,m_nHonor);			//荣誉数量
//	FIELD(UINT8							,m_Type);			//对应不同标签,0为热卖物品(用仙石购买)，1为普通物品(用仙石购买),2为宝石(用仙石购买)，3为得用礼券购买，4为得用灵石购买
//	FIELD(UINT8							,m_bBinded);		//是否要绑定
//	FIELD(UINT32						,m_EndTime);		//结束时间
//
//	FIELD_END();
//};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法宝品质
struct STalismanQualityCnfg
{
	STalismanQualityCnfg()
	{
		m_QualityLevel = 0;
		m_TalismanWorldLevel = 0;
		m_TalismanWorldType = 0;
		m_TotalProb = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT8					  	    ,m_QualityLevel);		//品质级别
	FIELD(UINT8						    ,m_TalismanWorldLevel);			//法宝级别
	FIELD(UINT8                         ,m_TalismanWorldType);                 //法宝世界类型enTalismanWorldType
	FIELD(std::vector<TGoodsID>	        ,m_vectGoodsAndProb);			//物品及概率，(GoodsID,概率)

	UINT32                               m_TotalProb;         //所有物品的总概率

	FIELD_END();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//法宝品质划分
struct STalismanQualityLevel
{
	FIELD_BEGIN();
	FIELD(UINT8					  	    ,m_QualityLevel);		//品质级别
	FIELD(UINT16					    ,m_QualityPoint);			//法宝品质点

	STalismanQualityLevel()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//效果值类型
enum enEffectValueType VC_PACKED_ONE
{
	enEffectValueType_Fixed    = 0 , //增减固定值
	enEffectValueType_Scale,         //比例(千分之)
	enEffectValueType_Final ,        //终值
	enEffectValueType_PropScale ,        //相对属性值的千分比
	enEffectValueType_RandomRange,      //随机范围
	enEffectValueType_Max,
}PACKED_ONE;

//效果配置
struct SEffectCnfg
{
	FIELD_BEGIN();
	FIELD(TEffectID					,m_EffectID);		//效果ID
	FIELD(UINT8					    ,m_EffectType);		//效果类型
	FIELD(INT16                     ,m_PropID);         //属性ID
	FIELD(UINT8                     ,m_ValueType);      //值类型
	FIELD(INT32                     ,m_Value);          //值	
	FIELD(INT32                     ,m_CommonParam);    //通用参数
	FIELD(std::string				,m_Descript);		//效果描述
	FIELD(UINT8						,m_bShowMsg);		//是否要显示在跑马灯公告栏上
	FIELD(INT32                     ,m_EffectDescLangID);  //效果描述语言ID	fly add

	SEffectCnfg()
	{
       MEM_ZERO(this);
	}


	FIELD_END();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//状态配置表
struct SStatusCnfg
{
	SStatusCnfg()
	{
		m_StatusID = 0;
		m_StatusType = 0;
		m_HitProb = 0;
		m_RecordType = 0;
		m_RoundNum = 0;
		m_TimeNum = 0;
		m_bAllActor = 0;
	}

	FIELD_BEGIN();
	FIELD(TStatusID					,m_StatusID);		//状态ID
	FIELD(UINT8                     ,m_StatusType);     //类型
	FIELD(std::string               ,m_strName);        //名称
	FIELD(std::vector<TEffectID>    ,m_vectEffect);     //效果列表
	FIELD(INT16                     ,m_HitProb);        //命中几率
	FIELD(UINT8						,m_RecordType);		//记录类型(1:持续回合数,2:持续时间)
	FIELD(INT16                     ,m_RoundNum);       //间隔次数
	FIELD(UINT32					,m_TimeNum);		//状态的持续时间
	FIELD(INT16                     ,m_EffectCount);     //生效次数
	FIELD(INT16                     ,m_AttackedCount);     //受攻击次数
	FIELD(std::vector<TStatusGroupID>	,m_vectGroupID);	//状态组ID
	FIELD(UINT16                    ,m_StatusResID); //动画
	FIELD(UINT8						,m_bShowInUserPanel);//是否显示在人物面板
	FIELD(UINT8						,m_bAllActor);			//是否对所有角色有效
	FIELD(UINT32               ,m_StatusNameLangID);   //状态名称语言ID	fly add


	FIELD_END();
};

//状态组配置表
struct SStatusGroupCnfg
{
	SStatusGroupCnfg()
	{
		m_StatusGroupID = 0;
		m_MaxStatusNum = 0;
		m_FullHandleType = 0;
	}

	FIELD_BEGIN();
	FIELD(TStatusGroupID			,m_StatusGroupID);	//状态组ID
	FIELD(UINT8						,m_MaxStatusNum);	//最多可同时容纳状态数
	FIELD(UINT8						,m_FullHandleType);	//状态数满时,再加状态时的处理类别
	FIELD(std::string				,m_TiShiDescript);	//提示文本

	FIELD_END();
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法术分类
enum enMagicClass VC_PACKED_ONE
{
	enMagicClass_Common    = 0 , //普通
	enMagicClass_Talisman =1,         //法宝
	enMagicClass_Sword =2 ,        //剑诀
	enMagicClass_Max,
}PACKED_ONE;

//法术效果类型
enum enMagicType VC_PACKED_ONE
{
	enMagicType_Damage    = 0 , //伤害
	enMagicType_Cure,         //治疗
	enMagicType_DamageAndCure,//对敌伤害，对自己治疗
	enMagicType_Max,
}PACKED_ONE;

//技能目标
enum enMagicTarget VC_PACKED_ONE
{
	enMagicTarget_Enemy   = 0 , //对敌
	enMagicTarget_Self,         //对自己	
	enMagicTarget_Max,
}PACKED_ONE;

//法术影响范围分类
enum enMagicAffectRange  VC_PACKED_ONE
{
	enMagicAffectRange_Single = 0, //单一
	enMagicAffectRange_Group = 1, //群体
	enMagicAffectRange_All = 2, //全体

} PACKED_ONE;

//法术配置信息
struct SMagicCnfg
{
	SMagicCnfg()
	{
		m_MagicID = 0;
		m_Class = 0;
		m_EffectType = 0;
		m_WuXing = 0;
		m_Target = 0;
		m_AffectRange = 0;
	}

	FIELD_BEGIN();
	FIELD(TMagicID					,m_MagicID);		//状态ID
	FIELD(std::string               ,m_strName);        //名称
	FIELD(UINT8                     ,m_Class);           //分类,取值enMagicClass
	FIELD(UINT8                     ,m_EffectType);           //效果类型,取值enMagicType
	FIELD(UINT8                     ,m_WuXing); //五行	
	FIELD(UINT8                     ,m_bPassive);       //是否是被动法术
	FIELD(UINT8                     ,m_Target); //目标,0对敌，1自己
	FIELD(UINT8                    ,m_AffectRange);      //影响范围
	FIELD(std::string				,m_strDescript);	//描述
	FIELD(UINT16					,m_IconID);			//图标资源ID
	FIELD(UINT16					,m_DongHuaID);		//动画资源ID
	FIELD(UINT16					,m_MagicLangID);		//法术名称语言ID	fly add
	FIELD(INT32                     ,m_CDTime);        //冷却时间

	FIELD_END();
};

//法术等级配置信息
struct SMagicLevelCnfg
{
	SMagicLevelCnfg()
	{
		m_MagicID = 0;
		m_Level = 0;
		m_NeedNimbus = 0;
		m_NeedLayer = 0;
		m_HitParam = 0;
		m_HitDdjustLevel = 0;
	}

	FIELD_BEGIN();
	FIELD(TMagicID					,m_MagicID);		//状态ID
	FIELD(UINT8                     ,m_Level);          //级别
	FIELD(std::vector<TEffectID>    ,m_vectEffect);     //效果列表
	FIELD(std::vector<TStatusID>    ,m_vectStatus);     //状态列表
	FIELD(std::vector<TStatusID>    ,m_vectTargetStatus);     //对方状态列表
	FIELD(INT32                     ,m_NeedNimbus);    //需要灵气
	FIELD(UINT8                     ,m_NeedLayer);    //需要境界
	FIELD(UINT16                    ,m_HitParam);      //命中参数(单位：%)
	FIELD(UINT8                     ,m_HitDdjustLevel); //命中调整等级
	FIELD(std::string				,m_Descript);		//描述
	FIELD(UINT16                     ,m_MagicLevelDescLangID); //法术等级描述语言ID		//fly add

	FIELD_END();
};

//帮派配置信息
struct SSyndicateCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8						,m_Level);			//帮派等级
	FIELD(UINT32					,m_UpLevelNeedExp);	//帮派升级需所经验
	FIELD(UINT16					,m_MaxMemberNum);	//该等级下帮派最多可容纳多少成员

	SSyndicateCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//帮派物品配置
struct SSynGoodsCnfg
{
	FIELD_BEGIN();
	FIELD(TGoodsID						,m_GoodsID);		//物品ID
	FIELD(UINT32						,m_Contribution);	//购买所需贡献值


	SSynGoodsCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//帮派技能配置
struct SSynMagicCnfg
{
	SSynMagicCnfg()
	{
		m_SynMagicID = 0;
		m_SynMagicLevel = 0;
		m_szSynMagicName[0] = 0;
		m_NeedContribution = 0;
		m_NeedStone = 0;
		m_NeedSynLevel = 0;
	}

	FIELD_BEGIN();
	FIELD(TSynMagicID					,m_SynMagicID);			//帮派技能ID
	FIELD(UINT8							,m_SynMagicLevel);		//帮派技能等级
	FIELD(TInt8Array<THING_NAME_LEN>	,m_szSynMagicName);		//帮派技能名称
	FIELD(std::vector<TEffectID>		,m_vectEffect);			//效果列表
	FIELD(std::string					,m_strSynMagicDes);		//帮派技能效果描述
	FIELD(UINT32						,m_NeedContribution);	//购买所需贡献值
	FIELD(UINT32						,m_NeedStone);			//购买所需灵石
	FIELD(UINT8							,m_NeedSynLevel);		//购买所需帮派等级
	FIELD(UINT16						,m_ResID);				//资源ID
	FIELD(UINT32						,m_SynMagicNameLangID);	//技能名称语言ID	fly add
	FIELD(UINT32						,m_SynMagicDescLangID);	//技能效果描述语言ID

	FIELD_END();
};

//帮派福利
struct SSynWelfareCnfg
{
	SSynWelfareCnfg() : m_SynWelfareID(0),m_WelfareType(0),m_NeedSynLevel(0), m_AddValue(0), m_ResID(0)
	{
	}

	FIELD_BEGIN();
	FIELD(TSynWelfareID					,m_SynWelfareID);		//帮派福利ID
	FIELD(UINT8							,m_WelfareType);		//帮派福利类型
	FIELD(std::string					,m_WelfareName);		//福利名称
	FIELD(UINT8							,m_NeedSynLevel);		//需求帮派等级
	FIELD(INT32							,m_AddValue);			//增加的值
	FIELD(UINT16						,m_ResID);				//资源ID
	FIELD(std::string					,m_WelfareDesc);		//帮派福利描述
	FIELD(INT32							,m_WelfareNameLangID);	//福利名称语言ID  //fly add
	FIELD(INT32							,m_WelfareDescLangID);	//福利描述语言ID

	FIELD_END();
};

//帮派保卫战
struct SSynCombatCnfg
{
	SSynCombatCnfg()
	{
		m_Mode = 0;
		m_DaYuanContribution = 0;
		m_NeiGeContribution = 0;
		m_DaTongExp = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT8                        ,m_Mode);				//模式
	FIELD(std::vector<UINT8>           ,m_vectEnterLevel);		//可进入的等级区间
	FIELD(TFuBenID                     ,m_SynFuBenID);			//帮派副本ID
	FIELD(INT16						   ,m_DaYuanContribution);	//大院贡献值
	FIELD(INT16						   ,m_NeiGeContribution);	//内阁贡献值
	FIELD(INT16						   ,m_DaTongExp);			//打通获得经验值
	FIELD_END();

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//剑冢配置
struct SGodSwordCnfg
{
	FIELD_BEGIN();
	FIELD(TGoodsID						,m_GodSwordID);			//仙剑物品ID
	FIELD(TInt8Array<THING_NAME_LEN>	,m_szGodSwordName);		//仙剑名字
	FIELD(UINT16						,m_FreeFlushRate);		//免费刷新几率
	FIELD(UINT16						,m_MoneyFlushRate);		//付费刷新几率
	FIELD(UINT32						,m_Stone);				//灵石数量
	FIELD(UINT8							,m_Level);				//等级

	SGodSwordCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//仙剑副本配置
struct SGodSwordFuBenCnfg
{
	FIELD_BEGIN();
	FIELD(TFuBenID                 ,m_FuBenID);  //副本ID
	FIELD(UINT8                    ,m_FuBenLevel); //副本等级
	FIELD(UINT8                    ,m_EnterLevel); //进入等级
	FIELD(TMapID                   ,m_MapID);      //对应的地图
	FIELD(UINT8 ,                   m_SwordSecretLevel); //奖励剑诀级别

	SGodSwordFuBenCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//副本类型
enum enFuBenType VC_PACKED_ONE
{
	enFuBenType_Common    = 0 , //伤害
	enFuBenType_Syndicate,         //帮派
	enFuBenType_GoldSword,         //仙剑
	enFuBenType_Max,
}PACKED_ONE;


//副本配置
struct SFuBenCnfg
{
	FIELD_BEGIN();
	FIELD(TFuBenID                 ,m_FuBenID);  //副本ID
	FIELD(UINT8                    ,m_EnterLevel); //进入等级
	FIELD(std::vector<TMapID>    ,m_MapID);      //对应的地图
	FIELD(std::string            ,m_strName); //副本名称
	FIELD(UINT8                   ,m_Type); //类型enFuBenType

	SFuBenCnfg()
	{
		m_FuBenID = 0;
		m_EnterLevel = 0;	
		m_Type = 0;
      
	}

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//物品使用
struct SGoodsUseCnfg
{
	SGoodsUseCnfg()
	{
		m_GoodsID = 0;
		m_GoodsUseType = 0;
		m_MaxUseNum = 0;
		m_CDTimerID = 0;
	}

	FIELD_BEGIN();

	 //物品ID
	FIELD(TGoodsID                 ,m_GoodsID);         

	FIELD(TInt8Array<THING_NAME_LEN>	,m_szGoodsName);	//仙剑名字

	FIELD(UINT8                    ,m_GoodsUseType);		//使用类型

	FIELD(std::vector<int>         ,m_vectParam);           //使用参数	

	FIELD(UINT32                   ,m_MaxUseNum);			//物品可使用次数

	FIELD(TCDTimerID			   ,m_CDTimerID);			//冷却ID,0表示没有冷却ID

	FIELD(UINT8					   ,m_CDTimeRange);			//冷却时间适用范围, 0:针对单个角色, 1:针对所有角色

	FIELD(std::string			   ,m_UseDesc);				//使用说明

	FIELD(UINT8						,m_bSelectUser);		//是否选择角色使用	

//fly add
	FIELD(std::string			   ,m_UseFail1);				//失败提示1

	FIELD(std::string			   ,m_UseFail2);				//失败提示2

	FIELD(std::string			   ,m_UseFail3);				//失败提示3

	FIELD(UINT16						,m_UseDescLangID);		//使用说明语言ID

	FIELD_END();

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//状态类型配置
struct SStatusTypeCnfg
{
	FIELD_BEGIN();
	  FIELD(UINT8                         ,m_StatusType);  //状态类型
	  FIELD(UINT8                         ,m_Priority);   //同一优先级的状态不能并存
	  FIELD(UINT16                        ,m_PhysicsHitAttackStatusParam);   //物理命中攻方状态参数
	  FIELD(UINT16                        ,m_PhysicsHitAttackedStatusParam);   //物理命中守方状态参数

	  FIELD(UINT16                        ,m_MagicHitStatusParam);   //法术命中状态参数

	   FIELD(UINT16                        ,m_PhysicsKnockingAttackStatusParam);   //攻方物理爆击状态参数

	   FIELD(UINT16                        ,m_PhysicsKnockingAttackedStatusParam);   //守方物理爆击状态参数

	  FIELD(UINT16                        ,m_MagicKnockingStatusParam);   //法术爆击状态参数

	  FIELD(UINT16                        ,m_PhysicsDamageAttackStatusParam);   //物理伤害攻方状态参数
	  FIELD(UINT16                        ,m_PhysicsDamageAttackedStatusParam);   //物理伤害守方状态参数


	  FIELD(UINT16                        ,m_MagicDamageStatusParam);   //法术伤害状态参数

	  SStatusTypeCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//角色等级配置
struct SActorLevelCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8                         ,m_Level); //等级
	FIELD(INT32                         ,m_NeedExp); //需要经验


	  SActorLevelCnfg()
	{
       MEM_ZERO(this);
	}


	FIELD_END();

};

//人物境界
struct SActorLayerCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8,					  m_LayerLevel);		   //境界等级
	FIELD(TInt8Array<THING_NAME_LEN>, m_LayerName);			   //境界名称
	FIELD(UINT8,					  m_NeedLevel);			   //升级需求等级
	FIELD(INT32,					  m_AddLevelDecNimbusNum); //提升境界等级需扣除玩家身上的灵气数
	FIELD(INT16,					  m_NimbusSpeed);		   //灵气速率
	FIELD(UINT32,					  m_NimbusUp);			   //灵气上限
	FIELD(INT32,					  m_AddAptitude);		   //增加的资质
	FIELD(INT32,					  m_AddSpirit);			   //增加的灵力
	FIELD(INT32,					  m_AddShield);			   //增加的护盾
	FIELD(INT32,					  m_AddBlood);			   //增加的气血
	FIELD(INT32,					  m_AddAvoid);			   //增加的身法

	  SActorLayerCnfg()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

//掉落配置
struct SDropGoods
{
	SDropGoods()
	{
		m_DropID = 0;
		m_DropNum = 0;
		m_nTotalDropGoodsRand = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT16,					  m_DropID);				//掉落ID
	FIELD(UINT8,		              m_DropNum);			//掉落物品的数量
	FIELD(std::vector<TGoodsID>,	  m_vectDropGoods);			//掉落物品,数量,几率,.....

	INT32	m_nTotalDropGoodsRand;	//总的掉落物品几率

	FIELD_END();
};

//修炼时的好友附加值
struct SXiuLianFriendAdd
{
	FIELD_BEGIN();
	FIELD(INT32,					   m_RelationRand);			//好友度范围
	FIELD(INT32,					   m_AddNum);				//好友附加值,多增加百分之几

	 SXiuLianFriendAdd()
	{
       MEM_ZERO(this);
	}

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//剑印世界配置
struct SGodSwordWorldCnfg
{
	FIELD_BEGIN();
	FIELD(TSwordSecretID,           m_SwordSecretID);   //剑诀
	FIELD(TMagicID ,                m_MagicID);         //法术
	FIELD(TFuBenID ,                m_FuBenID);         //副本ID

	
	 SGodSwordWorldCnfg()
	{
       MEM_ZERO(this);
	}
	

	FIELD_END();
};

//仙剑级别配置
struct SGodSwordLevelCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8                    ,m_Level);       //等级
	FIELD(INT32                    ,m_NeedNimbus);  //需要灵气
	FIELD(std::vector<UINT16>	   ,m_vectParam);	//需要材料,格式：物品ID,数量

	FIELD_END();

	 SGodSwordLevelCnfg()
	{
       MEM_ZERO(this);
	}
	

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum enPortalTarget VC_PACKED_ONE
{
	enPortalTarget_MainUI    = 0 , //主UI
	enPortalTarget_FuMeDong,         //伏魔洞
	enPortalTarget_HouShan ,        //后山
	enPortalTarget_LastScene ,        //上一个场景
	enPortalTarget_Assign,       //指定的场景
	enPortalTarget_Max,
}PACKED_ONE;


//传送门配置
struct SPortalCnfg
{
	FIELD_BEGIN();
	FIELD(TPortalID                  , m_PortalID);
	FIELD(TInt8Array<THING_NAME_LEN> , m_szName);  //名称
	FIELD(UINT8                      , m_Target) ;  //传送目的地enPortalTarget

	FIELD_END();

	 SPortalCnfg()
	{
       MEM_ZERO(this);
	}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法宝世界分类
enum enTalismanWorldType  VC_PACKED_ONE
{
	enTalismanWorldType_XiuLian           = 0,  //修炼类
	enTalismanWorldType_Clear             = 1, //清理类
	enTalismanWorldType_Gather            = 2, //采集
	enTalismanWorldType_Answer            = 3, //答题
	enTalismanWorldType_Treasure          = 4, //寻宝
	enTalismanWorldType_Max,

} PACKED_ONE;


//法宝世界配置
struct STalismanWorldCnfg
{
	STalismanWorldCnfg()
	{
		m_TalismanWorldID = 0;
		m_WorldType = 0;
		m_EnterLevel = 0;
		m_MapID = 0;
		m_TotalGameTime = 0;
	}


	FIELD_BEGIN();
	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	FIELD(std::string              ,m_strName);  //世界名称
	FIELD(UINT8                    ,m_WorldType);  //世界分类 enTalismanWorldType
	FIELD(UINT8                    ,m_EnterLevel);   //进入等级
	FIELD(TMapID                   ,m_MapID);        //场景地图ID
	FIELD(INT32                    ,m_TotalGameTime); //游戏时长

	FIELD_END();

	
};


//修炼类法宝世界配置参数
struct STalismanWorldXiuLianCnfg
{
	FIELD_BEGIN();

	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	
	FIELD(UINT16                   ,m_WhiteAirMassNum); //白气团数

	FIELD(UINT16                   ,m_BlackAirMassNum); //黑气团数	

	FIELD_END();

	 STalismanWorldXiuLianCnfg()
	{
       MEM_ZERO(this);
	}

};

//修炼游戏达成级别及奖励
struct SXiuLianGameAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8              ,m_FinishLevel);          //完成级别
	FIELD(UINT16             ,m_AwardQuality);         //奖励品质
	FIELD(INT16              ,m_DeterBlackAirMassNum); //阻止黑气团数
	FIELD(INT16              ,m_AllowWhiteAirMassNum); //允许破坏白气团数

	FIELD_END();

	 SXiuLianGameAwardCnfg()
	{
       MEM_ZERO(this);
	}

};

//修炼游戏配置参数
struct SXiuLianGameParam
{
	SXiuLianGameParam()
	{
		m_WhiteAirMassNimbus = 0;
		m_BlackAirMassNimbus = 0;
		m_MinGenerateAirMassIntervalTime = 0;
		m_SameAddrGenerateAirMassIntervalTime = 0;
	}


	FIELD_BEGIN();

	FIELD(INT16                   ,m_WhiteAirMassNimbus);       //白色气团代表的灵气值
	
	FIELD(INT16                   ,m_BlackAirMassNimbus); //黑色气团代表的灵气值

	FIELD(std::vector<INT32>      ,m_AirMassLifeTimeByAddr); //各地址所产生气团的存活时间


	FIELD(INT32                   ,m_MinGenerateAirMassIntervalTime); //气团产生最小间隔(单位：毫秒, 1000毫秒 = 1秒)

	FIELD(UINT16                   ,m_SameAddrGenerateAirMassIntervalTime); //同一地址产生气团间隔(单位：毫秒, 1000毫秒 = 1秒)

	FIELD(std::vector<std::vector<UINT16> >, m_vectAirMassTimeScale); //云团时间比例集合，从中随机选一种

	FIELD_END();
};

//清理类游戏配置
struct SQingLiGameConfig
{
	FIELD_BEGIN();
	FIELD(TTalismanWorldID,			m_TalismanWorldID);		//法宝世界ID
	FIELD(UINT16,					m_DiShuGuaiNum);		//地鼠怪数量
	FIELD(UINT16,					m_DiShuJingNum);		//地鼠怪数量
	FIELD(UINT16,					m_ShuJingNum);			//树精数量
	FIELD_END();
};

//清理类游戏配置参数
struct SQingLiGameParam
{
	SQingLiGameParam() : m_DiShuGuaiExpParam(0),m_DiShuJingExpParam(0),m_ShuJingExpParam(0),m_DiShuGuaiTime(0),
							m_DiShuJingTime(0),m_ShuJingTime(0),m_BornTimeLong(0),m_CanHitNum(0),
							m_SamePosTimeNum(0),m_SamePosDiShuJingNum(0),m_DiShuJingCanHitNum(0)
	{
	}
	FIELD_BEGIN();
	FIELD(INT16						,m_DiShuGuaiExpParam);	///地鼠怪经验参数
	FIELD(INT16						,m_DiShuJingExpParam);	///地鼠精经验参数
	FIELD(INT16						,m_ShuJingExpParam);	///树精经验参数
	FIELD(UINT32					,m_DiShuGuaiTime);		///地鼠怪持续存活时间(ms)
	FIELD(UINT32					,m_DiShuJingTime);		///地鼠精持续存活时间(ms)
	FIELD(UINT32					,m_ShuJingTime);		///树精持续存活时间(ms)
	FIELD(UINT32					,m_BornTimeLong);		///多久产生一个地鼠(ms)
	FIELD(UINT8						,m_CanHitNum);			///可击打单位数
	FIELD(UINT32					,m_SamePosTimeNum);		///同一个地方出地鼠的时间间隔(s)
	FIELD(UINT32					,m_SamePosDiShuJingNum);///出过地鼠精的位置在地鼠精消失后多久内不能出现任何类型的地鼠(s)
	FIELD(UINT8						,m_DiShuJingCanHitNum); ///地鼠精可击打次数
	FIELD(UINT32					,m_TimerDiShuIsDelete); ///多久判断一次地鼠持续时间是否结束(ms)
	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//寻宝类法宝世界配置参数,对应TalismanWorldXunBao.csv
struct SXunBaoGameConfig
{
	FIELD_BEGIN();

	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	
	FIELD(UINT16                   ,m_TotalDetectNum); //总探测次数

	FIELD(UINT16                   ,m_BaoGoodsNum); //可以获得的宝物数量

	FIELD(std::vector<UINT16>      ,m_vectGoods); //可选的宝物,叠加数及概率

	FIELD_END();

	INT32     m_TotalProbability;  //总概率

	 SXunBaoGameConfig()
	{
       MEM_ZERO(this);
	}

};

//寻宝游戏达成级别及奖励
struct SXunBaoGameAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8              ,m_FinishLevel);          //完成级别
	FIELD(UINT16             ,m_AwardQuality);         //奖励品质
	FIELD(INT16              ,m_ObtainGoodsNum);       //获得宝物数
	FIELD(INT16              ,m_TotalTimeLimit);       //时间限制

	FIELD_END();

	 SXunBaoGameAwardCnfg()
	{
       MEM_ZERO(this);
	}

};

//寻宝游戏配置参数
struct SXunBaoGameParam
{
	SXunBaoGameParam()
	{
		m_XDetectPoint = 0;
		m_YDetectPoint = 0;
	}

	FIELD_BEGIN();

	FIELD(INT16                                ,m_XDetectPoint);   //X坐标探测点数量

	FIELD(INT16                                ,m_YDetectPoint);   //Y坐标探测点数量
	
	FIELD(std::vector<UINT16>                   ,m_vectColorRange); //color黑红黄绿分别表示附近多少格有宝物，0表示无宝物

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//答题游戏

//题库
struct STiMuData
{
	FIELD_BEGIN();
	FIELD(UINT16								,m_TiMuID);		//题目ID
	FIELD(UINT8									,m_TiMuType);	//题目类型
	FIELD(std::string							,m_TiMuText);	//题目文本
	FIELD(std::string							,m_AnserA);		//答案A
	FIELD(std::string							,m_AnserB);		//答案B
	FIELD(std::string							,m_AnserC);		//答案C
	FIELD(std::string							,m_AnserD);		//答案D
	FIELD(UINT8									,m_RightAnser);	//正确答案索引(0,1,2,3对应答案A,B,C,D)

	FIELD_END();
};

//
struct SDaTiGameCnfg
{
	FIELD_BEGIN();
	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	FIELD(UINT8					   ,m_JXCQTiMuNum);			  //剑仙传奇知识题目数量
	FIELD(UINT8					   ,m_GameTiMuNum);			  //游戏知识题目数量
	FIELD(UINT8					   ,m_XianXiaTiMuNum);		  //仙侠题目数量
	FIELD(UINT8					   ,m_ZheXueTiMuNum);		  //哲学题目数量
	FIELD(UINT8					   ,m_NJJZWTiMuNum);		  //脑筋急转弯题目数量

	FIELD_END();
};


//答题游戏配置参数
struct SDaTiGameParam
{
	FIELD_BEGIN();
	FIELD(UINT16				   ,m_MaxDaTiTime);				///每道答题最长时间(ms)
	FIELD(UINT16				   ,m_RightTiMuNumOneLevel);	///游戏一级奖励要答对的问题数
	FIELD(UINT16				   ,m_RightTiMuNumTwoLevel);	///游戏二级奖励要答对的问题数
	FIELD(UINT16				   ,m_RightTiMuNumThreeLevel);	///游戏三级奖励要答对的问题数(-1为所有)

	FIELD_END();
};

//答题游戏的奖励
struct SDaTiGameLevelAward
{
	FIELD_BEGIN();
	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	FIELD(INT32					   ,m_OneLevelAwardStone);	  //一级奖励灵石数量
	FIELD(std::vector<UINT16>	   ,m_vectTwoLevelAwardGoods);//二级奖励物品，格式：物品数量，物品ID，几率，物品ID，几率，．．．．
	FIELD(INT32					   ,m_ThreeLevelAwardTicket); //三级奖励礼券数量
	FIELD_END();
};

//答题游戏的完成级别奖励
struct SDaTiGameAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8              ,m_FinishLevel);          //完成级别
	FIELD(UINT16             ,m_AwardQuality);         //奖励品质
	FIELD(UINT8				 ,m_RateTiMu);			   //答对题数的比例
	FIELD(UINT16			 ,m_RightTiMuNum);		   //答对和题数
	FIELD(UINT16			 ,m_TimeGameOver);		   //答对所有题目花的时间
	FIELD_END();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//采集类法宝世界配置参数,对应TalismanWorldGather.csv
struct SGatherGameConfig
{
	FIELD_BEGIN();

	FIELD(TTalismanWorldID         ,m_TalismanWorldID);       //法宝世界ID
	
	FIELD(UINT16                   ,m_TotalDetectNum); //总探测次数

	FIELD(UINT16                   ,m_BaoGoodsNum); //可以获得的宝物数量

	FIELD(UINT16                   ,m_GatherTime); //采集时间

	FIELD(std::vector<TGoodsID>    ,m_vectGoods); //可选的物品及概率

	FIELD_END();

	INT32     m_TotalProbability;  //总概率

	 SGatherGameConfig()
	{
       MEM_ZERO(this);
	}

};

//采集游戏达成级别及奖励
struct SGatherGameAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8              ,m_FinishLevel);          //完成级别
	FIELD(UINT16             ,m_AwardQuality);         //奖励品质
	FIELD(INT16              ,m_GatherNum);       //连续采集次数

	FIELD_END();

	 SGatherGameAwardCnfg()
	{
       MEM_ZERO(this);
	}

};

//寻宝游戏配置参数
struct SGatherGameParam
{
	SGatherGameParam()
	{
		m_XDetectPoint = 0;
		m_YDetectPoint = 0;
	}

	FIELD_BEGIN();

	FIELD(INT16                                ,m_XDetectPoint);   //X坐标探测点数量

	FIELD(INT16                                ,m_YDetectPoint);   //Y坐标探测点数量
	
	FIELD(std::vector<UINT16>                   ,m_vectColorRange); //color黑红黄绿分别表示附近多少格有宝物，0表示无宝物

	FIELD(std::vector<UINT16>                   ,m_vectAwardGoodsNum);  //连击次数对应奖励的物品数量

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//冷却时间配置表
struct SCDTimeCnfg
{
	FIELD_BEGIN();
	FIELD(TCDTimerID					,m_CDTime_ID);	//ID
	FIELD(UINT32						,m_CDTime);		//冷却时间多少

	FIELD_END();

	SCDTimeCnfg()
	{
       MEM_ZERO(this);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法宝世界奇遇
struct SAdventureAwardCnfg
{
	SAdventureAwardCnfg()
	{
		m_AdventureAwardID = 0;
		m_TalismanWorldID = 0;
		m_RewardType = 0;
		m_Param = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT16					,m_AdventureAwardID);	//奇遇ID
	FIELD(std::string               ,m_strName);			//法宝世界名称
	FIELD(TTalismanWorldID          ,m_TalismanWorldID);    //法宝世界ID
	FIELD(UINT8						,m_RewardType);				//奖励类型
	FIELD(UINT32					,m_Param);				//奖励参数
	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//GM命令配置
struct SGMCmdCnfg
{
	SGMCmdCnfg()
	{
		m_GMCmdID = 0;
	}

	FIELD_BEGIN();
	FIELD(UINT8						,m_GMCmdID);			//GM命令ID
	FIELD(std::string				,m_strGMCmd);			//文字命令
	FIELD(std::vector<TUserID>		,m_vectUserID);			//有权限的玩家向量
	FIELD(std::string				,m_strDescript);		//格式描述

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//清理土地的完成级别
struct SQingLiGameAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8              ,m_FinishLevel);          //完成级别
	FIELD(UINT16             ,m_AwardQuality);         //奖励品质
	FIELD(INT16              ,m_DestroyDiShuNum);	   //消灭地鼠个数
	FIELD(INT16              ,m_WuShangShuJingNum);	   //误伤树精个数

	FIELD_END();

	 SQingLiGameAwardCnfg()
	{
       MEM_ZERO(this);
	}	
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum enTaskType VC_PACKED_ONE
{
    enTaskType_Event,             //事件型

	enTaskType_SynGoods,		  //帮派物品型(任务刷新有几率)

	enTaskType_Contribution,	  //使用帮派贡献数

	enTaskType_AddSyn,			  //加入帮派任务

	enTaskType_GetSynWarScore,	  //获得帮派功勋

	enTaskType_Employee,		  //招募角色任务

	enTaskType_Max,

}PACKED_ONE;

//任务配置信息
struct STaskCnfg
{
	STaskCnfg()
	{
		m_TaskID = 0;
		m_TaskClass = 0;
		m_TaskType = 0;
		m_PreTaskID = 0;
		m_OpenLevel = 0;
		m_EventID = 0;
		m_AttainNum = 0;
		m_bSaveToDB = 0;
		m_bGuide = 0;
		m_NimbusStone = 0;
		m_GiftTicket = 0;
		m_Experience = 0;
		m_Nimbus = 0;
		m_GodSwordNimbus = 0;
		m_SynExp = 0;
		m_Honor = 0;
		m_PolyNimbus = 0;
		m_Credit = 0;
	}

	FIELD_BEGIN();
    FIELD(TTaskID                           ,m_TaskID);			//任务ID
	FIELD(std::string                       ,m_strName);		//任务名称
	FIELD(UINT8                             ,m_TaskClass);		//任务分类
	FIELD(UINT8                             ,m_TaskType);		//任务类型
	FIELD(TTaskID                           ,m_PreTaskID);		//前提任务
	FIELD(UINT8                             ,m_OpenLevel);		//开启等级
	FIELD(UINT8                             ,m_EventID);		//事件ID
	FIELD(std::vector<INT32>                ,m_vectParam);		//任务参数
	FIELD(UINT16                            ,m_AttainNum);		//达成次数
	FIELD(UINT8                             ,m_bSaveToDB);		//是否需要存盘
	FIELD(UINT8                             ,m_bGuide);			//是否需要引导
	FIELD(std::string                       ,m_strTaskDesc);	//任务描术
	FIELD(std::string                       ,m_strTaskTarget);	//任务目标
	FIELD(std::string                       ,m_strTaskAward);	//任务奖励
	FIELD(INT32                             ,m_NimbusStone);	//灵石
	FIELD(INT32                             ,m_GiftTicket) ;	//礼券
	FIELD(INT32                             ,m_Experience) ;	//经验
    FIELD(INT32                             ,m_Nimbus) ;		//灵气
	FIELD(INT32                             ,m_GodSwordNimbus); //仙剑灵气
	FIELD(INT32								,m_SynContribution);//帮派贡献
	FIELD(std::vector<UINT16>                ,m_vectGoods);      //物品及数量 
	FIELD(INT32								,m_SynExp);			//帮派经验
	FIELD(INT32								,m_Honor);			//荣誉	
	FIELD(INT32								,m_TaskNameLangID);	//任务名称语言ID	//fly add
	FIELD(INT32								,m_TaskDescLangID);	//任务描述语言ID
	FIELD(INT32								,m_TaskTargetLangID); //任务目标语言ID
	FIELD(INT32                             ,m_PolyNimbus) ;	//聚灵气
	FIELD(INT32                             ,m_Credit) ;	    //声望
	FIELD(INT32								,m_GhostSoul);		//灵魄

	FIELD_END();
};

//帮派物品任务
struct SSynGoodsTask
{
	FIELD_BEGIN();
	FIELD(TTaskID,		m_TaskID);		//任务ID
	FIELD(UINT16,		m_RandomNum);	//任务抽取概率

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//夺宝奖励
struct SDuoBaoAwardCnfg
{
	FIELD_BEGIN();
	FIELD(UINT8						,m_DuoBaoType);			//夺宝战类型
	FIELD(UINT8						,m_LevelRangeType);		//等级范围类别
	FIELD(std::vector<UINT16>		,m_FirstAwardVect);		//第一名奖励物品集合及概率
	FIELD(std::vector<UINT16>		,m_SecondAwardVect);	//第二名奖励物品集合及概率
	FIELD(std::vector<UINT16>		,m_ThirdAwardVect);		//第三名奖励物品集合及概率

	INT32	m_FirstTotalRandom;			//第一名总概率
	INT32	m_SecondTotalRandom;		//第二名总概率
	INT32	m_ThirdTotalRandom;			//第三名总概率

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//怪物法术库
struct SMonsterMagicCnfg
{
	FIELD_BEGIN();
	FIELD(TMagicID                 ,m_MagicID);    //法术ID
	FIELD(UINT8                    ,m_MagicType);    //法术类型
	FIELD(UINT8                    ,m_MinMagicNumLimit);    //需要满足最少法术数量
	FIELD(std::vector<TMagicID>    ,m_vectMutexMagic);  //与之斥的法术ID
	FIELD(UINT8                    ,m_MinLevel) ; //所适用的怪物的最小等级
	FIELD(UINT8                    ,m_MaxLevel) ; //所适用的怪物的最大等级
	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum enChengJiuType VC_PACKED_ONE
{
    enChengJiuType_Event,             //事件型
	enChengJiuType_MagicLvSec,		  //法术等级区间成就

	enChengJiuType_Max,

}PACKED_ONE;


//成就配置信息
struct SChengJiuCnfg
{
	FIELD_BEGIN();
	FIELD(TChengJiuID            , m_ChengJiuID); //成就ID
	FIELD(std::string             ,m_strName);    //名称
	FIELD(UINT8                  , m_Class); //类别
	FIELD(UINT8                   ,m_SubClass); //子类别
	FIELD(UINT8                   ,m_SyncThreePart); //是否需要同步到第三方
	FIELD(std::string             ,m_strTarget) ; //达成目标，获得方式
	FIELD(std::string             ,m_strProbDesc); //获得几率
	FIELD(UINT16                  ,m_LimitNum );  //可获得该成就的数量限制
	FIELD(UINT16                  ,m_ChengJiuPoint) ; //可获得的成就点
	FIELD(TTitleID                ,m_TitleID); //可获得称号
	FIELD(std::vector<TGoodsID>   ,m_vectAwardGoods) ;//奖励的物品及数量
	FIELD(std::vector<TChengJiuID> ,m_vectAutoGetChengJiuID); //可自动获得的成就
	FIELD(UINT8                    ,m_Type); //类型,enChengJiuType
	FIELD(UINT16                   ,m_EventID); //事件ID
	FIELD(std::vector<INT32>       ,m_vectParam); //事件参数
	FIELD(UINT16                   ,m_AttainNum);		//达成次数
	FIELD(UINT16                   ,m_ResetEventID) ; //复位事件
	FIELD(std::vector<INT32>       ,m_vectResetParam); //复位事件参数
	FIELD(UINT16				   ,m_ResID);		   //资源ID
	FIELD(UINT16				   ,m_ChengJiuLangID);		   //成就名称语言ID		fly add
	FIELD(UINT16				   ,m_GetTargetLangID);		   //获得方式语言ID

	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//称号
struct STitleCnfg
{
	FIELD_BEGIN();
	FIELD(TTitleID					,m_TitleID);
	FIELD(std::string				,m_TitleName);
	FIELD(INT32                  ,m_TitleNameLangID); //称号语言ID	fly add

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//签到奖励
struct SSignInAwardCnfg
{
	FIELD_BEGIN();

    FIELD(UINT8                 ,m_AwardID);  //奖励ID
    FIELD(UINT8                  ,m_SignInNum); //需要签到天数
	FIELD(std::vector<TGoodsID>  ,m_GoodsAndNum); //物品及数量
	FIELD(INT32                  ,m_NimbusStone); //灵石
	FIELD(INT32                  ,m_PolyNimbus); //聚灵气
	FIELD(INT32                  ,m_Credit);     //声望

   FIELD_END();
};

//在线奖励
struct SOnlineAwardCnfg
{
	SOnlineAwardCnfg()
	{
		m_AwardID = 0;
		m_OnlineTime = 0;
		m_NimbusStone = 0;
		m_Ticket = 0;
		m_PolyNimbus = 0;
		m_Credit = 0;
		
	}


	FIELD_BEGIN();
	
    FIELD(UINT8                 ,m_AwardID);  //奖励ID
	FIELD(INT32                 ,m_OnlineTime); //在线时间，单位:分钟
	FIELD(INT32                  ,m_NimbusStone); //灵石
    FIELD(INT32                  ,m_Ticket); //礼券
	FIELD(std::vector<TGoodsID>  ,m_GoodsAndNum); //物品及数量,概率
	FIELD(INT32                  ,m_PolyNimbus); //聚灵气
	FIELD(INT32                  ,m_Credit); //声望
	


	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//活动配置
struct SActivityCnfg
{
	SActivityCnfg()
	{
		m_ActivityID = 0;
		
		m_ResID = 0;
		m_EventID = 0;
		m_AttainNum = 0;
		m_Ticket = 0;
		m_GodStone = 0;
	}

	FIELD_BEGIN();

	FIELD(UINT16           ,   m_ActivityID);    //活动ID

	FIELD(std::string       ,m_strName) ; // 名称

	FIELD(UINT8				,m_ActivityType); //活动类型

	FIELD(std::string       ,m_strRole) ; // 活动规则

	FIELD(UINT32            ,m_BeginTime) ; //活动开始时间

	FIELD(UINT32            ,m_EndTime) ; //活动结束时间

	FIELD(UINT16            ,m_ResID);   //资源ID

	FIELD(UINT8             ,m_bFinished);  //初始状态

	FIELD(UINT16           ,m_EventID) ; //事件

	FIELD(std::vector<INT32> , m_vectParam); //参数

	FIELD(UINT16             , m_AttainNum) ; //目标次数

	FIELD(std::string       ,m_strAwardDesc) ; // 活动奖励描述

	FIELD(UINT16             , m_Ticket); //礼券

	FIELD(UINT16             , m_GodStone); //仙石

	FIELD(std::vector<INT32> , m_vectGoods); //物品ID和数量

	FIELD(TInt8Array<DESCRIPT_LEN_50>,		m_strSubject);		//邮件主题

	FIELD(TInt8Array<DESCRIPT_LEN_300>,		m_strContent);		//邮件内容

	FIELD(std::string,		m_strResFileUrl);		//资源文件

	INT16                   m_Order;  //排序号

	FIELD_END();

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//不可改名的角色名或者不能改成这名字
struct SNoChangeName
{
	FIELD_BEGIN();
	FIELD(std::string		, m_NoChangeName);
	FIELD(INT32		, m_NoChangeNameLangID);	//fly add	20121106

	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//挑战奖励
struct SChallengeForward
{
	FIELD_BEGIN();
	FIELD(UINT8,				m_LvGroup);	//enLevelGroup
	FIELD(UINT8,				m_Rank);
	FIELD(UINT32,				m_PolyNimbus);	//聚灵气
	FIELD(std::vector<UINT16>,	m_vectGoods);

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//强化传承
struct SStrongInherit
{
	FIELD_BEGIN();
	FIELD(UINT8,				m_Level);
	FIELD(INT32,				m_Random);

	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//自动刷新
struct SAutoFlushEmployee
{
	FIELD_BEGIN();
	FIELD(UINT16,				m_FlushNum);		//自动刷新的次数
	FIELD(INT32,				m_PerGodStone);		//单次刷新的次数
	FIELD(std::vector<UINT32>,	m_MustAptitude);	//必然出现角色资质及概率

	FIELD_END();

	INT32		m_TotalRandom;		//总概率
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//游戏语言类型		fly add
struct SLanguageTypeCnfg
{
	SLanguageTypeCnfg()
	{
		m_LanguageID = 0;
	}
	FIELD_BEGIN();
	FIELD(UINT16,			 m_LanguageID) ;		//语言ID
	FIELD(std::string       ,m_strEnglish) ; 		//转换的语言

	FIELD_END();

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//VIP功能
struct SVipConfig
{
	FIELD_BEGIN();

	FIELD(UINT8,			m_VipLevel);					//VIP等级
	FIELD(INT32,			m_TakeResAddStone);				//收取灵石额外获得灵石数
	FIELD(UINT8,			m_bCanAutoTakeRes);				//是否一键收取灵石
	FIELD(UINT8,			m_AutoKillAddExp);				//增加挂机获得经验(百分比)
	FIELD(UINT8,			m_AccelKillDesMoney);			//挂机加速减少多少仙石
	FIELD(UINT8,			m_AccelKillAddHour);			//挂机加速增加几小时
	FIELD(UINT8,			m_FinishTrainingFreeNum);		//每天练功可免费加速次数
	FIELD(UINT8,			m_AddTrainingNum);				//增加几次练功机会
	FIELD(UINT8,			m_BuySwordEmployMagicDesMoney);	//购买仙剑、招募角色、购买法术书费用减少(百分比)
	FIELD(UINT8,			m_AddGodSwordWorldNum);			//剑印世界增加次数
	FIELD(UINT8,			m_AddXiuLianPos);				//增加几个修炼空位
	FIELD(UINT8,			m_AddSynCombatNum);				//帮派保卫战增加几次
	FIELD(UINT8,			m_AutoKillSynCombat);			//是否快速攻打帮派保卫战
	FIELD(UINT8,			m_bFreeSellGoods);				//是否交易免寄售费
	FIELD(UINT8,			m_bFreeRemoveGem);				//是否免费摘除宝石
	FIELD(UINT8,			m_AddFuBenNum);					//副本免费进入增加几次
	FIELD(UINT8,			m_bAutoKillFuBen);				//是否快速攻打副本
	FIELD(UINT8,			m_UnLoadEmployAddExp);			//解雇角色获得经验增加(百分比)
	FIELD(UINT8,			m_AddStrongerRate);				//强化几率提升(百分比)
	FIELD(INT32,			m_AddMaxHonorToday);			//荣誉上限增加
	FIELD(INT32,			m_AddGetHonor);					//每次获得荣誉时增加
	FIELD(UINT8,			m_AddXlNimbus);					//增加获得的修炼灵气和仙剑灵气(百分比)
	FIELD(UINT8,			m_AddEnterTalismanNum);			//增加法宝世界进入次数
	FIELD(INT32,			m_AddDuoBaoCreditUp);			//增加夺宝声望上限
	FIELD(INT32,			m_AddSynWarCreditUp);			//增加帮战声望上限
	FIELD(INT32,			m_AddGetCredit);				//增加每次获得声望时

	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//获得聚灵气途径
enum enGetPolyNimbusID  VC_PACKED_ONE
{
	enGetPolyNimbusID_FuBenThrough       = 1, //打通副本
	//enGetPolyNimbusID_ChallengeWin		 = 2, //挑战胜利
	
	enGetPolyNimbusID_Max,
} PACKED_ONE;


//聚灵气获得配置		
struct SPolyNimbusCnfg
{
	SPolyNimbusCnfg()
	{
		m_OperationID = 0;
		m_PolyNimbusNum = 0;
	}
	FIELD_BEGIN();
	FIELD(UINT16,			 m_OperationID) ;		//日常操作ID
	FIELD(UINT32,			 m_PolyNimbusNum) ; 	//获得的聚灵气数量

	FIELD_END();

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//玄天奖励
struct SXuanTianForward
{
	FIELD_BEGIN();

	FIELD(INT32,					m_Rank);		//排名，第0名为最后一击者
	FIELD(INT32,					m_PolyNimbus);	//获得的聚灵气
	FIELD(UINT16,					m_DropID);		//掉落ID
	//FIELD(std::vector<TGoodsID>,	m_vecGoods);	//获得的物品及概率

	FIELD_END();

	//INT32		m_TotalRandom;		//总概率
};

//帮战参数配置
struct SSynCombatParam
{
	SSynCombatParam()
	{
		m_CreditUp = 0;
		m_WinParam = 0;
		m_FailParam = 0;
	}

	FIELD_BEGIN();

	FIELD(INT32,		m_CreditUp);		//声望上限
	FIELD(INT32,		m_WinParam);		//胜利帮战等级系数
	FIELD(INT32,		m_FailParam);		//失败帮战等级系数

	FIELD_END();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//剑诀升级配置
struct SDamageLevelCnfg
{

	SDamageLevelCnfg()
	{
		m_MagicLevel = 0;
		m_MagicType = 0;
		m_SwordNimbus = 0;
		m_PolyNimbus = 0;
		m_Damage = 0;
	
		m_ResID = 0;
	}

	FIELD_BEGIN();

	FIELD(UINT16,					m_MagicLevel);		//剑诀等级
	FIELD(UINT16,					m_MagicType);		//剑诀类型
	FIELD(INT32,					m_SwordNimbus);		//仙剑灵气数
	FIELD(INT32,					m_PolyNimbus);		//聚灵气数
	FIELD(std::vector<TGoodsID>,	m_vectGoods);		//物品及数量
	FIELD(UINT32,					m_Damage);		    //剑诀伤害值
	FIELD(UINT16,					m_ResID);			//图标资源ID

	FIELD_END();

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//声望技能配置
struct SCreditMagicCnfg
{
	SCreditMagicCnfg()
	{
		m_CreditMagicLevel = 0;
		m_CreditMagicType = 0;
		m_AddMagicNum = 0;
		m_CreditNum = 0;
		m_PolyNimbusNum = 0;
	
		m_ResID = 0;
	}

	FIELD_BEGIN();

	FIELD(UINT16,					m_CreditMagicLevel);	//技能等级
	FIELD(UINT16,					m_CreditMagicType);		//技能类型
	FIELD(INT32,					m_AddMagicNum); 	    //累计增加的技能值
	FIELD(INT32,					m_CreditNum);		    //需要声望值
	FIELD(INT32,					m_PolyNimbusNum);		//需要聚灵气
	FIELD(UINT16,					m_ResID);				//图标资源ID


	FIELD_END();

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//法宝世界配置
struct STalismanWorldInfo
{
	FIELD_BEGIN();

	FIELD(UINT8,					m_Level);			//级别
	FIELD(UINT8,					m_Floor);			//层数
	FIELD(TMapID,					m_MapID);			//地图ID
	FIELD(TMapID,					m_CombatMapID);		//战斗地图ID

	FIELD_END();
};

struct STalismanWorldParam
{
	FIELD_BEGIN();

	FIELD(UINT8,					m_Floor);				//层数
	FIELD(UINT32,					m_MinuteGetGhostSoul);	//每分钟得到的灵魄数
	FIELD(INT32,					m_PlaceName);			//战斗地图名字
	FIELD(INT32,					m_LangID);				//语言ID

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//装备制作配置
struct SEquipMakeCnfg
{
	SEquipMakeCnfg()
	{
		m_GoodsID = 0;
		m_PolyNimbus = 0;
		m_MapGoodsID = 0;
		m_MapNum = 0;
		m_NewGoodsID = 0;

	}

	FIELD_BEGIN();

	FIELD(UINT16,					m_GoodsID);				//原装备物品ID
	FIELD(INT32,					m_PolyNimbus);			//聚灵气、灵魄
	FIELD(UINT16,					m_MapGoodsID); 	        //图纸ID
	FIELD(INT32,					m_MapNum);		        //图纸数量
	FIELD(UINT16,					m_NewGoodsID);			//产出装备物品ID


	FIELD_END();

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//灵件升级配置
struct SUpGhostCnfg
{
	SUpGhostCnfg()
	{
		m_GhostID = 0;
		m_GhostLevel = 0;
		m_GhostSoul = 0;
		m_Type = 0;
		m_Addprop = 0;
		m_Material = 0;
		m_MaterialNum = 0;
		m_NextGhostID = 0;
	}

	FIELD_BEGIN();
	FIELD(TGoodsID						,m_GhostID);			//灵件ID
	FIELD(UINT8							,m_GhostLevel);			//灵件等级
	FIELD(INT32							,m_GhostSoul);			//升级所需灵魄
	FIELD(UINT8							,m_Type);				//增加属性类型
	FIELD(INT32							,m_Addprop);			//增加属性值
	FIELD(TGoodsID						,m_Material);			//材料ID
	FIELD(UINT8							,m_MaterialNum);		//材料数量
	FIELD(TGoodsID						,m_NextGhostID);		//灵件下一级ID


	FIELD_END();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//守宝配置
struct SDuoBaoCnfg
{
	FIELD_BEGIN();

	FIELD(INT32							,m_MinDuoBaoLevel);			//最小夺宝等级
	FIELD(INT32							,m_MaxDuoBaoLevel);			//最大夺宝等级
	FIELD(UINT32						,m_CreditUp);				//对应声望上限
	FIELD(UINT32						,m_TeamR);					//组队R
	FIELD(UINT32						,m_SingleR);				//单人R
	FIELD(UINT32						,m_WinRes);					//胜利Res
	FIELD(UINT32						,m_FailRes);				//失败Res
	FIELD(INT32							,m_TeamWinGetCredit);		//组队胜利获得声望值
	FIELD(INT32							,m_TeamFailGetCredit);		//组队失败获得声望值
	FIELD(INT32							,m_SingleWinGetCredit);		//单人胜利获得声望值
	FIELD(INT32							,m_SingleFailGetCredit);	//单人失败获得声望值
	FIELD(INT32							,m_TeamRunGetCredit);		//组队逃跑获得声望数
	FIELD(INT32							,m_SingleRunGetCredit);		//单逃跑获得声望数

	FIELD_END();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//套装外观
struct SSuitFacadeInfo
{
	FIELD_BEGIN();
	FIELD(INT16,                m_SuitID);
	FIELD(UINT16               ,m_DefaultFacadeID);
	FIELD(UINT16               ,m_UpgradeFacadeID);

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//提升资质
struct SUpAptitude
{
	FIELD_BEGIN();

	FIELD(INT32,				m_MinAptitudePoint);
	FIELD(INT32,				m_MaxAptitudePoint);
	FIELD(TGoodsID,				m_NeedGoods);
	FIELD(UINT16,				m_NeedNum);

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//帮战奖励
struct SSynWarForward
{
	FIELD_BEGIN();

	FIELD(INT32,			m_MinSynWarLevel);			//最小帮战等级
	FIELD(INT32,			m_MaxSynWarLevel);			//最大帮战等级
	FIELD(INT32,			m_SynWarLvR);				//帮战等级R
	FIELD(UINT8,			m_SynWarLvWinRes);			//帮战等级胜利Res
	FIELD(UINT8,			m_SynWarLvFailRes);			//帮战等级失败Res
	FIELD(INT32,			m_SynWarScoreR);			//帮战积分R
	FIELD(UINT8,			m_SynWarScoreWinRes);		//帮战积分胜利Res
	FIELD(INT32,			m_CreditUp);				//声望上限

	FIELD_END();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct IConfigServer
{
	virtual void Release() =0;

	virtual bool Create()=0;

	//杂项配置
	virtual const SGameConfigParam & GetGameConfigParam() = 0;

	//获得地图所有配置信息
	virtual std::vector<SMapConfigInfo> & GetAllMapConfigInfo() = 0;

	//获得单个地图配置信息
	virtual const SMapConfigInfo * GetMapConfigInfo(TMapID mapid) = 0;

		//获得单个地图怪物分布信息
	virtual const std::vector<SMonsterOutput> * GetMapMonsterCnfg(TMapID mapid) = 0;

	//创建角色数据配置
	virtual const SCreateActorCnfg & GetCreateActorCnfg() = 0;

	//获得怪物配置数据
	virtual const SMonsterCnfg* GetMonsterCnfg(TMonsterID MonsterID) = 0;

	//增加怪物配置
	virtual void Push_MonsterCnfg(const SMonsterCnfg & MonsterCnfg) = 0;

	//获得伏魔洞配置数据
	virtual const SFuMoDongCnfg* GetFuMoDongCnfg(UINT8 level) = 0;

	//根据玩家等级获得伏魔洞对应层配置
	virtual const SFuMoDongCnfg* GetFuMoDongCnfgByUserLv(UINT8 UserLevel) = 0;

	//物品配置数据
	virtual const SGoodsCnfg* GetGoodsCnfg(TGoodsID GoodsID) = 0;

	//物品合成配置
	virtual const SGoodsComposeCnfg * GetGoodsComposeCnfg(TGoodsID GoodsID) = 0;

	//获取物品合成全部配置
	virtual const std::hash_map<TGoodsID, SGoodsComposeCnfg> * GetAllGoodsComposeCnfg() = 0;

	//强化配置 key:十位数为强化到等级，个位数为强化类型
	virtual const SEquipStrongerCnfg * GetEquipStrongerCnfg(UINT8 key) = 0;

	//宝石配置
	virtual const SGemCnfg *   GetGemCnfg(UINT8  GemType) = 0;

	//招募角色配置
	virtual const SEmployeeDataCnfg * GetEmployeeDataCnfg(TEmployeeID EmployeeID) = 0;

	//获取该法术的所有等级信息
	virtual const std::vector<SMagicLevelCnfg *> GetMagicAllLevelCnfg(TMagicID MagicID) = 0;

	//随机(每条有机率大小不同)获取一个招募角色数据
	virtual const SEmployeeDataCnfg * RandGetEmployData(enFlushType FlushType)	= 0;

	//随机获得指定资质的招募角色
	virtual const SEmployeeDataCnfg * RandGetEmployApt(UINT32 Aptitude, enFlushType FlushType) = 0;

	//得到法术书配置
	virtual const SMagicBookCnfg * GetMagicBookCnfg(TMagicBookID MagicBookID) = 0;

	//随机(每条有机率大小不同)获取一个法术书数据
	virtual const SMagicBookCnfg * RandGetMagicBookCnfg(enFlushType FlushType) = 0;
	
	//得到商城抒写商店的物品配置集合
//	virtual const std::hash_map<TGoodsID, SShopMallCnfg> * GetShopMallCnfgByLable(enShopMallLabel lable) = 0;

	//放进商城配置表
//	virtual void  Push_ShopMallCnfg(const SShopMallCnfg & ShopMallCnfg) = 0;

	//清除商城配置信息
//	virtual void  ClearShopMallCnfg() = 0;

	//根据法宝品质级别及法宝世界级别，随机获取一个孕育物品
	virtual TGoodsID GetGestateGoodsID(UINT8 QualityLevel,UINT8 TalismanWorldLevel,UINT8 TalismanWorldType) = 0; 

	//根据品质点获取品质等级
	virtual UINT8 GetTalismanQualityLevel(INT32 QualityPoint) = 0;

	//获得效果配置信息
	virtual const SEffectCnfg * GetEffectCnfg(TEffectID  EffectID) = 0;

	//获得状态配置信息
	virtual const SStatusCnfg * GetStatusCnfg(TStatusID   StatusID) = 0;

	//获取状态组配置
	virtual const SStatusGroupCnfg * GetStatusGroupCnfg(TStatusGroupID StatusGroupID) = 0;

	//获取法术配置信息
	virtual const SMagicCnfg * GetMagicCnfg(TMagicID MagicID) = 0;

	//获取法术等级配置信息
	virtual const SMagicLevelCnfg * GetMagicLevelCnfg(TMagicID MagicID, UINT8 Level) = 0;

	//获取帮派配置信息
	virtual const SSyndicateCnfg * GetSyndicateCnfg(UINT8 Level) = 0;

	//副本配置信息
	virtual const SFuBenCnfg * GetFuBenCnfg(TFuBenID FuBenID) = 0;

	//获得仙剑副本
	virtual const SGodSwordFuBenCnfg * GetGodSwordFuBenCnfg(TFuBenID FuBenID, UINT8 Level) = 0;


	//获取所有帮派物品配置信息
	virtual const std::hash_map<TGoodsID, SSynGoodsCnfg> * GetAllSynGoodsCnfg() = 0;

	//获取所有帮派技能的配置信息
	virtual const std::vector<SSynMagicCnfg> GetAllSynMagicCnfg() = 0;

	//获取帮派技能的配置信息
	virtual const std::vector<SSynMagicCnfg> * GetSynMagicCnfg(TSynMagicID SynMagicID) = 0;

	//获取帮派技能的配置信息
	virtual const SSynMagicCnfg * GetSynMagicCnfg(TSynMagicID SynMagicID, UINT8 Level) = 0;

	//物品使用配置
	virtual const SGoodsUseCnfg * GetGoodsUseCnfg(TGoodsID GoodsID) = 0;

	//得到剑冢的配置信息
	virtual const SGodSwordCnfg * GetGodSwordShopCnfg(TGoodsID GodSwordID) = 0;

	//随机获取一个仙剑数据
	virtual const SGodSwordCnfg * RandGetGodSwordShopCnfg(enFlushType FlushType) = 0;

	//根据玩家等级获取合适的帮派保卫战模式
	virtual const SSynCombatCnfg * GetSynCombatCnfg(UINT8 ActorLevel) = 0;

		//根据模式获取配置信息
	virtual const SSynCombatCnfg * GetSynCombatCnfgByMode(UINT8 Mode) = 0;

	//状态类型参数
	virtual const SStatusTypeCnfg * GetStatusTypeCnfg(UINT8  StatusType) = 0;

	//获得角色等级配置
	virtual const SActorLevelCnfg * GetActorLevelCnfg(UINT8 Level) = 0;

	//获得人物境界配置
	virtual const SActorLayerCnfg * GetActorLayerCnfg(UINT8 Level) = 0;

	//获得掉落物品配置
	virtual const std::vector<SDropGoods> * GetDropGoodsCnfg(UINT16 DropID) = 0;

	//获得地图主将的掉落ID, bOpenHardType为是否开启困难模式
	virtual UINT16	GetMapBossDropID(TMapID MapID, bool bOpenHardType = false) = 0;

	//获得修炼时的好友附加值
	virtual const SXiuLianFriendAdd * GetXiuLianFriendAddCnfg(INT32 nRelationNum) = 0;

	//仙剑世界配置
	virtual const SGodSwordWorldCnfg * GetGoldSwordWorldCnfg(TSwordSecretID SwordSecretID)=0;

	//仙剑级别配置
	virtual const SGodSwordLevelCnfg * GetGodSwordLevelCnfg(UINT8  Level) = 0;


	//法宝世界配置
	virtual const STalismanWorldCnfg * GetTalismanWorldCnfg(TTalismanWorldID  TalismanWorldID) = 0;

	//修炼类法宝世界配置
	virtual const STalismanWorldXiuLianCnfg * GetTalismanWorldXiuLianCnfg(TTalismanWorldID  TalismanWorldID) = 0;

	//修炼游戏配置
	virtual const SXiuLianGameParam & GetXiuLianGameParam() = 0;

	//修炼奖励
	virtual std::vector<SXiuLianGameAwardCnfg> & GetXiuLianGameAwardCnfg() = 0;

	//传送门配置
	virtual const SPortalCnfg * GetPortalCnfg(TPortalID PortalID) = 0;

	//获得冷却时间
	virtual UINT32 GetCDTimeCnfg(TCDTimerID CDTime_ID) = 0;

	//获得指定法宝世界可获得的奇遇
	virtual const std::hash_map<UINT16, SAdventureAwardCnfg> * GetAdventureAwardCnfgVect(TTalismanWorldID TalismanWorldID) = 0;

	//获得指定奇遇
	virtual const SAdventureAwardCnfg * GetAdventureAwardCnfg(UINT16 AdventureAwardID) = 0;

	//获得GM命令
	virtual const SGMCmdCnfg * GetGMCmdCnfg(std::string strGMCmd) = 0;

	//获得任务列表
	virtual const std::hash_map<TTaskID,STaskCnfg> *  GetTaskList(UINT8 TaskClass) = 0;

	virtual const STaskCnfg * GetTaskCnfg(TTaskID TaskID) = 0;

	//战斗地图怪物分布
	virtual const SCombatMapMonster * GetCombatMapMonster(TMonsterID MonsterID) = 0;

	//得到战斗地图主将ID
	virtual const TMonsterID GetMainMonsterID(UINT32 CombatIndex) = 0;

	//随机获取一个帮派物品任务
	virtual const SSynGoodsTask * RandomGetSynGoodsTask() = 0;

	//得到帮派物品任务
	virtual const SSynGoodsTask * GetSynGoodsTask(TTaskID TaskID) = 0;

	//帮派任务数量
	virtual UINT8	GetSynGoodsTaskNum() = 0;

	//怪物法术列表
	virtual std::vector<TMagicID> GetMonsterMagicCnfg(UINT8 Level,INT32 num) = 0;

	//清理土地法宝游戏
	virtual const SQingLiGameParam & GetQingLiGameParam() = 0;

	virtual const SQingLiGameConfig *	 GetQingLiGameCnfg(TTalismanWorldID TalismanWorldID) = 0;

	//清理奖励
	virtual const std::vector<SQingLiGameAwardCnfg> & GetQingLiAwardCnfg() = 0;

		//寻宝法宝游戏
	virtual const SXunBaoGameParam & GetXunBaoGameParam() = 0;

	virtual const SXunBaoGameConfig *	 GetXunBaoGameCnfg(TTalismanWorldID TalismanWorldID) = 0;

	//寻宝奖励
	virtual const std::vector<SXunBaoGameAwardCnfg> & GetXunBaoGameAwardCnfg() = 0;

	//得到某类型的题目
	virtual const std::hash_map<UINT16/*TiMuID*/, STiMuData> * GetTiMuListCnfg(UINT8 TiMuType) = 0;

	//根据题目ID得到题目
	virtual const STiMuData * GetTiMuCnfg(UINT16  TiMuID) = 0;

	//得到答题配置
	virtual const SDaTiGameCnfg * GetDaTiCnfg(TTalismanWorldID) = 0;

	//得到答题参数配置
	virtual const SDaTiGameParam & GetDaTiParam() = 0;

	//得到答题完成级别奖励
	virtual const std::vector<SDaTiGameAwardCnfg> & GetDaTiGameAward() = 0;

	//得到答题奖励
	virtual const SDaTiGameLevelAward * GetDaTiGameLevelAward(TTalismanWorldID TalismanWorldID) = 0;

			//采集法宝游戏
	virtual const SGatherGameParam & GetGatherGameParam() = 0;

	virtual const SGatherGameConfig *	 GetGatherGameCnfg(TTalismanWorldID TalismanWorldID) = 0;

	//采集奖励
	virtual const std::vector<SGatherGameAwardCnfg> & GetGatherGameAwardCnfg() = 0;

	//获得指定成就
	virtual const SChengJiuCnfg * GetChengJiuCnfg(TChengJiuID ChengJiuID) = 0;

	//获得所有成就配置信息
	virtual const std::hash_map<TChengJiuID,SChengJiuCnfg> & GetAllChengJiuCnfg() = 0;

	//获到奖励
	virtual const std::map<UINT8,SSignInAwardCnfg> & GetAllSignInAwardCnfg() = 0;

	virtual const SSignInAwardCnfg * GetSignInAwardCnfg(UINT8 AwardID) = 0;

	//活动
	virtual const std::map< UINT16, SActivityCnfg> & GetAllActivityCnfg() = 0;

	virtual const SActivityCnfg * GetActivityCnfg(UINT16 ActivityID) = 0;

	virtual void Push_ActivityCnfg(const SActivityCnfg & Activity) = 0;

	virtual void DelActivityCnfg(UINT16 ActivityID) = 0;

	//称号
	virtual const STitleCnfg * GetTitleCnfg(TTitleID TitleID) = 0;
	//virtual const std::string * GetTitleName(TTitleID TitleID) = 0;

	//在线奖励
	virtual const SOnlineAwardCnfg * GetOnlineAwardCnfg(UINT16 AwardID) = 0;

	//套装配置信息
	virtual const SSuitCnfg*  GetSuitCnfg(UINT16 SuitID) = 0;

	//获取帮派福利
	virtual const SSynWelfareCnfg * GetSynWelfareCnfg(UINT8/*enWelfare*/ WelfareType, UINT8 SynLevel) = 0;

	//获取所有帮派福利
	virtual const std::vector<SSynWelfareCnfg> & GetAllSynWelfareCnfg() = 0;

	//得到不可改名的名字
	virtual const std::vector<SNoChangeName> & GetAllNotChangeNameCnfg() = 0;

		//获得客户端配置文件版本
	virtual const std::string & GetClientVersion() = 0;

		//获得客户端所有配置文件
	virtual const std::vector<std::string> & GetClientCnfgFile() = 0;

			//获得客户端资源文件版本
	virtual const std::string & GetClientResVersion() = 0;

	//获得客户端所有资源文件
	virtual const std::vector<std::string> & GetClientResFile() = 0;

	//得到奖励配置
	virtual const SChallengeForward * GetChallengeForward(UINT8 LvGroup, UINT8 nRank) = 0;

	//随机获得强化传承等级
	virtual UINT8 RandGetStrongInheritLeve(IEquipment * pEuipment, enGoodsCategory DesGoodsClass) = 0;

	//得到自动刷新招募角色指定次数的信息
	virtual const SAutoFlushEmployee * GetAutoFlushEmploy(UINT16 FlushNum) = 0;

	//随机获得一个必然出现的角色资质
	virtual INT32	RandGetMushAptitude(UINT16 FlushNum) = 0;

	//得到该挑战排名最多有多少挑战次数
	virtual INT32	GetMaxChallengeNum(UINT32 nRank) = 0;

	//得到语言类型配置  fly add 
	virtual const SLanguageTypeCnfg * GetLanguageTypeCnfg(TLanguageID LanguageID) = 0;

	//得到VIP配置信息
	virtual const SVipConfig * GetVipConfig(UINT8 vipLevel) = 0;

	//得到充值额度可以升到的VIP级别
	virtual UINT8	GetCanVipLevel(INT32 Recharge) = 0;

	//得到服务器状态
	virtual UINT8	GetServerStatus(INT32 UserNum) = 0;

	//得到聚灵气配置    
	virtual const SPolyNimbusCnfg * GetPolyNimbusCnfg(UINT16 OperationID) = 0;

	//得到玄天奖励
	virtual const SXuanTianForward * GetXTForward(INT32	Rank) = 0;

	//得到剑诀升级配置
	virtual const SDamageLevelCnfg * GetMagicLvCnfg(UINT16 SwordMagicLevel, UINT8 SwordMagicType) = 0;

	//得到帮战参数配置
	virtual void  GetSynCombatParam(INT32 SynCombatLevel, SSynCombatParam & SynCombatParam) = 0;

	//得到声望技能配置
	virtual const SCreditMagicCnfg * GetCreditMagicCnfg(UINT16 CreditMagicLevel,UINT8 MagicType) = 0;

	//得到法宝世界配置
	virtual const STalismanWorldInfo * GetTalismanWorldInfo(UINT8 level, UINT8 floor) = 0;

	//得到所有法宝世界配置
	virtual const std::map<UINT8,	std::vector<STalismanWorldInfo>> * GetAllTalismanWorldInfo() = 0;

	//根据玩家等级，获得法宝世界其它配置
	virtual const STalismanWorldParam *  GetTalismanWorldParam(UINT8 Floor) = 0;

	//根据玩家等级，获得进入法宝世界的级别
	virtual INT8  GetEnterTWLevel(UINT8 Level) = 0;

	//得到装备制作配置
	virtual const SEquipMakeCnfg * GetEquipMakeCnfg(UINT16 GoodsID) = 0;

	//获取装备制作全部配置
	virtual const std::map<UINT16, SEquipMakeCnfg> * GetAllEquipMakeCnfg() = 0;

	//获取灵件升级的配置信息
	virtual const std::vector<SUpGhostCnfg> * GetUpGhostCnfg(TGoodsID GhostID) = 0;

	//获取灵件升级的配置信息
	virtual const SUpGhostCnfg * GetUpGhostCnfg(TGoodsID GhostID, UINT8 Level) = 0;

	//获取灵件升级全部配置
	virtual const std::hash_map<UINT16, std::vector<SUpGhostCnfg>> * GetAllUpGhostCnfg() = 0;

	//获得夺宝配置
	virtual const SDuoBaoCnfg * GetDuoBaoCnfg(UINT32 DuoBaoLevel) = 0;

	//获得所有夺宝配置
	virtual const std::vector<SDuoBaoCnfg> * GetAllDuoBaoCnfg() = 0;

	//获得升级外观
	virtual UINT16 GetUpgradeFacadeID(INT16 SuitID,UINT16 DefaultFacadeID) = 0;

	//得到玩家当前可招募角色个数
	virtual UINT8 GetCanEmployNum(UINT8 Level) = 0;

	//根据玩家资质获取提升资质配置信息
	virtual const SUpAptitude * GetUpAptitude(INT32 Aptitude) = 0;

	//根据玩家的帮战等级获取帮战奖励
	virtual const SSynWarForward * GetSynWarForward(INT32 SynWarLv) = 0;

	//计算角色实际获得的经验值
	virtual INT32 AcotorRealGetExp(INT16 OldLevel,INT32 OldExp,INT16 NewLevel,INT32 NewExp) = 0;
};

BCL_API IConfigServer * CreateConfigServer();



#endif
