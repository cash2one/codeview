#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UIPageView.h"
#include "ui/UIImageView.h"
#include "ui/UITextField.h"
USING_NS_CC;
using namespace cocos2d::ui;



class LayerDzAddScore : public Layer
{
protected:
	TextField * text_num;
	Text* label_des,*label_change;

	int count,totalCount;

	Button *btn_left, *btn_right, *btn_confirm;

	void changeCount(int add);
	void btnClickHandler(Ref * sender);
public:

	
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	
	
	CREATE_FUNC(LayerDzAddScore);
};