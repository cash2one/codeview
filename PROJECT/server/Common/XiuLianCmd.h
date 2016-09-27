#ifndef __XJCQ_GAMESRV_XIULIAN_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_XIULIAN_CMD_PROTOCOL_H__

#pragma pack(push,1)





//修练消息命令字
enum enXiuLianCmd VC_PACKED_ONE
{
	enXiuLianCmd_Open              =0, //打开
	enXiuLianCmd_AloneXiuLian_Sync,    //独自修炼数据同步
	enXiuLianCmd_AloneXiuLian ,        //独自修炼
	enXiuLianCmd_CancelAloneXiuLian,   //独自修炼收功
	enXiuLianCmd_GetNimbus,            //获得灵气

	enXiuLianCmd_TwoXiuLian_Sync,     //同步双修数据
	enXiuLianCmd_AskTwoXiuLian,       //请求双修
	enXiuLainCmd_CancelTwoXiuLian,    //双修收功
	enXiuLainCmd_SC_Over,			  //双修结束		 (不使用)

    enXiuLianCmd_MagicXiuLian_Sync,   //同步修炼法术数据 (没用了)
	enXiuLianCmd_AskMagicXiuLian,     //请求修炼法术     (没用了)
	enXiuLianCmd_AcceptMagicXiuLian,  //接受修炼法术	 (没用了)
	enXiuLianCmd_RejectMagicXiuLian,  //拒绝修炼法术	 (没用了)
	enXiuLianCmd_CancelMagicXiuLian,  //取消法术修炼	 (没用了)

	enXiuLianCmd_ViewAskData,         //查看修炼请求	 (没用了)
	enXiuLianCmd_AddAskData,          //增加请求数据	 (没用了)
	enXiuLianCmd_CancelAskData,       //取消请求数据	 (没用了)

	enXiuLianCmd_EffectXLElement,	  //得到影响修炼的元素

	enXiuLianCmd_InXiuLianActor,	  //得到正在修炼中的角色UID

	enXiuLianCmd_Max,

}PACKED_ONE;

struct XiuLian_Header: public AppPacketHeader
{
	XiuLian_Header(enXiuLianCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_XiuLian;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//修炼操作相关结果码
enum enXiuLianRetCode VC_PACKED_ONE
{
	enXiuLianRetCode_OK            =  0,  //OK	
	enXiuLianRetCode_ErrorActor,          //无效的角色
	enXiuLianRetCode_HaveAloneXL,           //已有角色在独自修炼
	enXiuLianRetCode_HaveTwoXL,           //已有角色在双修炼
	enXiuLianRetCode_HaveMagicXL,           //已有角色在修炼法术
	enXiuLianRetCode_NoSpiritStone,         //灵石不足
	enXiuLianRetCode_NoAloneXL,             //没有角色在独自修炼
	enXiuLianRetCode_NoTwoXL,             //没有角色在双修炼
	enXiuLianRetCode_NoMagicXL,             //没有角色在修炼法术
	enXiuLianRetCode_NoAskData,             //修练请求已被取消了
	enXiuLianRetCode_NoMagic,               //没有学会该法术
	enXiuLianRetCode_NoGoods,                //没有修炼法术需要的道具
	enXiuLianRetCode_ErrRelation,			//好友度不够修炼法术
	enXiuLianRetCode_ErrTwoXiuLian,			//已有双修请求未处理，一次只能请求一份
	enXiuLianRetCode_ErrXiuLianMagic,		//已有修练法术请求未处理，一次只能请求一份

} PACKED_ONE;

//修炼类型
enum enXiuLianType VC_PACKED_ONE
{	
	enXiuLianType_Non,    //未修炼
	enXiuLianType_Alone , //独自修炼
	enXiuLianType_Two,    //双修
	enXiuLianType_Magic,  //修炼法术
	enXiuLianType_Max,
} PACKED_ONE;

//修炼方式
enum enXiuLianMode VC_PACKED_ONE
{
	enXiuLianMode_Study     =  0,  //学习
	enXiuLianMode_Teach,           //传授
	enXiuLianMode_Max,

} PACKED_ONE;

//修炼状态
enum enXiuLianState VC_PACKED_ONE
{
	enXiuLianState_Non     =0,  //未修炼
	enXiuLianState_Ask    ,  //发出请求未处理(没用了)
	enXiuLianState_Doing,         //修炼中
	enXiuLianState_Finish,        //完成
	enXiuLianState_Cancel,       //请求被取消(没用了)
	enXiuLianState_Stop,       //修炼被中止(没用了)

} PACKED_ONE;



//打开修炼
struct CS_OpenXiuLian_Req
{
	enXiuLianType m_XiuLianType;
};


//独自修炼数据同步
struct SC_AloneXiuLianData_Sync
{
	SC_AloneXiuLianData_Sync(): m_XiuLianState(enXiuLianState_Non),m_TotalTime(0),
		m_RemianTime(0),m_ActorNum(0)
	{

	}

	enXiuLianState  m_XiuLianState; //状态
	UINT32	m_TotalTime;			//总时间
	UINT32	m_RemianTime;			//剩余时间
	INT32	m_GetGodSwordNimbus;	//获得的仙剑灵气
    UINT8	m_ActorNum;				//修炼角色数量
	//SC_AloneXLData m_ActorData[m_ActorNum];
};

struct SC_AloneXLData
{
	SC_AloneXLData() : m_uidActor(UID()),m_GetNimbus(0),m_NimbusUp(0)
	{
	
	}

	UID		m_uidActor;
	INT32	m_GetNimbus;	//当前获得的灵气值
	INT32	m_NimbusUp;		//灵气上限
};


//独自修炼请求
struct CS_AlongXiuLian_Req
{
	UINT8 m_Hours;       //修练小时数
	UINT8 m_ActorNum;    //修炼角色数量
	//UID m_uidActor[m_ActorNum];
};

struct SC_AloneXiuLian_Rsp
{
   enXiuLianRetCode m_Result;
};

//取消
struct SC_CancelAloneXL_Rsp
{
	SC_CancelAloneXL_Rsp(){
		MEM_ZERO(this);
	}
	enXiuLianRetCode m_Result;
	UINT8            m_Hours;			//请求修练小时数
	UINT8			 m_RealHours;		//实际修练小时数
	UINT32			 m_Stone;			//共花费灵石数量
	UINT32			 m_TotalNimbus;		//总共获取到的灵气
	UINT8			 m_ActorNum;		//独自修练参与的角色数
	//SC_CancelSendName_Rsp SendName[m_ActorNum];	//发送名字
};

struct SC_CancelSendName_Rsp
{
	char			 m_szActorName[THING_NAME_LEN];	//角色名字		
};

//同步双修数据
struct SC_TwoXiuLianData_Sync
{
	SC_TwoXiuLianData_Sync(){
		MEM_ZERO(this);
	}
	enXiuLianState  m_XiuLianState;		//状态
	//TwoXiuLianData_Rsp  TwoXiuLianData;	//如果状态正在修炼中时，发送这个
};

struct TwoXiuLianData_Rsp
{
	INT32	m_RemianTime;					//剩余时间
	UINT32	m_TotalTime;					//总时间
	INT32	m_NimbusUp;						//当前灵气上限
	INT32	m_TotalNimbus;					//总共获得的灵气
	UID		m_uidActor;						//自己的角色
	UID		m_uidFriend;					//好友
	UINT16	m_FriendFacade;					//好友外观
	char	m_FriendName[THING_NAME_LEN];	//好友名字
};


//请求双修
struct CS_TwoXiuLian_Req
{
	UINT8  m_Hours;  //修练小时数
	UID   m_uidActor; //自己的角色
	UID   m_uidFriend; //好友
};

struct SC_TwoXiuLian_Rsp
{
    enXiuLianRetCode m_Result;
};


//接受双修
struct CS_AcceptTwoXiuLian_Req
{
	UINT32  m_AskSeq;  //请求序列号
	UID    m_uidActor; //自己的角色
};

struct SC_AcceptTwoXiuLian_Rsp
{
    enXiuLianRetCode m_Result;
};


//拒绝双修请求
struct CS_RejectTwoXiuLian_Req
{
	UINT32  m_AskSeq;  //请求序列号
};


struct SC_RejectTwoXiuLian_Rsp
{
    enXiuLianRetCode m_Result;
};

//取消双修
struct SC_CancelTwoXiuLian_Rsp
{
	 enXiuLianRetCode m_Result;
	 UINT8            m_Hours;          //请求修练小时数
	 UINT32			  m_RealTime;	    //实际修练时间(秒)
	 UINT32			  m_Stone;		    //共花费灵石数量
	 UINT32			  m_TotalNimbus;	//总共获取到的灵气
	 UID			  m_XiuLianActor;	//在双修角色的UID
};

//双修结束
struct SC_TwoXLOver
{
	 UINT8            m_Hours;          //请求修练小时数
	 UINT32			  m_RealTime;	    //实际修练时间(秒)
	 UINT32			  m_Stone;		    //共花费灵石数量
	 UINT32			  m_TotalNimbus;	//总共获取到的灵气
	 UID			  m_XiuLianActor;	//在双修角色的UID
};

//取消修炼法术
struct SC_CancelMagicXiuLian_Rsp
{
	SC_CancelMagicXiuLian_Rsp(){
		MEM_ZERO(this);
	}
	enXiuLianRetCode m_Result;
	UINT8            m_Hours;         //请求修练小时数
	UINT8			 m_RealHours;	  //实际修练小时数
	enXiuLianMode	 m_XiuLianMode;	  //修练模式
	char			 m_szActorName[THING_NAME_LEN];		//角色的名字
	char			 m_szFriendName[THING_NAME_LEN];	//好友名称
	TMagicID         m_MagicID; //法术
};

//同步修炼法术数据
struct SC_MagicXiuLianData_Sync
{
	enXiuLianState  m_XiuLianState; //状态
	//MagicLianXiuLianData_Rsp  MagicLianXiuLianData; //如果状态正在修炼中时发送
};

struct MagicLianXiuLianData_Rsp
{
	UINT32			m_AskSeq;						//请求序列号
	INT32			m_RemianTime;					//剩余时间
	enXiuLianMode	m_Mode;							//方式
	TMagicID		m_MagicID;						//法术ID
	UID				m_uidActor;						//自己的角色
	UID				m_uidFriend;					//好友
	char			m_szFriendName[THING_NAME_LEN];	//好友名称
};

//请求修炼法术
struct CS_MagicXiuLian_Req
{
	enXiuLianMode m_Mode; //方式
	TMagicID  m_MagicID; //法术ID
	UINT64   m_uidActor; //自己的角色
	UINT64   m_uidFriend; //好友
	char  m_szFriendName[THING_NAME_LEN];	//好友名称
};

struct SC_MagicXiuLian_Rsp
{
	enXiuLianRetCode m_Result;
};


//接受修炼法术
struct CS_AcceptMagicXiuLian_Req
{
	UINT32  m_AskSeq;  //请求序列号
	UID    m_uidActor; //自己的角色
};

struct SC_AcceptMagicXiuLian_Rsp
{
	enXiuLianRetCode m_Result;
};

//拒绝修炼法术请求
struct CS_RejectMagicXiuLian_Req
{
	UINT32  m_AskSeq;  //请求序列号
};

struct SC_RejectMagicXiuLian_Rsp
{
    enXiuLianRetCode m_Result;
};


//取消修炼法术
struct CS_CancelMagicXiuLian_Req
{
	UINT32  m_AskSeq;  //请求序列号
};

//struct SC_CancelMagicXiuLian_Rsp
//{
//	 enXiuLianRetCode m_Result;
//};



//修炼请求数据
struct XiuLianAskData
{
	UINT32           m_AskSeq;      //请求序列号
	UID              m_uidActor;    //发起请求者的角色
	UID              m_uidAcceptor; //接受者
	UID              m_uidFriendActor; //好友角色
	char             m_uidFriendName[THING_NAME_LEN]; //好友名称
	char			 m_SourceName[THING_NAME_LEN];	//发起者名称
	UINT8            m_Hours;       //请求修练小时数
	UINT32            m_RealTime;   //实际修练时间
    TMagicID         m_MagicID;     //修改法术ID,为零，表示双修
	enXiuLianMode    m_Mode;        //修改方式
	enXiuLianState   m_State;       //状态
	bool			 m_bIsLine;		//对方是否在线
};

//查看请求数据
struct CS_ViewXiuLianAskData_Req
{
	UINT32  m_AskSeq;  //查看该序列号之后的数据
};

struct SC_ViewXiuLianAskData_Rsp
{
	UINT8            m_AskDataNum; //数量
	//XiuLianAskData   m_AskData[m_AskDataNum];
};

//增加请求数据


//取消请求数据
struct CS_CancelAskSeq_Req
{
	CS_CancelAskSeq_Req() : m_AskSeqID(0){}
	UINT32			m_AskSeqID;
};

struct SC_CancelAskSeq_Rsp
{
	SC_CancelAskSeq_Rsp() : m_Result(enXiuLianRetCode_OK){}
	enXiuLianRetCode m_Result;
};

//得到影响修炼的元素
struct SC_EffectXLElement
{
	SC_EffectXLElement()
	{
		m_AddAloneXLPosNum = 0;
		m_SynWelfareHour = 0;
		m_DesNum = 0;
		m_VipAddPosNum  = 0;
		m_VipTakeResAdd = 0;
	}

	UINT8	m_AddAloneXLPosNum;		//额外增加的修炼位置数
	UINT8	m_SynWelfareHour;		//帮派福利增加的额外可选修炼小时数
	UINT8	m_DesNum;				//帮派福利减少的修炼费用(百分之几)
	UINT8	m_VipAddPosNum;			//VIP额外增加的修炼位置数
	UINT8	m_VipTakeResAdd;		//VIP修炼获得灵气和仙剑灵气的加成(百分比)
};

//正在修炼中的角色
struct SC_InXLActorUID
{
	SC_InXLActorUID()
	{
		m_TwoXLActorUID = UID();
		m_Num = 0;
	}

	UID		m_TwoXLActorUID;		//在双修的角色UID
	UINT8	m_Num;					//数量
//	UID		m_AloneUID[m_Num];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
