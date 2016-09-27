
#ifndef __COMBATSERVER_ISTATUS_H__
#define __COMBATSERVER_ISTATUS_H__

#include "BclHeader.h"

#include "UniqueIDGenerator.h"

#include "DSystem.h"
#include "GameSrvProtocol.h"

struct IStatusPart;
struct SStatusCnfg;

//状态优先级
enum enStatusPriority VC_PACKED_ONE
{
	enStatusPriority_DuHuo,			//毒火
	enStatusPriority_FenJin,		//封禁
	enStatusPriority_TimeAddProp,	//一段时间内增加能力的(一次只能加一种)
}PACKED_ONE;

//状态类型
enum enStatusType VC_PACKED_ONE
{
	enStatusType_Du = 1,			//毒
	enStatusType_Huo,				//火
	enStatusType_FenYin,			//封印
	enStatusType_JinGu,				//禁固
	enStatusType_XuanYun,			//眩晕
	enStatusType_AddBuildRes,		//增加建筑产量
	enStatusType_AddProp,			//增加玩家属性

	enStatusType_Max,
}PACKED_ONE;

//状态信息
struct SStatusInfo
{
	TStatusID	m_StatusID;
	UINT32		m_EndStatusTime;			//状态结束时间(如果是以时间做为结束判断的话)
	UID			m_uidCreator;				//源角色UID,即是谁发起或者创建的这个状态
	bool		m_bAlreadyStart;			//状态是否已启动
	TMagicID	m_MagicID;					//引发这个状态的法术
	INT16		m_LeftRoundNum;				//剩余轮数(如果是以回合数做为结束判断的话)
};

struct IStatus
{
		//结束该状态
	virtual void OverStatus() = 0;

	//启动状态
	virtual bool Start(IStatusPart *pStatusPart, UID uidCreator, TMagicID MagicID =INVALID_MAGIC_ID) = 0;//开始
	

	//获取状态剩余轮数
	virtual UINT32 GetLeftRoundNum() = 0;	

	virtual void SetLeftRoundNum(INT32 num) = 0;	

	virtual void SetLeftEffectCount(INT32 num) = 0;	

	//减少轮数
	virtual void DecreaseRoundNum() = 0;

	//状态配置数据
	virtual const SStatusCnfg* GetStatusCnfg() = 0;

	//获取状态的记录类型
	virtual enStatusRocordType GetStatusRecordType() = 0;

	//获取状态的结束时间
	virtual UINT32	GetEndTimeNum() = 0;

	//设置状态的结束时间
	virtual void	SetEndTimeNum(UINT32 EndTime) = 0;

	//获得引发这个状态的法术ID
	virtual TMagicID GetMagicID() = 0;

	//设置状态动态数据
	virtual void SetStatusData(INT32 value) = 0;

	//获得状态动态数据
	virtual INT32 GetStatusData( ) = 0;

	// 释放
	virtual void Release() = 0;

	//保存
	virtual void SaveData() = 0;
};



#endif
