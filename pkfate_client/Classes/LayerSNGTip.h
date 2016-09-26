#pragma once

#include "cocos2d.h"
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include "ui/UILayout.h"
#include "ui/UIText.h"
#include "DZControl.h"
#include "ui/UIScrollView.h"	
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;


class LayerSNGTip :public Layer
{
private:
	Node * rootNode;
	Text * msgTip;
	Button* btn, *btn_cancel, *btn_confirm;
public:
	
	void show();
	std::function<void()>  handler;
	std::function<void()> cancelHandler;
	void set(string tip, string label,bool only = true);
	void btnClickHandler(Ref * sender);
	CREATE_FUNC(LayerSNGTip);
	virtual bool init();
};