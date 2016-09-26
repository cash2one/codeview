#pragma once
#include "cocos2d.h"
USING_NS_CC;

class LayerSignin;

struct SRewards
{
	unsigned char byRewardsType;
	int numRewards;
	bool bTake = false;
};

class SigninControl :public Ref
{
public:
	static SigninControl* GetInstance();

	void requestSigninList();

	void setLayerSignin(LayerSignin* pLayerSignin);

	typedef std::map<int, std::vector<SRewards>> SigninRewardsMap;

protected:
	SigninControl();
	~SigninControl();
	
	void onDoneSigninTaskCallBack(Ref *pSender);

	void onRewardSigninTaskCallBack(Ref *pSender);

	void onGetSigninRewardsCallBack(Ref *pSender);

	void onRewardSigninTaskNotification(Ref *pSender);

	void onDoneSigninTaskNotification(Ref *pSender);

	void onClearLayerSigninNotification(Ref *pSender);

	void onRefreshSigninDayNumberNotification(Ref *pSender);

	void onRefreshDayRewardsNotification(Ref *pSender);
private:
	SigninRewardsMap _mapSigninRewards;

	int _numSigninDay;

	LayerSignin* _pLayerSignin;
};