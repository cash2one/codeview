#include "LayerBjlNew.h"
#include "cocostudio/CocoStudio.h"
#include "ApiGame.h"
#include "LayerLoading.h"
#include "comm.h"
#include "SceneBjlRoomP.h"
#include "GameControlBjl.h"
#include "UserControl.h"
#include "PKNotificationCenter.h"

#include "cmd.h"

// on "init" you need to initialize your instance
bool LayerBjlNew::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init())
    {
        return false;
    }
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    _currentButton = nullptr;
    _roomTypeId = _maxBet = 0;
    /*	载入cocos studio 资源
     */
    //-------初始化创建房间控件
    RootNode = CSLoader::createNode("bjl/LayerBjlNew.csb");
    if (!RootNode)
        return false;
    if (!(panelNew = RootNode->getChildByName<Layout*>("panelNew")))
        return false;
    if (!(btCreateSingle = panelNew->getChildByName<Button*>("btCreateSingle")))
        return false;
    auto lbSingleRoom=btCreateSingle->getChildByName<Text*>("lbSingleRoom");
    if(!lbSingleRoom)return false;
    lbSingleRoom->setString(Language::getStringByKey("SinglePlayer"));
    
    if (!(btCreateMulti = panelNew->getChildByName<Button*>("btCreateMulti")))
        return false;
    auto lbMultiRoom=btCreateMulti->getChildByName<Text*>("lbMultiRoom");
    if(!lbMultiRoom)return false;
    lbMultiRoom->setString(Language::getStringByKey("MultiplePlayer"));
    
    if (!(btCreateSmall = panelNew->getChildByName<Button*>("btCreateSmall")))
        return false;
    auto lbSmall=btCreateSmall->getChildByName<Text*>("lbSmall");
    if(!lbSmall)return false;
    lbSmall->setString(Language::getStringByKey("RoomPrimary"));
    
    if (!(btCreateMiddle = panelNew->getChildByName<Button*>("btCreateMiddle")))
        return false;
    auto lbMiddle=btCreateMiddle->getChildByName<Text*>("lbMiddle");
    if(!lbMiddle)return false;
    lbMiddle->setString(Language::getStringByKey("RoomMiddle"));
    
    if (!(btCreateBig = panelNew->getChildByName<Button*>("btCreateBig")))
        return false;
    auto lbBig=btCreateBig->getChildByName<Text*>("lbBig");
    if(!lbBig)return false;
    lbBig->setString(Language::getStringByKey("RoomHigh"));
    
    if (!(btCreateConfirm = panelNew->getChildByName<Button*>("btCreateConfirm")))
        return false;
	btCreateConfirm->setTitleText(Language::getStringByKey("Confirm"));
    if (!(chkNew = panelNew->getChildByName<CheckBox*>("chkNew")))
        return false;
    auto lbNew=panelNew->getChildByName<Text*>("lbNew");
    if(!lbNew)return false;
    lbNew->setString(Language::getStringByKey("Restart"));
    
    if (!(lbCreateMin = panelNew->getChildByName<Text*>("lbCreateMin")))
        return false;
    if (!(lbCreateMax = panelNew->getChildByName<Text*>("lbCreateMax")))
        return false;
  /*  
    auto lbTitle=panelNew->getChildByName<Text*>("lbTitle");
    if(!lbTitle)return false;
    
    lbTitle->setString(Language::getStringByKey("CreateRoom"));*/
    
    auto lbType=panelNew->getChildByName<Text*>("lbType");
    if(!lbType)return false;
    lbType->setString(Language::getStringByKey("Type"));
    
    auto lbLevel=panelNew->getChildByName<Text*>("lbLevel");
    if(!lbLevel)return false;
    lbLevel->setString(Language::getStringByKey("Level"));
    
    auto lbRange=panelNew->getChildByName<Text*>("lbRange");
    if(!lbRange)return false;
    lbRange->setString(Language::getStringByKey("BetRange"));
    
    btCreateSmall->setHighlighted(true);
    btCreateMiddle->setHighlighted(true);
    btCreateBig->setHighlighted(true);
    btCreateMulti->setEnabled(false);
    btCreateMulti->setBright(false);
    onTypeButtonClick(btCreateSmall);
    // add event
    btCreateSmall->addClickEventListener(CC_CALLBACK_1(LayerBjlNew::onTypeButtonClick, this));
    btCreateMiddle->addClickEventListener(CC_CALLBACK_1(LayerBjlNew::onTypeButtonClick, this));
    btCreateBig->addClickEventListener(CC_CALLBACK_1(LayerBjlNew::onTypeButtonClick, this));
    btCreateConfirm->addClickEventListener(CC_CALLBACK_1(LayerBjlNew::createRoom, this));
	btCreateConfirm->setPressedActionEnabled(true);
    this->addChild(RootNode);
    PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlNew::on_sg_baccarat_begin_game), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_BEGIN_GAME), NULL);
  
	
	Layout *touch_layer;
	if (!(touch_layer = RootNode->getChildByName<Layout*>("touch_layer")))
		return false;

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
			this->scheduleOnce([=](float dt){
			this->removeFromParent();
			PKNotificationCenter::getInstance()->postNotification("triggerShowRoom");
		
		}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
		return true;
	};

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	
	
	return true;
}
LayerBjlNew::~LayerBjlNew()
{
    //注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
void LayerBjlNew::on_sg_baccarat_begin_game(Ref *pSender)
{
    //get user info结果回调函数
    if (pSender == nullptr)
        return;
    RECVMSG* msg = (RECVMSG*)pSender;
    msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
    //使用主线程调用
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
        if (msg->code == 0)
        {
            //成功
           /* auto layerLoading = Director::getInstance()->getRunningScene()->getChildByName<LayerLoading*>("LayerLoading");
            if (layerLoading)
                layerLoading->removeFromParent();*/

			LayerLoading::Close();

            // init roomdata
            BjlRoomData *data = GameControlBjl::GetInstance()->InitRoomData(msg->data);
            if (data)
            {
                //成功
                //Api::Game::mo_baccarat_disconnect();	//断开mo游戏链接
				
                auto scene = SceneBjlRoomP::create(_maxBet);
                scene->SetRoomData(data);
                Director::getInstance()->pushScene(scene);
				this->removeFromParent();
            }
            else
            {
                //更新数据失败
                CCLOG("[%s]:%s\t%s", "on_sg_baccarat_begin_game", "init room data error.", msg->data.c_str());
            }
        }
        else
        {
            //失败
            CCLOG("[%s]:%s\t(code:%d) - %s", "on_sg_baccarat_begin_game", "room begin error.", msg->code, msg->data.c_str());
        }
        msg->release();		//清除msg数据，以免内存泄漏
    });
}
void LayerBjlNew::onTypeButtonClick(Ref *ref)
{
    Button *bt = (Button*)ref;
    if (bt == _currentButton)
        return;
    if (_currentButton)
        _currentButton->setHighlighted(true);
    _currentButton = bt;
    if (_currentButton == btCreateSmall)
    {
        ShowRoomType(1);
    }
    else if (_currentButton == btCreateMiddle)
    {
        ShowRoomType(2);
    }
    else if (_currentButton == btCreateBig)
    {
        ShowRoomType(3);
    }
    else
        return;
}
void LayerBjlNew::ShowRoomType(int roomTypeId)
{
    if (_roomTypeId == roomTypeId || roomTypeId < 1 || roomTypeId>3
        || GameControlBjl::GetInstance()->RoomTypeInfos.find(roomTypeId) == GameControlBjl::GetInstance()->RoomTypeInfos.end())
        return;
    _roomTypeId = roomTypeId;
    _maxBet = GameControlBjl::GetInstance()->RoomTypeInfos[roomTypeId]->max_bet;
    lbCreateMin->setString(Comm::GetShortStringFromInt64(_maxBet / 100));
    lbCreateMax->setString(Comm::GetShortStringFromInt64(_maxBet));
    _currentButton->setHighlighted(false);
}
void LayerBjlNew::createRoom(Ref *ref)
{
    //LayerLoading *layerLoading = LayerLoading::create();
    //layerLoading->setName("LayerLoading");
    //Director::getInstance()->getRunningScene()->addChild(layerLoading);		//loading需要显示在最上层
   
	LayerLoading::Wait();
	Api::Game::sg_baccarat_begin_game(chkNew->getSelectedState(), _roomTypeId);
}