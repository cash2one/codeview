#pragma once
#include "cocos2d.h"
#define max(a, b)  (((a) > (b)) ? (a) : (b))
USING_NS_CC;
using namespace cocos2d::ui;

enum BJLBETTYPE{
	BJLBETP = 1,
	BJLBETPP = 2,
	BJLBETB = 4,
	BJLBETBP = 8,
	BJLBETT = 16
};

enum BJLFPINDEX{
	BJLP1,
	BJLP2,
	BJLP3,
	BJLB1,
	BJLB2,
	BJLB3
};

enum STATUSBJLGAME{
	BETSTART,
	BETEND,
	FPSTART,
	FPEND
};

enum {
	TYPE_DEPOSIT = 1,  //储蓄
	TYPE_WITHDRAW=2,	 //取出
	TYPE_EXCHANGE=3,	 //购买
	TYPE_ADMIN_FREEZE=4, //冻结
	TYPE_CASINO=5,	   //下注
	TYPE_TASK = 6, //奖励
};

enum MSGCMD_BJLGAMEMO{
	MSGTYPE_BjlGameMo_Connect = 997,
	MSGTYPE_BjlGameMo_DisConnect = 998,
	MSGTYPE_BjlGameMo_Login = 0,
	MSGTYPE_BjlGameMo_EnterRoom = 1,
	MSGTYPE_BjlGameMo_LeaveRoom = 2,
	MSGTYPE_BjlGameMo_Chat = 3,
	MSGTYPE_BjlGameMo_RecvChat = 4,
	MSGTYPE_BjlGameMo_RoomsInfo = 7,
	MSGTYPE_BjlGameMo_RoomInfoUpdate = 8,
	MSGTYPE_BjlGameMo_NewRoomPlayer = 10,
	MSGTYPE_BjlGameMo_DelRoomPlayer = 11,
	MSGTYPE_BjlGameMo_RoomPlayers = 12,
	MSGTYPE_BjlGameMo_Sit = 13,
	MSGTYPE_BjlGameMo_RecvSit = 14,
	MSGTYPE_BjlGameMo_Bet = 1000,
	MSGTYPE_BjlGameMo_CutCard = 1004,
	MSGTYPE_BjlGameMo_BeginBet = 1005,
	MSGTYPE_BjlGameMo_ShowCards = 1006,
	MSGTYPE_BjlGameMo_Award = 1007,
	MSGTYPE_BjlGameMo_RoundOver = 1008,
	MSGTYPE_BjlGameMo_HistoryData = 1009,
	MSGTYPE_BjlGameMo_BetProfit = 1010,
	MSGTYPE_BjlGameMo_RoomResults = 1011,
	MSGTYPE_BjlGameMo_RecvRoomBetTotal = 1013,
	MSGTYPE_BjlGameMo_RoomTypesInfo = 1014,
	MSGTYPE_BjlGameMo_RecvRoomBet = 1016,
	PUSH_BEGIN_FLIP = 1017,
	PUSH_PLAYER_FLIP = 1018,
	CARD_SELECT = 1019,
	CARD_FLIP = 1020,
	CARD_OPEN = 1021,
	FLIP_OVER = 1022,
	PUSH_FLIP = 1023
};

struct BJLBETINFO
{
    
	

	int p, pp, b, bp, t;
	void Add(BJLBETINFO *betInfo)
	{
		p += betInfo->p;
		pp += betInfo->pp;
		b += betInfo->b;
		bp += betInfo->bp;
		t += betInfo->t;
	}
	void SafeSub(BJLBETINFO *betInfo)
	{
		p = max(0, p - betInfo->p);
		pp = max(0, pp - betInfo->pp);
		b = max(0, b - betInfo->b);
		bp = max(0, bp - betInfo->bp);
		t = max(0, t - betInfo->t);
	}
	std::string public_cards;
	std::string highest_cards;
	std::string cards;
};
//单据游戏数据
struct GameRoundDataBjl {
	BJLBETINFO betinfo;
	std::string create_time;
	std::string cardinfo;
	int round_id;
};
static const char* BjlBetTypeToString(BJLBETTYPE type)
{
	switch (type)
	{
	case BJLBETP:
		return "p";
	case BJLBETPP:
		return "pp";
	case BJLBETB:
		return "b";
	case BJLBETBP:
		return "bp";
	case BJLBETT:
		return "t";
	default:
		return "";
	}
}

struct CMINFO
{
	int64_t amount;
	std::string imgOn, imgOff, imgSmall;	//筹码选中，未选中，小筹码文件名，全部是plist中的文件名
};

class DateFromTo : public Ref
{
public:
	std::string from;
	std::string to;
	int flag;
};


