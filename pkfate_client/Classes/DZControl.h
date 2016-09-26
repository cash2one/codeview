#pragma once
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpResponse.h"
using namespace std;
USING_NS_CC;

struct DzpkRoomType{
	int type_id;
	string name;
	int bet_seconds;
	int ante;//大盲
	int card_count;//牌数目  现在有两种 54 和
	int init_chip; //携带
	int next_game_seconds;
	int reward_seconds;
	float fee_rate;
};



struct RoundInfo{
	int round_num;
	string public_cards;
	int big_blind;
	int small_blind;
};


struct BetBegin{
	int user_id;
	int round_num;
	int end_time;
};

struct RoomPlayer
{
	int user_id;
	int avatar;
	string nickname;
	int balance;
	int pos;
	int status;
};

struct RoomSitInfo
{
	int pos;
	int user_id;
	
};

struct BetInfo{
	int user_id;
	int bet_amount;
	int round_amount;
	int chip;
	int status;
	int action;
};




struct PlayerResult{
	int user_id;
	string hand_cards;
	int poker_type;
	int bonus;
	int chip;
	int bet_amount;
	int score;
	bool isWiner;
};
struct DzChatData
{
	int user_id;
	string msg;
};

struct ChipInfo
{
	int user_id;
	int chip;
};

struct RoomTempInfo
{
	vector<BetInfo*> betInfos;
	RoundInfo* roundInfo;
	BetBegin * betBegin;
};


enum DZROOMKIND
{
	DZ_GAME_ROOM = 0,//普通游戏房间
	DZ_GAME_SINGLE_CREATE_ROOM = 1,//德州单独创建的房间
	DZ_GAME_JOIN_CREATE_ROOM = 2,//德州加入的单独房间
};


struct SNGRoomType
{
	int apply_cost;
	int apply_currency_type;
	int bet_seconds;
	int card_count;
	int next_game_seconds;
	int open_players;
	int reward_amount;
	int reward_currency_type;
	int total_sessions;
	int type_id;
	string name;
	string open_time;
};

struct SNGRoomStatus
{
	int room_id;
	int type_id;
	int players;
	bool is_running;
	int session_num;
};

struct SNGRoundInfo
{
	int session_num;
	int round_num;
	int ante;
};

class DZControl :public Ref
{
public:
	static DZControl* GetInstance();
	bool Init();		//
	void setDzpkRoomView(Node * roomView);
	void setDzpkHallView(Node * busView);

	map<int, SNGRoomStatus*> sngRoomStatusMap;

	map<int, SNGRoomType*> SngRoomTypeInfos;

	map<int, DzpkRoomType *> RoomTypeInfos;
	map<int, RoomPlayer*> RoomPlayers;
	map<int, RoomSitInfo*> sitInfos;

	int getLogicPos(int ui_pos);//根据UI显示获取逻辑位置
	int getUIPos(int logic_pos);//根据逻辑pos获取UI显示位置
	
	int getOneNotSitPos();//获取一张没有坐过的位子

	int gameProcess;//游戏进程

	RoomSitInfo* getSitInfo(int user_id);

	RoomPlayer * getPlayerByPos(int pos);

	DzpkRoomType* getCurrentRoomType();

	int getAnte();
	int getBetSeconds();
	int getInitChip();
	float getFeeRate();

	int current_sng_type_id;
	int current_sng_room_id;

	string gameToken;
	int gameRoomType;//当前游戏房间类型
	DZROOMKIND roomKind;

	SNGRoundInfo * sngRoundInfo;

	void setRoomInfo(int gameRoomType = 1, DZROOMKIND roomKind = DZROOMKIND::DZ_GAME_ROOM, string gameToken = "");//进入房间设置

	bool isHallView();

	string getCurrencyTypeStr(int type);
	string getRoomTypeStr(int roomType);

	void closeRoom();

	bool isSNG;

	int buyChipTimes;

	int getCurrentPlayersNum();
private:
	int my_pos;

	void disposeRoom();
	void disposeHall();

	Node *roomView;
	Node *hallView;//候厅
	RoomTempInfo * roomTempInfo;

	void addNoticeListeners();
	void disposeNoticeListeners();

	void onSendConnectDezpk(Ref *pSender);
	void onSendJoinGame(Ref *pSender);

	void onPushEnterRoomCallBack(Ref *pSender);

	void onPushRoomType(Ref *pSender);
	bool InitRoomTypesInfoData(std::string jsonData);
	
	
	void onPushGameStatus(Ref *pSender);
	bool dealGameStatusData(std::string jsonData);

	void onPushChipFresh(Ref *pSender);

	void onPushBetInfo(Ref *pSender);
	bool dealBetInfo(string jsonData);

	void onPushFlopInfo(Ref *pSender);
	bool dealFlopInfo(string jsonData);
	
	void onPushRoundInfo(Ref *pSender);
	bool dealRoundInfo(string jsonData);

	void onPushGameResult(Ref *pSender);
	bool dealGameResult(string jsonData);

	void onPushNextTime(Ref *pSender);
	bool dealNextTime(string jsonData);
	
	
	void onPushBeginBet(Ref *pSender);
	bool dealBeginBet(string jsonData);

	void onPushGameStart(Ref *pSender);
	void onTpBetCallback(Ref *pSender);

	void onConnectCallBack(Ref *pSender);
	void onLoginCallBack(Ref *pSender);
	

	void onRecvSitCallBack(Ref* pSender);
	bool dealSitPlayers(string jsonData);

	void onRoomPlayersCallBack(Ref *pSender);
	bool dealRoomPlayers(string jsonData);

	void onRecvNewPlayerCallBack(Ref *pSender);
	bool dealNewPlayer(string jsonData);

	void onRecvDelPlayerCallBack(Ref *pSender);
	bool dealDelPlayer(string jsonData);


	void onChatCallBack(Ref *pSender);
	bool dealChatData(string jsonData);

	void onPushChipFreshCallback(Ref *pSender);
	bool dealChipFresh(string jsonData);

	void onPushBalanceInfo(Ref *pSender);
	bool dealBalanceInfo(string jsonData);

	void onTriggerCreateRoom(Ref* pSender);
	void onTriggerJoinCreateRoom(Ref* pSender);
	
	void onCreateRoom(Ref* pSender);
	bool dealCreateRoom(string jsonData);

	void onJoinRoom(Ref* pSender);
	bool dealJoinRoom(string jsonData);

	void onPushSngRoomType(Ref *pSender);
	bool dealPushSngRoomType(string jsonData);

	void onSngBeginApply(Ref *pSender);
	bool dealSngBeginApply(string jsonData);


	void onPushSngGameStatus(Ref *pSender);
	bool dealPushSngGameStatus(string jsonData);
	
	void checkAndDelRoomPlayers();

	void checkAndUpdateRoomTempInfo();

	void clearRoomTempInfo();

	void onConnectSNGCallBack(Ref *pSender);

	void onSendEnterSNGRoom(Ref *pSender);

	void onPushSNGRoundInfo(Ref *pSender);
	bool dealPushSNGRoundInfo(string jsonData);

	void onEnterSNGRoom(Ref *pSender);
	bool dealEnterSNGRoom(string jsonData);

	void onPushBalanceUpdateEx(Ref *pSender);
	bool dealPushBalanceUpdateEx(string jsonData);
	

	void onPushSNGSessionResult(Ref *pSender);
	bool dealPushSNGSessionResult(string jsonData);
	
	void onPushBuyChip(Ref *pSender);
	bool dealPushBuyChip(string jsonData);

	void onPushBuyChipTimes(Ref *pSender);
	bool dealPushBuyChipTimes(string jsonData);
	
	

		
	~DZControl();
	DZControl();
};

