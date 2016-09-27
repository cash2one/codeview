
#ifndef __THINGSERVER_ISTATUSPART_H__
#define __THINGSERVER_ISTATUSPART_H__

#include "IThingPart.h"
#include "UniqueIDGenerator.h"
#include "GameSrvProtocol.h"
#include "IPacketPart.h"

struct IStatus;
struct SStatusTypeCnfg;

struct IStatusPart : public IThingPart
{
	//增加一个状态
	virtual bool AddStatus(TStatusID lID, UID uidCreator,std::vector<UINT8> & vectDeleteStatusType, TMagicID MagicID = INVALID_MAGIC_ID,INT32 StatusRoundNum=0) = 0;

	//增加一个状态,通过使用物品得到的   bOk是否是点击确认使用
	virtual bool AddStatus_UseGoods(TStatusID lID, UID uidCreator, SAddStatus & AddStatus) = 0;

	//删除状态
	virtual bool RemoveStatus(TStatusID lStatusID, UID uidCreator = UID()) = NULL;

	//根据状态ID进行查询
	virtual IStatus* FindStatus(TStatusID lStatusID) = NULL;

	//效果控制接口！直接给生物加效果，生物销毁的时候会自动销毁效果，外部不要控制效果的生命周期！！！！
	virtual	bool	AddEffect(TEffectID effID)=NULL;

	virtual	void	RemoveEffect(TEffectID effID)=NULL;

	//获得生物已有状态中优先级最高的一个状态类型
	virtual const SStatusTypeCnfg * GetMaxPriorityStatusTypeCnfg() = 0;

	//打开人物面板，显示状态
	virtual void	ShowStatusOpenUserPanel() = 0;

	//检测是否要通知客户端取消效果公告
	virtual void Check_CancelViewEffectMsg(TStatusID lStatusID) = 0;

	//得到对所有角色共用的状态
	virtual std::vector<StatusShowInfo> GetAllActorStatus() = 0;

	//使用已有的状态
	virtual void UseAlreadyHaveStatus(IStatus * pStatus) = 0;
};





#endif
