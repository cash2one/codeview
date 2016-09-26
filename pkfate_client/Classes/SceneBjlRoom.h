#pragma once
#include "cocos2d.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UIButton.h"
#include "ui/UICheckBox.h"
#include "ui/UITextBMFont.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "comm.h"
#include "bjlcomm.h"
#include "SpriteCmSub.h"
#include "LayerBjlResult.h"
#include "LayerBjlLz.h"
#include "IrregularTest.h"
#include "GameControlBjl.h"
#include "UserControl.h"
#include "LayerBjlFp.h"
#include "LayerGuideMask.h"

class SceneBjlRoom :public Scene, public EditBoxDelegate,protected GuideHandler
{
protected:
	vector<CMINFO*> _cmInfos;
	IrregularTest _irregularTest;
	// ----------------×Ô¶¨Òå
	BjlPmInfos _pmInfos;		//ÅÆÃæÐÅÏ¢
	SpriteCms _spriteCms;		//³ïÂë¾«Áé×é
	SpriteBetAreas _spriteBetAreas;		//Í¶×¢ÇøÓò×é
	
	
	Layer *layerCm;		//³ïÂë²ã
	Vec2 _ptCardInit, _ptCardP, _ptCardB;	//·¢ÅÆ³õÊ¼Î»ÖÃ
	Vec2 _ptCmLose;		//输时,筹码目标位置
	Layer *layerCard, *layerCardP, *layerCardB;	//×¯ÏÐ·¢ÅÆÎ»ÖÃ
	LayerBjlResult *_layerResult;	//奖金结算layer	
	BjlLzSm *_bjlLzSm;				//路子缩略图控制
	LayerBjlLz *_layerBjlLz;				//路图控制
	LayerBjlFpM *_layerBjlFpM;				//翻牌layer
	float _scaleCard0, _scaleCard1;	//ÅÆÃæËõ·Å±¶Êý,³õÊ¼Óë×îÖÕ±¶Êý
	float _widthCard;	//ÅÆÃæ¿í¶È
	Sprite* spriteCardB;		//ÅÆ±³Ãæ
	Node *RootNode;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
	double _timeBetEnd;
	void funcTimeLeft(float d);
	CheckBox *_checkFP;
	Layout *panelCards;
	bool onCmClick(Touch* touch, Event* event);
	bool onCmBet(Touch* touch, Event* event);
	float _timeFP = 1.0f;
	TextBMFont *_textUserAmount;
	int64_t _userAmount;
	std::vector<int> _lzDatas;		//路子历史数据
	string _cardDatas;				//历史牌面数据
	void UpdateBalance(Ref *ref);	//更新余额
	Text *lbT, *lbB, *lbP, *lbPP, *lbBP, *lbC;
	Text *lbAmountPAll, *lbAmountPPAll, *lbAmountBAll, *lbAmountBPAll, *lbAmountTAll;
    Sprite *imgBgAmountPAll,*imgBgAmountPPAll,*imgBgAmountBAll,*imgBgAmountBPAll,*imgBgAmountTAll;
	EditBox *_editBoxMsg;
	Text *lbTime  ; //系统时间
	Node *NodeGameHideControl;
	Layout* panelCmArea;
	Text *awardTime,*lbMyChip;
	Layout *intervalTaskLayout;
	bool intervalTaskWaiting;
	Button* btnAward;
	bool isBarHide;

	virtual void addGuide();
	int64_t _curBet = 0;
	float _synBetTime = 2.0f;
public:
	UserData *_userData;
	int _seatId;
	BJLBETINFO _myBetInfoNoSeat;		//本人无座时的投注信息
	BjlCmControl *_playerCmControl;
	BjlTableControl *_tableControl;
	BjlSitChatControl *_sitChatControl;
	ListView *_listChat;
	Text *lbRoomId;
	TextBMFont *TextPtP, *TextPtB, *TextTimeLeft;
	STATUSBJLGAME _statusGame;
	map<int, BjlRoomPlayer*> _roomPlayers;
	RoomInfoBjl *_roomInfo;
	BjlRoomData *_roomData;
	void funcFP(const char* cards, bool flip);
	void ShowLz(Ref* ref);		//显示路子详细信息
	int TimeLeftMax, TimeForShowResult;
	void actFp(Node* node, BJLFPINDEX n);
	void actFpDone();
	void doAward();		//派奖
	void setTotalBetInfo(BJLBETINFO *betInfo);
	void sendChatMsg();
	CMINFO * getCurrentCMInfo();//获取当前cmInfo,获取不到nullptr
	bool isAreaCanBet(SpriteBetArea *betArea, CMINFO *cmInfo);//能够投注
	void checkAndSetAreaBetStatus(SpriteBetArea *betArea, CMINFO *cmInfo);//检测设置投注状态
	void checkAndSetAllAreaBetStatus();//
	void SetRoomData(BjlRoomData *data);
	virtual void onEnter();
	virtual void onExit();
	virtual void editBoxReturn(EditBox* editBox);
	bool init(RoomInfoBjl *roomInfo);
	static SceneBjlRoom* create(RoomInfoBjl *roomInfo);

	Node* getRootNode();
	
private:
	~SceneBjlRoom();
	void onCurrentIntervalTaskFinish(Ref *pSender); //累计在线任务完成

	void onEnterRoomCallBack(Ref *pSender);			//进入房间回调函数
	void onHistoryDataCallBack(Ref *pSender);		//历史数据回调函数
	void onBeginBetCallBack(Ref *pSender);			//开始投注回调函数
	void onShowCardsCallBack(Ref *pSender);			//牌局结果回调函数
	void onAwardCallBack(Ref *pSender);				//开奖回调函数
	void onRoundOverCallBack(Ref *pSender);			//牌局结束回调函数
	void onCutCardCallBack(Ref *pSender);			//切牌完成回调函数
	void onNewRoomPlayerCallBack(Ref *pSender);		//玩家进入房间
	void onDelRoomPlayerCallBack(Ref *pSender);		//玩家离开房间
	void onRoomPlayersCallBack(Ref *pSender);		//房间玩家数据
	void onChatCallBack(Ref *pSender);				//聊天数据
	void onSitClicked(Ref *pSender);				//玩家点击座位
	void onSitCallBack(Ref *pSender);				//座位信息变化
	void onRecvRoomBetCallBack(Ref *pSender);		//收到房间其他玩家投注信息
	void onRecvRoomBetTotalCallBack(Ref *pSender);	//收到房间玩家总投注信息
	void onRecvRoomBetProfitCallBack(Ref *pSender);	//收到房间玩家盈亏消息，此消息表示开始派奖,pos:0为本人，1－8为有座玩家
	void onRecvRoomAwardCallBack(Ref *pSender);		//收到房间牌局输赢结果消息
	void onRecvBetCallBack(Ref *pSender);			//收到投注返回消息
	void updateCurrentTime(float dt);				//更新当前时间
	void updateAwardTime(float dt);                //更新领奖倒计时
 	void onRecvFpCallBack(Ref *pSender);				// 收到翻牌消息,所有翻牌消息，都由此函数处理
	void onRecvFpBegin(int playerid, int bankerid);		// 开始翻牌消息
	void onRecvFpStatus(int status, double endTime);	// 翻牌状态切换,[0:闲开牌, 1:庄开牌, 2:闲补牌, 3:庄补牌], time:结束时间
	void onRecvFpSelect(int card);						// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
	void onFpSelect(int card);
	void onRecvFping(int x1, int y1, int x2, int y2);	// 翻牌中
	void onFping(int x1, int y1, int x2, int y2);
	void onRecvFpOpen(int card);						// 开牌
	void onFpOpen(int card);
	void onFpEnd();										// 结束翻牌
	void onRecvFpInit(int playerid, int bankerid, int status, double endTime, vector<int>cardStatus);	// 中途进入房间时，翻牌初始化信息
	bool _fpEnable;										// 是否本人在翻牌
};