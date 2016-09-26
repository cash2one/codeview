#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "ui/UIListView.h"
#include "comm.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerSetNickName : public Layer, public EditBoxDelegate
{
protected:
	bool _enableExit;		//是否允许退出，首次设置时，不允许退出
	Node *RootNode;
	Layout *panelSet, *panelImg;
	EditBox *_editBoxNickName;
	
	ListView *listAvatars;
	Button *btEnter;
	void DelayExit(float dt);
	void selectedItemEvent(Ref *pSender, ListView::EventType type);
	bool CheckNickName();
	void btEnterClicked(Ref *ref);
	Size _sizeImgPanel;
	std::string _nickname;
	void onSetNickNameCallBack(Ref *pSender);
	void onSetAvatarCallBack(Ref *pSender);
	void onCheckNickNameCallBack(Ref *pSender);
public:
	Text *lbNickName;
	int _avatarId;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();
	virtual void editBoxReturn(EditBox* editBox);
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerSetNickName);
};