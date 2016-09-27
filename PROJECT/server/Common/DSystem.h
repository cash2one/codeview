
#ifndef __JXCQ_DSYSTEM_H__
#define __JXCQ_DSYSTEM_H__

//关闭警告
 #pragma   warning(disable:4996)   //全部关掉


#ifdef _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include <crtdbg.h>
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "BclHeader.h"
#include "TBuffer.h"
#include "FieldDef.h"


//新浪Accept Token 长度
#define ACCEPT_TOKEN_LEN   34
#define ACCESS_SECRET_LEN  34

//用户ID
typedef   unsigned int  TUserID;   
#define   INVALID_USERID TUserID(0)

//角色ID
typedef	  unsigned int	TActorID;

//角色名长度
#define   ACTOR_NAME_LEN 18   

//用户名长度
#define   USER_NAME_LEN 80   



#define   NEW_TYPE_CREATE_USER

//md5长度
#define MD5_LEN  34

#define TICKET_LEN  32 

//sha256输出的长度
#define SHA256_LEN 66

//错误描述长度
#define ERROR_DESC_LEN   100


//服务器ID
typedef unsigned char  TServerID;
#define INVALID_SERVERID TServerID(0)


//世界ID
typedef unsigned short int TGameWorldID;


//游戏服务器名称长度
#define GAME_SERVER_NAME_LEN 18

//IP长度
#define IP_LEN   32


//thing名称长度
#define  THING_NAME_LEN 18

//地图名称长度
#define  THING_MAP_NAME_LEN 24



//服务器IP
typedef  unsigned int TIntIp ;

//每个主角可以招募角色的最大数量
#define MAX_EMPLOY_NUM   8


//装备栏最大装备数量
#define MAX_EQUIP_NUM   11


//保存外观字符串最大长度
#define MAX_FACADE_STR_LEN  128


//地图ID
typedef unsigned short int TMapID;
#define INVALID_MAP_ID  0

#define INVALID_SCENE_ID TSceneID(0)

#define WORLD_MAP_MAX 			1000	//世界游戏场景ID的最大取值范围，ID大于这个值的属于副本游戏场景


//场景ID
//注意场景ID内嵌服务器ID，地图id,场景序列号
struct	TSceneID
{
	union
	{
		unsigned int	m_id;
		struct
		{
			unsigned int m_mapid:10;
			unsigned int m_svrID:2;
			unsigned int m_sn : 20 ;
		};
	};
	TSceneID()
	{
		m_id = INVALID_SCENE_ID;
	}
	
	operator unsigned int(){ return m_id;}
	TSceneID(unsigned int id){ m_id = id;} 
	bool operator == (const TSceneID& sid) const 
	{
		return m_id == sid.m_id;
	}
	bool operator != (const TSceneID& sid) const 
	{
		return m_id != sid.m_id;
	}
	bool operator < (const TSceneID & sid) const
	{
		return m_id < sid.m_id;
	}
	//从游戏场景ID和序列号构造一个场景ID
	void	From(TServerID svrID, TMapID mapid, unsigned short sn)
	{
		m_sn = sn;
		m_mapid = mapid;
		m_svrID = svrID;
	}
	int	ToID(){return m_id;}
	void	From(unsigned int sid) { m_id = sid;}
	TMapID	GetMapID()const{return m_mapid;}	
	//服务器ID
	TServerID	GetServerID() const{return m_svrID;}
	//判断是不是世界游戏场景
	bool	IsWorldMap()const {return (m_id != INVALID_SCENE_ID);}


	bool	IsValid()const {return m_id != INVALID_SCENE_ID;}
};


//物品分类ID
typedef unsigned short   TGoodsID ;

#define INVALID_GOODS_ID  TGoodsID(0)

//最大单修人数

#define MAX_ALONE_XL_NUM  8

struct XPoint
{
	unsigned short x;
	unsigned short y;
};


typedef unsigned short TMonsterID;

#define INVALID_MONSTER_ID TMonsterID(0)


//等级
typedef unsigned char TLevel;


//物品ID
typedef unsigned short TGoodsID; 

#define INVALID_GOODS_ID TGoodsID(0)

//最大镶嵌宝石数量

#define MAX_INLAY_NUM   3

//招募角色ID
typedef unsigned int TEmployeeID;

//语言ID		fly add
typedef unsigned int TLanguageID;

//法术书ID
typedef unsigned short TMagicBookID;

//聚仙楼刷新一次出现的个数
#define FLUSH_NUM		  4

//法术ID
typedef unsigned short   TMagicID;

#define INVALID_MAGIC_ID  TMagicID(0)

//帮派技能ID
typedef unsigned short	 TSynMagicID;

//帮派福利ID
typedef unsigned short   TSynWelfareID;

//最多可学多少种帮派技能数量
#define MAX_SYNMAGICNUM  4

//最大独自修炼角色数
#define MAX_ALONE_XIULIAN_ACTOR_NUM    4

//100字节长度
#define	DESCRIPT_LEN_100	100

//75字节长度
#define	DESCRIPT_LEN_75	    75

//150字节长度
#define DESCRIPT_LEN_150	150

//50字节长度
#define	DESCRIPT_LEN_50	    50

//300字节长度
#define	DESCRIPT_LEN_300	300

//380字节长度
#define	DESCRIPT_LEN_380	380

//260字节长度
#define	DESCRIPT_LEN_260	260

//1024字节长度
#define DESCRIPT_LEN_1024	1024

//500字节长度
#define DESCRIPT_LEN_500	500

//200字节长度
#define DESCRIPT_LEN_200	200

//20字节长度
#define DESCRIPT_LEN_20		20

#define	DESCRIPT_LEN_600	600

//效果ID
typedef unsigned short  TEffectID;

#define INVALID_EFFECT_ID TEffectID(0)


//状态ID
typedef unsigned short  TStatusID;

#define INVALID_STATUS_ID TStatusID(0)

//状态组ID
typedef unsigned short TStatusGroupID;

#define INVALID_STATUSGROUP_ID TStatusGroupID(0)


//邮件ID
typedef unsigned int	 TMailID;	 

//帮派ID
typedef unsigned short   TSynID;

#define INVALID_SYN_ID TSynID(0)

//系统的模拟UserID
#define SYSTEMID		 0

//交易的模拟UserID
#define TRADEID			 1

//帮派的模拟UserID
#define SYNDICATEID		 2

//排行榜的模拟UserID
#define RANKID			 3


//可装备最大法术数量
#define MAX_EQUIP_MAGIC_NUM   3

//玩家可学最大法术数
#define MAX_STUDY_MAGIC_NUM   50

//最大参战角色数
#define MAX_COMBAT_ACTOR_NUM 5

//阵形位置总数量
#define MAX_LINEUP_POS_NUM		 9

//最大任务ID
#define MAX_TASK_ID   256

//无效的任务参数
#define INVALID_TASK_PARAM (-1)


//最大开启副本数
#define MAX_OPEN_FUBEN_NUM  10

//副本ID
typedef unsigned char  TFuBenID;

#define INVALID_FUBEN_ID  TFuBenID(0)


//法宝世界ID
typedef unsigned short  TTalismanWorldID ;

#define INVALID_TALISMANWORLD_ID TTalismanWorldID(0)

//目前开放的境界最大等级
#define OPEN_MAX_LAYER 7


//剑
typedef unsigned short   TSwordSecretID;   //剑诀ID
#define INVALID_SWORDSECRET_ID      TSwordSecretID(0) 


//参加法宝世界游戏角色数
#define MAX_TALISMAN_GAME_ACTOR_NUM  3


//传送门ID
typedef unsigned short TPortalID ;

#define INVALID_PORTAL_ID  TPortalID(0)

//冷却时间ID
typedef unsigned short TCDTimerID;

#define INVALID_CDTIMER_ID TCDTimerID(0)

//任务ID
typedef unsigned short TTaskID;

#define INVALID_TASK_ID TTaskID(0)

//可接的最大任务数
#define MAX_TASKNUM 50


//成就ID
typedef unsigned short TChengJiuID;
#define INVALID_CHENGJIU_ID TChengJiuID(0)


//称号
typedef unsigned char  TTitleID ;
#define INVALID_TITLE_ID TTitleID(0)


//称号长度,最大七个汉字
#define TITLE_NAME_LEN  16


//月最大签到领奖记录数
#define MAX_DAILY_AWARD_NUM  31

//怪物最大法术数量
#define MAX_MONSTER_MAGIC_NUM 5

//系统邮件最多可邮件物品数
#define MAX_MAIL_GOODS_NUM 10

//给各模块使用的标志
enum enUseFlag
{
	enUseFlag_First_EquipStronger = 0,	//第一次装备强化是否用过
	enUseFlag_First_TalismanStronger = 1,	//第一次法宝强化是否用过
	enUseFlag_First_FlushDouFaEnemy = 2,	//第一次刷新斗法对手是否用过
	enUseFlag_First_GodSwordStronger = 3,	//第一次仙剑强化是否用过

	enUseFlag_Max = 16,
};

//版本号字符串最大长度
#define   VERSION_LEN      18

//斗法对手数量
#define DOUFA_ENEMY_NUM	   3

//切磋对手数量
#define QIECUO_ENEMY_NUM   20


//加密算法类型
enum enCryptType VC_PACKED_ONE
{	
	enCryptType_No = 0,  //明文
	enCryptType_MD5 = 1, //MD5
	enCryptType_SHA1 = 2, //SHA1
	enCryptType_SHA256  =3 , //SHA256
	enCryptType_Max,

} PACKED_ONE;


//平台定义
enum enGamePlatformType VC_PACKED_ONE
{	
	enGamePlatformType_XunYou = 0,  //讯游
	enGamePlatformType_XinLang  =1 , //新浪
	enGamePlatformType_DangLe   = 2, //当乐
	enGamePlatformType_Max,

} PACKED_ONE;

//最大副本ID
#define MAX_FUBEN_ID   256

//帮派最多可容纳人数
#define MAX_SYN_USERNUM 128

//组队阵形每队最多可参战人数
#define MAX_TEAMCOMBAT_NUM 4


#define  MAX_TICKET_NO_LEN 17   //礼券码长度


#define INVALID_INDEX -1		//无效的夺宝玩家等级组下标

typedef int TGroupID;

#define INVALID_GROUPID 0		//无郊的组ID

#define XUAN_TIAN_DB 22			//玄天DB操作

#define TALISMAN_WORLD_DB 23	//法宝世界DB操作

#define MAX_SYNMEMBER_NUM 256	//帮派最大成员数

#define SYN_WAR_DB 22			//帮战DB操作

#define INVALID_TWLevel -1		//无效的法宝世界级别

#endif

