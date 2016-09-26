#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIImageView.h"
#include "ui/UILayout.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

class LayerMessageTip : public Layer
{
protected:
	Layout*tipLayer;
	ImageView *tipBg,*textBg;
	Text* tipMsg;
	std::function<void()> _confirmCallback;
	std::function<void()> _cancelCallback;

	Button* btnCancel, *btnConfirm;
public:

	
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void showTip(std::string msg, std::function<void()> confirmCallback, std::function<void()> cancelCallback = nullptr, bool isOnlyOk=false);
	
	CREATE_FUNC(LayerMessageTip);
};


class MessageTip :public Node
{
private:
	const unsigned short x_space, y_space;
	ImageView *tipBg;
	Text* tipMsg;
public:
	void setTip(std::string tip);
	MessageTip();
	CREATE_FUNC(MessageTip);
};
