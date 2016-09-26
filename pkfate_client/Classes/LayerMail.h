#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UILayout.h"
#include "ui/UITextBMFont.h"
#include "ui/UIListView.h"
#include "comm.h"
#include "cJSON.h"
#include "MailControl.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;


class MailItem :public Widget
{
	// ----------------自定义	
protected:
	
	Text *_title, *_date,*_from;

	Button* _btnItem;

	Sprite* read_icon, *unread_icon;

	MessageData * messageData;

public:
	MailItem();
	~MailItem();
	void setProperty(MessageData* messageData);

	CREATE_FUNC(MailItem);
	
};


class LayerMail :public Layer
{
	// ----------------自定义	
protected:	
	Button *_btExit;

	Text* mail_tip;

	ListView * mailList;

	int currentType;


public:
	LayerMail();
	~LayerMail();
	CREATE_FUNC(LayerMail);
	void onButtonExitClicked(Ref *ref);
	void updateMailList();
	int getMailType();
	// ----------------自定义结束
	virtual bool init();
};
