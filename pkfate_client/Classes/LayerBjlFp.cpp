#include "LayerBjlFp.h"
#include "cocostudio/CocoStudio.h"
#include "AvatarControl.h"
#include "TimeControl.h"
#include "ApiGame.h"
#include "PKNotificationCenter.h"
#include "LayerGuideMask.h"
using namespace cocos2d::experimental;
NodePlayerInfo* NodePlayerInfo::create(Node *node){
	if (!node)
		return nullptr;
	NodePlayerInfo *pRet = new (std::nothrow)NodePlayerInfo();
	if (pRet){
		pRet->spAvatar = node->getChildByName<Sprite*>("spAvatar");
		pRet->txtNickname = node->getChildByName<Text*>("txtNickname");
		pRet->txtBMTime = node->getChildByName<TextBMFont*>("txtBMTime");
		pRet->txtBMTime->setVisible(false);
		if (pRet->spAvatar&&pRet->txtNickname&&pRet->txtBMTime){
			pRet->node = node;
			return pRet;
		}
	}
	delete pRet;
	pRet = nullptr;
	return nullptr;
}

string NodePlayerInfo::getName()
{
	return name;
}


void NodePlayerInfo::changeAvatar(int avatarId, int index)
{
	spAvatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(avatarId, index));
}

int NodePlayerInfo::getX()
{
	return node->getPositionX();
}
void NodePlayerInfo::setUser(int userid, string nickname, int avatarId){
	if (userid <= 0){
		node->setVisible(false);
		return;
	}
	txtNickname->setString(nickname);
	name = nickname;
	spAvatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(avatarId, 0));
	node->setVisible(true);
}
void NodePlayerInfo::setTimeVisible(bool visible){
	//txtBMTime->setVisible(visible);
	txtBMTime->setVisible(false); 
}
void NodePlayerInfo::setTimeString(int seconds){
	//txtBMTime->setString(toString(seconds));
}

void NodePlayerInfo::setPlayerNameVisible(bool visible){
	//txtBMTime->setVisible(visible);
	txtNickname->setVisible(visible);
}



LayerBjlFp::LayerBjlFp() :
_nodePlayerInfoPlayer(nullptr),
_nodePlayerInfoBanker(nullptr),
transitionLayer(nullptr),
_fpFx(-1),
_pmInfos(nullptr),
_curPos(-1),
_isAutoExit(true){
}
LayerBjlFp::~LayerBjlFp(){
	if (_nodePlayerInfoPlayer)
		delete _nodePlayerInfoPlayer;
	if (_nodePlayerInfoBanker)
		delete _nodePlayerInfoBanker;
	if (transitionLayer){
		transitionLayer->removeFromParentAndCleanup(true);
		transitionLayer = nullptr;
	}
}
bool LayerBjlFp::init()
{
	onlyOpenCardIndex = -1;

	if (!Layer::init())
		return false;
	if (!(_nodeRoot = CSLoader::createNode("bjl/LayerBjlFp.csb")))
		return false;
	if (!(_nodePlayerInfoPlayer = NodePlayerInfo::create(_nodeRoot->getChildByName("nodePlayer"))))
		return false;
	if (!(_nodePlayerInfoBanker = NodePlayerInfo::create(_nodeRoot->getChildByName("nodeBanker"))))
		return false;
	_textLeftPt = (Text*)_nodeRoot->getChildByName("Text_LeftPt");
	_textRightPt = (Text*)_nodeRoot->getChildByName("Text_RightPt");
	_textInfo = (Text*)_nodeRoot->getChildByName("Text_Info");
	_timeNum = (TextBMFont*)_nodeRoot->getChildByName<TextBMFont*>("time_num");
	_timeNum->setString(toString(20));//
	_btExit = (Button*)_nodeRoot->getChildByName("Button_Exit");

	

	_progressPoint = (Sprite*)_nodeRoot->getChildByName("progress_point");
    _progressPoint->setVisible(false);
	_bgPlayer = (Sprite*)_nodeRoot->getChildByName("bgPlayer");
	_bgBanker = (Sprite*)_nodeRoot->getChildByName("bgBanker");
	_progressBg = (Sprite*)_nodeRoot->getChildByName("progress_bg");
	_timeTip = (Text*)_nodeRoot->getChildByName("time_tip");
	_timeTip->setText(Language::getStringByKey("FlipCountDown"));

	_leftTimeProgress = CCProgressTimer::create(CCSprite::create("bjl/fp/bar.png"));
	_leftTimeProgress->setPosition(ccp(956, 1028));
	_leftTimeProgress->setType(ProgressTimer::Type::BAR);

	//设置进度值范围[0,100]

	_leftTimeProgress->setPercentage(100);
	_leftTimeProgress->setMidpoint(ccp(1, 0));
	//反进度计时

	//_leftTimeProgress->setReverseProgress(true);

	_leftTimeProgress->setBarChangeRate(ccp(1,0));

	_nodeRoot->addChild(_leftTimeProgress,1);
	_leftTimeProgress->setVisible(false);
	_progressBg->setVisible(false);
	_timeTip->setVisible(false);
	_timeNum->setVisible(false);
	_nodeRoot->reorderChild(_progressPoint, 1);

	int pos = 0;
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Left1");
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Left2");
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Left3");
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Right1");
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Right2");
	_spCards[pos++] = (Sprite*)_nodeRoot->getChildByName("Sprite_Right3");
	_spCard0 = (Sprite*)_nodeRoot->getChildByName("Sprite_Card0");
	//_spCard0->setSpriteFrame("BR.png"); // 图不一致
	_spCard1 = (Sprite*)_nodeRoot->getChildByName("Sprite_Card1");

	_openNot = (Text*)_nodeRoot->getChildByName("open_not");
	_openNot->setText(Language::getStringByKey("PlzFlip"));
	_openNot->setPositionY(_openNot->getPositionY() - 100);
	_openNot->setVisible(false);
	_finger = (Sprite*)_nodeRoot->getChildByName("finger");
	_finger->setVisible(false);

	//SpriteFrameCache::getInstance()->addSpriteFramesWithFile("bjl/fp/openTip.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("bjl/fp/openCard.plist");
	//_openNot->setSpriteFrame("open_not.png");
	
	Animation *animation = Animation::create();
	animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("finger/open.png"));
	animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("finger/open_not.png"));
	animation->setDelayPerUnit(2.0f / 5.0f);
	Animate *animate = Animate::create(animation);
	_finger->runAction(RepeatForever::create(animate));
	


	_spSelectedCardBg = (Sprite*)_nodeRoot->getChildByName("selectedcardbg");
	CCASSERT(_btExit&&_textLeftPt&&_textRightPt&&_textInfo
		&&_spCards[0] && _spCards[1] && _spCards[2] && _spCards[3] && _spCards[4] && _spCards[5]
		&& _spCard0&&_spCard1&&_spSelectedCardBg, "init layerbjlfp failed");
	_btExit->addClickEventListener(CC_CALLBACK_1(LayerBjlFp::onButtonExitClicked, this));
	addChild(_nodeRoot);
	//初始化卡牌node
	clipCard0 = ClippingNode::create();
	stencilCard0 = DrawNode::create();
	rectCard = _spCard0->getBoundingBox();		//获取实际卡牌大小
	CCLOG("rb:%.2f,%.2f\t%.2f,%.2f", rectCard.origin.x, rectCard.origin.y, rectCard.size.width, rectCard.size.height);
	clipCard0->setStencil(stencilCard0);
	clipCard0->setInverted(true);
	_spCard0->removeFromParent();
	_spCard0->setAnchorPoint(Vec2(1, 0));
	_spCard0->setPosition(0, 0);
	clipCard0->addChild(_spCard0);
	clipCard0->setPosition(rectCard.origin.x, rectCard.origin.y);
	clipCard0->setContentSize(rectCard.size);		//设置clip大小
	_nodeRoot->addChild(clipCard0, 1);

	clipCard1 = ClippingNode::create();
	stencilCard1 = DrawNode::create();
	clipCard1->setPosition(rectCard.origin.x, rectCard.origin.y);
	clipCard1->setStencil(stencilCard1);
	clipCard1->setInverted(false);
	_spCard1->removeFromParent();
	_spCard1->setAnchorPoint(Vec2(1, 0));
	_spCard1->setPosition(0, 0);
	clipCard1->addChild(_spCard1);
	clipCard1->setVisible(false);
	clipCard1->setContentSize(rectCard.size);		//设置clip大小
	_nodeRoot->addChild(clipCard1, 1);
	//初始化全局参数
	rectCard = clipCard0->getBoundingBox();		//获取最终卡牌位置与大小
	fpSize = rectCard.size.width*0.1;
	drag = false;
	//设置翻牌区大小
	rectCardFp.setRect(rectCard.origin.x - fpSize, rectCard.origin.y - fpSize, rectCard.size.width + 2 * fpSize, rectCard.size.height + 2 * fpSize);
	//设置翻牌最大区域
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	rectMaxFp.setRect(origin.x + visibleSize.width*0.1, origin.y + visibleSize.height*0.1, visibleSize.width*0.8, visibleSize.height*0.8);

    _nodeRoot:reorderChild(_openNot, 1);
	// Register Touch Event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerBjlFp::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerBjlFp::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerBjlFp::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	clear();


	

	return true;
}
void LayerBjlFp::clear(){
	onlyOpenCardIndex = -1;

	_fpFx = -1;
	_textInfo->setString("");
	_textLeftPt->setString("");
	_textRightPt->setString("");
	for (int i = 0; i < 6; i++){
		setCardStatus(i, 0);
		_fxCards[i] = rand() % 2;
	}
	_hasShowResult = false;
	_spCards[2]->setVisible(false);		//不显示闲3
	_spCards[5]->setVisible(false);		//不显示庄3
	_spSelectedCardBg->setPosition(_spCards[0]->getPosition());
	stencilCard0->clear();
	stencilCard1->clear();
	clipCard0->setVisible(true);
	clipCard1->setVisible(false);
	clipCard1->setInverted(false);
}
void LayerBjlFp::setCards(BjlPmInfos *infos){
	_pmInfos = infos;
}
void LayerBjlFp::setCardStatus(int pos, int status){
	_fpFx = -1;
	if (pos < 0 || pos>5)
		return;
	switch (status)
	{
	case 0:		//关闭
		_spCards[pos]->setSpriteFrame("BD.png");
		_statusCards[pos] = status;
		break;
	case 1:		//选中
		_spSelectedCardBg->setPosition(_spCards[pos]->getPosition());
		if (_statusCards[pos] == 2){
			//如果选中的牌，状态为已经打开
			string cardName = getCard(pos);
			if (cardName.empty())
				return;
			stencilCard0->clear();
			stencilCard1->clear();
			clipCard1->setPosition(rectCard.origin.x, rectCard.origin.y);
			clipCard1->setAnchorPoint(Vec2(0, 0));
			clipCard1->setRotation(0);
			clipCard1->setInverted(true);
			clipCard1->setVisible(true);
			clipCard0->setVisible(false);
			_spCard1->setSpriteFrame(cardName + ".png");
			if (_fxCards[pos])
			{
				//顺时针90度
				_spCard1->setAnchorPoint(Vec2(1, 0));
				_spCard1->setRotation(90);
				_spCard1->setPosition(0, 0);
			}
			else
			{
				//逆时针90度
				_spCard1->setAnchorPoint(Vec2(0, 1));
				_spCard1->setRotation(-90);
				_spCard1->setPosition(0, 0);
			}
		}
		else{
			//未开
			string cardName = getCard(pos);
			if (cardName.empty())
				return;
			stencilCard0->clear();
			stencilCard1->clear();
			clipCard1->setInverted(false);
			clipCard1->setVisible(true);
			clipCard0->setVisible(true);
			_spCard1->setSpriteFrame(cardName + "N.png");
			if (_fxCards[pos])
			{
				//顺时针90度
				_spCard1->setAnchorPoint(Vec2(1, 0));
				_spCard1->setRotation(90);
				_spCard1->setPosition(0, 0);
			}
			else
			{
				//逆时针90度
				_spCard1->setAnchorPoint(Vec2(0, 1));
				_spCard1->setRotation(-90);
				_spCard1->setPosition(0, 0);
			}
		}
		_curPos = pos;
		break;
	case 2:		//打开
	{
		string cardName = getCard(pos);
		if (cardName.empty())
			return;
		_spCards[pos]->setSpriteFrame(cardName + ".png");
		_statusCards[pos] = status;
		if (_curPos == pos){
			//如果打开的牌为当前选中的牌
			stencilCard0->clear();
			stencilCard1->clear();
			clipCard1->setPosition(rectCard.origin.x, rectCard.origin.y);
			clipCard1->setAnchorPoint(Vec2(0, 0));
			clipCard1->setRotation(0);
			clipCard1->setInverted(true);
			clipCard1->setVisible(true);
			clipCard0->setVisible(false);
			_spCard1->setSpriteFrame(cardName + ".png");
			if (_fxCards[pos])
			{
				//顺时针90度
				_spCard1->setAnchorPoint(Vec2(1, 0));
				_spCard1->setRotation(90);
				_spCard1->setPosition(0, 0);
			}
			else
			{
				//逆时针90度
				_spCard1->setAnchorPoint(Vec2(0, 1));
				_spCard1->setRotation(-90);
				_spCard1->setPosition(0, 0);
			}
		}
		checkCard();		//每打开一张牌，检查一下
	}
		break;
	default:
		return;
	}
}
void LayerBjlFp::setFlip(int fpfx, float x, float y){
	enableTip(false);
	if (_fpFx != fpfx){
		//重设翻牌起始点
		switch (fpfx)
		{
		case LT:
			fpPt0X = rectCard.origin.x;
			fpPt0Y = rectCard.origin.y + rectCard.size.height;
			clipCard1->setAnchorPoint(Vec2(1, 1));
			break;
		case RT:
			fpPt0X = rectCard.origin.x + rectCard.size.width;
			fpPt0Y = rectCard.origin.y + rectCard.size.height;
			clipCard1->setAnchorPoint(Vec2(0, 1));
			break;
		case LB:
			fpPt0X = rectCard.origin.x;
			fpPt0Y = rectCard.origin.y;
			clipCard1->setAnchorPoint(Vec2(1, 0));
			break;
		case RB:
			fpPt0X = rectCard.origin.x + rectCard.size.width;
			fpPt0Y = rectCard.origin.y;
			clipCard1->setAnchorPoint(Vec2(0, 0));
			break;
		default:
			return;
		}
		_fpFx = fpfx;
		fpPt1X = fpPt0X;
		fpPt1Y = fpPt0Y;
	}
	calcPolygon();
}
void LayerBjlFp::calcPolygon(){
	Vec2 mPt((fpPt0X + fpPt1X) / 2, (fpPt0Y + fpPt1Y) / 2);
	Vec2 pt0(0, 0);
	Vec2 pt1(0, 0);
	float lenXB = 0;
	float lenDB = 0;
	float lenLB = 0;
	float xc, yc;
	//xc = Math.min(fpPt0X, fpPt1X) + Math.abs(fpPt1X - fpPt0X) / 2;
	//yc = Math.min(fpPt0Y, fpPt1Y) + Math.abs(fpPt1Y - fpPt0Y) / 2;
	fixZB();
	xc = (fpPt0X + fpPt1X) / 2;
	yc = (fpPt0Y + fpPt1Y) / 2;
	//console.log("debug62:" + fpPt0X + "," + fpPt0Y + " " + fpPt1X + "," + fpPt1Y);
	//cclog("debug62:%d,%d %d,%d", fpPt0X, fpPt0Y, fpPt1X, fpPt1Y);
	if (fpPt1Y == fpPt0Y)
	{
		//水平线
		pt0.x = xc;
		pt1.x = xc;
		if (fpPt0Y == rectCard.origin.y)
		{
			pt0.y = rectCard.origin.y + rectCard.size.height;
			pt1.y = rectCard.origin.y;
		}
		else
		{
			pt0.y = rectCard.origin.y;
			pt1.y = rectCard.origin.y + rectCard.size.height;
		}
		//console.log("debug622:" + pt1.y);
	}
	else if (fpPt1X == fpPt0X)
	{
		//垂直线
		pt0.y = yc;
		pt1.y = yc;
		if (fpPt0X == rectCard.origin.x)
		{
			pt0.x = rectCard.origin.x;
			pt1.x = rectCard.origin.x + rectCard.size.width;
		}
		else
		{
			pt0.x = rectCard.origin.x + rectCard.size.width;
			pt1.x = rectCard.origin.x;
		}
	}
	else
	{
		pt0.x = fpPt0X;
		pt1.y = fpPt0Y;
		pt0.y = yc + (xc - pt0.x)*(fpPt1X - fpPt0X) / (fpPt1Y - fpPt0Y);
		if (pt0.y > rectCard.origin.y + rectCard.size.height || pt0.y<rectCard.origin.y)
		{
			if (pt0.y>rectCard.origin.y + rectCard.size.height)
				pt0.y = rectCard.origin.y + rectCard.size.height;
			else
				pt0.y = rectCard.origin.y;
			pt0.x = xc + (yc - pt0.y)*(fpPt1Y - fpPt0Y) / (fpPt1X - fpPt0X);
			if (pt0.x<rectCard.origin.x || pt0.x>rectCard.origin.x + rectCard.size.width)
				return;
		}
		pt1.x = xc + (yc - pt1.y)*(fpPt1Y - fpPt0Y) / (fpPt1X - fpPt0X);
		//console.log("debug5:" + xc + "," + yc + "," + pt1.y + "," + fpPt1Y + "," + fpPt0Y + "," + fpPt1X + "," + fpPt0X);
		if (pt1.x<rectCard.origin.x || pt1.x>rectCard.origin.x + rectCard.size.width)
		{
			if (pt1.x<rectCard.origin.x)
				pt1.x = rectCard.origin.x;
			else
				pt1.x = rectCard.origin.x + rectCard.size.width;
			pt1.y = yc + (xc - pt1.x)*(fpPt1X - fpPt0X) / (fpPt1Y - fpPt0Y);
		}
	}
	if (pt0.x<rectCard.origin.x || pt0.x>rectCard.origin.x + rectCard.size.width || pt0.y>rectCard.origin.y + rectCard.size.height || pt0.y < rectCard.origin.y)
		return;

	// 计算多边形
	float offsetX, offsetY;	//drawImage偏移量
	int countPolygon = 0;	//多边形数量
	switch (_fpFx)
	{
	case LT:
		offsetX = -rectCard.size.width;
		offsetY = 0;
		polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y + rectCard.size.height);
		if (pt0.x > fpPt0X)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y);
		polygonCardBack0[countPolygon++].set(pt0.x, pt0.y);
		polygonCardBack0[countPolygon++].set(pt1.x, pt1.y);
		if (pt1.y < fpPt0Y)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y + rectCard.size.height);
		break;
	case RT:
		offsetX = 0;
		offsetY = 0;
		polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y + rectCard.size.height);
		if (pt0.x < fpPt0X)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y);
		polygonCardBack0[countPolygon++].set(pt0.x, pt0.y);
		polygonCardBack0[countPolygon++].set(pt1.x, pt1.y);
		if (pt1.y < fpPt0Y)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y + rectCard.size.height);
		break;
	case LB:
		offsetX = -rectCard.size.width;
		offsetY = -rectCard.size.height;
		polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y);
		if (pt0.x > fpPt0X)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y + rectCard.size.height);
		polygonCardBack0[countPolygon++].set(pt0.x, pt0.y);
		polygonCardBack0[countPolygon++].set(pt1.x, pt1.y);
		if (pt1.y > fpPt0Y)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y);
		break;
	case RB:
		offsetX = 0;
		offsetY = -rectCard.size.height;
		polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y);
		if (pt0.x < fpPt0X)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x + rectCard.size.width, rectCard.origin.y + rectCard.size.height);
		polygonCardBack0[countPolygon++].set(pt0.x, pt0.y);
		polygonCardBack0[countPolygon++].set(pt1.x, pt1.y);
		if (pt1.y > fpPt0Y)
			polygonCardBack0[countPolygon++].set(rectCard.origin.x, rectCard.origin.y);
		break;
	}
	if (countPolygon > 0)
	{
		for (int i = 0; i < countPolygon; i++)
		{
			polygonCardBack0[i].set(polygonCardBack0[i].x - rectCard.origin.x, polygonCardBack0[i].y - rectCard.origin.y);
			polygonCardBack1[i].set(mirrorX(polygonCardBack0[i].x, rectCard.size.width / 2), polygonCardBack0[i].y);
		}
		stencilCard0->clear();
		stencilCard0->drawPolygon(polygonCardBack0, countPolygon, Color4F::WHITE, 1, Color4F::WHITE);
		stencilCard1->clear();
		stencilCard1->drawPolygon(polygonCardBack1, countPolygon, Color4F::WHITE, 1, Color4F::WHITE);

		lenDB = pt1.y - pt0.y;
		lenLB = pt1.x - pt0.x;
		float a = atan(lenDB / lenLB);
		a = M_PI - 2 * a;
		clipCard1->setPosition(fpPt1X, fpPt1Y);
		clipCard1->setRotation(a * 180 / M_PI);
	}
}
//修正坐标
void LayerBjlFp::fixZB()
{
	switch (_fpFx)
	{
	case LT:
		if (fpPt1X<fpPt0X)
			fpPt1X = fpPt0X;
		if (fpPt1Y>fpPt0Y)
			fpPt1Y = fpPt0Y;
		break;
	case RT:
		if (fpPt1X > fpPt0X)
			fpPt1X = fpPt0X;
		if (fpPt1Y > fpPt0Y)
			fpPt1Y = fpPt0Y;
		break;
	case LB:
		if (fpPt1X < fpPt0X)
			fpPt1X = fpPt0X;
		if (fpPt1Y<fpPt0Y)
			fpPt1Y = fpPt0Y;
		break;
	case RB:
		if (fpPt1X>fpPt0X)
			fpPt1X = fpPt0X;
		if (fpPt1Y < fpPt0Y)
			fpPt1Y = fpPt0Y;
		break;
	}
}
//获取镜像坐标
float LayerBjlFp::mirrorX(float x, float m)
{
	return m - (x - m);
}
string LayerBjlFp::getCard(int pos){
	if (!_pmInfos)
		return "";
	switch (pos)
	{
	case 0:
		return _pmInfos->Player.cards[0];
	case 1:
		return _pmInfos->Player.cards[1];
	case 2:
		return _pmInfos->Player.cards[2];
	case 3:
		return _pmInfos->Banker.cards[0];
	case 4:
		return _pmInfos->Banker.cards[1];
	case 5:
		return _pmInfos->Banker.cards[2];
	default:
		return "";
	}
}



bool LayerBjlFp::onTouchBegan(Touch* touch, Event* event)
{
	//CCLOG("onTouchBegan id = %d, x = %f, y = %f", touch->getID(), touch->getLocation().x, touch->getLocation().y);
	auto touchPoint = touch->getLocation();
	if (rectCardFp.containsPoint(touchPoint))
	{
		if (_statusCards[_curPos] == 2)
			return false;
		//点击牌面
		if (fabs(touchPoint.x - rectCard.origin.x) < fpSize)
		{
			if (fabs(rectCard.origin.y + rectCard.size.height - touchPoint.y) < fpSize)
			{
				//左上角
				setFlip(LT, touchPoint.x, touchPoint.y);
				drag = true;
				clipCard1->setVisible(true);
			}
			else if (fabs(touchPoint.y - rectCard.origin.y) < fpSize)
			{
				//左下角
				setFlip(LB, touchPoint.x, touchPoint.y);
				drag = true;
				clipCard1->setVisible(true);
			}
			else
				return false;
		}
		else if (fabs(rectCard.origin.x + rectCard.size.width - touchPoint.x) < fpSize)
		{
			if (fabs(rectCard.origin.y + rectCard.size.height - touchPoint.y) < fpSize)
			{
				//右上角
				setFlip(RT, touchPoint.x, touchPoint.y);
				drag = true;
				clipCard1->setVisible(true);
			}
			else if (fabs(touchPoint.y - rectCard.origin.y) < fpSize)
			{
				//右下角
				setFlip(RB, touchPoint.x, touchPoint.y);
				drag = true;
				clipCard1->setVisible(true);
			}
			else
				return false;
		}
		else
			return false;
	}
	else{
		// 检查是否点击6张牌位置

		if (onlyOpenCardIndex != -1){
			
			if (onlyOpenCardIndex!=-1000&&_spCards[onlyOpenCardIndex]->getBoundingBox().containsPoint(touchPoint)){
				onClickCard(onlyOpenCardIndex);
			}
		}
		else{
			for (int i = 0; i < 6; i++){
				if (_spCards[i]->getBoundingBox().containsPoint(touchPoint)){
					onClickCard(i);
					break;
				}
			}
		}
		
	}
	return true;
}

void LayerBjlFp::onlyOpenCardClick(int index)
{
	onlyOpenCardIndex = index;
}

void LayerBjlFp::onTouchMoved(Touch* touch, Event* event)
{
	//CCLOG("onTouchMoved id = %d, x = %f, y = %f", touch->getID(), touch->getLocation().x, touch->getLocation().y);
	//CCLOG("clipCard0:%.2f,%.2f\t%.2f,%.2f", clipCard1->getBoundingBox().origin.x, clipCard1->getBoundingBox().origin.y, clipCard1->getBoundingBox().size.width, clipCard1->getBoundingBox().size.height);	
	if (drag)
	{
		//正在翻牌
		auto touchPoint = touch->getLocation();
		fpPt1X = touchPoint.x;
		fpPt1Y = touchPoint.y;
		calcPolygon();
	}
}
void LayerBjlFp::onTouchEnded(Touch* touch, Event* event)
{
	if (drag)
	{
		drag = false;
		stencilCard0->clear();
		stencilCard1->clear();
		Vec2 touchPoint = touch->getLocation();
		if (rectMaxFp.containsPoint(touchPoint)){
			//未超出范围
			clipCard1->setVisible(false);
			//enableTip(true);
		}
		else{
			//超出范围，直接开牌
			setCardStatus(_curPos, 2);
		}
		_fpFx = -1;
	}
}
void LayerBjlFp::onButtonExitClicked(Ref *ref){

}
void LayerBjlFp::onClickCard(int pos){

}

void LayerBjlFp::turnViewHandler(bool isPlayer)
{

}
void LayerBjlFp::enableTip(bool enable)
{

}



void LayerBjlFp::addGuide()
{
	_btExit->setEnabled(true);
	if (!GuideControl::GetInstance()->checkInGuide()) return;
	_btExit->setEnabled(false);//禁用

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N6, "flip_card_1", [=](){
		LayerGuideMask::GetInstance()->setSwallowTouches(false);
		onlyOpenCardClick(-1000);
		LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT_TOP, ccp(1525, 65));
	});
	LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N6, Director::getInstance()->getRunningScene());
}

//检查牌面情况
void LayerBjlFp::checkCard(){
	//统计点数
	int ptL = 0, ptR = 0;
	if (_statusCards[0] == 2)
		ptL += _pmInfos->Player.points[0];
	if (_statusCards[1] == 2)
		ptL += _pmInfos->Player.points[1];
	if (_statusCards[2] == 2)
		ptL += _pmInfos->Player.points[2];
	if (_statusCards[3] == 2)
		ptR += _pmInfos->Banker.points[0];
	if (_statusCards[4] == 2)
		ptR += _pmInfos->Banker.points[1];
	if (_statusCards[5] == 2)
		ptR += _pmInfos->Banker.points[2];
	_textLeftPt->setText(toString(ptL % 10));
	_textRightPt->setText(toString(ptR % 10));
	if (_statusCards[0] == 2 && _statusCards[1] == 2 && _statusCards[3] == 2 && _statusCards[4] == 2)
	{
		//四张牌全开了
		bool showResult = true;
		//检查是否需要增加新牌
		if (_pmInfos->Banker.count >2 || _pmInfos->Player.count >2)
		{
			//闲是否有第3张牌
			if (_pmInfos->Player.count == 3)
			{
				if (!_spCards[2]->isVisible())
				{
					//附加判断,庄前2张小于3时,同时补牌
					ptR = (_pmInfos->Banker.points[0] + _pmInfos->Banker.points[1]) % 10;
					if (ptR < 3)
					{
						//庄闲,同补
						_textInfo->setText(Language::getStringByKey("BothAddCard"));
						_spCards[5]->setVisible(true);
						_spCards[2]->setVisible(true);
					}
					else
					{
						//闲先补
						_textInfo->setText(Language::getStringByKey("PlayerAddCard"));
						_spCards[2]->setVisible(true);
					}
					return;
				}
				showResult &= _statusCards[2] == 2;
				//showResult &= *(int*)_spLeft3->getUserData();
			}
			//庄家是否有第3张牌
			if (_pmInfos->Banker.count == 3)
			{
				if (!_spCards[5]->isVisible())
				{
					//显示第3张牌背面
					_textInfo->setText(Language::getStringByKey("BankerAddCard"));
					_spCards[5]->setVisible(true);

					GuideControl::GetInstance()->sendVitualTrigger("player_flip_third");

					return;
				}
				showResult &= _statusCards[5] == 2;
			}
		}
		if (showResult&&!_hasShowResult)
		{
			_hasShowResult = true;
			
			if (_pmInfos->Banker.point > _pmInfos->Player.point)
			{
				_textInfo->setText(Language::getStringByKey("BankerWin"));
			}
			else if (_pmInfos->Banker.point < _pmInfos->Player.point)
			{
				_textInfo->setText(Language::getStringByKey("PlayerWin"));
			}
			else
				_textInfo->setText(Language::getStringByKey("Peace"));
			// 允许自动退出时，牌全开，可以直接退出
			if (_isAutoExit){
				scheduleOnce([=](float d){
					this->removeFromParent();
				}, 0.5f, "exit");
			}
		}
	}
}
//============ 单人翻牌
void LayerBjlFpS::onClickCard(int pos){
	bool isPlayerCard = (pos >= 0 && pos <= 2);
	if (isPlayerCard == _isPlayer){
		//自己的牌，选中，并放到中间
		setCardStatus(pos, 1);
	}
	else{
		//对面的牌，直接打开
		setCardStatus(pos, 2);
	}
}
void LayerBjlFpS::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
void LayerBjlFpS::setCards(BjlPmInfos *infos, bool isPlayer){
	LayerBjlFp::setCards(infos);
	_isPlayer = isPlayer;
	enableTip(true);
	if (isPlayer){
		_bgPlayer->setSpriteFrame("panel.png");
		_bgBanker->setSpriteFrame("panel_off.png");
		setCardStatus(0, 1);
		_textInfo->setText(Language::getStringByKey("PlayerFlip"));
	}
	else{
		_bgPlayer->setSpriteFrame("panel_off.png");
		_bgBanker->setSpriteFrame("panel.png");
		setCardStatus(3, 1);
		_textInfo->setText(Language::getStringByKey("BlankerFlip"));
	}
}

void LayerBjlFpS::enableTip(bool enable)
{
	_openNot->stopAllActions();
	_openNot->setVisible(enable);
	if (enable){

		CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(0.6), FadeOut::create(0.6), nullptr));
		_openNot->runAction(action);

	}
	else
	{
		
		

	}
}
//============ 多人翻牌
#define TIMESPANFPING	0.2f		//翻牌动画时间
bool LayerBjlFpM::init()
{
	_layoutVS = nullptr;
	if (!LayerBjlFp::init())
		return false;
	if (!(_layoutVS = _nodeRoot->getChildByName<Layout*>("layoutVS")))
		return false;
	_layoutVS->setZOrder(1);

	_isAutoExit = false;		//禁止牌全开时，自动退出
	_ptPlayerAvatar = _nodePlayerInfoPlayer->spAvatar->getPosition();
	_ptPlayerNickName = _nodePlayerInfoPlayer->txtNickname->getPosition();
	_ptBankerAvatar = _nodePlayerInfoBanker->spAvatar->getPosition();
	_ptBankerNickName = _nodePlayerInfoBanker->txtNickname->getPosition();
	_scaleAvatar0 = 1.0f;
	_scaleAvatar1 = 0.8f;
	_checkFP = nullptr;
	schedule(schedule_selector(LayerBjlFpM::updateTime), 1.0f);
	scheduleUpdate();
	clear();
	return true;
}
void LayerBjlFpM::clear(){
	LayerBjlFp::clear();
	if (!_layoutVS)
		return;
	_layoutVS->setVisible(true);
	_layoutVS->setOpacity(255);
	_nodePlayerInfoPlayer->node->setZOrder(3);
	_nodePlayerInfoBanker->node->setZOrder(3);
	_nodePlayerInfoPlayer->spAvatar->setPosition(_ptPlayerAvatar);
	_nodePlayerInfoPlayer->spAvatar->setScale(_scaleAvatar0);
	_nodePlayerInfoPlayer->txtNickname->setPosition(_ptPlayerNickName);
	_nodePlayerInfoPlayer->txtNickname->setScale(_scaleAvatar0);
	_nodePlayerInfoPlayer->txtNickname->setString("");
	_nodePlayerInfoBanker->spAvatar->setPosition(_ptBankerAvatar);
	_nodePlayerInfoBanker->spAvatar->setScale(_scaleAvatar0);
	_nodePlayerInfoBanker->txtNickname->setPosition(_ptBankerNickName);
	_nodePlayerInfoBanker->txtNickname->setScale(_scaleAvatar0);
	_nodePlayerInfoBanker->txtNickname->setString("");
	_btExit->setVisible(false);
	_enableControl = false;
	_finger->setVisible(false);
	//enableTip(false);
	_endTime = -1;
	_fpStatus = -1;
	_timeSpanFping = TIMESPANFPING;
	_timeSpanMove = TIMESPANFPING;
	_finger->setVisible(false);
}
void LayerBjlFpM::updateTime(float dt){
	if (_endTime > 0){
		int seconds = max(0, _endTime - TimeControl::GetInstance()->GetServerTime());
		if (_isPlayer)
			_nodePlayerInfoPlayer->setTimeString(seconds);
		else
			_nodePlayerInfoBanker->setTimeString(seconds);

		_timeNum->setString(toString(seconds));
		//_leftTimeProgress->setPercent(_leftTimeProgress->getPercent()-5);
	
		if (seconds == 0&&!GuideControl::GetInstance()->checkInGuide()){
			_enableControl = false;
			_timeSpanFping = TIMESPANFPING;			//停止动画
			_endTime = -1;			//停止updateTime
			switch (_fpStatus)
			{
			case 0:
				setCardStatus(0, 2); setCardStatus(1, 2);		//打开闲家两张牌
				break;
			case 1:
				setCardStatus(0, 2); setCardStatus(1, 2); setCardStatus(3, 2); setCardStatus(4, 2);		//打开前4张牌
				break;
			case 2:
				setCardStatus(0, 2); setCardStatus(1, 2); setCardStatus(2, 2); setCardStatus(3, 2); setCardStatus(4, 2);		//打开前5张牌
				break;
			case 3:
				setCardStatus(0, 2); setCardStatus(1, 2); setCardStatus(2, 2); setCardStatus(3, 2); setCardStatus(4, 2); setCardStatus(5, 2);		//打开所有牌
				break;
			default:
				break;
			}
		}
	}
}
void LayerBjlFpM::update(float dt){
	if (_enableControl){
		//本人控制中，且正在翻牌
		if (drag){
			if (_timeSpanMove >= TIMESPANFPING){
				Api::Game::mo_baccarat_fp_flip(_fpFx, 0, (int)fpPt1X, (int)fpPt1Y);	//发送翻牌消息
				_timeSpanMove = 0.0f;
			}
			else{
				_timeSpanMove += dt;
			}
		}
	}
	else{
		//非本人控制
		if (_timeSpanFping < TIMESPANFPING){
			_timeSpanFping += dt;
			fpPt1X += _ptStep.x*dt;
			fpPt1Y += _ptStep.y*dt;
			calcPolygon();
		}
	}
}
//停止翻牌
void LayerBjlFpM::onButtonExitClicked(Ref *ref){
	_btExit->setVisible(false);			//隐藏退出按钮
	_enableControl = false;				//交出控制权
	enableTip(false);
	Api::Game::mo_baccarat_fp_end();	//发送结束消息
}
void LayerBjlFpM::onClickCard(int pos){
	switch (_fpStatus)
	{
	case 0:		//闲,允许点击闲1，闲2，庄无人时，允许点击庄1，庄2，不允许点击闲3，庄3
		if (pos == FPCARD::FPLEFT3 || pos == FPCARD::FPRIGHT3)
			return;
		if (pos == FPCARD::FPRIGHT1 || pos == FPCARD::FPRIGHT2){
			if (_bankerId == 0){
				setCardStatus(pos, 2);		//对面无人时，直接开对方牌
				Api::Game::mo_baccarat_fp_open(pos);
			}
			return;
		}
		break;
	case 1:		//庄
		if (GuideControl::GetInstance()->checkInGuide())
		{

			LayerGuideMask::GetInstance()->cleanMask();
			switch (pos)
			{
			case 0:
			{
				onlyOpenCardClick(1);
				LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT, ccp(405, 490));
			}
				break;
			case 1:
			{
				GuideControl::GetInstance()->injectHander(GUIDESTEP::N8, "flip_card_2", [=](){
					LayerGuideMask::GetInstance()->setSwallowTouches(false);
					onlyOpenCardClick(4);
					LayerGuideMask::GetInstance()->addArrow(DIRECTION::RIGHT,ccp(1515,490));
				});
				LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N8, this);  //打开之后显示打开闲家箭头
			}
				break;
			case 4:
			{
				onlyOpenCardClick(-1000);
				LayerGuideMask::GetInstance()->setSwallowTouches(false);
				LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT_TOP, ccp(1525, 65));
				
			}
				break;

			case 5:
			{
				
				
			}
				break;
			default:
				break;
			}

		}


		if (pos == FPCARD::FPLEFT3 || pos == FPCARD::FPRIGHT3)
			return;
		if (pos == FPCARD::FPLEFT1 || pos == FPCARD::FPLEFT2){
			if (_playerId == 0){
				setCardStatus(pos, 2);		//对面无人时，直接开对方牌
				Api::Game::mo_baccarat_fp_open(pos);
				
			}
			return;
		}
		break;
	case 2:		//闲补
		if (pos == FPCARD::FPRIGHT3){
			if (_bankerId == 0){
				setCardStatus(pos, 2);		//对面无人时，直接开对方牌
				Api::Game::mo_baccarat_fp_open(pos);
			}
			return;
		}
		break;
	case 3:		//庄补
		if (pos == FPCARD::FPLEFT3){
			if (_playerId == 0){
				setCardStatus(pos, 2);		//对面无人时，直接开对方牌
				Api::Game::mo_baccarat_fp_open(pos);
			}
			return;
		}
		break;
	default:
		return;
	}
	setCardStatus(pos, 1);
	Api::Game::mo_baccarat_fp_select(pos);		//发送选牌消息
}
bool LayerBjlFpM::onTouchBegan(Touch* touch, Event* event)
{
	if (!this->isVisible())
		return false;
	if (!_enableControl)
		return true;
	return LayerBjlFp::onTouchBegan(touch, event);
}
void LayerBjlFpM::onTouchMoved(Touch* touch, Event* event)
{
	if (!_enableControl)
		return;
	LayerBjlFp::onTouchMoved(touch, event);
}
void LayerBjlFpM::onTouchEnded(Touch* touch, Event* event)
{
	if (!_enableControl || !drag)
		return;
	LayerBjlFp::onTouchEnded(touch, event);
	if (_statusCards[_curPos]==2){
		//当前牌被打开，发送开牌消息

		Api::Game::mo_baccarat_fp_open(_curPos);

		LayerGuideMask::GetInstance()->cleanMask();

		if (GuideControl::GetInstance()->checkInGuide())
		{
			switch (_curPos)
			{
			case 3:
			{
				LayerGuideMask::GetInstance()->cleanMask();
				GuideControl::GetInstance()->injectHander(GUIDESTEP::N7, "click_card_mask", [=](){
				    LayerGuideMask::GetInstance()->setSwallowTouches(false);
					onlyOpenCardClick(0);
					LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT,ccp(405,790));
				});
				LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N7, this);  //打开之后显示打开闲家箭头
			}
				break;
			case 4:
			{
				GuideControl::GetInstance()->injectHander(GUIDESTEP::N9, "flip_card_3", [=](){
					LayerGuideMask::GetInstance()->setSwallowTouches(false);
					onlyOpenCardClick(-1000);
					LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT_TOP, ccp(1535, 55));
				});

				LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N9, this);//打开之后运气真不好
			    
			}
				break;
			case 5:
			{

				GuideControl::GetInstance()->injectHander(GUIDESTEP::N10, "flip_award", [=](){
					LayerGuideMask::GetInstance()->cleanTech();
					GuideControl::GetInstance()->sendVitualTrigger("player_award");
				});

				LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N10, this);

			}
				break;

			default:
				break;
			}
		}

	}
	else{
		//当前牌未打开，发送消息，将牌复位，即合上
		Api::Game::mo_baccarat_fp_flip(_fpFx, 0, (int)fpPt0X, (int)fpPt0Y);	//发送复位牌面消息，即起始与结束点相同，
	}
}
//设置翻牌状态
void LayerBjlFpM::setFpStatus(int status, double endTime){
	
	

	_fpStatus = status;
	if (GuideControl::GetInstance()->checkInGuide()){
		_endTime = TimeControl::GetInstance()->GetServerTime() + 20;
	}
	else{
		_endTime = endTime;
	}
	
	/*if (!_nodePlayerInfoBanker->spAvatar->isVisible() || !_nodePlayerInfoPlayer->spAvatar->isVisible()){
		return;
	}*/
	_timeSpanFping = TIMESPANFPING;			//停止动画
	_fpFx = (FPFX)-1;		//重置翻牌方向
	switch (_fpStatus)
	{
	case 0:		//闲
		_textInfo->setText(Language::getStringByKey("PlayerFlip"));
		_isPlayer = true;
		_enableControl = (_playerId == _myId);
		setCardStatus(0, 1);		//选中闲1
		break;
	case 1:		//庄
		_textInfo->setText(Language::getStringByKey("BankerFlip"));
		_isPlayer = false;
		_enableControl = (_bankerId == _myId);
		setCardStatus(3, 1);		//选中庄1
		if (_playerId != 0) {
			setCardStatus(0, 2); setCardStatus(1, 2);		//打开闲家两张牌
		}
		break;
	case 2:		//闲补
		_textInfo->setText(Language::getStringByKey("PlayerAddCard"));
		_isPlayer = true;
		_enableControl = (_playerId == _myId);
		setCardStatus(2, 1);		//选中闲3
		setCardStatus(0, 2); setCardStatus(1, 2); setCardStatus(3, 2); setCardStatus(4, 2);		//打开前4张牌
		break;
	case 3:		//庄补
		_textInfo->setText(Language::getStringByKey("BankerAddCard"));
		_isPlayer = false;
		_enableControl = (_bankerId == _myId);
		setCardStatus(5, 1);		//选中庄3
		setCardStatus(0, 2); setCardStatus(1, 2); setCardStatus(3, 2); setCardStatus(4, 2);		//打开前4张牌
		
		if (_playerId > 0)
			setCardStatus(2, 2);
		break;
	default:
		return;
	}
	_btExit->setVisible(_enableControl);
	updateTime(0);
	enableTip(_enableControl);
	//test
	turnViewHandler(_isPlayer);

	if (_enableControl&&_checkFP&&!_checkFP->isSelected()){
		//如果不需要手动翻牌，则延迟退出
		_btExit->setVisible(false);			//隐藏退出按钮
		_enableControl = false;				//交出控制权
		enableTip(false);
		//scheduleOnce([=](float dt){
		//	Api::Game::mo_baccarat_fp_end();	//发送结束消息
		//}, 0.5f, "DelayExitFp");

		this->retain();
		runAction(Sequence::createWithTwoActions(DelayTime::create(0.5f), CallFunc::create([=](){
			Api::Game::mo_baccarat_fp_end();	//发送结束消息
			this->release();
		})));
	}
}


void LayerBjlFpM::enableTip(bool enable)
{
	
	_openNot->stopAllActions();
	_openNot->setVisible(enable);
	if (enable){
	    
		//验证完成任务 首次搓牌
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(11)));

		CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(0.6), FadeOut::create(0.6), nullptr));
		_openNot->runAction(action);

	}
	else
	{

		if (_isPlayer)
		{
			
			_textInfo->setText(Language::getStringByKey("PlayerFlip"));
			
		}
		else
		{

			_textInfo->setText(Language::getStringByKey("BankerFlip"));
		}

	 }

}

void LayerBjlFpM::turnViewHandler(bool isPlayer)
{
	_nodePlayerInfoPlayer->setTimeVisible(isPlayer);
	_nodePlayerInfoBanker->setTimeVisible(!isPlayer);
	
	if (isPlayer)
	{
		//_finger->runAction(MoveTo::create(0.3, ccp(_nodePlayerInfoPlayer->getX(), 950)));
	    _finger->setPosition(ccp(_nodePlayerInfoPlayer->getX(), 950));
	}
	else
	{
		//_finger->runAction(MoveTo::create(0.3, ccp(_nodePlayerInfoBanker->getX(), 950)));
		_finger->setPosition(ccp(_nodePlayerInfoBanker->getX(), 950));
	}

	_bgBanker->setSpriteFrame(!isPlayer ? "panel.png" : "panel_off.png");
	_bgPlayer->setSpriteFrame(isPlayer ? "panel.png" : "panel_off.png");

	_leftTimeProgress->stopAllActions();
	_leftTimeProgress->setPercentage(100);//重置倒计时状态
	int seconds = max(0, _endTime - TimeControl::GetInstance()->GetServerTime());
	_timeNum->setString(toString(seconds));
	CCProgressTo *to = CCProgressTo::create(seconds, 0);
	_leftTimeProgress->runAction(to);

	//_progressPoint->setPositionX(700);
	//_progressPoint->setPositionX(_progressPoint->getPositionX() + 25.75);
	_progressPoint->stopAllActions();
	_progressPoint->setPositionX(695);
	_finger->setVisible(true);
	_timeNum->setVisible(true);
	_leftTimeProgress->setVisible(true);
	_progressBg->setVisible(true);
	_timeTip->setVisible(true);
	_progressPoint->setVisible(true);
	
	_progressPoint->runAction(Sequence::create(MoveBy::create(seconds, ccp(520, 0)),
	CallFunc::create([&]
	{
		_progressPoint->setVisible(false);
		_leftTimeProgress->setVisible(false);
		_progressBg->setVisible(false); 
		_timeTip->setVisible(false);
		_timeNum->setVisible(false);
	}), nullptr));

	/*if (GuideControl::GetInstance()->checkInGuide())
	{
		_leftTimeProgress->pause();
		_progressPoint->pause();
		_endTime = 0;

		this->scheduleOnce([&](float dt) {
			_progressPoint->resume();
			_leftTimeProgress->resume();
			_endTime = TimeControl::GetInstance()->GetServerTime() + 20;
		}, 15, "resumeTimeProgress");

	}*/
}






void LayerBjlFpM::setFpBegin(int playerId, string playerName, int playerAvatarId, int bankerId, string bankerName, int bankerAvatarId, int myId, bool runAction){
	_playerId = playerId;
	_bankerId = bankerId;
	_myId = myId;
	_nodePlayerInfoPlayer->setUser(playerId, playerName, playerAvatarId);
	_nodePlayerInfoBanker->setUser(bankerId, bankerName, bankerAvatarId);

	_nodePlayerInfoPlayer->setPlayerNameVisible(false);
	_nodePlayerInfoBanker->setPlayerNameVisible(false);

	_layoutVS->setVisible(false);

	

	float backStayPoint = 0.8; //左右背景停留时间点
	float backFadePoint = 1.2;//背景开始fade时间点
	float backMissPoint = 1.8;//背景消失时间点
	float avatarTimePoint = 0.7;// 开始时间点
	float avatarStayPoint = 1; //头像停留时间点
	float avatarFlyPoint = 1.8;  //头像飞出时间点
	float avatarOriginPoint = 2;//头像归位时间点
	float avatarOffset = 300;

	float vsOffset = 200;
	float vsFlyOffset = 120;

	float vsTimePoint = 0.7; // 出现时间点
	float vsStayPoint = 0.8; //停留时间点
	float vsFlyPoint = 1.7; // s飞时间点
	float vsMissPoint = 1.9; // s消失

	if (transitionLayer == nullptr){
		transitionLayer = Layer::create();
		Scene* runningScene = Director::getInstance()->getRunningScene(); //析构函数中再处理tansitionLayer
		runningScene->addChild(transitionLayer, 100);
	}
	Size size = Director::getInstance()->getVisibleSize();
	
	setVisible(false);
	if (runAction){

		/**********************VS处理**************************/
		Sprite * v = Sprite::create("bjl/fp/v.png");
		Sprite * s = Sprite::create("bjl/fp/s.png");
		v->setOpacity(0);
		s->setOpacity(0);
		v->setAnchorPoint(ccp(1, 0));
		s->setAnchorPoint(ccp(0, 0));

		v->setPosition(size.width / 2 - vsOffset, size.height / 2 + 40);
		s->setPosition(size.width / 2 + vsOffset, size.height / 2 + 40);
		transitionLayer->addChild(v, 3);
		transitionLayer->addChild(s, 3);
		Action* vAction = Sequence::create(
			DelayTime::create(vsTimePoint),
			FadeIn::create(0.1),
			EaseBackInOut::create(MoveBy::create(vsStayPoint - vsTimePoint, ccp(vsOffset + 40, 0))),
			DelayTime::create(vsFlyPoint - vsStayPoint),
			Spawn::create(FadeOut::create(vsMissPoint - vsFlyPoint),
			MoveBy::create(vsMissPoint - vsFlyPoint, ccp(-vsFlyOffset, 0)),
			ScaleBy::create(vsMissPoint - vsFlyPoint, 2)
			, nullptr),
			CallFunc::create([=](){
			v->removeFromParent();
		}), nullptr);

		Action* sAction = Sequence::create(
			DelayTime::create(vsTimePoint),
			FadeIn::create(0.1),
			EaseBackInOut::create(MoveBy::create(vsStayPoint - vsTimePoint, ccp(-vsOffset - 40, 0))),
			DelayTime::create(vsFlyPoint - vsStayPoint),
			Spawn::create(FadeOut::create(vsMissPoint - vsFlyPoint),
			MoveBy::create(vsMissPoint - vsFlyPoint, ccp(vsFlyOffset, 0)),
			ScaleBy::create(vsMissPoint - vsFlyPoint, 2)
			, nullptr),
			CallFunc::create([=](){
			s->removeFromParent();
		}), nullptr);

		v->runAction(vAction);
		s->runAction(sAction);

		/**********************左右幕布处理**************************/
		Sprite * left = Sprite::create("bjl/fp/left.png");
		Sprite * right = Sprite::create("bjl/fp/right.png");
		left->setAnchorPoint(ccp(1, 0.5));
		right->setAnchorPoint(ccp(0, 0.5));
		left->setPosition(0, size.height / 2);
		right->setPosition(size.width, size.height / 2);
		transitionLayer->addChild(left, 2);
		transitionLayer->addChild(right, 2);
		Action* leftAction = Sequence::create(
			EaseSineIn::create(
			MoveBy::create(backStayPoint, ccp(size.width / 2 + 1, 0))),
			DelayTime::create(backFadePoint - backStayPoint),
			CallFunc::create([=](){
			        setVisible(true);
		    }),
			MoveBy::create(backMissPoint - backFadePoint, ccp(-size.width / 2 - 1, 0)),
			//FadeOut::create(backMissPoint - backFadePoint),
			CallFunc::create([=](){
			left->removeFromParent();
		}), nullptr);

		Action* rightAction = Sequence::create(
			EaseSineIn::create(MoveBy::create(backStayPoint, ccp(-size.width / 2 - 1, 0))),
			DelayTime::create(backFadePoint - backStayPoint), 
			//FadeOut::create(backMissPoint - backFadePoint),
			MoveBy::create(backMissPoint - backFadePoint, ccp(size.width / 2 + 1, 0)),
			CallFunc::create([=](){
			right->removeFromParent();
		}), nullptr);

		left->runAction(leftAction);
		right->runAction(rightAction);
	
	}


	if (playerId > 0){
		if (runAction){
			
			_nodePlayerInfoPlayer->spAvatar->setVisible(false);
			Sprite *player = Sprite::create();
			player->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(playerAvatarId, 1));
			transitionLayer->addChild(player, 3);
			player->setPosition(ccp(size.width/2-120,320));
			Vec2 stayPosition = player->getPosition();
			player->setPositionX(stayPosition.x - avatarOffset);

			Action *playerAction = Sequence::create(CCDelayTime::create(avatarTimePoint),
				CCFadeIn::create(0.01),
				//Spawn::create(EaseBackInOut::create(MoveBy::create(avatarStayPoint - avatarTimePoint, ccp(avatarOffset, 0))), FadeIn::create(0.2)),//平移
				EaseBackInOut::create(MoveBy::create(avatarStayPoint - avatarTimePoint, ccp(avatarOffset, 0))),
				CCDelayTime::create(avatarFlyPoint - avatarStayPoint),//停留
				MoveTo::create(avatarOriginPoint - avatarFlyPoint, ccp(400, 900)),//飞出
				CallFunc::create([=](){
				player->removeFromParent();
				_nodePlayerInfoPlayer->setPlayerNameVisible(true);
				_nodePlayerInfoPlayer->spAvatar->setVisible(true);
				_nodePlayerInfoPlayer->spAvatar->setScale(_scaleAvatar1);
				_nodePlayerInfoPlayer->spAvatar->setPosition(ccp(0, 0));
				_nodePlayerInfoPlayer->txtNickname->setPosition(ccp(0, 0));
				//手指点
			}),
				nullptr);
			player->runAction(playerAction);
		}
		else{
			_nodePlayerInfoPlayer->spAvatar->setPosition(ccp(0, 0));
			_nodePlayerInfoPlayer->spAvatar->setScale(_scaleAvatar1);
			_nodePlayerInfoPlayer->txtNickname->setPosition(ccp(0, 0));
			_nodePlayerInfoPlayer->txtNickname->setScale(_scaleAvatar1);
		}
	}
	
	if (bankerId>0){
		if (runAction){
	        
			_nodePlayerInfoBanker->spAvatar->setVisible(false);
			Sprite *banker = Sprite::create();
			banker->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(bankerAvatarId, 1));
			transitionLayer->addChild(banker, 3);
			banker->setPosition(ccp(size.width / 2 + 120, 320));
			Vec2 stayPosition = banker->getPosition();
			banker->setPositionX(stayPosition.x + avatarOffset);
			banker->setFlippedX(true);

			Action *playerAction = Sequence::create(CCDelayTime::create(avatarTimePoint),
				CCFadeIn::create(0.01),
				EaseBackInOut::create(MoveBy::create(avatarStayPoint - avatarTimePoint, ccp(-avatarOffset, 0))),//平移
				CCDelayTime::create(avatarFlyPoint - avatarStayPoint),//停留
				MoveTo::create(avatarOriginPoint - avatarFlyPoint, ccp(1520, 900)),//飞出
				CallFunc::create([=](){
				banker->removeFromParent();
				//vs 动画
				//手指点
				
				_nodePlayerInfoBanker->setPlayerNameVisible(true);
				_nodePlayerInfoBanker->spAvatar->setVisible(true);
				_nodePlayerInfoBanker->spAvatar->setScale(_scaleAvatar1);
				_nodePlayerInfoBanker->spAvatar->setPosition(ccp(0, 0));
				_nodePlayerInfoBanker->txtNickname->setPosition(ccp(0, 0));

				
				
			   }),
				nullptr);
			   banker->runAction(playerAction);
		}
		else{
			_nodePlayerInfoBanker->spAvatar->setPosition(ccp(0, 0));
			_nodePlayerInfoBanker->spAvatar->setScale(_scaleAvatar1);
			_nodePlayerInfoBanker->txtNickname->setPosition(ccp(0, 0));
			_nodePlayerInfoBanker->txtNickname->setScale(_scaleAvatar1);
		}
	}

	addGuide();

}



// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
void LayerBjlFpM::setFpSelect(int card){
	if (_enableControl)
		return;
	setCardStatus(card, 1);
}
// 翻牌中,处理服务端传来的翻牌消息
void LayerBjlFpM::setFping(int x1, int y1, int x2, int y2){
	if (_enableControl)
		return;
	//x1,代表翻牌方向
	setFlip(x1, x2, y2);
	_timeSpanFping = 0.0f;				//重置动画时间
	_ptStep.x = (x2 - fpPt1X) / TIMESPANFPING;		//计算移动步长
	_ptStep.y = (y2 - fpPt1Y) / TIMESPANFPING;		//计算移动步长
	calcPolygon();
	//CCLOG("fping: %f,%f" _ptStep.x, _ptStep.y);
}
// 开牌
void LayerBjlFpM::setFpOpen(int pos){
	if (_enableControl)
		return;
	_timeSpanFping = TIMESPANFPING;		//停止翻牌动画
	setCardStatus(pos, 2);
}
// 中途进入房间时，翻牌初始化信息
void LayerBjlFpM::setFpInit(int playerId, string playerName, int playerAvatarId, int bankerId, string bankerName, int bankerAvatarId, int myId, int status, double endTime, vector<int>cardStatus){
	setFpBegin(playerId, playerName, playerAvatarId, bankerId, bankerName, bankerAvatarId, myId, false);
	setFpStatus(status, endTime);
	_playerName = playerName.c_str();
	_bankerName = bankerName.c_str();
	if (cardStatus.size() != 6)
		return;
	for (int i = 0; i < 6; i++){
		setCardStatus(i, cardStatus[i]);
	}
}