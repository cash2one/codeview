#include "LayerBjlCalculator.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"


bool LayerBjlCalculator::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	RootNode = CSLoader::createNode("bjl/LayerBjlCalculator.csb");
	if (!RootNode)
		return false;
	if (!(panelCalculator = RootNode->getChildByName<Layout *>("panelCalculator")))
		return false;
	if (!(btRound = panelCalculator->getChildByName<Button *>("btRound"))
		|| !(btAll = panelCalculator->getChildByName<Button *>("btAll"))
		|| !(btCopySeed = panelCalculator->getChildByName<Button *>("btCopySeed"))
		|| !(btCopySecret = panelCalculator->getChildByName<Button *>("btCopySecret"))
		|| !(btCopyNextSecret = panelCalculator->getChildByName<Button *>("btCopyNextSecret"))
		|| !(lbData = panelCalculator->getChildByName<Text *>("lbData"))
		|| !(lbSeed = panelCalculator->getChildByName<Text *>("lbSeed"))
		|| !(lbSecret = panelCalculator->getChildByName<Text *>("lbSecret"))
		|| !(lbNextSecret = panelCalculator->getChildByName<Text *>("lbNextSecret"))
		|| !(scrollAllCards = panelCalculator->getChildByName<ScrollView *>("scrollAllCards")))
		return false;
	//scrollAllCards->setTouchEnabled(true);
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerBjlCalculator::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	//add bt event
	btAll->addClickEventListener([=](Ref *ref){ShowAll(); });
	btRound->addClickEventListener([=](Ref *ref){ShowRound(); });
	btCopySeed->addClickEventListener(CC_CALLBACK_1(LayerBjlCalculator::btCopySeedClicked, this));

	btCopySeed->setTitleText(Language::getStringByKey("Copy"));
	btCopySecret->setTitleText(Language::getStringByKey("Copy"));
	btCopyNextSecret->setTitleText(Language::getStringByKey("Copy"));

	/*title = panelCalculator->getChildByName<Text *>("title");
	title->setText(Language::getStringByKey("GameVerify"));*/
	thisSecretText = panelCalculator->getChildByName<Text *>("thisSecretText");

	
	thisSecretText->setText(Language::getStringByKey("ThisRoundSecret"));

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistSmallCard.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistCard.plist");
	return true;
}
bool LayerBjlCalculator::onTouchBegan(Touch *touch, Event *event)
{
	if (!panelCalculator->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce([=](float dt){
	
		CloseWithAction(this);

	}, 0.1f, "DealyExit");	//延迟调用退出，以免touch事件传送到下层
	return true;
}
void LayerBjlCalculator::ShowAll()
{
	btAll->setEnabled(false);
	btAll->setHighlighted(true);
	btRound->setEnabled(true);
	btRound->setHighlighted(false);
	//初始化All
	scrollAllCards->removeAllChildren();
	_seed="";
	_data="";
	lbData->setString("");
	lbSeed->setString("");
	lbSecret->setString("");
	lbNextSecret->setString("");
	if (_roomData->history_rounds.size() < 1)
		return;
	BjlRoomDataRound *allRound = &(_roomData->history_rounds[0]);
	_seed=allRound->seed;
	_data=allRound->data;
	if (_cardDatas.length() > 20)
		lbData->setString(_cardDatas.substr(0, 18) + "...");
	else
		lbData->setString(_cardDatas);
	if (allRound->seed != "0000000000000000000000000000000000000000000000000000000000000000")
		lbSeed->setString(allRound->seed.length()>26?allRound->seed.substr(0,26)+"...":allRound->seed);
	lbSecret->setString(allRound->secret.substr(0,26)+"...");
	Sprite *sp = Sprite::createWithSpriteFrameName("BR.png");		//SmallCardB1.png
	Size cardSize = sp->getContentSize();
	Size scrollSize = scrollAllCards->getContentSize();
	float scale = scrollSize.height / cardSize.height;
	float cardWidth = cardSize.width*scale;
	float pos = 0.0f, spacing = 10.0f;
	const char *cards = _cardDatas.c_str();
	char name[3] = { 0 };
	CCLOG("data len:%d\rdata size:%d", _cardDatas.length(), sizeof(cards));
	for (int i = 0; i < _cardDatas.length() / 2; i++)
	{
		strncpy(name, cards + i * 2, 2);
		if (strcmp(name, "??") == 0)
			sp = Sprite::createWithSpriteFrameName(StringUtils::format("BR.png", name));
		else
			sp = Sprite::createWithSpriteFrameName(StringUtils::format("%s.png", name));
		if (!sp)
		{
			CCLOG("LayerBjlCalculator::SetData\tcreate sprite error\tname:%s", StringUtils::format("%s.png", name).c_str());
			exit(-1);
		}
		sp->setScale(scale);
		sp->setAnchorPoint(Vec2::ZERO);
		sp->setPosition(pos, 0);
		scrollAllCards->addChild(sp);
		pos += 16;
	}
	scrollAllCards->setInnerContainerSize(Size(pos + cardWidth, scrollSize.height));
}
void LayerBjlCalculator::ShowRound()
{
	btAll->setEnabled(true);
	btAll->setHighlighted(false);
	btRound->setEnabled(false);
	btRound->setHighlighted(true);
	//初始化round
	scrollAllCards->removeAllChildren();
	_seed="";
	_data="";
	lbData->setString("");
	lbSeed->setString("");
	lbSecret->setString("");
	lbNextSecret->setString("");
	//判断是否已经开始了1局以上的牌局
	int roundSize = _roomData->history_rounds.size();
	if (roundSize < 2)
		return;
	BjlRoomDataRound *lastRound = &(_roomData->history_rounds[roundSize - 1]);
	_seed=lastRound->seed;
	_data=lastRound->data;
	lbData->setString(lastRound->data);
	lbSeed->setString(lastRound->seed.substr(0,26)+"...");
	lbSecret->setString(lastRound->secret.substr(0,26)+"...");
	lbNextSecret->setString(_roomData->inning_secret.substr(0,26)+"...");
	Sprite *sp = Sprite::createWithSpriteFrameName("SmallCardB1.png");
	Size cardSize = sp->getContentSize();
	Size scrollSize = scrollAllCards->getContentSize();
	float scale = scrollSize.height / cardSize.height;
	float cardWidth = cardSize.width*scale;
	float pos = 0.0f, spacing = 10.0f;
	const char *cards = lastRound->data.c_str();
	char name[3] = { 0 };
	int cardCount = lastRound->data.length() / 2;
	//根据总牌数，计算第1张牌的位置，让所有牌居中
	float totoalWidth = cardCount*cardWidth + (cardCount - 1)*spacing;
	pos = (scrollSize.width - totoalWidth) / 2;
	for (int i = 0; i < cardCount; i++)
	{
		strncpy(name, cards + i * 2, 2);
		if (strcmp(name, "??") == 0)
			sp = Sprite::createWithSpriteFrameName("SmallCardB2.png");
		else
			sp = Sprite::createWithSpriteFrameName(StringUtils::format("SmallCard%s.png", name));
		if (!sp)
		{
			CCLOG("LayerBjlCalculator::SetData\tcreate sprite error\tname:%s", StringUtils::format("%s.png", name).c_str());
			exit(-1);
		}
		sp->setScale(scale);
		sp->setAnchorPoint(Vec2::ZERO);
		sp->setPosition(pos, 0);
		scrollAllCards->addChild(sp);
		pos += (cardWidth + spacing);
	}
	scrollAllCards->setInnerContainerSize(Size(pos, scrollSize.height));
}
void LayerBjlCalculator::SetData(BjlRoomData *data)
{
	if (!data)
		return;
	_roomData = data;
	_cardDatas = "";
	for (int i = 0; i < _roomData->history_rounds.size(); i++)
	{
		_cardDatas.append(_roomData->history_rounds[i].data);
	}
	ShowRound();
}
void LayerBjlCalculator::btCopySeedClicked(Ref *ref)
{
	string seed =_seed;// lbSeed->getString();
	string data =_data;// lbData->getString();
	if (seed.empty())
		return;
	Application::getInstance()->openURL(StringUtils::format("http://www.md5calc.com/sha256/%s%s", seed.c_str(), data.c_str()));
}