#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UITextBMFont.h"
#include "ui/UIScrollView.h"
#include "comm.h"
USING_NS_CC;
using namespace cocos2d::ui;
//, public EditBoxDelegate
class LayerPassword : public Layer, public EditBoxDelegate
{
protected:
	//bool _enableExit;		//是否允许退出，首次设置时，不允许退出
	Node *RootNode;
	TextBMFont *txtCoins, *txtLevel;
	Layout *touch_layer;
	Button *btnConfirm;
	Sprite *oldPassCheck, *newPassCheck, *confirmPassCheck, *avatar;
	EditBox *_oldPassEditBox, *_newPassEditBox, *_confirmPassEditBox;
	LoadingBar *imgLevelBar;
	//

	void SetBalanceData(Ref *ref);
	void onUpdatePasswordCallBack(Ref *pSender);
	void funcChangePass();
public:
	/*Text *lbNickName;*/
	int _avatarId;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	virtual void editBoxReturn(EditBox* editBox);
	bool CheckOldPassword();
	bool CheckNewPassword();
	bool CheckConfirmPassword();
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	bool onTouchEnd(Touch *touch, Event *event);
	CREATE_FUNC(LayerPassword);
};