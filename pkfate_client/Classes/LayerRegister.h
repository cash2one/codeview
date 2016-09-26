#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "comm.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerRegister : public Layer, public EditBoxDelegate
{
protected:
	Node *RootNode;
	Layout *panelRegister;
	void DelayExit(float dt);
	
	Button *btRegister;
	void btRegisterClicked(Ref *ref);
	void onRegisterCallBack(Ref *pSender);
	Sprite *imgCheckUserName, *imgCheckPassword, *imgCheckPassword1;
	void funcRegister();
	void btExistUser(Ref *ref);
public:
	Text *lbUserName, *lbPassword, *lbPassword1;
	EditBox *_editBoxUserName, *_editBoxPassword, *_editBoxPassword1;
	bool CheckUserName(), CheckPassword(), CheckPassword1();
	void onLoginCallBack(Ref *pSender);
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void editBoxReturn(EditBox* editBox);
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerRegister);
	virtual ~LayerRegister();
};