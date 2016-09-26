#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UISlider.h"
#include "ui/UILayout.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerSetting : public Layer
{
protected:
	Node *RootNode;
	void SliderEvent(Ref *pSender, Slider::EventType type);
	void DelayExit(float dt);
public:
	void TestFunc();
	Slider *sliderMusic, *sliderSound;
	Button *btnMuteMusic, *btnMuteSound;
	Layout* touch_layer;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch* touch, Event* event);
	// music,sound func
	void SetMusicMute(bool mute);
	void SetMusicVolume(float vol);
	void SetSoundMute(bool mute);
	void SetSoundVolume(float vol);
	CREATE_FUNC(LayerSetting);
};