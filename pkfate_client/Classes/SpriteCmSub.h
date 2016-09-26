#pragma once
#include "cocos2d.h"
#include "comm.h"
#include "bjlcomm.h"
#include "ui/UIText.h"
USING_NS_CC;
using namespace std;
class SpriteCmSub :public Sprite
{
	Vec2 _ptLose;				//筹码目标位置_输
	bool _isScale;				//是否缩放
	float _scale0;				//原始大小
	float _scale1;				//目标大小	
	float _actTime;				//动画时间
	void onWinDone();			//结束赢钱动画时
public:
	Vec2 _ptWin;				//筹码目标位置_赢
	BJLBETTYPE type;			//投注类型
	int pos;					//所属玩家位置
	int Amount;				//对应金额
	//初始化子筹码精灵 筹码缓存名称,金额,母筹码,赢时筹码目标位置,输时筹码目标位置
	static SpriteCmSub* create(std::string frameName, int amount, bool scale, Vec2 ptWin, Vec2 ptLose);
	void doBet(Vec2 pt);		//投注,筹码位置
	void doBet(Vec2 pt, float delay);		//投注,筹码位置
    void doWin(float delay=0.0f);				//赢,cancel时,也使用这个动画
    void doTie(float delay=0.0f);				//和
    void doLose(float delay=0.0f);				//输
};

class BjlCmControl
{
private:
	struct BETCMINFO
	{
		int amount;
		int count;
	};
	int _maxCm, _minCm;
	map<int, string> _cmFrameNames;
	vector<Vec2> _ptPlayers;
	Vec2 _ptBanker;
	Node *_rootNode;
	float _timeMove, _timeAward;
	vector<SpriteCmSub*> _spCms;
	map<BJLBETTYPE, Rect> _rectBetArea;
	bool addCm(int pos, BJLBETTYPE type, int amount);
	bool calcAmount(int amount, int cmSize, vector<BETCMINFO> *cms);
public:
	static BjlCmControl* create(Node *rootNode, vector<CMINFO*> *_cmInfos, vector<Vec2> ptPlayers, Vec2 ptBanker, vector<Rect> rectBetArea, float timeAward);
	bool bet(map<int, BJLBETINFO*>* betDatas);
	bool standup(int pos);
	void award(int wintype);
	void clear();
};
class SpriteCm :public Ref
{
public:
	int amount;			//筹码对应的金额
	Sprite *spCm;			//筹码精灵
	std::string imgOn, imgOff;
	SpriteCm(int f, Sprite* sp, std::string imgon, std::string imgoff);
	void SetSelected(bool isSelected);
};
class SpriteCms
{
public:
	Vector<SpriteCm *>spCms;			//³ïÂë¾«Áé×é
	SpriteCm *_curCm = NULL;				//µ±Ç°¾«Áé
	Sprite* getCurrentCmSprite();
	void setCurrentCm(Sprite* sp);
};
class SpriteBetArea :public Node
{
	Vector<SpriteCmSub*> _cmSubs;	//子筹码
	float _tmTotalCmSubsMove = 2.0f;		//全部筹码移动需要的时间	
public:
	BJLBETTYPE type;		//投注类型
	int64_t amount;		//当前投注额
	Text *textAmount;	//投注额控件
    Sprite *textBg;     //投注额背景图片
	Sprite *spriteArea, *spriteLight;	//投注区域和高亮区域 sprite
	Sprite *red, *green;
	SpriteBetArea(BJLBETTYPE tp, Sprite* area, Sprite* light, Text* text, Sprite* textBg,Sprite*red,Sprite *green);
	void setSelectStatus(unsigned short select);
	void onCmWinDone(EventCustom* event);
	void Bet(SpriteCmSub* cm);
	void Win();
	void Lose();
	void Tie();
	void clear();
};
class SpriteBetAreas
{
public:
	SpriteBetArea *getBetArea(BJLBETTYPE type);
	Vector<SpriteBetArea*> betAreas;
	SpriteBetArea *getBetArea(Sprite* sp);
	void addBetArea(BJLBETTYPE type, Sprite *area, Sprite *light, Text *text, Sprite* textBg, Sprite*red, Sprite *green);
	void addBet(Sprite* sp, SpriteCmSub* cm);
	float getBetAmount(BJLBETTYPE type);
	void clearBet();
	void setWinArea(BJLBETTYPE type);
	void setLoseArea(BJLBETTYPE type);
	void setTieArea(BJLBETTYPE type);
};
class BjlPmInfo
{
	//获取当前牌点数
	static int getPoint(const char *rcards);
public:
	char cards[3][3];		//最多3张牌
	int points[3];			//每张牌点数	
	int point;				//总点数
	int count;				//牌数量
	void clear();
	bool addCard(const char*rcards);
};
// 牌面信息
class BjlPmInfos
{
public:
	BjlPmInfo Banker, Player;
	char Cards[13];
	bool SetCards(const char* rcards);
};