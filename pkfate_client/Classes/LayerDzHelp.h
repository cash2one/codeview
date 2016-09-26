#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UIPageView.h"
#include "ui/UIImageView.h"
USING_NS_CC;
using namespace cocos2d::ui;



class LayerDzHelp : public Layer
{
protected:
	

	Layout *optPanel, *rulePanel;
	Button *btnOpt, *btnRule, *btnExit;
	PageView * btnOpt_guidePageView, *btnRule_guidePageView;
	Node * btnOpt_slipNode, *btnRule_slipNode, *bg;
	unsigned short btnOpt_selectIndex;
	unsigned short btnRule_selectIndex;
	void chooseOptPanel(bool isOpt);

	void btnOpt_pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type);

	void btnRule_pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type);

	void selectOptPointIndex(unsigned short index);
	void selectRulePointIndex(unsigned short index);
	bool onTouchBegan(Touch *touch, Event *event);

public:

	
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	
	
	CREATE_FUNC(LayerDzHelp);
};