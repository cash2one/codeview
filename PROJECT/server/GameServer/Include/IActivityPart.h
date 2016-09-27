
#ifndef __THINGSERVER_IACTIVITY_PART_H__
#define __THINGSERVER_IACTIVITY_PART_H__


#include "IThingPart.h"


struct IActivityPart : public IThingPart
{
	//打开日常
	virtual void  OpenDialy() = 0;

	//打开签到
	virtual void OpenSignIn() = 0;

	//签到
	virtual void SignIn() = 0;

	//领取签到奖励
	virtual void TakeSignInAward(UINT8  AwardID) = 0;

	//打开活动
	virtual void OpenActivity() = 0;


		//领取活动奖励
	virtual void TakeActivityAward(UINT16  ActivityID) = 0;

	//领取在线奖励
	virtual void TakeOnlineAward() = 0;

		//推进进度
	virtual void AdvanceProgress(UINT16  ActivityID ) = 0;

	   //通知领取在线奖励
     virtual void OnlineAwardNotic()=0;

	 //获得新手指引的步骤
	 virtual INT16 GetNewPlayerGuideIndex() = 0;

	 //设置新手指引步骤
	 virtual void SetNewPlayerGuideIndex(INT16 Index) = 0;


	 	 //获得新手指引上下文
	 virtual const char* GetNewPlayerGuideContext() = 0;

	 //设置新手指引上下文
	 virtual void SetNewPlayerGuideContext(const char * pContext) = 0;

	 //多倍经验
	 virtual void OnMultipExp() = 0;

};








#endif


