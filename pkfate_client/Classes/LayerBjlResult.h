#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"

USING_NS_CC;
using namespace cocos2d::ui;
class LayerBjlResult :public Layer
{
	// ----------------自定义	
protected:	
	Text *_textResultWin, *_textResultLose, *_textResultTotal;
public:
	float _amountWin, _amountLose;
	CREATE_FUNC(LayerBjlResult);
	void addAmount(float amount);
	void resetAmount();
	float getAmount();
	// ----------------自定义结束
	virtual bool init();
};