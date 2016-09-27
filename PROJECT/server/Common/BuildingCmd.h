#ifndef __XJCQ_GAMESRV_BUILDING_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_BUILDING_CMD_PROTOCOL_H__

#pragma pack(push,1)

//聚仙楼每次刷新角色和法术书数量
#define FLUSH_EMPLOYEE_NUM   4

#define FLUSH_MAGICBOOK_NUM  4

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//建筑消息命令字
enum enBuildingCmd VC_PACKED_ONE
{
	enBuildingCmd_OpenBuilding = 0,   //打开自己的建筑
	enBuildingCmd_TakeRes,    //领取资源
	enBuildingCmd_Collection, //代收

	enBuildingCmd_VisitBuilding, //访问别人的建筑

	enBuildingCmd_EnterFuMoDong,     //进入伏魔洞
	enBuildingCmd_AutoKillMonster,   //自动打怪
	enBuildingCmd_CancelKillMonster, //取消自动打怪
	enBuildingCmd_AccelKillMonster,  //打怪加速
    enBuildingCmd_KillMonster,       //手动打怪
	enBuildingCmd_SyncOnHookData,    //同步挂机状态


	enBuildingCmd_EnterTrainingHall, //进入练功堂
	enBuildingCmd_StartTraining,     //开始练功
	enBuildingCmd_StopTraining,      //停止练功
	enBuildingCmd_SyncTrainingData,  //同步练功数据


	enBuildingCmd_EnterGatherGodHouse,  //进入聚仙楼
	enBuildingCmd_View,                 //查看聚仙楼所售的东西
	enBuildingCmd_Flush,               //刷新数据
	enBuildingCmd_Buy,                  //购买
	enBuildingCmd_SyncEmployee,        //同步雇佣角色数据
	enBuildingCmd_SyncBook,            //同步法术书

	enBuildingCmd_EnterHouShan,			//进入后山

	enBuildingCmd_EnterGodSwordShop,	//进入剑冢
	enBuildingCmd_FlushGodSwordShop,	//刷新数据
	enBuildingCmd_BuyGodSword,			//购买仙剑
	enBuildingCmd_SyncGodSwordShop,		//同步仙剑数据

	enBuildingCmd_ViewBuildRecord,		//查看资源产出建筑记录

	enBuildingCmd_TrainingFinishNow,	//练功堂立即完成

	enBuildingCmd_SynResBuildInfo,		//同步资源建筑信息

	enBuildingCmd_SC_GiveAutoKillForword,	//伏魔洞自动打怪奖励

	enBuildingCmd_AutoFlushEmploy,		//自动刷新招募角色

	enBuildingCmd_Set_AutoTakeRes,		//设置自动收取灵石


	enBuildingCmd_StartXiWu,			//开始习武
	enBuildingCmd_CancelXiWu,			//取消习武
	enBuildingCmd_SC_Syc_XiWu,			//同步习武
	
	enBuildingCmd_Max,

} PACKED_ONE;


//建筑，消息头
struct BuildingHeader : public AppPacketHeader
{
	BuildingHeader(enBuildingCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Building;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};


//建筑类型
enum enBuildingType VC_PACKED_ONE
{
	enBuildingType_Beast = 0,         //灵兽园
	enBuildingType_Grass ,            //百草园
	enBuildingType_Stone ,            //灵石山
	enBuildingType_FuMoDong,          //伏魔洞
	enBuildingType_HouShan,           //后山	
	enBuildingType_TrainingHall,      //练功堂
	enBuildingType_GatherGodHouse,    //聚仙楼
	enBuildingType_SwordDeath,        //剑冢

	enBuildingType_Max,
} PACKED_ONE;

//资源产出建筑
#define ResOutputBuildingType_Max  (enBuildingType_Stone+1)

//打开建筑
struct CS_OpenBuilding_Req
{
	enBuildingType m_BuildingType;
};

struct SC_OpenBuilding_Rsp
{
	SC_OpenBuilding_Rsp()
	{
		MEM_ZERO(this);
	}

	enBuildingType m_BuildingType;
     INT32         m_ResOutputRemainingTime;            //资源产出余下时间
	 INT32         m_ResOutputSpace;                    //资源产出总间隔

};



//领取资源
struct CS_TakeRes_Req
{
	enBuildingType m_BuildingType;
};

struct SC_TakeRes_Rsp
{
	UID		m_uidBuilding;
	INT32   m_TakeResNum; //领取的资源数量,如果为零则提示玩家当前无资源可领取
};

enum enBuildingRetCode  VC_PACKED_ONE
{
	enBuildingRetCode_OK = 0,
	enBuildingRetCode_NoVip,		//您不是VIP，不能开启
	enBuildingRetCode_ErrVipLv,		//您VIP等级不足以开启
} PACKED_ONE;


//进入建筑应答
struct SC_EnterBuilding_Rsp
{
	enBuildingRetCode  m_RetCode;
};


//访问别人的建筑
struct CS_VisitOtherBuilding_Req
{
	enBuildingType m_BuildingType;
	UID            m_uidOther;
};

struct SC_VisitOtherBuilding_Rsp
{
	SC_VisitOtherBuilding_Rsp()
	{
		MEM_ZERO(this);
	}
	 UID      m_UidBuilding;//建筑标识
	 INT32   m_ResOutputRemainingTime;  //资源产出余下时间

	 INT32   m_TotalCollectResOfDay; //今天已窃取的资源数
};




//窃取操作结果码
enum enFilchRetCode VC_PACKED_ONE
{
	enFilchRetCode_OK    =  0,  //OK
	enFilchRetCode_ErrNoRes, //没有资源可窃取
	enFilchRetCode_ErrResLimit,  //已达当天窃取/代收上限

} PACKED_ONE;


//代收
struct CS_CollectionOther_Req
{
	enBuildingType m_BuildingType;
	UID            m_uidOther;
};

struct SC_CollectionOther_Rsp
{
	enFilchRetCode m_RetCode;
	INT32          m_ResNum;  //代收得到的资源数量
	INT32          m_OtherResNum; //被代收获得的资源数量
	UID			   m_uidBuild;
};

//查看资源产出建筑记录
struct CS_ViewBuildRecord_Req
{
	CS_ViewBuildRecord_Req() : m_BuildingType(enBuildingType_Max){}
	enBuildingType m_BuildingType;
};

struct SC_RecordNum_Rsp
{
	SC_RecordNum_Rsp() : m_RecordNum(0){}
	UINT16			m_RecordNum;
	//SC_ViewBuildRecord_Rsp BuildRecord[m_RecordNum];
};

struct SC_ViewBuildRecord_Rsp
{
	UINT32			m_HappenTime;						//发生时间
	char			m_szOtherUserName[THING_NAME_LEN];	//对方名字
	char			m_szHandleName[THING_NAME_LEN];		//操作名字
	char			m_szContext[DESCRIPT_LEN_50];		//内容
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//伏魔洞

//进入伏魔洞


//挂机状态同步
struct SC_OnHookData_Sync
{
	INT32  m_RemainingTime; //本次挂机剩余时间
	INT32  m_AccelNum;      //加速次数
	UINT8  m_Level;			//伏魔洞层次
	INT32  m_Price;			//消耗的仙石或礼卷
};

//自动打怪
struct CS_AutoKillMonster_Req
{
	INT32  m_Level; //伏魔洞层次
};

//打怪结果码
enum enKMRetCode VC_PACKED_ONE
{
	enKMRetCode_OK    =  0,  //OK
	enKMRetCode_ErrOnHook, //已在挂机
	enKMRetCode_ErrNoOnHook, //不在挂机状态
	enKMRetCode_ErrLvLimit, //级别不足，无法挂机 
    enKMRetCode_AccelNumLimit, //挂机加速次数已耗尽
	enKMRetCode_NoMoney,  //仙石或礼卷不足
	enKMRetCode_ErrCreateMonsterFail, //创建怪物失败
	enKMRetCode_ErrCombat, //不能和怪物战斗
	enDKRetCode_ErrLevelLimit,	//级别不足，不能进入
	enKMRetCode_ErrInDuoBao, //无法进入，正在匹配夺宝战
	enKMRetCode_ErrHaveTeam, //组队中，无法进入
	enKMRetCode_ErrWaitTeam, //等待组队中，无法操作
} PACKED_ONE;

struct SC_AutoKillMonster_Rsp
{
	SC_AutoKillMonster_Rsp()
	{
		m_Result = enKMRetCode_OK;
	}

	enKMRetCode  m_Result;  //结果
};


//取消自动打怪
struct CS_CancelKillMonster_Req
{
	
};

struct SC_CancelKillMonster_Rsp
{
	enKMRetCode  m_Result;  //结果
};


//加速
struct CS_AccelKillMonster_Req
{

};

struct SC_AccelKillMonster_Rsp
{
	enKMRetCode  m_Result;  //结果
};

//手动打怪
struct CS_KillMonster_Req
{
	INT32  m_Level; //伏魔洞层次
};

//THING_MAP_NAME_LEN     24字节

struct SC_KillMonster_Rsp
{
	enKMRetCode  m_Result;  //结果
	INT32  m_Level; //伏魔洞层次
//	KillMonsterData Data;	//如果OK,才发送这
};

/*
struct KillMonsterData
{
	char		 m_ScernName[THING_MAP_NAME_LEN];	//战斗地点名字
	////CombatCombatData CombatData;
};
*/

//伏魔洞自动打怪获得奖励
struct SC_AutoKillMonsterForward
{
	INT32		m_GetExp;
	UINT16		m_Num;
//	TGoodsID	GoodsID[m_Num];
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//练功结果码
enum enTrainingRetCode VC_PACKED_ONE
{
	enTrainingRetCode_OK    =  0,  //OK
	enTrainingRetCode_ErrOnHook, //已在挂机
	enTrainingRetCode_ErrNoOnHook, //不在挂机状态	
    enTrainingRetCode_TrainingNumLimit, //次数限制
	enTrainingRetCode_NoStone,			//灵石不足
	enTrainingRetCode_NoMoneyOrTicket,  //仙石或礼卷不足
	enTrainingRetCode_ErrInDuoBao,		//无法进入，正在匹配夺宝战
	enTrainingRetCode_ErrHaveTeam,		//组队中，无法进入
	enTrainingRetCode_ErrWaitTeam,		//等待组队中，无法操作

	enTrainingRetCode_HaveXiWu,			//已有角色在习武中
	enTrainingRetCode_NotMaster,		//主角不可习武
	enTrainingRetCode_NoActor,			//请选择角色
	enTrainingRetCode_NoMoreMaster,		//招募角色等级不可高于主角
} PACKED_ONE;


//练功堂
//进入练功堂

//进入练功堂应答
struct SC_EnterTrainingHall_Rsp
{
	SC_EnterTrainingHall_Rsp(){
		m_Result = enTrainingRetCode_OK;
	}

	enTrainingRetCode m_Result; //结果
};

//开始练功

//开始练功应答
struct SC_StartTraining_Rsp
{
	enTrainingRetCode m_Result; //结果
	
};

//取消练功

//取消练功应答
struct SC_StopTraining_Rsp
{
	enTrainingRetCode m_Result; //结果
};

enum enTrainingStatus VC_PACKED_ONE
{
	enTrainingStatus_Non = 0,	//不在练功中
	enTrainingStatus_Do,		//正在练功中
	enTrainingStatus_Pause,		//在暂停中

}PACKED_ONE;

//同步练功数据
struct SC_TrainingData_Sync
{
	INT16				m_CurOnHookNum;		//当前挂机次数
	INT32				m_RemainTime;		//剩余时间
	INT32				m_GetExp;			//总共获得经验
	INT16				m_TotalOnHookNum;	//总挂机数
	INT32				m_Price;			//立即完成需消耗的仙石或礼卷数量
	enTrainingStatus	m_Status;			//练功状态
	INT32				m_Charge;			//练功消耗灵石数
	UINT32				m_TotalTime;		//练功总时间
	UINT8				m_VipFinishTrainNum;//已使用VIP立即完成次数
	UINT32				m_DecTrainingTime;	//已减少的练功时间
};

//立即完成练功
struct SC_TrainingFinishNow
{
	SC_TrainingFinishNow() : m_Result(enTrainingRetCode_OK)
	{
	}

	enTrainingRetCode m_Result; //结果
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//聚仙楼

//操作码
enum enGatherGodCode  VC_PACKED_ONE
{
	enGatherGodCode_OK = 0,		//操作成功
	enGatherGodCode_Time,		//自动刷新时间未到
    enGatherGodCode_NoMoney,	//仙石或礼卷不足
	enGatherGodCode_NumLimit,	//超出可雇用上限
	enGatherGodCode_PacketFull, //背包已满，请清理背包
	enGatherGodCode_NoItem,		//列表中没此物品
	enGatherGodCode_NoConfigItem,//物品配置表中找不到此物品
	enGatherGodCode_NoStone,	//灵石不足
	enGatherGodCode_ErrName,	//已有相同名字招募角色，招募失败
	enGatherGodCode_NoMoney_AutoFlush,	//仙石不足，无法自动刷新

} PACKED_ONE;

//刷新方式
enum enFlushType        VC_PACKED_ONE
{
	enFlushType_Free,	//免费刷新
    enFlushType_Money,  //付费刷新，需要消耗仙石
} PACKED_ONE;


//聚仙楼出售的东西类型
enum enSellItemType        VC_PACKED_ONE
{
	enSellItemType_Employee, //雇用兵
    enSellItemType_Book,     //法术书

	enSellItemType_Max,
} PACKED_ONE;

//进入聚仙楼

//雇佣人员数据
struct SEmployeeData
{
	SEmployeeData() : m_EmployeeID(0), m_Price(0)
	{
	}

	UINT32	m_EmployeeID;				//id
	INT32	m_Price;					//价格
};

//购买法术书数据
struct SMagicBookData
{
	SMagicBookData() : m_MagicBookID(0), m_PriceStone(0)
	{
	}

	TGoodsID m_MagicBookID;				//id
	INT32	 m_PriceStone;				//价格，耗灵石的
};

//查看Item
struct CS_ViewItem_Req
{
     enSellItemType m_ItemType;
};


//请求刷新
struct CS_FlushItem_Req
{
	enFlushType     m_FlushType;  //刷新方式
	enSellItemType  m_ItemType;   //Item
};

//同步雇佣人员数据
struct SC_EmployeeData_Sync
{
	SC_EmployeeData_Sync() : NextFlushRemainTime(0), m_PriceFlush(0), m_MoneyDes(0)
	{
	}

	//下次刷新剩余时间
	INT32  NextFlushRemainTime;
	INT32  m_PriceFlush;	//刷新费用
	UINT8  m_MoneyDes;		//减少的费用(百分比)
	SEmployeeData m_EmployeeData[FLUSH_EMPLOYEE_NUM];
};

//请求角色招募刷新结果
struct SC_FlushItem_Rsp
{
	SC_FlushItem_Rsp(){
		MEM_ZERO(this);
	}
	enGatherGodCode m_Result;
};

//同步法术书数据
struct SC_MagicBookData_Sync
{
	SC_MagicBookData_Sync() : NextFlushRemainTime(0), m_PriceFlush(0), m_MoneyDes(0)
	{
	}

	//下次刷新剩余时间
	INT32  NextFlushRemainTime;
	INT32  m_PriceFlush;	//刷新费用
	UINT8  m_MoneyDes;		//费用减少多少(百分比)
	SMagicBookData  m_MagicBookData[FLUSH_MAGICBOOK_NUM]; //
};

//请求法术书刷新结果
struct SC_FlushMagicBook_Rsp
{
	SC_FlushMagicBook_Rsp(){
		MEM_ZERO(this);
	}
	enGatherGodCode m_Result;
};


//购买,招募
struct CS_BuyItem_Req
{
	CS_BuyItem_Req() : m_ItemType(enSellItemType_Max), m_Index(0)
	{
	}

    enSellItemType  m_ItemType;  //Item
	UINT8			m_Index;	 //第几个，从0~3
};

//购买结果
struct SC_BuyItem_Rsp
{
	SC_BuyItem_Rsp() : m_Result(enGatherGodCode_OK)
	{
	}

   enGatherGodCode m_Result;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//剑冢

enum enGodSwordShopRetCode VC_PACKED_ONE
{
	enGodSwordShopRetCode_OK = 0,
	enGodSwordShopRetCode_ErrorCDTime,		//免费刷新的CD时间还没到
	enGodSwordShopRetCode_FullPacket,		//背包已满，请清理背包
	enGodSwordShopRetCode_ErrorStone,		//灵石不足
	enGodSwordShopRetCode_ErrorMoney,		//仙石或礼卷不足
	enGodSwordShopRetCode_ErrorNoGoods,		//没有这个物品

	enGodSwordShopRetCode_Max,
}PACKED_ONE;

struct GodSwordInfo
{
	GodSwordInfo() : m_GodSwordID(0), m_Price(0)
	{
	}

	TGoodsID	m_GodSwordID;	//ID	
	INT32		m_Price;		//价格,灵石
};

//同步查看剑冢界面
struct SC_SynGodSwordShop_Rsp
{
	SC_SynGodSwordShop_Rsp() : m_RemainCDTime(0), m_PriceFlush(0), m_MoneyDes(0)
	{
	}

	UINT32			m_RemainCDTime;			//免费刷新剩余时间
	INT32			m_PriceFlush;			//刷新价格
	UINT8			m_MoneyDes;				//价格减少多少(百分比)
	GodSwordInfo	m_GodSword[FLUSH_NUM];	//剑冢的仙剑数组
};



//返回操作结果码
struct SC_GodSwordShopRetCode_Rsp
{
	SC_GodSwordShopRetCode_Rsp() : enRetCode(enGodSwordShopRetCode_OK){}
	enGodSwordShopRetCode enRetCode;
};

//刷新剑冢
struct CS_FlushGodSwordShop_Req
{
	CS_FlushGodSwordShop_Req() : m_FlushType(enFlushType_Free){}
	enFlushType m_FlushType;					//刷新方式
};

//在剑冢购买仙剑
struct CS_BuyGodSwordShop_Req
{
	CS_BuyGodSwordShop_Req() : m_Index(0)
	{
	}

	UINT8		m_Index;	//第几个，从0~3
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//同步资源建筑信息
struct SC_SynResBuildInfo
{
	UID			   m_uidLingShouBuild;					//灵兽园UID
	INT32          m_LingShouRemainingTime;				//灵兽园资源产出余下时间

	UID			   m_uidBaiCaoBuild;					//百草园UID
	INT32          m_BaiCaoRemainingTime;				//百草园资源产出余下时间

	UID			   m_uidLingShiBuild;					//灵石山UID
	INT32          m_LingShiRemainingTime;				//灵石山资源产出余下时间
};

//自动刷新招募角色
struct CS_AutoFlushEmploy
{
	UINT16			m_FlushNum;							//目标次数
	INT32			m_Aptitude;							//目标资质
};

struct SC_AutoFlushEmploy
{
	SC_AutoFlushEmploy(){
		m_RetCode = enGatherGodCode_OK;
		m_RealFlushNum = 0;
		m_UseGodStone  = 0;
		m_bOk		   = false;
		m_AptitudeUp   = 0;
	}

	enGatherGodCode m_RetCode;
	bool			m_bOk;			//是否成功刷到指定资质的角色
	UINT16			m_RealFlushNum;	//实际刷新次数
	INT32			m_UseGodStone;	//实际使用仙石数量
	INT32			m_AptitudeUp;	//刷到的最高资质
};

///////////////////////////////////
//设置自动收取灵石
struct CS_Set_AutoTakeRes
{
	bool			m_bOpen;		//是否开启
};


struct SC_Set_AutoTakeRes
{
	SC_Set_AutoTakeRes(){
		
		m_RetCode = enBuildingRetCode_OK;
	}

	enBuildingRetCode m_RetCode;
};

////////////////////////////////////////////////////////////

//开始习武
struct CS_StartXiWu
{
	UID		m_uidActor;		//习武角色的UID
};

struct SC_StartXiWu
{
	SC_StartXiWu()
	{
		m_RetCode = enTrainingRetCode_OK;
	}
	enTrainingRetCode m_RetCode;
};

//取消习武
struct SC_CancelXiWu
{
	SC_CancelXiWu()
	{
		m_RetCode = enTrainingRetCode_OK;
	}
	enTrainingRetCode m_RetCode;
};

//同步习武
struct SC_Syc_XiWu
{
	SC_Syc_XiWu()
	{
		m_LeftTime = 0;
	}

	UINT32	m_LeftTime;		//习武剩余时间
	UID		m_uidActor;		//参加习武的角色
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
