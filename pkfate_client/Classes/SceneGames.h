#pragma once

#include "cocos2d.h"
#include "ui/UILayout.h"
#include "ui/UIListView.h"
#include "ui/UIButton.h"
#include "ui/UIText.h"
#include "ui/UITextBMFont.h"
#include "ui/UILoadingBar.h"
#include "comm.h"

USING_NS_CC;
using namespace cocos2d::ui;

class SceneGames : public Scene
{
protected:
	Node *RootNode,*currentPanel;
	Layout *panelStatus, *panelContent;
	Text *txtNickName;
	TextBMFont *txtCoins, *txtLevel;
	ListView *listGames;
	Button *btnBuy, *btnReport, *btnSetting, *btnQuit, *btnRank, *btnTask,*btnMail,*btnFriend;
private:
	void addGames();
	bool isMoved;
	void SetUserInfo(Ref *ref);
	void SetBalanceData(Ref *ref);
	void showMall(Ref *ref);
	void showRank(Ref *ref);
	void triggerLoginAward(Ref *ref);
	void triggerRequestTask(Ref *ref);
	void triggerMailUnRead(Ref *ref);

	void triggerRequestNotice(Ref *ref);
	void triggerSwitchGameType(Ref *ref); 
	void TriggerLotteryTask(Ref *ref);
	void TriggerGuide(Ref* ref);
	void TriggerShowNotice(Ref* ref);
	void UpdateSceneGameInfo(Ref* ref);
	

	void onSelectedRoomEvent(Ref *pSender, ListView::EventType type);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	void playAction();


	void rescue(float dt);

	void onOpenSigninUINotification(Ref* ref);
	
public:
	Sprite *imgHead;
	LoadingBar *imgLevelBar;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();
	// implement the "static create()" method manually
	CREATE_FUNC(SceneGames);
	virtual ~SceneGames();

	Node* getRootNode();
	Layout* getPanelContent();
	ListView* getListGames();
	Node* getCurrentPanel();
};