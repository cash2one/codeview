#include "LayerRank.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "RankControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "LayerDuang.h"
using namespace Rank;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "ProxyInterface.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#else		 
// do nothings
#endif


const Color4B RankItem::NAME_COLORS[3] = { ccc4(179, 249, 0, 255), ccc4(24, 255, 255, 255), ccc4(20, 230, 255, 255) };
RankItem::RankItem () 
{
	
	Node *_nodeRoot = CSLoader::createNode("RankItem.csb");
	addChild(_nodeRoot);
	_itemBg = (Sprite*)_nodeRoot->getChildByName("item_bg");
	_award = (Sprite*)_nodeRoot->getChildByName("award");
	_avatar = (Sprite*)_nodeRoot->getChildByName("spAvatar");
	_icon = (Sprite*)_nodeRoot->getChildByName("icon");
	_name = (Text*)_nodeRoot->getChildByName("name");
	_vipLevel = (Text*)_nodeRoot->getChildByName("vip_level");
	_fortune = (Text*)_nodeRoot->getChildByName("fortune");
	_rank = (TextBMFont*)_nodeRoot->getChildByName("rank");
}



void RankItem::setProperty(RankData *rankData, int iRank)
{
	if (iRank > 3)
	{
		_award->setVisible(false);
		_rank->setVisible(true);
		_rank->setString(toString(iRank));
	}
	else
	{
		//_itemBg->setTexture(TextureCache::getInstance()->addImage(StringUtils::format("rank/itemBg_%d.png",iRank)));
		_award->setTexture(TextureCache::getInstance()->addImage(StringUtils::format("common/award_%d.png", iRank)));
		_award->setVisible(true);
		_rank->setVisible(false);

	}
	_avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(rankData->avatar, 0));
	_name->setText(rankData->nickname);//nickname or username?
	_name->setTextColor(iRank > 3 ? ccc4(255, 255, 255, 255):NAME_COLORS[iRank-1]);
	_vipLevel->setText(StringUtils::format("VIP%d", rankData->vip_level));
	if (rankData->profit != DBL_MIN){
		_icon->setTexture(TextureCache::getInstance()->addImage("common/earn.png"));
		_fortune->setText(toString(rankData->profit));//显示盈利
	}else if(rankData->winRate != -1){
		_icon->setTexture(TextureCache::getInstance()->addImage("common/rate.png"));
		_fortune->setText(StringUtils::format("%d%%",rankData->winRate));//显示胜率,2个%显示%
	}
	else{
		_icon->setTexture(TextureCache::getInstance()->addImage("common/money_bag.png"));
		_fortune->setText(Comm::GetShortStringFromInt64(rankData->balance));//很大
	}
	
}

RankItem::~RankItem()
{

}


LayerRank::LayerRank():_selectItem(nullptr),_selectType(nullptr),_selectDate(nullptr){

}

bool LayerRank::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("LayerRank.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;
	 _btExit = (Button*)_nodeRoot->getChildByName("Button_Exit");
	 _btExit->setPressedActionEnabled(true);
	 _btExit->addClickEventListener(CC_CALLBACK_1(LayerRank::onButtonExitClicked, this));
	 
	 rankList = (ListView*)_nodeRoot->getChildByName("rank_list");
	 rankList->setBackGroundColorOpacity(0);
	 rankList->setItemsMargin(135);
	 
	 _rankInfo = (Text*)_nodeRoot->getChildByName("rank_info");

	 _spAvatar = (Sprite*)_nodeRoot->getChildByName("spAvatar");
	 _txtNickname = (Text*)_nodeRoot->getChildByName("txtNickname");
	
	 _spAvatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(UserControl::GetInstance()->GetUserData()->avatar, 0));
	 _txtNickname->setText(UserControl::GetInstance()->GetUserData()->nickname);

	 Button* btnFortune = (Button*)_nodeRoot->getChildByName("btn_fortune");
	 btnFortune->setTitleText(Language::getStringByKey("Fortune"));
	 btnFortune->setUserObject(String::create(toString(1)));//传递信息
	 btnFortune->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectType(btnFortune);
	 });

	 Button* btnProfit = (Button*)_nodeRoot->getChildByName("btn_profit");
	 btnProfit->setTitleText(Language::getStringByKey("Profit"));
	 btnProfit->setUserObject(String::create(toString(2)));//传递信息
	 btnProfit->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectType(btnProfit);
	 });


	 Button* btnWinRate = (Button*)_nodeRoot->getChildByName("btn_winRate");
	 btnWinRate->setTitleText(Language::getStringByKey("WinRate"));
	 btnWinRate->setUserObject(String::create(toString(3)));//传递信息
	 btnWinRate->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectType(btnWinRate);
	 });

	 Button* btnYear = (Button*)_nodeRoot->getChildByName("btn_year");
	/* Text* btnYearText = (Text*)btnYear->getChildByName("btn_text");
	 btnYearText->setText(Language::getStringByKey("YearRank"));*/
	 btnYear->setUserObject(String::create(toString(4)));//传递信息
	 btnYear->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectDate(btnYear);
	 });

	 Button* btnMonth = (Button*)_nodeRoot->getChildByName("btn_month");
	 /*Text* btnMonthText = (Text*)btnMonth->getChildByName("btn_text");
	 btnMonthText->setText(Language::getStringByKey("MonthRank"));*/
	 btnMonth->setUserObject(String::create(toString(3)));//传递信息
	 btnMonth->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectDate(btnMonth);
	 });

	 Button* btnWeek = (Button*)_nodeRoot->getChildByName("btn_week");
	/* Text* btnWeekText = (Text*)btnWeek->getChildByName("btn_text");
	 btnWeekText->setText(Language::getStringByKey("WeekRank"));*/
	 btnWeek->setUserObject(String::create(toString(2)));//传递信息
	 btnWeek->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectDate(btnWeek);
	 });

	 Button* btnDay = (Button*)_nodeRoot->getChildByName("btn_day");
	/* Text* btnDayText = (Text*)btnDay->getChildByName("btn_text");
	 btnDayText->setText(Language::getStringByKey("DayRank"));*/
	 btnDay->setUserObject(String::create(toString(1)));//传递信息
	 btnDay->addClickEventListener([=](Ref *sender){
		 SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		 selectDate(btnDay);
	 });

	
	 RankControl::GetInstance()->setRankView(this); //添加视图

	 selectType(btnFortune,false); //default type
	 selectDate(btnWeek,false); //default date

	 PKNotificationCenter::getInstance()->postNotification("RankSwitch", String::create(getRankKey())); //第一次 默认切换

	 
	 return true;
}



void LayerRank::selectType(Button * btn,bool sendRq)
{   
	

	if (btn == _selectType) { return; } // 已禁用
	if (_selectType)
	{
		_selectType->setBright(true);
		_selectType->setEnabled(true);
		_selectType = btn;
		
	}
	_selectType = btn;
	_selectType->setEnabled(false);
	_selectType->setBright(false);
	if (sendRq)
	{
		PKNotificationCenter::getInstance()->postNotification("RankSwitch",String::create(getRankKey()));//当_selectType 存在,主动点击通知控制层数据刷新
	}
}

void LayerRank::selectDate(Button * btn,bool sendRq)
{
	if (btn == _selectDate){ return; }
	if (_selectDate)
	{
		_selectDate->setBright(true);
		_selectDate->setEnabled(true);
		_selectDate->getChildByName("label_normal")->setVisible(true);
		_selectDate->getChildByName("label_select")->setVisible(false);
	}
	_selectDate = btn;
	_selectDate->setEnabled(false);
	_selectDate->setBright(false);
	_selectDate->getChildByName("label_normal")->setVisible(false);
	_selectDate->getChildByName("label_select")->setVisible(true);
	if (sendRq)
	{
		PKNotificationCenter::getInstance()->postNotification("RankSwitch", String::create(getRankKey()));//当_selectType 存在,主动点击通知控制层数据刷新
	}
}

string LayerRank::getRankKey()
{
	if (!_selectType||!_selectDate){
		return "";
	}
	String* select =(String*)_selectType->getUserObject();
	String* date = (String*)_selectDate->getUserObject();
	return StringUtils::format("%s_%s", select->getCString(), date->getCString());
}

void LayerRank::UpdateRankList(vector<RankData*> *list)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	int myRank = 0;
	for (int i = 0, length = (*list).size(); i < length; i++){  //数据已顺序排列
		RankItem *item = (RankItem *)rankList->getItem(i);// list item 重复利用
		if (item == nullptr){
			item = new RankItem();
			rankList->pushBackCustomItem(item);//少了加
		}

		RankData *rankData = (*list)[i];
		item->setProperty(rankData, i + 1);
		if (rankData->user_id == user_id){
			myRank = i + 1;
		}
	}
	int mi = rankList->getChildrenCount() - (*list).size()  ;// 多出item 数目
	while (mi-- > 0)
	{
		rankList->removeLastItem();

	}

	if (myRank == 0) //对于自己的排名 数据中有就显示，没有则隐藏
	{
		//_rankInfo->setVisible(false);
		_rankInfo->setText(Language::getStringByKey("NoRank"));
	}
	else
	{
		//_rankInfo->setVisible(true);
		_rankInfo->setText(toString(myRank));
	}

	rankList->refreshView();//记得刷新，更新innerContainer size
	rankList->jumpToTop();

 }




LayerRank::~LayerRank()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
	 RankControl::GetInstance()->setRankView(nullptr); //释放视图
}


void LayerRank::onButtonExitClicked(Ref *ref){
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
	CloseWithAction(this);
	
	LayerDuang::panelStatus();
	LayerDuang::uiGameMenu();
	LayerDuang::dzGamesRoom();
	LayerDuang::bjlGamesRoom();
	LayerDuang::bjlPanelLobby();
	LayerDuang::dzPanelLobby();
}
