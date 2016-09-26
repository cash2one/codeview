#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIScrollView.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "GameControlBjl.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

class LayerBjlCalculator :public Layer
{
	// ----------------自定义	
protected:
	Node *RootNode;		//根节点,容器节点
	Layout *panelCalculator;
	Button *btRound, *btAll, *btCopySeed, *btCopySecret, *btCopyNextSecret;
	Text *lbData, *lbSeed, *lbSecret, *lbNextSecret,*title,*thisSecretText;
	ScrollView *scrollAllCards;
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	BjlRoomData *_roomData;
	string _cardDatas;
	string _seed;
	string _data;
	void btCopySeedClicked(Ref *ref);
public:
	void ShowAll();
	void ShowRound();
	CREATE_FUNC(LayerBjlCalculator);
	virtual bool init();
	void SetData(BjlRoomData *data);
};
