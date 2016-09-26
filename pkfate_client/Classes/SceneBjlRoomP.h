#pragma once
#include "cocos2d.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UIButton.h"
#include "ui/UICheckBox.h"
#include "ui/UITextBMFont.h"
#include "comm.h"
#include "bjlcomm.h"
#include "SpriteCmSub.h"
#include "LayerBjlResult.h"
#include "LayerBjlLz.h"
#include "LayerBjlFp.h"
#include "IrregularTest.h"
#include "GameControlBjl.h"
#include "UserControl.h"


class SceneBjlRoomP :public Scene
{
protected:
	int _maxBet;
	UserData *_userData;
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
	LayerBjlFpS *_layerBjlFpS;				//翻牌control
	float _scaleCard0, _scaleCard1;	//ÅÆÃæËõ·Å±¶Êý,³õÊ¼Óë×îÖÕ±¶Êý
	float _widthCard;	//ÅÆÃæ¿í¶È
	Sprite* spriteCardB;		//ÅÆ±³Ãæ
	Node *RootNode;
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
	void funcTimeLeft(float d);
	CheckBox *_checkFP;
	Text *lbRoomId, *TextTimeLeft;
	TextBMFont *TextPtP, *TextPtB;
	Layout *panelCards;
	bool onCmClick(Touch* touch, Event* event);
	bool onCmBet(Touch* touch, Event* event);
	float _timeFP = 1.0f;
	STATUSBJLGAME _statusGame;
	Button *_btBet, *_btCancel;				//确定,取消按钮
	void onButtonBetClicked(Ref* ref);
	void onButtonCancelClicked(Ref* ref);
	TextBMFont *_textUserAmount;
	double _userAmount;
	std::vector<int> _lzDatas;		//路子历史数据
	string _cardDatas;				//历史牌面数据
	void UpdateBalance(Ref *ref);	//更新余额
	void OnBetCallBack(Ref *pSender);
	Text *lbT, *lbB, *lbP, *lbPP, *lbBP, *lbC;
	BjlTableControl *_tableControl;
	Node *NodeGameHideControl;
	bool isBarHide;
	Text *lbTime; //系统时间

	Text *awardTime, *lbMyChip;
	Layout *intervalTaskLayout;
	bool intervalTaskWaiting;
	Button* btnAward;

public:
	BjlRoomData *_roomData;
	BetResultData *_betResultData;	//投注返回结果
	void funcFP(const char* cards);
	void ShowLz(Ref* ref);		//显示路子详细信息
	void addRoomLeaveEvent(ccFuncEventCustom);
	
	CMINFO * getCurrentCMInfo();//获取当前cmInfo,获取不到nullptr
	bool isAreaCanBet(SpriteBetArea *betArea, CMINFO *cmInfo);//能够投注
	void checkAndSetAreaBetStatus(SpriteBetArea *betArea, CMINFO *cmInfo);//检测设置投注状态
	void checkAndSetAllAreaBetStatus();//
	virtual void onEnter();
	int TimeLeftMax, TimeLeft, TimeForShowResult;
	void actFp(Node* node, BJLFPINDEX n);
	void actFpDone();
	void doAward();		//派奖
public:
	void SetRoomData(BjlRoomData *data);
	bool init(int maxBet);
	static SceneBjlRoomP* create(int maxBet);
private:
	~SceneBjlRoomP();
	void updateCurrentTime(float dt);					   //更新当前时间
	void onCurrentIntervalTaskFinish(Ref *pSender); //累计在线任务完成
};