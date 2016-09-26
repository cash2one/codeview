#include "LayerGuideMask.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
#include "AudioEngine.h"
#include "time.h"
#include "SimpleAudioEngine.h"
#include "PKNotificationCenter.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID||(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32))
#include "CCVideoPlayer.h"
#endif

using namespace CocosDenshion;
using namespace experimental;

#define max(a, b)  (((a) > (b)) ? (a) : (b))
static LayerGuideMask *m_pInstance = nullptr;
LayerGuideMask::LayerGuideMask():clickHandler(nullptr), _textSay(nullptr){}

// on "init" you need to initialize your instance
bool LayerGuideMask::init()
{
	
	if (!Layer::init())
	{
		return false;
	}


	/*CCLayerColor* back = CCLayerColor::create(ccc4(0, 0, 0, 80));

	this->addChild(back);*/

	this->_clickData = nullptr;
	this->clip = nullptr;
	this->idSound = -1;
	this->video_x = 20;
	this->video_y = 0;

	listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){

		if (_waitRect.containsPoint(touch->getLocation())){

			handlerWaitTouch();
		}

		if (this->_clickData != nullptr){
			Vec2 p = touch->getLocation();
			checkClick(p);
		}
		return true;
	};

	Button* jumpBtn = Button::create("common/btn_green_normal.png", "common/btn_green_normal.png");
	jumpBtn->setAnchorPoint(ccp(1, 0));
	this->addChild(jumpBtn, 4);
	jumpBtn->setOpacity(220);
	jumpBtn->setTitleFontName("font/msyhbd.ttf");
	jumpBtn->setTitleFontSize(48);
	jumpBtn->setTitleText(Language::getStringByKey("JumpGuide"));
	jumpBtn->setScale(0.6);
	jumpBtn->setPosition(ccp(1920, 0));

	jumpBtn->addClickEventListener([=](Ref* pSender){
		LayerGuideMask::GetInstance()->close();
		GuideControl::GetInstance()->finishGuideStep();
		PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GuideControl::GetInstance()->current_tech_game_type)));
	});

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	return true;
}


void LayerGuideMask::close()
{
	this->cleanTech();
	this->removeFromParent();
}

void LayerGuideMask::setSwallowTouches(bool is)
{
	listener->setSwallowTouches(is);
}

void LayerGuideMask::checkClick(Vec2 p)
{
	if (abs(p.x - this->_clickData->pos.x) < this->_clickData->width / 2 && abs(p.y - this->_clickData->pos.y) < this->_clickData->height / 2){
		std::function<void()>  nextHandler = this->_clickData->handler;
		
		if (nextHandler)
		{   
			cleanTech();
			nextHandler();
		}
	}
}


void LayerGuideMask::cleanVideo()
{

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID||(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32))
	CCVideoPlayer* player = this->getChildByName<CCVideoPlayer*>("CCVideoPlayer");
	if (player) {
		player->closeVideo();
		this->removeChild(player);
	}
#endif

}
void LayerGuideMask::cleanTech()
{   
	cleanVideo();
	if (clip){ clip->removeFromParent(); clip = nullptr; }
	this->_clickData = nullptr;
	_textSay->setVisible(false);
	_maskLayer->removeAllChildren();
}

void LayerGuideMask::removeFromMask(string name)
{
	_maskLayer->removeChildByName(name);
}

void LayerGuideMask::cleanMask()
{
	_maskLayer->removeAllChildren();
}

GuideMessage* LayerGuideMask::getTextSay()
{
	return _textSay;
}

void LayerGuideMask::clickAction(TechData* data)
{
	if (data == nullptr || data->width == 0 || data->height == 0) return;

	wait();//等待
	

	clip = ClippingNode::create();  //设置裁剪节点
	clip->setInverted(true);  //设置ALPHA镂空的阈值， ture:裁剪区域镂空，其他区域显示底板颜色 false：裁剪区域显示底板颜色，其他区域镂空
	clip->setAlphaThreshold(1000.5f); //设置ALPHA的测试参考值   //奇怪的东东  /设置alpha值（0~1），这个很重要，裁剪是按像素抠图的，所以只有大于这个alpha值的模版像素才会被画出来  
	//默认是1，也就是完全裁剪。  
	this->addChild(clip, 1); //创建一个剪辑区域

	CCLayerColor* back = CCLayerColor::create(ccc4(0, 0, 0, 160));
	clip->addChild(back);//给clip加一个颜色层，clip不受影响，其他区域有层级遮挡


	this->_clickData = data;

	Size visibleSize = Director::getInstance()->getVisibleSize();
	float display_width = visibleSize.width;
	float display_height = visibleSize.height;

	//
	////  以下模型是drawnode遮罩
	
	ccColor4F fill = { 1.0, 0.0, 0.0, 0.2f };

	ccColor4F backBorder = { 0.5, 0.0, 1.0, 0.0 };

	float center_x = data->pos.x - display_width / 2;
	float center_y = data->pos.y - display_height / 2;

	CCPoint rect[4] = { ccp(center_x - data->width / 2, center_y + data->height / 2),
		ccp(center_x + data->width / 2, center_y + data->height / 2),
		ccp(center_x + data->width / 2, center_y - data->height / 2),
		ccp(center_x - data->width / 2, center_y - data->height / 2) };

	CCDrawNode* front = CCDrawNode::create();

	front->drawPolygon(rect, 4, fill, 12, backBorder); //绘制四边形
	front->setPosition(ccp(visibleSize.width / 2, visibleSize.height / 2));

	clip->setStencil(front); //一定要有，设置裁剪模板

	//front = CCDrawNode::create();

	//front->drawPolygon(rect, 4, fill, 1, backBorder); //绘制四边形
	//front->setPosition(ccp(visibleSize.width / 2, visibleSize.height / 2));
	//this->addChild(front);
	//front->setName("front");

	
}


//字幕
void LayerGuideMask::sayAction(TechData* data)
{
	if (data == nullptr || data->res.empty() ) return;

	if (_textSay == nullptr){
		_textSay = GuideMessage::create();
	    _textSay->retain();
		
		//demand changes
		_textSay->setAnchorPoint(ccp(0.5, 0));
		this->addChild(_textSay,10);
	}
	_textSay->setVisible(true);
	_textSay->setPosition(ccp(data->pos.x, data->pos.y));
	_textSay->setTip(Language::getStringByKey(data->res.c_str()));
}
//声音
void LayerGuideMask::soundAction(TechData* data)
{
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	if (data == nullptr || data->res.empty()) return;

	//if (this->idSound != -1){
	//	AudioEngine::stop(this->idSound);
	//}
	//this->idSound = AudioEngine::play2d(data->res, false, 0.5f);
	//SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);
	SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(1);
	SimpleAudioEngine::getInstance()->playBackgroundMusic(data->res.c_str(), false);
//#endif
}

//视频
void LayerGuideMask::videoAction(TechData* data)
{
	if (data == nullptr || data->res.empty()) return;
    
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID||(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32))
    CCVideoPlayer* player = this->getChildByName<CCVideoPlayer*>("CCVideoPlayer");
    if (player) {
        player->closeVideo();
        this->removeChild(player);
    }
    player = CCVideoPlayer::create(data->res.c_str());
    if(player)
    {
		this->video_x = data->pos.x;
		this->video_y = data->pos.y;
		player->setPosition(ccp(data->pos.x, data->pos.y));
        player->setContentSize(Size(player->m_width,player->m_height));
		player->setAnchorPoint(ccp(0, 0));
        this->addChild(player,9);
        
        player->setName("CCVideoPlayer");
		player->playVideo();
    }
#endif
    
}

void LayerGuideMask::wait()
{
	
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID||(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32))
	
	string wait_url = GuideControl::GetInstance()->getGuideWaitVideo();
	if(wait_url.empty())return;

	Vec2 pos = ccp(this->video_x,this->video_y);
	CCVideoPlayer* player = this->getChildByName<CCVideoPlayer*>("CCVideoPlayer");
	if (player) {
		if (player->getVideoPath() == wait_url){
			return;
		}

		pos = player->getPosition();
		player->closeVideo();
		this->removeChild(player);
	}
	player = CCVideoPlayer::create(wait_url.c_str(),true);
	if (player)
	{
		player->setPosition(pos);
		player->setContentSize(Size(player->m_width, player->m_height));
		player->setAnchorPoint(ccp(0, 0));
		this->addChild(player, 9);
		player->setName("CCVideoPlayer");
        
		player->playVideo();

		_waitRect = CCRectMake(pos.x, pos.y, player->m_width, player->m_height);
	}
#endif
	
}

void LayerGuideMask::handlerWaitTouch(){
	
	CCLOG("等待视频被点击 %d", 1);
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//	_waitRect = CCRectMake(0,0,0,0);
//	Vec2 pos = ccp(0, 0);
//	CCVideoLayer* player = this->getChildByName<CCVideoLayer*>("CCVideoLayer");
//	if (player) {
//		pos = player->getPosition();
//		player->closeVideo();
//		this->removeChild(player);
//	}
//    playKavayiVideo(pos);
//
// 
//#endif
	
	
}
                                                                                       
void LayerGuideMask::playKavayiVideo(Vec2 pos)
{
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID||(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32))
   
    srand((unsigned)time(NULL));
    int i = 1 + rand() % 4;
    auto player = CCVideoPlayer::create(StringUtils::format("guide/video/t%d.mp4",i).c_str());
    if (player)
    {
        player->setPosition(ccp(40,40));
        player->setContentSize(Size(player->m_width, player->m_height));
        player->setAnchorPoint(ccp(0, 0));
        this->addChild(player, 9);
        player->setName("CCVideoPlayer");
        
        player->setVideoEndCallback([=](){
            wait();
        });
        player->playVideo();
    }
#endif

}


//教学
void LayerGuideMask::techAction(std::vector<TechData*> list)
{
	if (list.empty()) return;
	SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);
	for (TechData* techData : list)
	{
		switch (techData->type)
		{
		case TECHTYPE::VIDEO:
		{
			if (!techData->res.empty()){
				string soundUrl = techData->res.substr(0,techData->res.find(".mp4"))+".mp3";
				SimpleAudioEngine::getInstance()->preloadBackgroundMusic(soundUrl.c_str());
			}
			
			runAction(Sequence::createWithTwoActions(DelayTime::create(techData->delay), CallFunc::create([=](){
				videoAction(techData);
			})));
			
		}
			break;
		case TECHTYPE::SOUND:
		{
			runAction(Sequence::createWithTwoActions(DelayTime::create(techData->delay), CallFunc::create([=](){
				soundAction(techData);
			})));
		}
			break;
		 case TECHTYPE::SAY:
		{
			runAction(Sequence::createWithTwoActions(DelayTime::create(techData->delay), CallFunc::create([=](){
				sayAction(techData);
			})));
			
		}
			break;
		 case TECHTYPE::CLICK:
		{
			 runAction(Sequence::createWithTwoActions(DelayTime::create(techData->delay), CallFunc::create([=](){
				 clickAction(techData);
			 })));
		}
			break;
		 case TECHTYPE::HAND:
		 {
			 runAction(Sequence::createWithTwoActions(DelayTime::create(techData->delay), CallFunc::create([=](){
				 if (techData->handler != nullptr){
					 if (techData->wait){
						 wait();
					 }
					 techData->handler();
				 }
			 })));
		 }
			 break;
		default:
			break;
		}
	}
}

bool LayerGuideMask::validateStep(int step)
{
	return GuideControl::GetInstance()->checkInGuide(step);
}

//播放教学 step  1,2,3
void LayerGuideMask::playTechAction(int step,Node* content)
{
	if (!validateStep(step)) return;
	if (content == nullptr) {
		if (!m_pInstance->getParent()) return;
	}
	else{
		if (m_pInstance->getParent()){

			m_pInstance->removeFromParent();

			content->addChild(m_pInstance, 100);
		}
		else{
			content->addChild(m_pInstance, 100);
		}
	}
	_waitRect = CCRectMake(0,0,0,0);
	setSwallowTouches(true);
	std::vector<TechData*>* data = GuideControl::GetInstance()->getTechData(step);
	if (data != nullptr){
		techAction(*data);
	}

	GuideControl::GetInstance()->uploadGameStep(step);
}

Sprite* LayerGuideMask::addArrow(DIRECTION direction, Vec2 p)
{
	Sprite* arrow = Sprite::create("guide/green_arrow_v.png"); //默认朝下
	arrow->setPosition(p);
	addTo(arrow);
	switch (direction)
	{
	case LEFT:
		arrow->setRotation(90);

		arrow->runAction(
			RepeatForever::create(Sequence::createWithTwoActions(MoveBy::create(0.3f, ccp(-10, 0)), MoveBy::create(0.3f, ccp(10, 0)))));

		break;
	case TOP:
		arrow->setFlippedY(true);

		arrow->runAction(
			RepeatForever::create(Sequence::createWithTwoActions(MoveBy::create(0.3f, ccp(0, 10)), MoveBy::create(0.3f, ccp(0, -10)))));
		break;
	case RIGHT:
		arrow->setRotation(-90);

		arrow->runAction(
			RepeatForever::create(Sequence::createWithTwoActions(MoveBy::create(0.3f, ccp(10, 0)), MoveBy::create(0.3f, ccp(-10, 0)))));

		break;
	case BOTTOM:
		arrow->runAction(
			RepeatForever::create(Sequence::createWithTwoActions(MoveBy::create(0.3f, ccp(0, -10)), MoveBy::create(0.3f, ccp(0, 10)))));
		break;
	case LEFT_TOP:
		arrow->setFlippedY(true);
		arrow->setRotation(-45);
		arrow->runAction(
			RepeatForever::create(Sequence::createWithTwoActions(MoveBy::create(0.3f, ccp(-10, 10)), MoveBy::create(0.3f, ccp(10, -10)))));
	default:
		break;
	}
	return arrow;
}



void LayerGuideMask::addTo(Node* nodeMask)
{
    if (!nodeMask) return;
	nodeMask->setVisible(true);
	if (nodeMask->getParent() != _maskLayer){
		nodeMask->retain();
		nodeMask->removeFromParent();
		_maskLayer->addChild(nodeMask);
		nodeMask->release();
	}
}

void LayerGuideMask::setClickHandler(std::function<void()>  clickHandler)
{
	this->clickHandler = clickHandler;
}

LayerGuideMask* LayerGuideMask::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = LayerGuideMask::create();
		m_pInstance->setName("LayerGuideMask");
		m_pInstance->_maskLayer = Layer::create();
		m_pInstance->addChild(m_pInstance->_maskLayer, 20);
		m_pInstance->retain();
	}
	return m_pInstance;
}

Layer* LayerGuideMask::getMaskLayer()
{
	return _maskLayer;
}


void LayerGuideMask::textArrow(const std::string& text, const Vec2 &position, bool isLeft, DIRECTION dir)
{
	GuideMessage *textSay = GuideMessage::create();
	textSay->setTipColor(ccc4(0, 255, 0, 255));
	textSay->setTip(text);
	textSay->setPosition(position);
	textSay->setAnchorPoint(ccp(0.5, 0.5));
	textSay->getTipLayoutBg()->setVisible(false);
	
	Vec2 arrowPos;
	Vec2 arrowBgPos;

	if (isLeft)
	{
		arrowPos = ccp(textSay->getPosition().x - textSay->getContentSize().width / 2 - 30,
			textSay->getPosition().y);
	}
	else
	{
		arrowPos = ccp(textSay->getPosition().x + textSay->getContentSize().width / 2 + 30,
			textSay->getPosition().y);
	}

	Sprite* sprite = addArrow(dir, arrowPos);

	Size size = Size(sprite->getContentSize().width + textSay->getContentSize().width + 20,
		sprite->getContentSize().height + 20);

	Layout* arrowBg = Layout::create();
	arrowBg->setBackGroundColor(Color3B::BLACK);
	arrowBg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);

	if (isLeft)
	{
		arrowBg->setPosition(ccp(sprite->getPosition().x - 50 - sprite->getContentSize().width / 2,
			sprite->getPosition().y - 10 - sprite->getContentSize().height / 2));
	}
	else
	{
		arrowBg->setPosition(ccp(textSay->getPosition().x - textSay->getContentSize().width / 2 + 10,
			sprite->getPosition().y - 10 - sprite->getContentSize().height / 2));
	}

	arrowBg->setBackGroundColorOpacity(100);
	arrowBg->setAnchorPoint(ccp(0, 0));
	arrowBg->setContentSize(size);

	getMaskLayer()->addChild(arrowBg, sprite->getZOrder()-1);
	getMaskLayer()->addChild(textSay);
}


GuideMessage::GuideMessage() :x_space(70), y_space(18)
{
	//demand changes
	//tipBg = ImageView::create("img/bg_1240_110.png");
	//tipBg->setContentSize(Size(size.width, 95));
	//tipBg->setAnchorPoint(ccp(0, 0));
	//tipBg->setScale9Enabled(true);
	//unsigned int w = 409;
	//unsigned int h = 95;
	//tipBg->setCapInsets(CCRectMake(w, h, 1240 - w * 2, 110 - h * 2));
	//this->addChild(tipBg);

	Size size = Director::getInstance()->getWinSize();
	tipMsg = Text::create();
	tipMsg->setFontName("font/msyhbd.ttf");

	tipMsg->setAnchorPoint(ccp(0.5, 0.5));
	tipMsg->setTextColor(ccc4(255, 255, 0, 255));
	tipMsg->setFontSize(36);
	
	tipLayoutBg = Layout::create();
	tipLayoutBg->setBackGroundColor(Color3B::BLACK);
	tipLayoutBg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
	tipLayoutBg->setPosition(ccp(0, 0));
	tipLayoutBg->setBackGroundColorOpacity(200);
	//tipLayoutBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	this->addChild(tipLayoutBg);
	this->addChild(tipMsg);
	
}

void GuideMessage::setTipColor(const Color4B color)
{
	tipMsg->setTextColor(color);
}

void GuideMessage::setTip(std::string tip)
{
	tipMsg->setText(tip);
	Size size = tipMsg->getContentSize();
	//int width = max(size.width + 2 * x_space, 1240);
	//int width = max(size.width + 2 * x_space, 1000);
	int width = size.width + x_space;
	//tipBg->setContentSize(CCSizeMake(width, size.height + 2 * y_space));
	setTipContentSize(CCSizeMake(width, size.height + 2 * y_space));
}

void GuideMessage::setTipContentSize(const cocos2d::Size &contentSize)
{
	tipLayoutBg->setContentSize(contentSize);
	Size size2 = tipLayoutBg->getContentSize();
	this->setContentSize(size2);
	tipMsg->setPosition(ccp((size2.width) / 2, (size2.height) / 2));
}

Layout* GuideMessage::getTipLayoutBg()
{
	return tipLayoutBg;
}