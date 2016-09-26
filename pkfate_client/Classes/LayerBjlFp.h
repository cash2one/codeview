#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UITextBMFont.h"
#include "ui/UIButton.h"
#include "SpriteCmSub.h"
#include "ui/UILayout.h"
#include "ui/UICheckBox.h"
#include "ui/UILoadingBar.h"
#include "LayerGuideMask.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;
class NodePlayerInfo
{
	TextBMFont *txtBMTime;
public:
	Node *node;
	Text *txtNickname;
	string name;
	Sprite *spAvatar;
	static NodePlayerInfo* create(Node *node);
	void setUser(int userid, string nickname, int avatarId);
	void setTimeVisible(bool visible);
	void setPlayerNameVisible(bool visible);
	void setTimeString(int seconds);
	void changeAvatar(int avatarId, int index);
	int getX();
	string getName();
};
enum FPFX{ LT, RT, LB, RB };
enum FPCARD{ FPLEFT1, FPLEFT2, FPLEFT3, FPRIGHT1, FPRIGHT2, FPRIGHT3 };
class LayerBjlFp :public Layer,GuideHandler
{
protected:
	Layer * transitionLayer;
	bool _hasShowResult;
	Node *_nodeRoot;
	NodePlayerInfo *_nodePlayerInfoPlayer, *_nodePlayerInfoBanker;
	Button *_btExit;
	Text *_textInfo, *_textLeftPt, *_textRightPt, *_openNot, *_timeTip;
	TextBMFont *_timeNum;
	//LoadingBar* _leftTimeProgress;
	CCProgressTimer *_leftTimeProgress;
	Sprite *_spCards[6], *_spCard0, *_spCard1, *_spSelectedCardBg, *_progressPoint, *_bgPlayer, *_bgBanker, *_finger,*_progressBg;
	int _fxCards[6];		//6 张牌的方向，随机生成
	int _statusCards[6];		//6张牌的状态,0关闭，1选中，2打开
	int _curPos;			// 当前选中的牌
	Vec2 polygonCardBack0[5], polygonCardBack1[5];		//多边形数组，最多5边	
	DrawNode *stencilCard0, *stencilCard1;			//卡牌多边形遮罩层
	ClippingNode *clipCard0, *clipCard1;				//卡牌clip
	Rect rectCard;
	Rect rectCardFp;	//翻牌触摸区大小,略大于实际牌面
	Rect rectMaxFp;		//翻牌最大区域，超过此区域时，直接开牌
	void calcPolygon();
	void fixZB();
	float mirrorX(float x, float m);				//获取镜像坐标
	float fpPt0X, fpPt0Y, fpPt1X, fpPt1Y;	//翻牌的起点和终点
	int _fpFx;
	float fpSize;	//翻牌角尺寸
	bool drag;		//是否开始翻
	bool _isAutoExit;		//是否允许自动退出
	bool _isShowTip;
	BjlPmInfos *_pmInfos;
	string getCard(int pos);
	virtual void onButtonExitClicked(Ref *ref);
	virtual bool onTouchBegan(Touch* touch, Event* event);
	virtual void onTouchMoved(Touch* touch, Event* event);
	virtual void onTouchEnded(Touch* touch, Event* event);
	virtual void onClickCard(int pos);		//点击某张牌
	void checkCard();		//检查牌面情况
	virtual void enableTip(bool enable);
	virtual void turnViewHandler(bool isPlayer);

	void onlyOpenCardClick(int index);

	int onlyOpenCardIndex;
	

	virtual void addGuide();

public:
	CREATE_FUNC(LayerBjlFp);
	virtual bool init();
	LayerBjlFp();
	~LayerBjlFp();
	
	void setCards(BjlPmInfos *infos);
	void setCardStatus(int pos, int status);
	void setSelect(int pos);
	void setFlip(int fpfx, float x, float y);
	virtual void clear();
};
class LayerBjlFpS :public LayerBjlFp
{
private:
	bool _isPlayer;
protected:
	virtual void onButtonExitClicked(Ref *ref);
	virtual void onClickCard(int pos);
	virtual void enableTip(bool enable);
public:
	CREATE_FUNC(LayerBjlFpS);
	void setCards(BjlPmInfos *infos, bool isPlayer);
};
class LayerBjlFpM :public LayerBjlFp
{
protected:
	Layout *_layoutVS;
	bool _enableControl;		//是否允许控制
	int _fpStatus;				//当前状态,[0:闲开牌, 1:庄开牌, 2:闲补牌, 3:庄补牌]
	double _endTime;			//当前状态结束时间
	int _playerId, _bankerId, _myId;		//闲ID，庄ID，本人ID
	void updateTime(float dt);
	bool _isPlayer;
	Action *progressAction;
	
	string  _playerName,_bankerName;
	Vec2 _ptStep;
	Vec2 _ptPlayerAvatar, _ptPlayerNickName, _ptBankerAvatar, _ptBankerNickName;		//头像原始及目标位置
	float _scaleAvatar0, _scaleAvatar1;		//头像缩放原始及目标值
	float _timeSpanFping, _timeSpanMove;
	virtual void onButtonExitClicked(Ref *ref);
	virtual void onClickCard(int pos);
	virtual bool init();
	virtual bool onTouchBegan(Touch* touch, Event* event);
	virtual void onTouchMoved(Touch* touch, Event* event);
	virtual void onTouchEnded(Touch* touch, Event* event);
	virtual void update(float dt);
	virtual void enableTip(bool enable);
	virtual void turnViewHandler(bool isPlayer);
	
public:
	CheckBox *_checkFP;		//是否翻牌控件
	CREATE_FUNC(LayerBjlFpM);
	void setFpBegin(int playerId, string playerName, int playerAvatarId, int bankerId, string bankerName, int bankerAvatarId, int myId, bool runAction = true);	// 开始翻牌
	void setFpStatus(int status, double endTime);					// 设置翻牌状态
	void setFpSelect(int card);						// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
	void setFping(int x1, int y1, int x2, int y2);	// 翻牌中
	void setFpOpen(int pos);						// 开牌
	void setFpInit(int playerId, string playerName, int playerAvatarId, int bankerId, string bankerName, int bankerAvatarId, int myId, int status, double endTime, vector<int>cardStatus);	// 中途进入房间时，翻牌初始化信息
	void setNeedFp(bool needFp);		//设置是否需要手动翻牌
	virtual void clear();		//清理控件,恢复默认
};