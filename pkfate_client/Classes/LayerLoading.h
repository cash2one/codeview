#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UISlider.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerLoading : public Layer
{
protected:

	Node *RootNode;
	Text *txtLoading;
	Sprite *spLoading;
	bool _isSteady;//稳固的不会被删除
public:
	Button *btCancel;
	void setSteady(bool isSteady);
	bool isSteady();
	void SetTimeout(float timeout);//设置loading超时时间,某些等待操作时间超过normal，比如苹果接口调入
	void SetString(const std::string msg);		//设置显示文字
	void Tip(const std::string msg);
	static void Wait(const std::string msg = "", float timeout = 5.0f, bool isSteady = false);
	static void Stop(const std::string msg = "");
	static void CloseWithTip(const std::string msg = "",bool force = false);
	static void Close(bool force = false);
	static void SetVisible(bool visible);
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerLoading);

	//LayerLoading event callback.
	//typedef std::function<void()> loadingEventCallback;

	/**
	* Set a event handler to the LayerLoading.
	* @param callback The callback in `loadingEventCallback`.
	*/
	//void addEventListener(const loadingEventCallback& callback);
	


	//Cover the CCNode removeFromParent

protected:
	//loadingEventCallback _eventListener;
};