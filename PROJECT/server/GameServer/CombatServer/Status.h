
#ifndef __COMBATSERVER_STATUS_H__
#define __COMBATSERVER_STATUS_H__

#include "IStatus.h"
#include "IThingPart.h"
#include "UniqueIDGenerator.h"
#include <vector>
#include "IEventServer.h"
#include "ITimeAxis.h"


struct IStatusPart;
struct SStatusCnfg;
struct IEffect;

class Status : public IStatus,public IEventListener,public ITimerSink
{
	//定时器ID
	enum enStatusTimerID
	{
		enStatusTimerID_TimeNumStatus = 0,	//一次性时间
		enStatusTimerID_TimeIntervalStatus ,    //定时
		
	};

public:
	Status();
	virtual ~Status();

public:
	//结束该状态
	virtual void	OverStatus();
	
    //获取状态剩余轮数
	virtual UINT32	GetLeftRoundNum();

	virtual void	SetLeftRoundNum(INT32 num);

	virtual void	SetLeftEffectCount(INT32 num);

	//减少轮数
	virtual void	DecreaseRoundNum(); 

	//状态配置数据
	virtual const	SStatusCnfg* GetStatusCnfg(); 
	
	//获取状态的记录类型
	virtual enStatusRocordType GetStatusRecordType();

	//获取状态的结束时间
	virtual UINT32	GetEndTimeNum();

	//设置状态的结束时间
	virtual void	SetEndTimeNum(UINT32 EndTime);

		//获得引发这个状态的法术ID
	virtual TMagicID GetMagicID(); 

	//设置状态动态数据
	virtual void SetStatusData(INT32 value);

		//获得状态动态数据
	virtual INT32 GetStatusData( ) ;

	// 释放
	virtual void Release();

	//保存
	virtual void SaveData();

public:
	virtual void OnEvent(XEventData & EventData);

	virtual void OnTimer(UINT32 timerID);

public:
	bool Create(const SStatusCnfg* pStatusInfo);									// 创建

	bool Create(const SStatusCnfg* pStatusInfo, const SStatusInfo & StatusInfo);	// 创建

	bool CanStart(IStatusPart *pStatusPart, UID uidCreator);						// 是否可以开始

	bool Start(IStatusPart *pStatusPart, UID uidCreator, TMagicID MagicID =INVALID_MAGIC_ID);	//开始

	Status* Clone();																// clone一个本状态

private:
	//启动效果
void StartEffect();


//暂停效果
void StopEffect();

protected:
	bool	 m_bAlreadyStart;			//状态是否已启动
	UID		 m_uidCreator;				//源角色UID,即是谁发起或者创建的这个状态
	TMagicID m_MagicID;					//引发这个状态的法术

	INT16    m_LeftRoundNum;			//剩余轮数(如果是以定时间隔做为结束判断的话)

	UINT32	 m_EndStatusTime;			//状态结束时间(如果是以时间做为结束判断的话)

	IStatusPart*						m_pStatusPart;				//本状态所属管理器
	const SStatusCnfg*					m_pStatusInfo;				//本状态的配置,指向

	typedef std::vector<IEffect*>		vecEffects;
	typedef vecEffects::iterator		vecEffectsItr;
	vecEffects							m_Effects;					//状态带的效果对象

	INT32                      m_DynamicStatusData;  //状态的动态数据


	INT16              m_LeftEffectCount; //剩余生效次数
};






#endif
