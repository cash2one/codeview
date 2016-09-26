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
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UITextField.h"
#include "ui/UICheckBox.h"
#include "FriendControl.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;




class LayerInviteDzWait :public Layer
{
	// ----------------自定义	
protected:	


	
	Button *btn_confirm, *btn_wait;

	void inviteFriend();

	void wait();

public:
	LayerInviteDzWait();
	~LayerInviteDzWait();
	
	CREATE_FUNC(LayerInviteDzWait);
	void onButtonExitClicked(Ref *ref);
	
	// ----------------自定义结束
	virtual bool init();
};
