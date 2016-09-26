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
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UITextField.h"
#include "ui/UICheckBox.h"
#include "ui/UISlider.h"
#include "FriendControl.h"
#include "SceneDzRoom.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;



class LayerInviteDzRoom :public Layer
{
	// ----------------自定义	
protected:	
	Node *_nodeRoot;
	Button *btn_confirm, *btn_royal, *btn_normal;
	
	Slider* take_slider;

	Text* text_blind, *text_take;
	short currentType;
	short currentIndex;

	void selectType(short type);
	void selectIndex(short index);

	void SliderEvent(Ref *pSender, Slider::EventType type);

	void createRoom();

	void onCreateRoomSucc(Ref* pSender);

	DzpkRoomType* select_roomType;

	void clickSelect(int index);

public:
	LayerInviteDzRoom();
	~LayerInviteDzRoom();
	
	CREATE_FUNC(LayerInviteDzRoom);
	void onButtonExitClicked(Ref *ref);
	
	// ----------------自定义结束
	virtual bool init();
};
