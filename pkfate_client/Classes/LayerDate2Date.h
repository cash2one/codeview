//
//  LayerDate2Date.h
//  PKFate
//
//  Created by claudis on 15-9-10.
//
//

#ifndef __PKFate__LayerDate2Date__
#define __PKFate__LayerDate2Date__

#include "cocos2d.h"
#include "ui/UIButton.h"
#include "cocostudio/CocoStudio.h"
#include "LayerDate.h"

USING_NS_CC;
using namespace cocos2d::ui;

class LayerDate2Date : public Layer
{
public:
	virtual bool init(const char *from, const char *to, std::string flag);

	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch* touch, Event* event);

	void DelayExit(float dt);

	void okCallBack();
	void cancleCallBack();

	static LayerDate2Date* create(const char *from, const char *to, std::string flag);
public:
	Layout *panelDate;
	Node *RootNode;
	LayerDate *DateFrom, *DateTo;
	Button *btOk, *btCancel;
	std::string mflag;
};

#endif /* defined(__PKFate__LayerDate2Date__) */
