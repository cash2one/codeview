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


class FriendSelectMenu :public Widget
{
	// ----------------自定义	
protected:

	Node * _nodeRoot;

public:
	FriendSelectMenu();
	~FriendSelectMenu();
	
	CREATE_FUNC(FriendSelectMenu);

};

class FriendItem :public Widget
{
	// ----------------自定义	
protected:
	

	int user_id;
	string nickname;
	int avatar;
	Text *_title, *_date;

	Button* _btnItem;

	Node * _nodeRoot;

	void selectedEvent(cocos2d::Ref *pSender, CheckBoxEventType type);

public:

	void reset();

	FriendItem();
	~FriendItem();
	void setProperty(FriendData* friendData,int i);
	CREATE_FUNC(FriendItem);
	
};


enum FriendSelect
{
	FRIEND_PANEL = 1,
	INVITE_PANEL = 2,
};

class LayerFriend :public Layer, public EditBoxDelegate
{
	// ----------------自定义	
protected:	
	Text* friend_tip,*text_invite_code,*text_invite_num,*text_invite_bet;

	Button *_btExit, *btn_bind, *btn_send, *btn_invite, *btn_search, *btn_phone_invite, *btn_share_invite;

	Layout* invite_panel, *validate_panel, *handler_panel, *invite_info_panel;
	Layout* friend_panel;

	Button *btnFriend;
	Button *btnInvite;

	ListView * friendList;
	EditBox *_textPhoneEditBox, *_textCodeEditBox,*_searchTextEditBox;

public:
	LayerFriend();
	~LayerFriend();

	
	CREATE_FUNC(LayerFriend);
	void onButtonExitClicked(Ref *ref);

	void refreshInvitePanel();

	virtual void editBoxReturn(EditBox* editBox);
	void selectPanel(FriendSelect select);
	void onUpdateFriendList(Ref* pSender);
	void updateFriendList(vector<FriendData*> vec);

	void onWatchOtherInfo(Ref* pSender);
	void funcShare();
	void funcSearch();
	// ----------------自定义结束
	virtual bool init();
};
