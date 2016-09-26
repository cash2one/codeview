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
#include "RankControl.h"


USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

class RankItem :public Widget
{
	// ----------------自定义	
protected:
	TextBMFont *_rank;
	const static  Color4B  NAME_COLORS[3];

	//Node *_selectItem;

	Sprite * _avatar, *_award ,*_itemBg,*_icon;

	Text *_name, *_vipLevel, *_fortune;
public:
	RankItem();
	~RankItem();
	CREATE_FUNC(RankItem);
	void setProperty(Rank::RankData *rankData, int iRank);

	
};


class LayerRank :public Layer
{
	// ----------------自定义	
protected:	
	Button *_btExit;

	Button* _selectType;//选择的类型

	Button* _selectDate;//选择的日期

	TextBMFont *_txtCoins;

	Node *_selectItem;

	Sprite *_spAvatar;

	Text *_txtNickname, *_rankInfo;

	ListView * rankList;



	//Text *_textResultWin, *_textResultLose, *_textResultTotal;
public:
	LayerRank();
	~LayerRank();
	CREATE_FUNC(LayerRank);
	void UpdateRankList(vector<Rank::RankData*> *list);//更新rankList
	void onButtonExitClicked(Ref *ref);
	void selectType(Button * btn,bool sendRq=true);
	void selectDate(Button * btn,bool sendRq=true);
	

	string getRankKey();

	// ----------------自定义结束
	virtual bool init();
};
