#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UIPageView.h"
#include "ui/UIImageView.h"
USING_NS_CC;
using namespace cocos2d::ui;

class GuideTip :public Node
{
private:
	const unsigned short x_space, y_space;
	ImageView *tipBg;
	Text* tipMsg;
public:
	void setTip(std::string tip);
	GuideTip();
	CREATE_FUNC(GuideTip);
};

class LayerHelp : public Layer
{
protected:
	
	Button *btnExit;
	
	PageView * guidePageView;

	Node * slipNode;

	unsigned short selectIndex;

	void pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type);

	void selectPointIndex(unsigned short index);
public:

	
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	
	
	CREATE_FUNC(LayerHelp);
};