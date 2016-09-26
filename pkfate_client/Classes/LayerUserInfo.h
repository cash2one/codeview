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
#include "comm.h"
#include "UserControl.h"
USING_NS_CC;
using namespace cocos2d::ui;
//, public EditBoxDelegate
class LayerUserInfo : public Layer
{
protected:
	//bool _enableExit;		//是否允许退出，首次设置时，不允许退出
	Node *RootNode;
	//TextBMFont *txtCoins, *txtLevel;
	Layout *touch_layer;
	//Button *btnInfo, *btnPass, *btnConfirm;
	//Sprite *oldPassCheck, *newPassCheck, *confirmPassCheck;
	//EditBox *_oldPassEditBox, *_newPassEditBox, *_confirmPassEditBox;
	LoadingBar *imgLevelBar;
	//Button* btnChange;
	//
	Node* bjl_node, *dz_node, *label_change_bg;
	TextBMFont* txtCoins, *txtLevel;
	Text* label_tip, *year_fortune, *year_winRate, *year_profit, *nick_name, *label_change;
	Layout* select_avatar;
	ScrollView * info_scrollview;
	ListView *listAvatars;
	Sprite* avatar;
	//Button *btEnter;
	//void DelayExit(float dt);
	void selectedItemEvent(Ref *pSender, ListView::EventType type);

	void funcChangeAvatar();
	void SetBalanceData(Ref *ref);

	void onSetAvatarCallBack(Ref *pSender);

	
	
public:
	void lookUserInfo(int user_id,string nickname, int avatar);
	void lookGameData(int user_id);
	/*Text *lbNickName;*/
	int _avatarId;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	//virtual void editBoxReturn(EditBox* editBox);

	void updateGameDatas(map<int, GameData*> tempMap);

	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	//void onTouchEnded(Touch* touch, Event  *event);
	CREATE_FUNC(LayerUserInfo);
};



class LayerOthersInfo : public LayerUserInfo
{
public:
	virtual bool init();
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerOthersInfo);
};