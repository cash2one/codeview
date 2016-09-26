#pragma once

#include "cocos2d.h"
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include "ui/UILayout.h"
#include "ui/UIText.h"
#include "ui/UICheckBox.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;
class LayerBjlNew : public Layer
{
protected:
	Node *RootNode;
	Layout *panelNew;
	Button *btCreateSingle, *btCreateMulti, *btCreateSmall, *btCreateMiddle, *btCreateBig, *btCreateConfirm;
	Text *lbCreateMin, *lbCreateMax;
	Button *_currentButton;		//当前选定按钮
	void onTypeButtonClick(Ref *ref);
	void on_sg_baccarat_begin_game(Ref *pSender);
	int _roomTypeId, _maxBet;
public:
	void ShowRoomType(int roomTypeId);
	CheckBox *chkNew;
	void createRoom(Ref *ref);
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	// implement the "static create()" method manually
	CREATE_FUNC(LayerBjlNew);
	~LayerBjlNew();
};