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
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UITextField.h"
#include "ui/UICheckBox.h"
#include "FriendControl.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

enum FROMPAGE
{
	WRITE_MAIL_PAGE=1,
	WAIT_INVITE_PAGE=2
};

class FriendInviteItem :public Widget
{
	// ----------------自定义	
protected:
	

	

	Text *_title, *_date;

	Button* _btnItem;

	Node * _nodeRoot;
	
	void selectedEvent(cocos2d::Ref *pSender, CheckBoxEventType type);

public:
	
	
	CheckBox * currentClickCheckBox;
	void reset();
	FriendInviteItem();
	~FriendInviteItem();
	CheckBox* setProperty(FriendData* friendData,int i);
	CREATE_FUNC(FriendInviteItem);
	
};



class LayerFriendInvite :public Layer
{
	// ----------------自定义	
protected:	
	short from;

	Button *_btExit;

	Text* friend_tip;
	
	Layout* friend_panel;

	
	Button *btnInvite;

	ListView * friendList;
	
	int select_user_id;

	CheckBox * lastClickCheckBox;

	std::vector<CheckBox*> boxList;

	void writeInviteMail();
public:
	LayerFriendInvite();
	~LayerFriendInvite();
	void setFrom(FROMPAGE from);
	
	CREATE_FUNC(LayerFriendInvite);
	void onButtonExitClicked(Ref *ref);
	void onSelectFriend(Ref *ref);
	

	void onUpdateFriendList(Ref* pSender);
	void updateFriendList(vector<FriendData*> vec);
	void funcInvite();
	
	// ----------------自定义结束
	virtual bool init();
};
