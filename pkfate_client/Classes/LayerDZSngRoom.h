#pragma once

#include "cocos2d.h"
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include "ui/UILayout.h"
#include "ui/UIText.h"
#include "LayerBjlLz.h"
#include "LayerBjlNew.h"
#include "LayerGuideMask.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;
struct spRoomInfo
{
	Text *txtCount;
	Text *txtId;
	BjlLzSm *lz;
};
enum BJLROOMTYPE{ Small = 1, Middle, Big, New };
class LayerDZSngRoom : public Layer,protected GuideHandler
{
protected:
	Node *RootNode;
	Button *btBack, *btNew, *btRoomS, *btRoomM, *btRoomB;
	void onConnectCallBack(Ref *pSender);
	void onLoginCallBack(Ref *pSender);
	void onEnterRoomCallBack(Ref *pSender);
	void onMsgRoomTypesInfo(Ref *pSender);
	void onMsgRoomInfo(Ref *pSender);
	void onGetRoomResultsCallBack(Ref *pSender);
	void onRoomUpdate(Ref *pSender);
	void triggerShowRoom(Ref *pSender);

	map<int, spRoomInfo*> spRoomsInfo;
	void onSelectedRoomEvent(Ref *pSender, ListView::EventType type);
	virtual void addGuide();
public:
	BJLROOMTYPE _roomType;
	ListView *listRooms;
	Layout *panelGames, *panelLobby;
	LayerBjlNew *layerBjlNew;
	void showRooms(BJLROOMTYPE type);
	void updateRoomsInfo(int roomId);
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	CREATE_FUNC(LayerDZSngRoom);

	ListView* getListRooms();
	Layout* getPanelLobby();
private:
	~LayerDZSngRoom();
};