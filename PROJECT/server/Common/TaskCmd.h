#ifndef __XJCQ_GAMESRV_TASK_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_TASK_CMD_PROTOCOL_H__

#pragma pack(push,1)


#define MAX_TASK_AWARD_GOODS_NUM   4  //任务最大奖励物品数


//任务命令定义
enum enTaskCmd VC_PACKED_ONE
{
	enTaskCmd_CS_OpenTask    = 0, //打开任务栏
	enTaskCmd_SC_AddTask,         //增加任务
	enTaskCmd_SC_DelTask,         //删除任务
	enTaskCmd_SC_Update,          //更新任务状态
	enTaskCmd_TakeAward,         //领取奖励
	enTaskCmd_SC_UpdateIcon,	 //通知客户端更新主界面图标


	enTaskCmd_Max,

}PACKED_ONE;

//任务消息
struct  TaskHeader  : public AppPacketHeader
{
	TaskHeader(enTaskCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Task;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//任务结果码定义
enum enTaskRetCode VC_PACKED_ONE
{
	enTaskRetCode_Ok = 0,
	enTaskRetCode_ErrPacketFull,	//背包已满，请清理背包
	
} PACKED_ONE;

//任务分类
enum enTaskClass VC_PACKED_ONE
{
	enTaskClass_Mainline = 0,	//主线
	enTaskClass_Daily ,			//日常
	enTaskClass_Gang ,			//帮派任务
	enTaskClass_Max,

} PACKED_ONE;

struct SAwardGoods
{
	TGoodsID  m_GoodsID;
	UINT16    m_Number; //数量
};

//任务奖励
struct STaskAwardInfo
{
	STaskAwardInfo(){
		memset(this, 0, sizeof(*this));
	}
	INT32               m_NimbusStone;     //灵石
	INT32               m_GiftTicket;      //礼券
	INT32               m_Experience;      //经验
	INT32               m_Nimbus;          //灵气
	INT32               m_GodSwordNimbus;  //仙剑灵气
	UINT32				m_SynContribution;	//帮派贡献
	INT32				m_SynExp;			//帮派经验
	INT32				m_Honor;			//荣誉
	SAwardGoods         m_Goods[MAX_TASK_AWARD_GOODS_NUM];  //物品
	INT32				m_PolyNimbus;		//聚灵气
	INT32				m_Credit;		    //声望
	INT32				m_GhostSoul;			//灵魄
};


//一个任务数据
struct STaskData
{
	TTaskID             m_TaskID;          //任务ID
	enTaskClass          m_TaskClass;        //任务分类
	UINT32                m_FinishTime;       //完成时间
	bool                m_bGuide; //是否需要引导
	INT16               m_TargetCount; //完成任务需要达到的目标计数
	INT16               m_CurCount;   //当前已达到的计数

	//奖励
    STaskAwardInfo      m_TaskAward;

	//char              m_szTaskName[];    //任务名称
	//char              m_szTaskTarget[];  //任务目标
	//char              m_szTaskDesc[] ;  //任务描述
	//char              m_szTaskAward[];  //任务奖励

};

//打开任务栏应答
struct SC_TaskOpenTask_Rsp
{
	UINT16        m_TaskNum;  //任务数量

	//STaskData   m_Task[m_TaskNum];
};

//增加任务
//STaskData

//删除任务
struct SC_TaskDeleteTask
{
	TTaskID         m_TaskID;   //任务ID
};

//更新任务
struct SC_TaskUpdateTask
{
	TTaskID        m_TaskID;
	bool           m_bFinished;
	UINT16         m_nCurCount; //当前计数
};



//领取奖励
struct CS_TaskTakeAward_Req
{
	TTaskID           m_TaskID;
	enTaskClass       m_TaskClass;        //任务分类
};

struct SC_TaskTakeAward_Rsp
{
	enTaskRetCode      m_Result;

	TTaskID            m_TaskID;

		//奖励
    STaskAwardInfo      m_TaskAward;
};

//任务主界面显示状态
enum enTaskStatus VC_PACKED_ONE
{
	enTaskStatus_Common = 0,	//普通任务状态
	enTaskStatus_NewTask,		//新开任务状态
	enTaskStatus_FinishTask,	//完成任务状态

	enTaskStatus_Max,
}PACKED_ONE;

//通知客户端改变任务状态
struct SC_ChangeTaskStatus
{
	SC_ChangeTaskStatus() : m_TaskStatus(enTaskStatus_Max)
	{
	}

	enTaskStatus m_TaskStatus;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
