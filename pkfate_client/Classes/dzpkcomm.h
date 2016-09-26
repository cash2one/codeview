#pragma once
#include "cocos2d.h"
#include <algorithm> 
#define max(a, b)  (((a) > (b)) ? (a) : (b))
USING_NS_CC;
using namespace cocos2d::ui;


enum MSGCMD_DZPKGAMEMO{
	MSGTYPE_DzpkGameMo_Connect = 997,
	MSGTYPE_DzpkGameMo_DisConnect = 998,
	MSGTYPE_DzpkGameMo_Login = 0,
	MSGTYPE_DzpkGameMo_EnterRoom = 1,
	MSGTYPE_DzpkGameMo_LeaveRoom = 2,
	MSGTYPE_DzpkGameMo_Chat = 3,
	MSGTYPE_DzpkGameMo_RecvChat = 4,
	MSGTYPE_DzpkGameMo_RoomsInfo = 7,
	MSGTYPE_DzpkGameMo_RoomInfoUpdate = 8,
	MSGTYPE_DzpkGameMo_NewRoomPlayer = 10,
	MSGTYPE_DzpkGameMo_DelRoomPlayer = 11,
	MSGTYPE_DzpkGameMo_RoomPlayers = 12,
	MSGTYPE_DzpkGameMo_Sit = 13,
	MSGTYPE_DzpkGameMo_RecvSit = 14,
	PUSH_BLANCE_UPDATE = 15,

	TP_PUSH_ROOM_TYPES = 2000,
	TP_JOIN_GAME = 2001,
	TP_PUSH_GAME_STATUS = 2002,
	TP_PUSH_CHIP_FRESH = 2003,
	TP_BET = 2004,
	TP_PUSH_BET_INFO = 2005,
	TP_PUSH_PRE_FLOP_INFO = 2006,
	TP_PUSH_ROUND_INFO = 2007,
	TP_PUSH_GAME_RESULT = 2008,
	TP_PUSH_BEGIN_BET = 2009,
	TP_PUSH_BEGIN_START = 2010,
	TP_PUSH_NEXT_TIME = 2011,

	TP_CREATE_ROOM = 2012,
	TP_JOIN_ROOM = 2013,

	TP_SNG_PUSH_ROOM_TYPES = 3000,
	TP_SNG_PUSH_ROOM_STATUS = 3001,
	TP_SNG_BEGIN_APPLY = 3002,

	TP_SNG_PUSH_ROUND_INFO = 3003,

	TP_SNG_PUSH_SESSION_RESULT = 3004,

	PUSH_BALANCE_UPDATE_EX = 17,

	TP_SNG_BUY_CHIP = 3005,
	TP_SNG_PUSH_BUY_CHIP_TIMES = 3006,
};





enum BETACTION{
	CALL = 0,
	CHECK = 1,
	RAISE = 2,
	FOLD = 3,
	BIG_BLIND = 4,
	SMALL_BLIND = 5,
};


enum TPPokerType {
	ROYAL_FLUSH = 10,//
	STRAIGHT_FLUSH = 9,//
	FOUR_OF_A_KIND = 8,//剩余3张中排序最大的一张组合
	FULL_HOURSE = 7,//
	FLUSH = 6,//
	STRAIGHT = 5,//
	THREE_OF_A_KIND = 4,//剩余4张中取最大2张组合
	TWO_PAIR = 3,//剩余3张中排序最大的一张组合
	ONE_PAIR = 2, //剩余5张中排序最大3张 组合
	HIGH_CARD = 1, //已处理
};

#define MAX_ROUND_BET_NUM 3

struct Poker
{
	int num;
	string color;
	int pos;
};

enum GameProcess
{
	IS_WAITING = 0,
	IS_GAMING = 1,
};

enum TPParticipantStatus
{
	GAMING = 0,
	ALL_IN = 1,
	FOLD_ = 2,
	LEAVE = 3,
	LEAVE_ROOM = 4,
};

struct PokerResult
{
	vector<Poker*> pokers;

	int pokerType;
};


class DzPokerUtils{


public:



static int calChipN(int cN)
{
	int eN = cN % 3;
	int mN = ceil(cN / 3.0f);

	int fchip = 10;
	if (eN == 1){
		fchip = 10 * pow(10, mN - 1);
	}
	else if (eN == 2)
	{
		fchip = 20 * pow(10, mN - 1);
	}
	else
	{
		fchip = 50 * pow(10, mN - 1);
	}
	return fchip;
}

static int getMinChip(int chip, int fchip)
{
	int sN = getSpaceN(chip);
	int fN = getSpaceN(fchip) + 1;

	if (fN >= sN){
		return chip;
	}

	return calChipN(fN);
}

static int getChipN(int chip, int minChip, int percent)
{
	if (percent == 100) return chip;

	int spaceN = getSpaceN(chip);
	int spaceMinN = getSpaceN(minChip);

	if (spaceMinN >= spaceN){
		return chip;
	}

	int addN = (spaceN - spaceMinN)*(percent*0.01);

	int cN = spaceMinN + addN;


	return calChipN(cN);
	
}


static int getSpaceN(int num)
{
	if (num <= 0) return 0;
	if (num <= 10) return 1;

	float d = log10(num); // 对数 次
	int f = floor(d);
	float k = num / pow(10, f);
	f = (f-1) * 3;
	if (k > 5){
		f = f + 4;
	}else if(k > 2){
		f = f + 3;
	}else if(k>1){
		f = f + 2;
	}else if (k > 0){
		f = f + 1;
	}
	return f;
}




static void checkAndGetFlush(vector<Poker*> &pokers, vector<Poker*> &diff, vector<Poker*> &flush)
{
	flush.clear();
	vector<Poker*> diffTmp;
	if (diff.empty()){
		for (size_t i = 0, j = pokers.size(); i < j; i++)
		{
			if (pokers[i]->color == pokers[0]->color){
				flush.push_back(pokers[i]);
			}
			else{
				diffTmp.push_back(pokers[i]);
			}
		}
	}
	else
	{
		for (size_t i = 0, j = diff.size(); i < j; i++)
		{
			if (diff[i]->color == diff[0]->color){
				flush.push_back(diff[i]);
			}
			else{
				diffTmp.push_back(diff[i]);
			}
		}

	}
	  
	if (flush.size() > 4 || diffTmp.empty()){

		return;
	}
	else{
		checkAndGetFlush(pokers, diffTmp, flush);
	}
}


static bool compare(Poker* a, Poker* b)
{
	return a->num>b->num; //升序排列，如果改为return a>b，则为降序
}

static void combineFive(vector<Poker*> &need, vector<Poker*> &pokers)
{
	while (need.size() < 5)
	{
		for (Poker* combine : pokers)
		{
			auto iter = find(need.begin(), need.end(), combine);
			if (iter == need.end()){
				need.push_back(combine);
				break;
			}
		}
	}
}


static void checkAndGetPokerType(vector<Poker*> &pokers, PokerResult* pokerResult)
{
	sort(pokers.begin(), pokers.end(), compare);//倒叙

	vector<int> zeros = { 0 };
	int ones = 0;

	vector<Poker*>  zeros_t;
	vector<Poker*>  ones_t;

	for (size_t i = 1, j = pokers.size(); i < j; i++)
	{
		int p_num = pokers[i]->num - pokers[i - 1]->num;

		if (p_num == 0){
			zeros[zeros.size() - 1] = zeros[zeros.size() - 1] + 1;
			
			if (zeros_t.empty() || zeros_t[zeros_t.size() - 1]->num != pokers[i - 1]->num){
				zeros_t.push_back(pokers[i - 1]);
			}
			zeros_t.push_back(pokers[i]);
		    
		}
		else if (p_num != -1){
			zeros.push_back(0);
			
			if (ones < 3 || (ones == 3 && pokers[i]->num != 2)){ //排除 非顺子，非A+2345顺子
				ones = 0;
			}
			
		}
		else{//=-1

			if (ones_t.empty() || ones_t[ones_t.size() - 1]->num != pokers[i - 1]->num){
				ones_t.push_back(pokers[i - 1]);
			}
			ones_t.push_back(pokers[i]);
			ones = ones + 1;
			zeros.push_back(0);
		}
	}


	if (ones > 3){
		pokerResult->pokerType = TPPokerType::STRAIGHT;
		pokerResult->pokers.swap(ones_t);
		return;
	}

	if (ones == 3){ // 带A
		Poker* last = ones_t[ones_t.size()-1];

		if (last->num == 2 && pokers[0]->num == 14){

			ones_t.push_back(pokers[0]);

			for (size_t i = 1, j = pokers.size(); i < j; i++)
			{
				if (pokers[i]->num != 14){
					break;
				}
				if (pokers[i]->color == last->color){
					ones_t[ones_t.size() - 1] = pokers[i];
					break;
				}
			}


			pokerResult->pokerType = TPPokerType::STRAIGHT;

			pokerResult->pokers.swap(ones_t);;

			return;
		}
		
		
	}

	sort(zeros.begin(), zeros.end());// 排序

	pokerResult->pokers.swap(zeros_t);

	if (zeros[zeros.size() - 1] == 3){
		pokerResult->pokerType = TPPokerType::FOUR_OF_A_KIND;//四条
		combineFive(pokerResult->pokers, pokers);
	}
	else if (zeros[zeros.size() - 1] == 2){
		if (zeros[zeros.size() - 2] >= 1){
			pokerResult->pokerType = TPPokerType::FULL_HOURSE;//三条一对
			while (pokerResult->pokers.size()>5){
				pokerResult->pokers.erase(pokerResult->pokers.end()-1);
			}
		}
		else{
			pokerResult->pokerType = TPPokerType::THREE_OF_A_KIND;//三条
			combineFive(pokerResult->pokers, pokers);
		}
		
	}
	else if (zeros[zeros.size() - 1] == 1){
		if (zeros[zeros.size() - 2] == 1){
			pokerResult->pokerType = TPPokerType::TWO_PAIR;
			while (pokerResult->pokers.size()>4){
				pokerResult->pokers.erase(pokerResult->pokers.end() - 1);
			}
			combineFive(pokerResult->pokers, pokers);
		}
		else{
			pokerResult->pokerType = TPPokerType::ONE_PAIR;
			combineFive(pokerResult->pokers, pokers);
		}

	}
	else{
		

		combineFive(pokerResult->pokers, pokers);

		pokerResult->pokerType = TPPokerType::HIGH_CARD;
	}

}

static void checkPokerType(PokerResult * result)
{
	if (result == nullptr || result->pokers.empty()) return;
	

	vector<Poker*> pokers = result->pokers;

	checkAndGetPokerType(pokers, result);//验证牌型

	vector<Poker*> diff; //验证同花
	vector<Poker*> flush;
	checkAndGetFlush(pokers, diff, flush);

	if (flush.size() >= 5){//同花

		PokerResult * flushResult = new PokerResult; //检验同花 的牌型
		checkAndGetPokerType(flush, flushResult);
		if (flushResult->pokerType == TPPokerType::STRAIGHT){ //同花顺
			if (flushResult->pokers[0]->num == 14){ // 为A，皇家同花顺
				result->pokerType = TPPokerType::ROYAL_FLUSH;
			}
			else{
				result->pokerType = TPPokerType::STRAIGHT_FLUSH;
			}
			result->pokers = flushResult->pokers;//用同花的牌
		}
		else{//非同花顺,则取同花
			if (result->pokerType < 6) // <=同花 则用同花
			{
				result->pokerType = TPPokerType::FLUSH;
				sort(flush.begin(), flush.end(), compare);//倒序
				result->pokers = flush;//用同花的牌
			}
		}

	}

}


static void validatePokerCards(string handCards, string publicCards, int *type, map<int, bool> &pokerMap)
{
	if (handCards.empty() || publicCards.empty()) return ;

	string pokerCards = handCards + publicCards;
	vector<Poker*> pokers;

	for (int i = 0, j = pokerCards.length() / 2; i < j; i++){
		Poker* poker = new Poker;
		poker->color = pokerCards.substr(i*2, 1);
		string pokerNum = pokerCards.substr(i*2+1, 1);
		if (pokerNum == "T"){
			poker->num = 10;
		}
		else if (pokerNum == "J"){
			poker->num = 11;
		}
		else if (pokerNum == "Q"){
			poker->num = 12;
		}
		else if (pokerNum == "K"){
			poker->num = 13;
		}
		else if (pokerNum == "A" || pokerNum == "1"){
			poker->num = 14;
		}
		else{
			poker->num = atoi(pokerNum.c_str());
		}
		poker->pos = i+1;
		pokers.push_back(poker);
	}

	PokerResult *result = new PokerResult;
	result->pokers = pokers;
	DzPokerUtils::checkPokerType(result);


	vector<Poker*> resultPokers = result->pokers;

	while (resultPokers.size()>5){//delete多余的元素
		resultPokers.erase(resultPokers.end() - 1);
	}

	CCLOG("---validatePokerCards-pokerType-----%d-----", result->pokerType);
	for (size_t i = 0; i < resultPokers.size(); i++)
	{
		CCLOG("---validatePokerCards---%s--%d---%d--", resultPokers[i]->color.c_str(), resultPokers[i]->num, resultPokers[i]->pos);
	   pokerMap[resultPokers[i]->pos] = true;
	}

	*type = result->pokerType;


	for (auto poker:pokers)
	{
		delete poker;
	}

	delete result;

}


};

