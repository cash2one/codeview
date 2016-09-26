#include "LayerNotice.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "MailControl.h"




LayerNotice::LayerNotice():currentBtn(nullptr){

}

bool LayerNotice::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("notice/LayerNotice.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;
	notice_title = _nodeRoot->getChildByName<Text*>("notice_title");
	title_list = _nodeRoot->getChildByName<ListView*>("title_list");
	content_scrollView = _nodeRoot->getChildByName<ScrollView*>("content_scrollView");
	
	if (!notice_title || !title_list || !content_scrollView) return false;
	
	title_list->setItemsMargin(60);

	Layout* contentLayout = content_scrollView->getInnerContainer();
	notice_content = Text::create("", "font/msyhbd.ttf", 32);
	contentLayout->addChild(notice_content);
	notice_content->setAnchorPoint(ccp(0, 1));
	

	Layout *touch_layer;
	if (!(touch_layer = _nodeRoot->getChildByName<Layout*>("touch_layer")))
		return false;

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
			this->scheduleOnce([=](float dt){
			

			this->removeFromParent();
		
		}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
		return true;
	};
	//
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	updateNoticeList();
	return true;
}


void LayerNotice::updateNoticeList()
{

	vector<NoticeData*> list = MailControl::GetInstance()->getNoticeList();

	for (int i = 0, length = list.size(); i < length; i++){  //数据已顺序排列
		Layout *item = (Layout *)title_list->getItem(i);// list item 重复利用
		if (item == nullptr)
		{
			item = new Layout();// list item 重复利用
			
			Button* btn = Button::create("notice/unselect.png","notice/select.png","notice/select.png");
			item->addChild(btn);
			btn->setName("title_btn");
			btn->setTitleFontName("font/msyhbd.ttf");
			btn->setTitleFontSize(48);

			item->setContentSize(btn->getContentSize());
			title_list->pushBackCustomItem(item);//少了加
			btn->setAnchorPoint(ccp(0, 0));
			
			btn->addClickEventListener([=](Ref* sender){
				selectBtn((Button*)sender);
			});
		}
		Button* title_btn = item->getChildByName<Button*>("title_btn");
		NoticeData *noticeData = list[i];
		title_btn->setUserData(noticeData);
		title_btn->setTitleText(noticeData->title);

		if (i == 0){
			selectBtn(title_btn);
		}

	}

	int mi = title_list->getChildrenCount() - list.size();// 多出item 数目
	while (mi-- > 0)
	{
		title_list->removeLastItem();

	}

	title_list->refreshView();//记得刷新，更新innerContainer size
	title_list->jumpToTop();
}

void LayerNotice::selectBtn(Button* itemBtn)
{
	if (currentBtn != nullptr){
		currentBtn->setEnabled(true);
		currentBtn->setBright(true);
	}
	currentBtn = itemBtn;
	currentBtn->setEnabled(false);
	currentBtn->setBright(false);
	
	NoticeData *noticeData = (NoticeData *)itemBtn->getUserData();
	notice_title->setString(noticeData->title);

	notice_content->setString(noticeData->content);


	Size textSize = notice_content->getContentSize();

	Size scrollSize = content_scrollView->getSize();
	
	Size setSize = CCSize();
	setSize.width = scrollSize.width;
	setSize.height = scrollSize.height > textSize.height ? scrollSize.height : textSize.height;

	content_scrollView->setInnerContainerSize(setSize);
	notice_content->setPositionY(setSize.height);

	content_scrollView->jumpToTop();
	fromSet = false;
	//显示公告内容
}

LayerNotice::~LayerNotice()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);

	 if (!fromSet)
	 {
		PKNotificationCenter::getInstance()->postNotification("TriggerRequestTask", nullptr);
	 }
}
