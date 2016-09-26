#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UITextBMFont.h"
#include "ui/UIScrollView.h"
#include "ui/UITextField.h"
#include "comm.h"
#include "MailControl.h"
#include "SceneDzRoom.h"
USING_NS_CC;
using namespace cocos2d::ui;
//, public EditBoxDelegate
class LayerMailReader : public Layer
{
protected:
	//bool _enableExit;		//是否允许退出，首次设置时，不允许退出
	Node *RootNode;
	
	Layout *touch_layer;
	Button *btnConfirm,*btnCancel;
	TextField *text_content;
	Text* text_title,*from;
	MessageData* messageData;

	void agreeInvite();
	void agreeFriend();
	void reply();
	void refuseFriend();

	void finishRead();
public:
	/*Text *lbNickName;*/
	int _avatarId;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	void setMessageData(MessageData* messageData);
	

	CREATE_FUNC(LayerMailReader);
};