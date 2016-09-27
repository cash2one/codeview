
#ifndef __THINGSERVER_ICHENGJIU_H__
#define __THINGSERVER_ICHENGJIU_H__



#include "IThingPart.h"
#include <string>


struct IChengJiuPart : public IThingPart
{
	//推进成就进度
	virtual void AdvanceProgress(TChengJiuID  ChengJiuID ) = 0;

	//复位成就进度
	virtual void ResetProgress(TChengJiuID  ChengJiuID ) = 0;

	//是否已获得某成就
	virtual bool IsAttainChengJiu(TChengJiuID  ChengJiuID) = 0;

	//打开成就栏
	virtual void OpenChengJiuPanel() = 0;

	//更换称号
	virtual void ChangeTitle(TTitleID  TitleID) = 0;

	//获得当前称号
	virtual std::string GetTitle() = 0;

	//得到当前称号ID
	virtual TTitleID	GetTitleID() = 0;

	//获取成就完成时间
	virtual UINT32		GetChengJiuFinishTime(TChengJiuID ChengJiuID) = 0;

	//发送自己的成就数据给别的玩家（成就对比用）
	virtual void		SendMeChengJiuToOtherUser(IActor * pRecvActor) = 0;

};



#endif
