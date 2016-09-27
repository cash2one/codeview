#ifndef __XJCQ_GAMESRV_CHENGJIU_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_CHENGJIU_CMD_PROTOCOL_H__

#pragma pack(push,1)

//TChengJiuID 16
//TTitleID 8
//TGoodsID 16


//成就命令定义
enum enChengJiuCmd VC_PACKED_ONE
{
	enChengJiuCmd_CS_OpenTask    = 0, //打开成就栏
	enChengJiuCmd_SC_Update,          //更新成就状态
	enChengJiuCmd_ChangeTitle,        //更改称号
	enChengJiuCmd_Get_UserChengJiu,	  //得到某角色有进度或完成的成就数据(成就对比有用到)

	enChengJiuCmd_Max,

}PACKED_ONE;

//成就消息
struct  ChengJiuHeader  : public AppPacketHeader
{
	ChengJiuHeader(enChengJiuCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_ChengJiu;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//成就结果码定义
enum enChengJiuRetCode VC_PACKED_ONE
{
	enChengJiuRetCode_Ok = 0,
	enChengJiuRetCode_ErrTitleID,	//您当前没有此称号
	
} PACKED_ONE;

//成就类型
enum enChengJiuClass VC_PACKED_ONE
{
	enChengJiuClass_XiuXian    = 0,		//修仙类
	enChengJiuClass_Othet,				//其它

	enChengJiuClass_Max,
}PACKED_ONE;

//修仙类的子类
enum enChengJiuSubClass VC_PACKED_ONE
{
	enChengJiuSubClass_Actor	= 0,	//人物
	enChengJiuSubClass_FuBen,			//副本
	enChengJiuSubClass_DouFa,			//斗法
	enChengJiuSubClass_TalismanWorld,	//法宝世界
	enChengJiuSubClass_Syn,				//帮派
	enChengJiuSubClass_Magic,			//法术
	enChengJiuSubClass_Other,			//其它的

	enChengJiuSubClass_Max,
}PACKED_ONE;

//其它类的子类
enum enOtherChengJiuSubClass VC_PACKED_ONE
{
	enOtherChengJiuSubClass_Actor = 0,	//人物
	enOtherChengJiuSubClass_DouFa,		//斗法
	enOtherChengJiuSubClass_TalismanWorld,	//法宝世界
	enOtherChengJiuSubClass_Syn,		//帮派
	enOtherChengJiuSubClass_Magic,		//法术
	enOtherChengJiuSubClass_Goods,		//物品
	enOtherChengJiuSubClass_Money,		//贷币
	enOtherChengJiuSubClass_Friend,		//仙缘
	enOtherChengJiuSubClass_JiSha,		//击杀
	enOtherChengJiuSubClass_Stronger,	//强化
	enOtherChengJiuSubClass_Other,		//其它
	enOtherChengJiuSubClass_Only,		//唯一

	enOtherChengJiuSubClass_Max,
}PACKED_ONE;

struct SChengJiuAwardGoods
{
	TGoodsID  m_GoodsID;
	UINT16    m_Number; //数量
};

//一个成就的数据
struct SChengJiuDataCnfg
{
	TChengJiuID         m_ChengJiuID;
	UINT8 	            m_Class;			//分类
	UINT8               m_SubClass;			//子分类
	UINT16              m_ChengJiuPoint;	//成就点
	UINT32              m_FinishTime;		//完成时间
	TTitleID            m_TitleID;			//可获得称号
	INT16               m_TargetCount;		//完成任务需要达到的目标计数
	INT16               m_CurCount;			//当前已达到的计数
	UINT16				m_ResID;			//资源ID
	UINT8               m_AwardGoodsNum;	//可获得奖励的物品种类
	//SChengJiuAwardGoods     m_AwardGoods[m_AwardGoodsNum];  //物品
	//char              m_szChengJiuName[]; //成就名称，以零结束
	//char              m_szTargetDesc[];   //获得方式
	//char              m_szAttainProb[];   //获得几率
	//char              m_szTitleName[];    //可获得称号   

};

//打开成就栏应答
struct SC_ChengJiuOpenChengJiu_Rsp
{
	INT32         m_ChengJiuPoint; //已获得的成就点
	INT32         m_TotalChengJiuPoint; //总成就点
	TTitleID      m_ActiveTitleID;      //当前使用的称号ID
    char          m_szTitleName[TITLE_NAME_LEN]; //称号名称
	UINT16		  m_OkChengJiuNum;	//已完成成就数量
	UINT16        m_ChengJiuNum;  //成就数量

	//SChengJiuDataCnfg   m_Task[m_TaskNum];
};



//更新成就
struct SC_ChengJiuUpdateChengJiu
{
	TChengJiuID    m_ChengJiuID;
	UINT32         m_FinishedTime; //完成时间
	UINT16         m_nCurCount; //当前计数
};

//更换称号
struct CS_ChengJiuChangeTitle_Req
{
	TTitleID      m_TitleID;
};


//应答
struct SC_ChengJiuChangeTitle_Rsp
{
	SC_ChengJiuChangeTitle_Rsp() : m_Result(enChengJiuRetCode_Ok), m_TitleID(INVALID_TITLE_ID) { }

	enChengJiuRetCode  m_Result;
	TTitleID           m_TitleID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//查看某角色有进度或者完成了的成就数据
struct CS_GetUserChengJiu
{
	UID				m_uidUser;
};

struct SC_GetUserChengJiu
{
	SC_GetUserChengJiu(){
		m_Result = enChengJiuRetCode_Ok;
		m_ActiveTitleID  = 0;
		m_Num			 = 0;
	}

	enChengJiuRetCode	m_Result;
	TTitleID			m_ActiveTitleID;		//当前启用的称号ID
	UINT16				m_Num;
//	UserChengJiuData	m_ChengJiuData[m_Num];
};

struct UserChengJiuData
{
	TChengJiuID  m_ChengJiuID;			//成就ID
	UINT32       m_FinishTime;			//达成时间，为零表示未达成
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
