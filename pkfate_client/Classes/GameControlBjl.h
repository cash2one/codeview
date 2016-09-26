#pragma once
#include "cocos2d.h"
#include "ui/UIButton.h"
#include "ui/UIText.h"
#include "comm.h"
#include "bjlcomm.h"
#include "cJSON.h"
#include "ui/UIImageView.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;
struct BjlRoomType{
	int type_id;
	int max_bet;
	int players_limit;
	bool auto_show_cards;
	int reward_seconds;
	int bet_seconds;
	int next_game_seconds;
	int show_cards_seconds;
};
struct RoomInfoBjl{
	int id;
	int count;
	int roomType;
	BjlRoomType *type;
};
struct BjlRoomDataRound
{
	int num;
	string secret, seed, data;
};
struct BjlRoomData
{
	string uuid;
	string secret;
	int inning_num;
	string inning_secret;
	vector<BjlRoomDataRound> history_rounds;
};
struct BjlRoomPlayer
{
	int id;
	int avatar;
	string nickname;
	int64_t balance;
};
struct BjlRoomSitInfo
{
	int pos;
	int userid;
	int64_t balance;
};
struct ChatData
{
	int id;
	string msg;
};

//投注返回数据格式
struct BetResultData
{
	int bonus, next_game_num;
	string secret, seed, next_secret, data, balance;
	bool is_end;
};

struct BJLBETPROFIT{
	int pos;
	int64_t balance;
	int64_t amount;
};

struct BjlBet{
	BJLBETTYPE m_type;
	int64_t m_amount;
	int m_gameNum;
};

class GameControlBjl :public Ref
{
public:
	static GameControlBjl* GetInstance();
	bool InitRoomTypesInfoData(std::string jsonData);
	bool InitRoomsInfoData(std::string jsonData);
	map<int, BjlRoomType *> RoomTypeInfos;
	map<int, RoomInfoBjl *> RoomInfos;
	map<int, string> GetResultsFromData(string jsonData);
	vector<int> JsonResults2BetType(string jsonResults);
	int UpdateRoomInfo(string jsonData);
	//多人游戏函数
	BjlRoomData *InitRoomData(string jsonData);
	BetResultData *InitBetResultData(string jsonData);
	bool InitBeginBetData(string jsonData, BjlRoomData *roomData, double *time);
	bool InitShowCardsData(string jsonData, BjlRoomDataRound *roundData, double *time,bool *isFlip);
	bool InitRoundOverData(string jsonData, BjlRoomDataRound *roundData, double *time);
	bool InitCutCardData(string jsonData, BjlRoomDataRound *roundData, double *time);
	bool InitRoomPlayers(string jsonData, map<int, BjlRoomPlayer*> *roomPlayers);
	bool InitSitPlayers(string jsonData, vector<BjlRoomSitInfo> *roomPlayers);
	bool InitBjlBetInfo(string jsonData, BJLBETINFO *betInfo);
	bool InitBjlBetInfo(cJSON *jsonMsg, BJLBETINFO *betInfo);
	bool InitBjlRoomBet(string jsonData, map<int, BJLBETINFO*> *roomBet);
	bool InitBetResultInfo(string jsonData, int64_t *pBalance, int *pNum, BJLBETINFO *pBetInfo);
	bool InitBetProfits(string jsonData, vector<BJLBETPROFIT> *pBetProfits);
	bool InitChatData(string jsonData, ChatData *chatData);
	bool InitGameRoundData(string jsonData, GameRoundDataBjl* gameRoundData);
	//单人游戏函数
	BjlRoomData *InitRoomHistoryData(string jsonData);
	static bool InitFpBeginData(string jsonData, int *playerid, int *bankerid);		// 开始翻牌消息
	static bool InitFpStatusData(string jsonData, int *status, double *endTime);	// 翻牌状态切换,[0:闲开牌, 1:庄开牌, 2:闲补牌, 3:庄补牌], time:结束时间
	static bool InitFpSelectData(string jsonData, int *card);						// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
	static bool InitFpingData(string jsonData, int *x1, int *y1, int *x2, int *y2);	// 翻牌中
	static bool InitFpOpenData(string jsonData, int *card);						// 开牌
	static bool InitFpInitData(string jsonData, int *playerid, int *bankerid, int *status, double *endTime, vector<int> *cardStatus);	// 中途进入房间时，翻牌初始化信息

	bool SyncBet();

	void Bet(BjlBet bet);

	void FinishBetTask(RoomInfoBjl *info);

protected:
	std::vector<BjlBet> _bet;

private:
	GameControlBjl();
	~GameControlBjl();
};
class BjlTablePlayerPanel :public Ref
{
protected:
	Sprite *imgAvatar, *imgBgAmount;
	int _index, _flip;
	Button *btSit;
	Text *lbNickName, *lbAmount;
	void btSitClicked(Ref *pSender);
public:
	BjlRoomPlayer *_playerData;
	int64_t _winAmount;			//本局赢的金额
	Node *RootNode;
	Sprite *imgCm;		//此控件需要外部访问
	Vec2 ptBigWinner;		//大赢家位置 中心点
	int num;
	static BjlTablePlayerPanel* create(Node *root,int num, int index, bool flip);
	void SetPlayer(BjlRoomPlayer *player);
	void SetProfit(int64_t balance);
	void SetBigWinner(bool enable);
	bool Empty();
};
class BjlTableControl
{
protected:
	Sprite *spMask;
	Animate *animateBigWinner;
	Sprite *spBigWinner;
public:
	Node *RootNode;
	map<int, BjlTablePlayerPanel*> _playerPanels;
	static BjlTableControl* create(Node *root);
	bool SetPlayerOnSeat(int pos, BjlRoomPlayer *player);
	void SetPlayerProfit(int pos, int64_t balance);			//设置用户盈亏
	void ShowBigWinner(float dt);		//显示大赢家,dt显示的时间
	int GetSitIdFromUserId(int uid);	//根据用户ID，获取座位ID，无座时，返回0
	void Test();
};
class BjlSitChat
{
protected:
	#define MARGIN_LEFT 18.0f
	#define MARGIN_RIGHT 18.0f
	#define MARGIN_TOP 30.0f
	#define MARGIN_BOTTOM 8.0f
	Node *_nodeRoot;
	ImageView *_imgBg;
	Text *_lbMsg;
	float _dt;
	int _sid;
	float _lbMaxWidth;
public:
	static BjlSitChat* create(int sid, Node *node, float dt = 3.0f);
	void setString(string msg);									//设置聊天信息
};
class BjlSitChatControl
{
protected:
	map<int, BjlSitChat*> _bjlSitChats;
	Node *_nodeRoot;
public:
	static BjlSitChatControl* create(Node *node);
	void ShowChat(int pos, string msg);
	~BjlSitChatControl();
};