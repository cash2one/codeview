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


class LayerSngRule :public Layer
{
private:
	int roomId;
public:
	std::function<void()>  handler;
	void btnClickHandler(Ref * sender);
	CREATE_FUNC(LayerSngRule);
	virtual bool init();
};
