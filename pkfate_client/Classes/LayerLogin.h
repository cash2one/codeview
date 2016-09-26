#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "comm.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerLogin : public Layer, public EditBoxDelegate
{
protected:
	Node *RootNode;
	Layout *panelLogin;
	cocos2d::ui::EditBox* _editBoxUserName, *_editBoxPassword;
	Button *btLogin;
	void btLoginClicked(Ref *ref);
	void onLoginCallBack(Ref *pSender);
	void login();
public:
	Text *msgUserName, *msgPassword,*errorMsg;
	bool CheckUserName(), CheckPassword();
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void editBoxReturn(EditBox* editBox);
	
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerLogin);
	virtual ~LayerLogin();
};