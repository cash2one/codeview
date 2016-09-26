#pragma once

#include "cocos2d.h"
#include "ui/UILayout.h"
#include "ui/UIButton.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerLang : public Layer
{
protected:
	Node *RootNode;
	Layout *panelSetting;
	Button *btZh, *btEn;
	void btClicked(Ref *ref);
	void setButtonStatus(bool isZh);
public:
	static void SetLangResPath();
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch* touch, Event* event);
	CREATE_FUNC(LayerLang);
};