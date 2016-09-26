#include "LayerGameRecordBjl.h"
#include "comm.h"
#include "SpriteCmSub.h"
bool LayerGameRecordBjl::init() {
	if (!Layer::init()) {
		return false;
	}

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("report/LayerGameRecordBjl.csb");
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
	if (!(txtPlayerBet = RootNode->getChildByName<Text*>("txtPlayerBet")))
		return false;
	if (!(txtTieBet = RootNode->getChildByName<Text*>("txtTieBet")))
		return false;
	if (!(txtBankerBet = RootNode->getChildByName<Text*>("txtBankerBet")))
		return false;
	if (!(txtPlayerPairBet = RootNode->getChildByName<Text*>("txtPlayerPairBet")))
		return false;
	if (!(txtBankerPairBet = RootNode->getChildByName<Text*>("txtBankerPairBet")))
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
    
    /*auto lbTitle=RootNode->getChildByName<Text*>("lbTitle");
    if(!lbTitle)return false;
    lbTitle->setString(Language::getStringByKey("GameRecords"));*/
    
    auto txtPlayerTitle=RootNode->getChildByName<Text*>("txtPlayerTitle");
    if(!txtPlayerTitle)return false;
    txtPlayerTitle->setString(Language::getStringByKey("Player"));
    
    auto txtBankerTitle=RootNode->getChildByName<Text*>("txtBankerTitle");
    if(!txtBankerTitle)return false;
    txtBankerTitle->setString(Language::getStringByKey("Banker"));
    
    auto txtPlayer=RootNode->getChildByName<Text*>("txtPlayer");
    if(!txtPlayer)return false;
    txtPlayer->setString(Language::getStringByKey("Player"));
    
    auto txtBanker=RootNode->getChildByName<Text*>("txtBanker");
    if(!txtBanker)return false;
    txtBanker->setString(Language::getStringByKey("Banker"));
    
    auto txtTie=RootNode->getChildByName<Text*>("txtTie");
    if(!txtTie)return false;
    txtTie->setString(Language::getStringByKey("Tie"));
    
    auto txtPlayerPair=RootNode->getChildByName<Text*>("txtPlayerPair");
    if(!txtPlayerPair)return false;
    txtPlayerPair->setString(Language::getStringByKey("PlayerPair"));
    
    auto txtBankerPair=RootNode->getChildByName<Text*>("txtBankerPair");
    if(!txtBankerPair)return false;
    txtBankerPair->setString(Language::getStringByKey("BankerPair"));
    
    auto txtBetAmount=RootNode->getChildByName<Text*>("txtBetAmount");
    if(!txtBetAmount)return false;
    txtBetAmount->setString(Language::getStringByKey("BetAmount"));
    
    auto txtProfit=RootNode->getChildByName<Text*>("txtProfit");
    if(!txtProfit)return false;
    txtProfit->setString(Language::getStringByKey("Profit"));

	this->addChild(RootNode);

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerGameRecordBjl::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerGameRecordBjl::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerGameRecordBjl::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}

bool LayerGameRecordBjl::onTouchBegan(Touch *touch, Event *event) {
	if (!spBg->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerGameRecordBjl::DelayExit), 0.1f);
	return true;
}

void LayerGameRecordBjl::onTouchMoved(Touch *touch, Event *event) {

}

void LayerGameRecordBjl::onTouchEnded(Touch* touch, Event* event) {

}

void LayerGameRecordBjl::DelayExit(float dt)
{
	this->removeFromParent();
}
void LayerGameRecordBjl::ShowGameRound(Report::GameDetailRecord *record, GameRoundDataBjl *data){
	if (!record || !data)
		return;
	txtPlayerBet->setString(toString(data->betinfo.p));			//下注闲家的额度
	txtPlayerPairBet->setString(toString(data->betinfo.pp));		//下注闲对的额度
	txtBankerBet->setString(toString(data->betinfo.b));			//下注庄家的额度
	txtBankerPairBet->setString(toString(data->betinfo.bp));		//下注庄对的额度
	txtTieBet->setString(toString(data->betinfo.t));				//下注和局的额度

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

	BjlPmInfos pmInfos;
	pmInfos.SetCards(data->cardinfo.c_str());
	sprintf(strbuf, "SmallCard%s.png", pmInfos.Player.cards[0]);
	SpCard0->setSpriteFrame(strbuf);
	sprintf(strbuf, "SmallCard%s.png", pmInfos.Player.cards[1]);
	SpCard1->setSpriteFrame(strbuf);
	string sp2 = string(pmInfos.Player.cards[2]);
	if (sp2 != "") {
		sprintf(strbuf, "SmallCard%s.png", pmInfos.Player.cards[2]);
		SpCard2->setSpriteFrame(strbuf);
	}
	else {
		SpCard2->setVisible(false);
	}

	sprintf(strbuf, "SmallCard%s.png", pmInfos.Banker.cards[0]);
	SpCard3->setSpriteFrame(strbuf);
	sprintf(strbuf, "SmallCard%s.png", pmInfos.Banker.cards[1]);
	SpCard4->setSpriteFrame(strbuf);
	string sb2 = string(pmInfos.Banker.cards[2]);
	if (sb2 != "") {
		sprintf(strbuf, "SmallCard%s.png", pmInfos.Banker.cards[2]);
		SpCard5->setSpriteFrame(strbuf);
	}
	else {
		SpCard5->setVisible(false);
	}
}