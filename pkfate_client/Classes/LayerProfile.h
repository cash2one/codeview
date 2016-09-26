#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerProfile : public Layer
{
protected:
	
	Button *btnCancel, *btnOk, *btnAgree;
	std::function<void()> _profileCallback;
public:

	void addProfileCallback(const std::function<void()> &callback);

	void setWithAgree(bool withAgree);
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	
	
	CREATE_FUNC(LayerProfile);
};