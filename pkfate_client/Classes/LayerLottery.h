#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UILayout.h"
#include "ui/UITextBMFont.h"
#include "comm.h"
#include "cJSON.h"
using namespace std;
USING_NS_CC;
using namespace cocos2d::ui;

class LotteryItem :public Node
{
private:
	Text* _coinText;
	unsigned int coin;
public:
	unsigned int getCoin();
	LotteryItem();
	~LotteryItem();
	CREATE_FUNC(LotteryItem);

	void setProperty(unsigned int coin, int coinTag);
	// ----------------自定义结束
};

class LayerLottery :public Layer
{
	// ----------------自定义	
protected:	
	unsigned int order ; // 当前序号
	unsigned int add ;//跳动
	unsigned int step ;//步
	unsigned int addIndex;//缓动开始增序号
	unsigned int minus;//缓冲大小
	unsigned int minusStep;//缓冲步长
	
	bool _enableQuit;
	Node*  _nodeRoot,*_itemLayer;
	Button *_btnStart;

	Sprite *_light, *_startMask, *_lotteryMask;//发光
	
	


	void funcLottery();
	void onLotteryCallBack(Ref *pSender);
	void lotteryAction(unsigned short index);
	

	void lotterySchedule(float dt);

	int getIndexOfLottery(unsigned int bonus);

	void lotteryFinish(unsigned short index);

	bool onTouchBegan(Touch *touch, Event *event);

	//Text *_textResultWin, *_textResultLose, *_textResultTotal;
public:
	void lottery(unsigned int bonus);
	void lotteryError();
	~LayerLottery();
	LayerLottery();
	CREATE_FUNC(LayerLottery);
	
	void onButtonExitClicked(Ref *ref);
	

	// ----------------自定义结束
	virtual bool init();
};