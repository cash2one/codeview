#pragma once

#include "cocos2d.h"
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include "ui/UILayout.h"
#include "ui/UIText.h"
#include "DZControl.h"
#include "ui/UIScrollView.h"	
#include "LayerGuideMask.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;

class SNGRoomItem :public Widget
{
private:
	Sprite* master_icon, *sng_label;
	Node *rootNode;
	Button *btn_room_item;
	Text *condition_text, *start_time, *start_count, *start_num, *room_num, *room_status;
	void btnClickHandler(Ref * sender);
	SNGRoomStatus* sNGRoomStatus;
	void enterRoom();
public:
	void setProperty(SNGRoomStatus* sNGRoomStatus);
	CREATE_FUNC(SNGRoomItem);

	SNGRoomItem();
	~SNGRoomItem();
};


enum DZROOMTYPE{ DSmall = 1, DMiddle, DBig, DNew };

class LayerDzGames : public Layer ,protected GuideHandler
{
protected:
	Node *RootNode;
	Button *btBack, *btRoomS, *btRoomM, *btRoomB, *btNew, *royal, *normal,*master,*elite;
	Layout *panelGames, *panelLobby, *sng_room_panel;
	Text* royal_big, *royal_small, *normal_big, *normal_small, *coin_num, *gold_num, *sliver_num, *diamond_num, *txtVipLevel;

	ScrollView * room_list;

	ListView *sng_room_list;

	void enterDzpkRoom(Ref* pSender);

	void closeSnsRooms(Ref* pSender);
	virtual void addGuide();
public:

	void openMall();

	void enableRoom(bool enable);

	void UpdateBalance(Ref *ref);

	DZROOMTYPE _roomType;
	void selectEnterRoom(DzpkRoomType* roomType);
	
	void typeSelectHandler(Ref* sender);

	void selectSngType(int type);


	void updateSngRoom(SNGRoomStatus* sNGRoomStatus);
	void updateSngRooms(map<int, SNGRoomStatus*> & map);
	void updateRoomsTypes();
	void showRooms(DZROOMTYPE type);


	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
//	bool onTouchBegan(Touch *touch, Event *event);

	LayerDzGames();
	~LayerDzGames();
	CREATE_FUNC(LayerDzGames);

	Node* getRoyal();
	Node* getNormal();

	Node* getMaster();
	Node* getElite();

	Layout* getPanelLobby();
private:


};

