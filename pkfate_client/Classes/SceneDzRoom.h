#pragma once
#include "cocos2d.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UIButton.h"
#include "ui/UICheckBox.h"
#include "ui/UITextBMFont.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIText.h"
#include "DZControl.h"
#include "dzpkcomm.h"
#include "LayerGuideMask.h"
USING_NS_CC;
using namespace cocos2d::ui;
class BetAddPanel 
{
private:
	Node * UI;
	Text * bet_amount;
	Sprite * all_light, *btn_slip;
	bool isHitSlip ;
	Button *btn_x1_2, *btn_x2_3, *btn_x1, *btn_all;
	CCProgressTimer* betProgress;
	int percent;
	int chip;
	int gamePoolBet;
	int minBet;
	int addBet;
	//加注slip
	//四个控制
	//顶端显示数额,all in
	
public:
	void updateGamePoolBet(int bet);
	int getBet();
	void updateChip(int chip);
	void setVisible(bool is);
	bool isVisible();
	
	void injectUI(Node * UI);
	void setPercent(int percent);
	void reset();
	void setMinBet(int minBet);
};



class Player
{
protected:
	short pos;
	
	int chip;//剩余携带
	int bonus;//收回bet
	int bet;//场上单局投注额
	int roundBet;//单轮投注
	int betStatus;//投注状态
	int betAction;//投注行为
	string handCards;
	int round;//一轮开局所属回合
	bool isPlayFlip;//正在翻手牌
	deque<Node*> actionFlips;//翻牌
	bool isGameCallEnd;
	
public:
	RoomPlayer * roomPlayer;

	virtual void updateBetInfo(BetInfo* info) = 0;;
	virtual void updateChip(int chip) = 0;;
	virtual void updateBet(int bet,int roundBet) = 0;;
	virtual void updatePlayerInfo(RoomPlayer* roomPlayer) = 0;;
	virtual void startBeting(BetBegin* betBegin) = 0;;//开始投注中
	virtual void stopBeting() = 0;//结束
	virtual void setBetVisible(bool is) = 0;
	virtual void updateResult(PlayerResult* result) = 0;;
	virtual void flyCards(float delay,float interval) = 0;
	virtual short getPos() = 0;
	virtual void setPos(short pos) = 0;
	
	virtual int getBet() = 0;
	virtual void validatePoker(int type, map<int, bool>& pokerMap, bool light = false) = 0;
	virtual string getHandCards()=0;
	virtual void setGameStart() =0;
	virtual void updateRound(int round) = 0;
	virtual void playFlipCards(Node* pokerFront, float delay = 0.0) = 0;
	virtual void win() = 0;
	virtual void playBetCollectAction()=0;
	virtual void playBetToPoolAction() = 0;
	virtual bool isCallEnd() = 0;
	virtual void leave()=0;
	virtual void updateAction(int status, int action) = 0;//游戏行为
	virtual void updateStatus(int status)=0;//玩家状态
	virtual void finishGame() = 0;
	virtual void clearSeat() = 0;
};



class DzPlayer :public Player
{
private:
	short avatarIndex;
	bool isSeat;
	Sprite *avatar, *card_left, *card_right, *progressSprite, *progress_bg;
	Text* name;
	Text* betAmount, *betAmount2;
	ProgressTimer * waitProgress;
	//倒计时的
	Node* UI;
	
public:
	virtual void updateBetInfo(BetInfo* info);
	virtual void updateChip(int chip);
	virtual void updateBet(int bet, int roundBet);
	virtual void updatePlayerInfo(RoomPlayer* roomPlayer);
	virtual void startBeting(BetBegin* betBegin);//开始投注中
	virtual void stopBeting();//结束
	virtual void setBetVisible(bool is);
	virtual void updateResult(PlayerResult* result);
	virtual void flyCards(float delay, float interval);
	virtual short getPos();
	virtual void setPos(short pos);

	
	virtual int getBet();//获取当前round bet金额
	virtual void validatePoker(int type, map<int, bool>& pokerMap, bool light = false);
	virtual string getHandCards();
	virtual void updateRound(int round);
	virtual void playFlipCards(Node* pokerFront, float delay = 0.0);
	virtual void win();
	virtual void playBetCollectAction();
	virtual void playBetToPoolAction();
	virtual bool isCallEnd();//跟结束
	virtual void leave();
	virtual void updateAction(int status, int action);
	virtual void updateStatus(int status);
	virtual void finishGame();
	virtual void clearSeat();

	void setAvatarIndex(short index);
	void injectUI(Node * UI);
	void setGameStart();
	void seatPlayer(bool seating);
	
};

class HandPanel {
private:
	Node* UI;
	Sprite * card_left, *card_right,*left_light,*right_light,*two_light;
	bool isHandHide;
public:
	void setGameStart();
	void injectUI(Node * UI);
	void show(string handCards);
	void throwCards();//丢牌
	void lightCard(map<int, bool>& pokerMap);
	void hide();
	bool isHide();
};

class OperaterPanel:public Player
{

private:
	
	BetBegin *betBegin;
	
	HandPanel * handPanel;
	Node *waitMyselfPanel, *waitOthersPanel;
	Layout * user_panel, * gamingPanel;
	Sprite * avatar, *progressSprite, *progress_bg,*progress_font;
	ProgressTimer * waitProgress;
	Text* betAmount, *name,*betAmount2,*tip;
	Button  *btnSit, *btn1, *btn2, *btn3;
	CheckBox  *btn4, *btn5, *btn6;
	Node *UI;
	int autoIndex;
	int callBet;//需要跟的注码大小
	int addCount;//每round 加注次数   最多3次
	bool isDisableOperator;//是否禁用面板
	void leaveTipHandler();

public:
	BetAddPanel * betAddPanel;
	bool isGaming;
	virtual void updateBetInfo(BetInfo* info);
	virtual void updateChip(int chip);
	virtual void updateBet(int bet, int roundBet);
	virtual void updatePlayerInfo(RoomPlayer* roomPlayer);
	virtual void startBeting(BetBegin* betBegin);//开始投注中
	virtual void stopBeting();//结束
	virtual void setBetVisible(bool is);
	virtual void updateResult(PlayerResult* result);
	virtual void flyCards(float delay, float interval);
	virtual short getPos();
	virtual void setPos(short pos);
	
	virtual int getBet();//获取当前round bet金额
	virtual string getHandCards();
	virtual void validatePoker(int type, map<int, bool>& pokerMap, bool light= false);
	virtual void updateRound(int round);
	virtual void playFlipCards(Node* pokerFront, float delay = 0.0);
	virtual void win()  ;
	virtual void playBetCollectAction();
	virtual void playBetToPoolAction();
	virtual bool isCallEnd();//跟结束
	virtual void leave();
	virtual void updateAction(int status, int action);
	virtual void updateStatus(int status);
	virtual void finishGame();
	virtual void clearSeat();
	bool isMe();
	void validateCallBet(int call_bet);
	void waitOthers();//等待别人
	void waitMyself();//等待自己
	void disableOperator();
	void setGameStart();
	void injectUI(Node * UI);
	void autoSelect(int autoIndex);
	void betAct(int action,int bet=0);
	void updateHandCards(string handCards);
	void cancelAuto(int autoIndex);
	void hideHand();

	void showAddPanel();
	~OperaterPanel();
};



class CardPublicPanel 
{
	//公共牌面
   // 动画显示 高亮
protected: 
	Node* UI;
	Text *tip,*poolLabel,*left_time;
	Node *imgCm, *imgBgAmount;
	int bet;
	int gamePoolBet;
	bool isPlayFlip;
	deque<Node*> actionFlips;
	vector<Node*> cardExtras;
	string publicCards;
	bool hasValidate;
	bool isPlayNumAction;
	float fee;
	int left;
public:
	    void injectUI(Node * UI);
		void updateCards(RoundInfo * info);
		void setGameStart();
		void playFlipCards(Node* pokerFront, float delay = 0.0);
		void onUpdateGamePoolBet(EventCustom* evt);
		void updateBet(int bet);
		void flopCards();//发牌
		void validatePoker(int type, map<int, bool>& pokerMap, bool light = false);
		void playNumAction();
		string getPublicCards();
		void finishGame();
		void startLeftTime(int seconds);
		void timeLeftHandler(float dt);
		~CardPublicPanel();
		
};




class SceneDzRoom :public Scene, public EditBoxDelegate, protected GuideHandler
{
protected:
		//输时,筹码目标位置

	Node *RootNode;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
	double _timeBetEnd;
	void funcTimeLeft(float d);
		//历史牌面数据
	bool isForceLeave;
	
	EditBox *_editBoxMsg;
	Text *lbTime, *leaveTip; //系统时间
	TextBMFont *txtCoins;
	Node *NodeGameHideControl;
	Sprite * spAddBg,*chatWarn;
	Text *awardTime;
	Layout *intervalTaskLayout,*infoPanel,*panelChat;
	bool intervalTaskWaiting;
	Button* btnAward,*btnStand, *btInvite, *btnFriend, *btnMail, *btn_add_score;
	bool isBarHide;

	map<int, Player*> dzPlayerMap;
	OperaterPanel * operaterPanel;
	CardPublicPanel * cardPublicPanel;
	
	Player* getPlayer(int user_id);

	virtual void addGuide();

	Player* betingPlayer;
    DzpkRoomType* roomType;
public:
	
	ListView *_listChat;
	void UpdateBalance(Ref *ref);	//更新余额

	void updateRoomInfo(RoundInfo* roundInfo, vector<BetInfo*> &vec, BetBegin* betBegin);
	void updateSitInfos();
	void updateSitInfo(RoomSitInfo* sitInfo);

	void updateBetBegin(BetBegin* betBegin);
	void updateFlopInfo(string info);
	void updateRoundInfo(RoundInfo * info);
	void updateBetInfo(BetInfo* into);
	void updateGameResults(vector<PlayerResult*> &vec);
	void updateGameResult(PlayerResult* playResult);
	void updateChatData(DzChatData* chatData);
	void updatePlayerChip(ChipInfo *chipInfo);
	void setGameStart(bool gameStart=true);

	void finishGame();
	void joinGame();
	void doAward();		//派奖
	void sendChatMsg();
	
	void closeRoom();

	void enableExchange(bool enable);

	virtual void onEnter();
	virtual void onExit();
	virtual void editBoxReturn(EditBox* editBox);
	virtual bool init();
	CREATE_FUNC(SceneDzRoom);
private:
	~SceneDzRoom();
	
	void funcInvite();

	void funcWaitInvite();

	void onCurrentIntervalTaskFinish(Ref *pSender); //累计在线任务完成
	void updateCurrentTime(float dt);				//更新当前时间
	void updateAwardTime(float dt);                //更新领奖倒计时
 										// 是否本人在翻牌
};