#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UILayout.h"
#include "ui/UITextBMFont.h"
#include "ui/UIListView.h"
#include "comm.h"
#include "cJSON.h"
#include "TaskControl.h"
#include "SigninControl.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

class SigninControl;

class LayerSignin :public Layer
{
public:
	LayerSignin();
	~LayerSignin();
	CREATE_FUNC(LayerSignin);

	virtual bool init();

	bool refreshDayRewards(SigninControl::SigninRewardsMap map);

	bool refreshObtainmentRoot(std::vector<SRewards> vec);

	bool showLayoutObtainment();
	
	virtual void onExit();

	void setSigninDayNumber(int day);

	void onTakeButtonClickEvent(Ref *ref);

protected:
	Node *_nodeSigninRoot, *_nodeObtainmentRoot;

};
