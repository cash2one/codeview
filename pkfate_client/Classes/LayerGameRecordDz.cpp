#include "LayerGameRecordDz.h"
#include "comm.h"
#include "SpriteCmSub.h"


bool LayerGameRecordDz::init() {
	if (!Layer::init()) {
		return false;
	}

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("report/LayerGameRecordDz.csb");
	if (!RootNode)
		return false;
	if (!(spBg = RootNode->getChildByName<Layout*>("spBg")))
		return false;
	if (!(txtID = RootNode->getChildByName<Text*>("txtID")))
		return false;
	if (!(txtType = RootNode->getChildByName<Text*>("txtType")))
		return false;
	if (!(txtTime = RootNode->getChildByName<Text*>("txtTime")))
		return false;
	
	if (!(txtBetAmountNum = RootNode->getChildByName<Text*>("txtBetAmountNum")))
		return false;
	if (!(txtProfitNum = RootNode->getChildByName<Text*>("txtProfitNum")))
		return false;
	if (!(SpCard0 = RootNode->getChildByName<Sprite*>("SpCard0")))
		return false;
	if (!(SpCard1 = RootNode->getChildByName<Sprite*>("SpCard1")))
		return false;
	if (!(SpCard2 = RootNode->getChildByName<Sprite*>("SpCard2")))
		return false;
	if (!(SpCard3 = RootNode->getChildByName<Sprite*>("SpCard3")))
		return false;
	if (!(SpCard4 = RootNode->getChildByName<Sprite*>("SpCard4")))
		return false;
	if (!(SpCard5 = RootNode->getChildByName<Sprite*>("SpCard5")))
		return false;
	if (!(SpCard6 = RootNode->getChildByName<Sprite*>("SpCard6")))
		return false;
	if (!(SpCard7 = RootNode->getChildByName<Sprite*>("SpCard7")))
		return false;
	if (!(SpCard8 = RootNode->getChildByName<Sprite*>("SpCard8")))
		return false;
    /*auto lbTitle=RootNode->getChildByName<Text*>("lbTitle");
    if(!lbTitle)return false;
    lbTitle->setString(Language::getStringByKey("GameRecords"));*/
    
    
    
    auto txtBetAmount=RootNode->getChildByName<Text*>("txtBetAmount");
    if(!txtBetAmount)return false;
    txtBetAmount->setString(Language::getStringByKey("BetAmount"));
    
    auto txtProfit=RootNode->getChildByName<Text*>("txtProfit");
    if(!txtProfit)return false;
    txtProfit->setString(Language::getStringByKey("Profit"));

	this->addChild(RootNode);

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerGameRecordDz::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerGameRecordDz::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerGameRecordDz::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}

bool LayerGameRecordDz::onTouchBegan(Touch *touch, Event *event) {
	if (!spBg->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerGameRecordDz::DelayExit), 0.1f);
	return true;
}

void LayerGameRecordDz::onTouchMoved(Touch *touch, Event *event) {

}

void LayerGameRecordDz::onTouchEnded(Touch* touch, Event* event) {

}

void LayerGameRecordDz::DelayExit(float dt)
{
	this->removeFromParent();
}
void LayerGameRecordDz::ShowGameRound(Report::GameDetailRecord *record, GameRoundDataBjl *data){
	if (!record || !data)
		return;
		//下注和局的额度

	char strbuf[256];
	if (record) {
		//记录id
		sprintf(strbuf, "%d", record->round_id);
		txtID->setString(strbuf);

		//游戏类型
		switch (record->game_type) {
		case BJL:
		{
			txtType->setString("Baccarat");
		}
			break;
		case DZPK:
		{
			txtType->setString("Pocker");
		}
			break;
		case DZPK1:
		{
			txtType->setString("Royal Pocker");
		}
			break;
		case SGJ:
			txtType->setString("Fruit");
			break;
		default:
			txtType->setString("Unknown");
			break;
		}

		//游戏结算时间
		//NBUtil::getFormatDate(strbuf,record_data->records[curIdx].create_time);
		txtTime->setString(Comm::GetLocalTimeStrngFromUTCString(record->create_time));


		//总投注额
		sprintf(strbuf, "%ld", record->bet_amount);
		txtBetAmountNum->setString(strbuf);
		//盈亏
		sprintf(strbuf, "%ld", record->profit);
		txtProfitNum->setString(strbuf);
		if (record->profit >= 0) {
			txtProfitNum->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			txtProfitNum->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}

	

	string highestCards = data->betinfo.highest_cards;
	string cards = data->betinfo.cards;
	string publicCards = data->betinfo.public_cards;
	string allCards = publicCards+highestCards+cards;

	for (int i = 0; i < 9; i++)
	{
		string cardType = allCards.substr(i * 2, 2);
		Sprite * card = RootNode->getChildByName<Sprite*>(StringUtils::format("SpCard%d", i ));
		card->setSpriteFrame("SmallCard" + cardType + ".png");
	}

}