#include "SpriteCmSub.h"
#include "SoundControl.h"
#include "comm.h"
SpriteCmSub* SpriteCmSub::create(std::string frameName, int amount, bool scale, Vec2 ptWin, Vec2 ptLose)
{
	SpriteCmSub *sprite = new(std::nothrow)SpriteCmSub();
	if (sprite && sprite->initWithSpriteFrameName(frameName))
	{
		sprite->autorelease();
		sprite->_isScale = scale;
		if (scale)
		{
			sprite->_scale0 = 4.0f;
			sprite->_scale1 = 1.0f;
			sprite->setScale(sprite->_scale0);
		}
		sprite->_actTime = 0.5f;
		sprite->Amount = amount;
		sprite->_ptWin = ptWin;
		sprite->_ptLose = ptLose;
		sprite->setPosition(ptWin);
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}
void SpriteCmSub::doBet(Vec2 pt,float delay)
{
	if (delay <= 0.0f)
	{
		//无延迟
		if (_isScale)
			runAction(Spawn::create(MoveTo::create(_actTime / 2, pt), ScaleTo::create(_actTime / 2, _scale1), nullptr));
		else
			runAction(MoveTo::create(_actTime / 2, pt));
		SoundControl::PlayEffect("sound/betarea.mp3");
	}
	else
	{
		//有延迟
		if (_isScale)
			runAction(Sequence::create(DelayTime::create(delay), Spawn::create(MoveTo::create(_actTime / 2, pt), ScaleTo::create(_actTime / 2, _scale1), CallFunc::create([](){SoundControl::PlayEffect("sound/betarea.mp3"); }), nullptr), nullptr));
		else
			runAction(Sequence::create(DelayTime::create(delay), Spawn::create(MoveTo::create(_actTime / 2, pt), CallFunc::create([](){SoundControl::PlayEffect("sound/betarea.mp3"); }), nullptr), nullptr));
	}
	
}
void SpriteCmSub::doBet(Vec2 pt)
{
	doBet(pt, 0.0f);
}
void SpriteCmSub::onWinDone()
{
	_eventDispatcher->dispatchCustomEvent("CmWinDone", this);
}
void SpriteCmSub::doWin(float delay)
{
	if (delay > 0)
	{
		if (_isScale)
			runAction(Sequence::create(DelayTime::create(delay), Spawn::create(MoveTo::create(_actTime, _ptWin), ScaleTo::create(_actTime, _scale0), nullptr), Hide::create(), CallFunc::create(CC_CALLBACK_0(SpriteCmSub::onWinDone, this)), nullptr));
		else
			runAction(Sequence::create(DelayTime::create(delay), MoveTo::create(_actTime, _ptWin), Hide::create(), CallFunc::create(CC_CALLBACK_0(SpriteCmSub::onWinDone, this)), nullptr));
	}
	else
	{
		if (_isScale)
			runAction(Sequence::create(Spawn::create(MoveTo::create(_actTime, _ptWin), ScaleTo::create(_actTime, _scale0), nullptr), Hide::create(), CallFunc::create(CC_CALLBACK_0(SpriteCmSub::onWinDone, this)), nullptr));
		else
			runAction(Sequence::create(MoveTo::create(_actTime, _ptWin), Hide::create(), CallFunc::create(CC_CALLBACK_0(SpriteCmSub::onWinDone, this)), nullptr));
	}
}
void SpriteCmSub::doTie(float delay)
{
    if(delay>0)
    {
        if (_isScale)
            runAction(Sequence::create(DelayTime::create(delay),Spawn::create(MoveTo::create(_actTime, _ptWin), ScaleTo::create(_actTime, _scale0), nullptr), Hide::create(), nullptr));
        else
            runAction(Sequence::create(DelayTime::create(delay),MoveTo::create(_actTime, _ptWin), Hide::create(), nullptr));
    }
    else
    {
        if (_isScale)
            runAction(Sequence::create(Spawn::create(MoveTo::create(_actTime, _ptWin), ScaleTo::create(_actTime, _scale0), nullptr), Hide::create(), nullptr));
        else
            runAction(Sequence::create(MoveTo::create(_actTime, _ptWin), Hide::create(), nullptr));
    }
}
void SpriteCmSub::doLose(float delay)
{
	if(delay>0){
        if (_isScale)
            runAction(Sequence::create(DelayTime::create(delay),Spawn::create(MoveTo::create(_actTime, _ptLose), ScaleTo::create(_actTime, _scale0 / 2), nullptr), Hide::create(), nullptr));
        else
            runAction(Sequence::create(DelayTime::create(delay),MoveTo::create(_actTime, _ptLose), Hide::create(), nullptr));
    }
    else{
        if (_isScale)
            runAction(Sequence::create(Spawn::create(MoveTo::create(_actTime, _ptLose), ScaleTo::create(_actTime, _scale0 / 2), nullptr), Hide::create(), nullptr));
        else
            runAction(Sequence::create(MoveTo::create(_actTime, _ptLose), Hide::create(), nullptr));
    }
}

SpriteCm::SpriteCm(int f, Sprite* sp, std::string imgon, std::string imgoff)
{
	amount = f;
	spCm = sp;
	imgOn = imgon;
	imgOff = imgoff;
}
void SpriteCm::SetSelected(bool isSelected)
{
	if (spCm)
	{
		spCm->setSpriteFrame(isSelected ? imgOn : imgOff);
	}
}

Sprite* SpriteCms::getCurrentCmSprite()
{
	if (!_curCm)
		return NULL;
	return _curCm->spCm;
}
void SpriteCms::setCurrentCm(Sprite* sp)
{
	for (SpriteCm* cm : spCms)
	{
		if (cm->spCm == sp)
		{
			_curCm = cm;
			break;
		}
	}
}

SpriteBetArea::SpriteBetArea(BJLBETTYPE tp, Sprite* area, Sprite* light, Text* text, Sprite* textBg,Sprite *green,Sprite *red)
{
	type = tp;
	amount = 0;
	spriteArea = area;
	spriteLight = light;
	textAmount = text;
	textAmount->setString("");
	this->green = green;
	this->red = red;
    this->textBg=textBg;
	//green->setOpacity(200);
	//red->setOpacity(220);
    this->textBg->setVisible(false);
	setSelectStatus(0);// 不显示
	/*//注册筹码动画完成事件
	//setEventDispatcher(new EventDispatcher());		//自定义新事件
	if (!_isRegEvent)
	{
	_eventDispatcher->addCustomEventListener("CmWinDone", CC_CALLBACK_1(SpriteBetArea::onCmWinDone, this));
	_isRegEvent = true;
	}*/
}

void SpriteBetArea::setSelectStatus(unsigned short select)
{
	this->green->stopAllActions();
	this->red->stopAllActions();
	switch (select)
	{
	case 0:{
		this->green->setVisible(false);
		this->red->setVisible(false);
		

	}break;
	case 1:{
		this->red->setVisible(false);
		this->green->setVisible(true);
		/*CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(1), FadeOut::create(2), nullptr));
		this->green->runAction(action);*/
	}break;
	case 2:{
		this->red->setVisible(true);
		this->green->setVisible(false);
	/*	CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(1), FadeOut::create(2), nullptr));
		this->red->runAction(action);*/
	}break;
	default:
		break;
	}
	
}
void SpriteBetArea::onCmWinDone(EventCustom* event)
{
	//筹码结束赢钱动画
	void*p = this;
	SpriteCmSub* cm = (SpriteCmSub*)event->getUserData();
}
void SpriteBetArea::Bet(SpriteCmSub* cm)
{
	if (!cm)
		return;
	_cmSubs.pushBack(cm);		//add sub cm to vector
	amount += cm->Amount;
	if (textAmount)
		textAmount->setText(Comm::GetFloatShortStringFromInt64(amount));

    if(textBg&&!textBg->isVisible())
        textBg->setVisible(true);
}
void SpriteBetArea::Win()
{
	//中奖
	if (spriteLight)
	{
		spriteLight->setVisible(true);
		spriteLight->runAction(Blink::create(2.0f, 3));
		/*
		DelayTime *delay = DelayTime::create(0.3f);
		Show *show = Show::create();
		Hide *hide = Hide::create();
		spriteLight->runAction(Sequence::create(show, delay, hide, delay, show, delay, hide, delay, show, delay, hide, delay, show, nullptr));*/
		//spriteLight->runAction(Sequence::create(FadeIn::create(0.5f), FadeOut::create(0.5f), FadeIn::create(0.5f), FadeOut::create(0.5f), FadeIn::create(0.5f), nullptr));
	}
	int size = _cmSubs.size();
	if (size <= 0)
		return;
	float delay = 0.0f;
	float step = _tmTotalCmSubsMove / size;
	for (SpriteCmSub* cm : _cmSubs)
	{
		cm->doWin(delay);
		delay += step;
	}
}
void SpriteBetArea::Lose()
{
	//输
	for (SpriteCmSub* cm : _cmSubs)
		cm->doLose();
}
void SpriteBetArea::Tie()
{
	//和
	for (SpriteCmSub* cm : _cmSubs)
		cm->doTie();
}
void SpriteBetArea::clear()
{
	//清除,子筹码在layerCM中一次性统一清除,此处不需要处理
	amount = 0;
	textAmount->setText("");
    textBg->setVisible(false);
	if (spriteLight)
		spriteLight->setVisible(false);
	_cmSubs.clear();
}

SpriteBetArea * SpriteBetAreas::getBetArea(BJLBETTYPE type)
{
	for (SpriteBetArea* area : betAreas)
	{
		if (area->type == type)
			return area;
	}
	return NULL;
}
SpriteBetArea * SpriteBetAreas::getBetArea(Sprite* sp)
{
	for (SpriteBetArea* area : betAreas)
	{
		if (area->spriteArea == sp)
			return area;
	}
	return NULL;
}
void SpriteBetAreas::addBetArea(BJLBETTYPE type, Sprite *area, Sprite *light, Text *text, Sprite* textBg, Sprite* green ,Sprite * red)
{
	betAreas.pushBack(new SpriteBetArea(type, area, light, text, textBg,green,red));
}
void SpriteBetAreas::addBet(Sprite* sp, SpriteCmSub* cm)
{
	if (!sp || !cm)
		return;
	SpriteBetArea *area = getBetArea(sp);
	if (!area)
		return;
	area->Bet(cm);
}
float SpriteBetAreas::getBetAmount(BJLBETTYPE type)
{
	SpriteBetArea *area = getBetArea(type);
	if (!area)
		return -1;		//出错
	return area->amount;
}
void SpriteBetAreas::clearBet()
{
	for (SpriteBetArea* area : betAreas)
	{
		area->clear();
	}
}
void SpriteBetAreas::setWinArea(BJLBETTYPE type)
{
	SpriteBetArea *area = getBetArea(type);
	if (!area)
		return;
	area->Win();
}
void SpriteBetAreas::setLoseArea(BJLBETTYPE type)
{
	SpriteBetArea *area = getBetArea(type);
	if (!area)
		return;
	area->Lose();
}
void SpriteBetAreas::setTieArea(BJLBETTYPE type)
{
	SpriteBetArea *area = getBetArea(type);
	if (!area)
		return;
	area->Tie();
}
//获取当前牌点数
int BjlPmInfo::getPoint(const char *rcards)
{
	if (rcards[0] == 'S' || rcards[0] == 'H' || rcards[0] == 'C' || rcards[0] == 'D')
	{
		if (rcards[1] >= '1'&&rcards[1] <= '9')
			return rcards[1] - '0';
		else if (rcards[1] == 'T' || rcards[1] == 'J' || rcards[1] == 'Q' || rcards[1] == 'K')
			return 0;
		else
			return -1;
	}
	else
		return -1;
}
void BjlPmInfo::clear()
{
	memset(cards, 0, 9);
	count = 0;
	point = 0;
}
bool BjlPmInfo::addCard(const char*rcards)
{
	if (!rcards)
		return false;
	int pt = getPoint(rcards);
	if (pt < 0)
		return false;
	strncpy(cards[count], rcards, 2);
	points[count] = pt;
	point = (point + pt) % 10;
	count++;
	return true;
}

bool BjlPmInfos::SetCards(const char* rcards)
{
	memset(Cards, 0, 13);
	if (!rcards)
		return false;
	int len = strlen(rcards);
	if (len < 8)
		return false;
	Player.clear();
	Banker.clear();
	if (!Player.addCard(rcards))
		return false;
	if (!Banker.addCard(rcards + 2))
		return false;
	if (!Player.addCard(rcards + 4))
		return false;
	if (!Banker.addCard(rcards + 6))
		return false;

	if (Player.point > 7 || Banker.point > 7)
	{
		//天生赢家
	}
	else if (Player.point > 5 && Banker.point > 5)
	{
		//叉烧
	}
	else if (Player.point<6)
	{
		//闲 0,1,2,3,4,5
		//庄 0,1,2,3,4,5,6,7
		//闲补牌
		if (len < 10)
			return false;
		Player.addCard(rcards + 8);
		if ((Banker.point<3)
			|| ((Banker.point == 3) && (Player.points[2] == 0 || Player.points[2] == 1 || Player.points[2] == 2 || Player.points[2] == 3 || Player.points[2] == 4 || Player.points[2] == 5 || Player.points[2] == 6 || Player.points[2] == 7 || Player.points[2] == 9))
			|| ((Banker.point == 4) && (Player.points[2] == 2 || Player.points[2] == 3 || Player.points[2] == 4 || Player.points[2] == 5 || Player.points[2] == 6 || Player.points[2] == 7))
			|| ((Banker.point == 5) && (Player.points[2] == 4 || Player.points[2] == 5 || Player.points[2] == 6 || Player.points[2] == 7))
			|| ((Banker.point == 6) && (Player.points[2] == 6 || Player.points[2] == 7)))
		{
			//庄补牌
			if (len < 12)
				return false;
			Banker.addCard(rcards + 10);
		}
	}
	else
	{
		//闲 6,7
		//庄 0,1,2,3,4,5
		//庄补牌
		if (len < 10)
			return false;
		Banker.addCard(rcards + 8);
	}
	strncpy(Cards, rcards, (Player.count + Banker.count) * 2);		//保留原始牌信息
	return true;
}
// 创建bjlcmcontrol,rectBetArea(p,pp,b,bp,t)
BjlCmControl* BjlCmControl::create(Node *rootNode, vector<CMINFO*> *cmInfos, vector<Vec2> ptPlayers, Vec2 ptBanker, vector<Rect> rectBetArea, float timeAward)
{
	if (!rootNode || !cmInfos || ptPlayers.size() != 9 || rectBetArea.size() != 5)
		return nullptr;
    //SpriteBatchNode *batchNode=SpriteBatchNode::create("bjl/room/PlistChips.png");
    //rootNode->addChild(batchNode);
	BjlCmControl *pRet = new BjlCmControl();
	pRet->_rootNode = rootNode;
	pRet->_maxCm = INT32_MIN;
	pRet->_minCm = INT32_MAX;
	pRet->_timeMove = 1.0f;
	pRet->_timeAward = timeAward;
	for (auto cmInfo : *cmInfos)
	{
		if (cmInfo)
		{
			if (((int)cmInfo->amount) > pRet->_maxCm)
				pRet->_maxCm = (int)cmInfo->amount;
			if (((int)cmInfo->amount) < pRet->_minCm)
				pRet->_minCm = (int)cmInfo->amount;
			pRet->_cmFrameNames[cmInfo->amount] = cmInfo->imgSmall;
		}
	}
	pRet->_ptPlayers = ptPlayers;
	pRet->_ptBanker = ptBanker;
	pRet->_rectBetArea[BJLBETTYPE::BJLBETP] = rectBetArea[0];
	pRet->_rectBetArea[BJLBETTYPE::BJLBETPP] = rectBetArea[1];
	pRet->_rectBetArea[BJLBETTYPE::BJLBETB] = rectBetArea[2];
	pRet->_rectBetArea[BJLBETTYPE::BJLBETBP] = rectBetArea[3];
	pRet->_rectBetArea[BJLBETTYPE::BJLBETT] = rectBetArea[4];
	return pRet;
}
//根据总投注额，计算筹码数量
bool BjlCmControl::calcAmount(int amount, int cmSize, vector<BETCMINFO> *cms)
{
	if (!cms || amount < _minCm)
		return false;
	int i = amount / cmSize;
	if (i > 0)
	{
		BETCMINFO cm;
		cm.amount = cmSize;
		cm.count = i;
		cms->push_back(cm);
		amount -= i*cmSize;
	}
	if (amount == 0)
		return true;
	return calcAmount(amount, cmSize / 10, cms);
}
bool BjlCmControl::addCm(int pos, BJLBETTYPE type, int amount)
{
	//计算筹码数量
	vector<BETCMINFO> cms;
	if (!calcAmount(amount, _maxCm, &cms) || cms.size() <= 0)
		return false;
	//获取投注区域
	Rect rectBetArea = _rectBetArea[type];
	float x0 = rectBetArea.origin.x;
	float x1 = x0 + rectBetArea.size.width;
	float y0 = rectBetArea.origin.y;
	float y1 = y0 + rectBetArea.size.height;
	SpriteCmSub *spCm;
	for (auto cm : cms)
	{
		for (int i = 0; i < cm.count; i++)
		{
			//生成筹码
			spCm = SpriteCmSub::create(_cmFrameNames[cm.amount], (float)cm.amount, false, _ptPlayers[pos], _ptBanker);
			if (!spCm)
				return false;
			//设置玩家位置与投注区域
			spCm->type = type;
			spCm->pos = pos;
			//加入数组与场景
			_spCms.push_back(spCm);
			_rootNode->addChild(spCm);
			Rect rect = spCm->getBoundingBox();
			Vec2 pt(random(x0, x1), random(y0, y1));		//计算随机位置
			//Vec2 pt(x0,y0);		//计算随机位置
			spCm->doBet(pt, (float)i*1.0f / cm.count);			//投注
		}
	}
	return true;
}
bool BjlCmControl::bet(map<int, BJLBETINFO*>* betDatas)
{
	if (!betDatas)
		return false;
	int pos;
	BJLBETINFO *betInfo;
	Vec2 ptWin;
	for (auto betData : *betDatas)
	{
		pos = betData.first;
		if (pos < 0 || pos>8)
			continue;
		betInfo = betData.second;
		if (!betInfo)
			continue;
		if (betInfo->p > 0)
			if (!addCm(pos, BJLBETTYPE::BJLBETP, betInfo->p))
				return false;
		if (betInfo->pp > 0)
			if (!addCm(pos, BJLBETTYPE::BJLBETPP, betInfo->pp))
				return false;
		if (betInfo->b > 0)
			if (!addCm(pos, BJLBETTYPE::BJLBETB, betInfo->b))
				return false;
		if (betInfo->bp > 0)
			if (!addCm(pos, BJLBETTYPE::BJLBETBP, betInfo->bp))
				return false;
		if (betInfo->t > 0)
			if (!addCm(pos, BJLBETTYPE::BJLBETT, betInfo->t))
				return false;
	}
	return true;
}
bool BjlCmControl::standup(int pos)
{
	if (pos < 1 || pos>8)
		return false;
	for (auto spCm : _spCms)
	{
		if (spCm&&spCm->pos == pos)
		{
			spCm->_ptWin = _ptPlayers[0];			//将所有筹码win位置，设置为无座玩家win位置
			spCm->pos = 0;							//将位置改为0,无座玩家
		}
	}
	return true;
}
void BjlCmControl::award(int wintype)
{
	float delayspan = 0;
	float delay = 0;
	if (_spCms.size() > 1)
		delayspan = (_timeAward - _timeMove) / (_spCms.size() - 1);		//动画间隔时间，以使筹码平均移动
	for (auto spCm : _spCms)
	{
		if (spCm->type&wintype)
		{
			//win
			spCm->doWin(delay);
		}
		else
		{
			//tie or lose
			if (wintype&BJLBETT)
			{
				//和,且押闲和庄时，doTie
				if (spCm->type&(BJLBETP | BJLBETB))
					spCm->doTie(delay);
				else
					spCm->doLose(delay);
			}
			else
			{
				//lose
				spCm->doLose(delay);
			}
		}
		delay += delayspan;
	}
}
void BjlCmControl::clear()
{
	_spCms.clear();
	_rootNode->removeAllChildren();
}