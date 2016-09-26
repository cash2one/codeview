#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UISlider.h"
#include "cocostudio/CocoStudio.h"
#include "bjlcomm.h"
#include "ReportControl.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerGameRecordBjl : public Layer
{
protected:
	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch* touch, Event* event);

	void DelayExit(float dt);
private:
	Node *RootNode;
	Text *txtID, *txtType, *txtTime, *txtPlayerBet, *txtTieBet, *txtBankerBet, *txtPlayerPairBet, *txtBankerPairBet,
		*txtBetAmountNum, *txtProfitNum;
	Layout *spBg;
	Sprite *SpCard0, *SpCard1, *SpCard2, *SpCard3, *SpCard4, *SpCard5;
public:
	void ShowGameRound(Report::GameDetailRecord *record, GameRoundDataBjl *data);
	virtual bool init();
	CREATE_FUNC(LayerGameRecordBjl);
};