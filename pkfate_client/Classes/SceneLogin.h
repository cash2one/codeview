#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
USING_NS_CC;
using namespace cocos2d::ui;

class SceneLogin : public Scene
{
protected:
	Node *RootNode;
	Button *btRegister, *btLoginLocal, *btLoginPkfate;
	void btRegisterClicked(Ref *ref);
	void btLoginPkfateClicked(Ref *ref);
	void btLoginLocalClicked(Ref *ref);
	void onLocalRegisterCallBack(Ref *pSender);
	void onLoginCallBack(Ref *ref);
	void dealLocalLogin();
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	// implement the "static create()" method manually
	bool LocalLogin(std::string username, std::string password);
	bool AutoLogin(std::string username, std::string password);

	
	CREATE_FUNC(SceneLogin);
};