#include "GameControlBjl.h"
#include "cJSON.h"
#include "comm.h"
#include "AvatarControl.h"
#include "SoundControl.h"
#include "PKNotificationCenter.h"
#include "UserControl.h"
#include "ApiGame.h"
static GameControlBjl *m_pInstance = nullptr;
GameControlBjl::GameControlBjl()
{

}
GameControlBjl::~GameControlBjl()
{
	if (RoomInfos.size()>0)
	{
		for (auto room : RoomInfos)
		{
			delete room.second;
		}
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
GameControlBjl* GameControlBjl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new GameControlBjl();
	return m_pInstance;
}
//从服务端返回的json数据，生成room types
bool GameControlBjl::InitRoomTypesInfoData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	for (auto info : RoomTypeInfos)
		delete info.second;
	RoomTypeInfos.clear();
	int size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < size; i++)
	{
		cJSON *typeinfo, *type_id, *max_bet, *players_limit, *auto_show_cards, *reward_seconds, *bet_seconds, *next_game_seconds, *show_cards_seconds;
		if (!(typeinfo = cJSON_GetArrayItem(jsonMsg, i)) || typeinfo->type != cJSON_Object
			|| !(type_id = cJSON_GetObjectItem(typeinfo, "type")) || type_id->type != cJSON_Number
			|| !(max_bet = cJSON_GetObjectItem(typeinfo, "max_bet")) || max_bet->type != cJSON_Number
			|| !(players_limit = cJSON_GetObjectItem(typeinfo, "players_limit")) || players_limit->type != cJSON_Number
			|| !(auto_show_cards = cJSON_GetObjectItem(typeinfo, "auto_show_cards")) || (auto_show_cards->type != cJSON_True&&auto_show_cards->type != cJSON_False)
			|| !(reward_seconds = cJSON_GetObjectItem(typeinfo, "reward_seconds")) || reward_seconds->type != cJSON_Number
			|| !(bet_seconds = cJSON_GetObjectItem(typeinfo, "bet_seconds")) || bet_seconds->type != cJSON_Number
			|| !(next_game_seconds = cJSON_GetObjectItem(typeinfo, "next_game_seconds")) || next_game_seconds->type != cJSON_Number
			|| !(show_cards_seconds = cJSON_GetObjectItem(typeinfo, "show_cards_seconds")) || show_cards_seconds->type != cJSON_Number)
			continue;
		BjlRoomType *type = new BjlRoomType();
		type->type_id = type_id->valueint;
		type->max_bet = max_bet->valueint;
		type->players_limit = players_limit->valueint;
		type->auto_show_cards = auto_show_cards->type == cJSON_True;
		type->reward_seconds = reward_seconds->valueint;
		type->bet_seconds = bet_seconds->valueint;
		type->next_game_seconds = next_game_seconds->valueint;
		type->show_cards_seconds = show_cards_seconds->valueint;
		RoomTypeInfos[type->type_id] = type;
	}
	cJSON_Delete(jsonMsg);
	return true;
}
//从服务端返回的json数据，生成rooms info
bool GameControlBjl::InitRoomsInfoData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	/*for (auto room : RoomInfos)
		delete room.second;
	RoomInfos.clear();*/
	int size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < size; i++)
	{
		cJSON *room, *id, *count, *type_id;
		BjlRoomType *type;
		if (!(room = cJSON_GetArrayItem(jsonMsg, i)) || room->type != cJSON_Array || cJSON_GetArraySize(room) != 3
			|| !(id = cJSON_GetArrayItem(room, 0)) || id->type != cJSON_Number
			|| !(count = cJSON_GetArrayItem(room, 1)) || count->type != cJSON_Number
			|| !(type_id = cJSON_GetArrayItem(room, 2)) || type_id->type != cJSON_Number
			|| !(type = RoomTypeInfos[type_id->valueint]))
			continue;
		RoomInfoBjl *info = RoomInfos[id->valueint];
		if (!info)
		{
			info = new RoomInfoBjl();
			info->id = id->valueint;
			RoomInfos[info->id] = info;
		}
		info->count = count->valueint;
		info->type = type;
	}
	cJSON_Delete(jsonMsg);
	return true;
}
//更新房间人数信息，返回－1时出错，正常时返回更新房间id
int GameControlBjl::UpdateRoomInfo(string jsonData)
{
	if (jsonData.empty())
		return -1;
	cJSON *room = cJSON_Parse(jsonData.c_str());
	if (room == nullptr)
		return -1;
	if (room->type != cJSON_Array || cJSON_GetArraySize(room) != 3)
	{
		cJSON_Delete(room);
		return -1;
	}
	cJSON *id = cJSON_GetArrayItem(room, 0);
	if (id == nullptr || id->type != cJSON_Number)
	{
		cJSON_Delete(room);
		return -1;
	}
	cJSON *count = cJSON_GetArrayItem(room, 1);
	if (count == nullptr || count->type != cJSON_Number)
	{
		cJSON_Delete(room);
		return -1;
	}
	cJSON *type_id = cJSON_GetArrayItem(room, 2);
	BjlRoomType *type;
	if (type_id == nullptr || type_id->type != cJSON_Number
		|| !(type = RoomTypeInfos[type_id->valueint]))
	{
		cJSON_Delete(room);
		return -1;
	}
	RoomInfoBjl *info = RoomInfos[id->valueint];
	if (info == nullptr)
	{
		info = new RoomInfoBjl();
		info->id = id->valueint;
		RoomInfos[info->id] = info;
	}
	info->count = count->valueint;
	info->type = type;
	cJSON_Delete(room);
	return info->id;
}
//从服务端返回的json数据，生成游戏房间结果results
map<int, string> GameControlBjl::GetResultsFromData(string jsonData)
{
	map<int, string> results;
	if (!jsonData.empty())
	{
		cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
		if (jsonMsg)
		{
			if (jsonMsg->type == cJSON_Object)
			{
				cJSON *room = jsonMsg->child;
				while (room != nullptr)
				{
					results[atoi(room->string)] = cJSON_PrintUnformatted(room);
					room = room->next;
				}
			}
			cJSON_Delete(jsonMsg);
		}
	}
	return results;
}
vector<int> GameControlBjl::JsonResults2BetType(string jsonResults)
{
	vector<int> ret;
	if (!jsonResults.empty())
	{
		cJSON *jsonMsg = cJSON_Parse(jsonResults.c_str());
		if (jsonMsg)
		{
			if (jsonMsg->type == cJSON_Array)
			{
				cJSON *type = jsonMsg->child;
				while (type != nullptr)
				{
					if (strcmp(type->valuestring, "b") == 0)
						ret.push_back(BJLBETTYPE::BJLBETB);
					else if (strcmp(type->valuestring, "p") == 0)
						ret.push_back(BJLBETTYPE::BJLBETP);
					else if (strcmp(type->valuestring, "t") == 0)
						ret.push_back(BJLBETTYPE::BJLBETT);
					type = type->next;
				}
			}
			cJSON_Delete(jsonMsg);
		}
	}
	return ret;
}
BjlRoomData* GameControlBjl::InitRoomData(string jsonData)
{
	if (jsonData.empty())
		return nullptr;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return nullptr;
	cJSON *inning_num, *inning_secret, *secret, *uuid, *history_rounds;
	int roundSize;
	if (jsonMsg->type != cJSON_Object
		|| !(inning_num = cJSON_GetObjectItem(jsonMsg, "inning_num")) || inning_num->type != cJSON_Number
		|| !(inning_secret = cJSON_GetObjectItem(jsonMsg, "inning_secret")) || inning_secret->type != cJSON_String
		|| !(secret = cJSON_GetObjectItem(jsonMsg, "secret")) || secret->type != cJSON_String
		|| !(uuid = cJSON_GetObjectItem(jsonMsg, "uuid")) || uuid->type != cJSON_String
		|| !(history_rounds = cJSON_GetObjectItem(jsonMsg, "history_rounds")) || history_rounds->type != cJSON_Array || (roundSize = cJSON_GetArraySize(history_rounds)) < 1)
	{
		cJSON_Delete(jsonMsg);
		return nullptr;
	}
	BjlRoomData *roomData = new BjlRoomData();
	for (int i = 0; i < roundSize; i++)
	{
		cJSON *round, *round_num, *round_secret, *round_seed, *round_data;
		if (!(round = cJSON_GetArrayItem(history_rounds, i)) || round->type != cJSON_Object
			|| !(round_num = cJSON_GetObjectItem(round, "num")) || round_num->type != cJSON_Number
			|| !(round_secret = cJSON_GetObjectItem(round, "secret")) || round_secret->type != cJSON_String
			|| !(round_seed = cJSON_GetObjectItem(round, "seed")) || round_seed->type != cJSON_String
			|| !(round_data = cJSON_GetObjectItem(round, "data")) || round_data->type != cJSON_String)
		{
			roomData->history_rounds.clear();
			delete roomData;
			cJSON_Delete(jsonMsg);
			return nullptr;
		}
		BjlRoomDataRound dataRound;
		dataRound.num = round_num->valueint;
		dataRound.secret = round_secret->valuestring;
		dataRound.seed = round_seed->valuestring;
		dataRound.data = round_data->valuestring;
		roomData->history_rounds.push_back(dataRound);
	}
	roomData->uuid = uuid->valuestring;
	roomData->secret = secret->valuestring;
	roomData->inning_num = inning_num->valueint;
	roomData->inning_secret = inning_secret->valuestring;
	cJSON_Delete(jsonMsg);
	return roomData;
}
BjlRoomData* GameControlBjl::InitRoomHistoryData(string jsonData)
{
	if (jsonData.empty())
		return nullptr;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return nullptr;
	int size;
	if (jsonMsg->type != cJSON_Array || (size = cJSON_GetArraySize(jsonMsg)) < 1)
	{
		cJSON_Delete(jsonMsg);
		return nullptr;
	}
	BjlRoomData *roomData = new BjlRoomData();
	cJSON *round, *round_num, *round_secret, *round_seed, *round_data;
	int roundSize;
	for (int i = 0; i < size; i++)
	{

		if (!(round = cJSON_GetArrayItem(jsonMsg, i)) || round->type != cJSON_Array || (roundSize = cJSON_GetArraySize(round)) != 4
			|| !(round_num = cJSON_GetArrayItem(round, 0)) || round_num->type != cJSON_Number
			|| !(round_seed = cJSON_GetArrayItem(round, 1)) || round_seed->type != cJSON_String
			|| !(round_secret = cJSON_GetArrayItem(round, 2)) || round_secret->type != cJSON_String
			|| !(round_data = cJSON_GetArrayItem(round, 3)) || round_data->type != cJSON_String)
		{
			roomData->history_rounds.clear();
			delete roomData;
			cJSON_Delete(jsonMsg);
			return nullptr;
		}
		BjlRoomDataRound dataRound;
		dataRound.num = round_num->valueint;
		dataRound.seed = round_seed->valuestring;
		dataRound.secret = round_secret->valuestring;
		dataRound.data = round_data->valuestring;
		roomData->history_rounds.push_back(dataRound);
	}
	roomData->uuid = "";
	roomData->secret = "";
	roomData->inning_num = 0;
	roomData->inning_secret = "";
	cJSON_Delete(jsonMsg);
	return roomData;
}
bool GameControlBjl::InitBeginBetData(string jsonData, BjlRoomData *roomData, double *time)
{
	if (jsonData.empty() || !roomData || !time)
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		cJSON  *next_event_time, *game_num, *secret;
		if (jsonMsg->type == cJSON_Object
			&& (next_event_time = cJSON_GetObjectItem(jsonMsg, "next_event_time")) && next_event_time->type == cJSON_Number
			&& (game_num = cJSON_GetObjectItem(jsonMsg, "game_num")) && game_num->type == cJSON_Number
			&& (secret = cJSON_GetObjectItem(jsonMsg, "secret")) && secret->type == cJSON_String)
		{
			*time = next_event_time->valuedouble;
			roomData->inning_num = game_num->valueint;
			roomData->inning_secret = secret->valuestring;
			result = true;
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}
bool GameControlBjl::InitShowCardsData(string jsonData, BjlRoomDataRound *roundData, double *time, bool *isFlip)
{
	//{"seconds":5,"secret":"e74edc8fd033ed2051489df7112f3a1d0e1f24ecd16ffe4a0a670d778728f50a","seed":"b2e198a29a4fe185c94320ad2c013ca276292021ab97a442d55d34f2e4b98689","data":"HTC5H6H6D1","game_num":5}
	if (jsonData.empty() || !roundData || !time|| !isFlip)
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
        cJSON  *next_event_time,*flip, *game_num, *secret, *seed, *data;
		if (jsonMsg->type == cJSON_Object
			&& (next_event_time = cJSON_GetObjectItem(jsonMsg, "next_event_time")) && next_event_time->type == cJSON_Number
            && (flip = cJSON_GetObjectItem(jsonMsg, "flip"))
			&& (game_num = cJSON_GetObjectItem(jsonMsg, "game_num")) && game_num->type == cJSON_Number
			&& (secret = cJSON_GetObjectItem(jsonMsg, "secret")) && secret->type == cJSON_String
			&& (seed = cJSON_GetObjectItem(jsonMsg, "seed")) && seed->type == cJSON_String
			&& (data = cJSON_GetObjectItem(jsonMsg, "data")) && data->type == cJSON_String)
		{
			*time = next_event_time->valueint;
            *isFlip=flip->type==cJSON_True;
			roundData->num = game_num->valueint;
			roundData->secret = secret->valuestring;
			roundData->seed = seed->valuestring;
			roundData->data = data->valuestring;
			result = true;
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}
bool GameControlBjl::InitCutCardData(string jsonData, BjlRoomDataRound *roundData, double *time)
{
	if (jsonData.empty() || !roundData || !time)
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		cJSON  *next_event_time, *game_uuid, *secret, *data;
		if (jsonMsg->type == cJSON_Object
			&& (next_event_time = cJSON_GetObjectItem(jsonMsg, "next_event_time")) && next_event_time->type == cJSON_Number
			&& (secret = cJSON_GetObjectItem(jsonMsg, "secret")) && secret->type == cJSON_String
			&& (game_uuid = cJSON_GetObjectItem(jsonMsg, "game_uuid")) && game_uuid->type == cJSON_String
			&& (data = cJSON_GetObjectItem(jsonMsg, "data")) && data->type == cJSON_String)
		{
			*time = next_event_time->valueint;
			roundData->secret = secret->valuestring;
			roundData->seed = game_uuid->valuestring;
			roundData->data = data->valuestring;
			result = true;
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}

bool GameControlBjl::InitRoundOverData(string jsonData, BjlRoomDataRound *roundData, double *time)
{
	if (jsonData.empty() || !roundData || roundData->num != 0 || !time)
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		cJSON  *next_event_time, *secret, *seed, *data;
		if (jsonMsg->type == cJSON_Object
			&& (next_event_time = cJSON_GetObjectItem(jsonMsg, "next_event_time")) && next_event_time->type == cJSON_Number
			&& (secret = cJSON_GetObjectItem(jsonMsg, "secret")) && secret->type == cJSON_String
			&& (seed = cJSON_GetObjectItem(jsonMsg, "seed")) && seed->type == cJSON_String
			&& (data = cJSON_GetObjectItem(jsonMsg, "data")) && data->type == cJSON_String)
		{
			*time = next_event_time->valueint;
			roundData->secret = secret->valuestring;
			roundData->seed = seed->valuestring;
			roundData->data = data->valuestring;
			result = true;
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}
bool GameControlBjl::InitChatData(string jsonData, ChatData *chatData)
{
	if (jsonData.empty() || !chatData) {
		return false;

	}
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		cJSON  *id, *msg;
		int size;
		if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg)) == 2
			&& (id = cJSON_GetArrayItem(jsonMsg, 0)) && id->type == cJSON_Number
			&& (msg = cJSON_GetArrayItem(jsonMsg, 1)) && msg->type == cJSON_String)
		{
			chatData->id = id->valueint;
			chatData->msg = msg->valuestring;
			result = true;
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}
bool GameControlBjl::InitGameRoundData(string jsonData, GameRoundDataBjl* gameRoundData) {

	if (jsonData.empty() || !gameRoundData)
		return false;

	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str()))) {
		return false;
	}

	cJSON *create_time, *data, *round_id;
	if (jsonMsg->type != cJSON_Object || !InitBjlBetInfo(cJSON_GetObjectItem(jsonMsg, "bet_info"), &(gameRoundData->betinfo))
		|| !(create_time = cJSON_GetObjectItem(jsonMsg, "create_time")) || create_time->type != cJSON_String
		|| !(data = cJSON_GetObjectItem(jsonMsg, "data")) || data->type != cJSON_String
		|| !(round_id = cJSON_GetObjectItem(jsonMsg, "round_id")) || round_id->type != cJSON_Number){
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		gameRoundData->create_time = create_time->valuestring;
		gameRoundData->cardinfo = data->valuestring;
		gameRoundData->round_id = round_id->valueint;
	}
	return true;
}

//vector<ProfitInfoRecord> GameControlBjl::InitProfitForm(string jsonData) {
//	
//	vector<ProfitInfoRecord> vecProfitInfoRecords;
//	vecProfitInfoRecords.clear();
//	
//	if (jsonData.empty()) {
//		return vecProfitInfoRecords;
//	}
//	
//	cJSON *jsonMsg;
//	if (!(jsonMsg = cJSON_Parse(jsonData.c_str()))) {
//		return vecProfitInfoRecords;
//	}
//	
//	if (jsonMsg->type != cJSON_Array)
//	{
//		cJSON_Delete(jsonMsg);
//		return vecProfitInfoRecords;
//	} else {
//		cJSON *jsonMsgInner = cJSON_GetArrayItem(jsonMsg,0);
//		
//		if (jsonMsgInner->type != cJSON_Array)
//		{
//			cJSON_Delete(jsonMsg);
//			return vecProfitInfoRecords;
//		} else {
//			int size = cJSON_GetArraySize(jsonMsgInner);
//			for (int i = 0; i < size; i++) {
//				cJSON *item;
//				cJSON *status, *num, *uuid, *game_type, *id, *secret, *create_time,
//				*data, *seed;
//				item = cJSON_GetArrayItem(jsonMsgInner, i);
//				
//				if (!(item = cJSON_GetArrayItem(jsonMsgInner, i)) || item->type != cJSON_Object
//					|| !(status = cJSON_GetObjectItem(item, "status")) || status->type != cJSON_Number
//					|| !(num = cJSON_GetObjectItem(item, "num")) || num->type != cJSON_Number
//					|| !(uuid = cJSON_GetObjectItem(item, "uuid")) || uuid->type != cJSON_String
//					|| !(game_type = cJSON_GetObjectItem(item, "game_type")) || game_type->type != cJSON_Number
//					|| !(id = cJSON_GetObjectItem(item, "id")) || id->type != cJSON_Number
//					|| !(secret = cJSON_GetObjectItem(item, "secret")) || secret->type != cJSON_String
//					|| !(create_time = cJSON_GetObjectItem(item, "create_time")) || create_time->type != cJSON_Number)
//				{
//					vecProfitInfoRecords.clear();
//					cJSON_Delete(jsonMsg);
//					return vecProfitInfoRecords;
//				}
//				
//				if (status->valueint == 2) {
//					if (!(data = cJSON_GetObjectItem(item, "data")) || data->type != cJSON_String
//						|| !(seed = cJSON_GetObjectItem(item, "seed")) || seed->type != cJSON_String) {
//						vecProfitInfoRecords.clear();
//						cJSON_Delete(jsonMsg);
//						return vecProfitInfoRecords;
//					} else {
//						ProfitInfoRecord records_item;
//						records_item.status = status->valueint;
//						//records_item.close_time = close_time->valueint;
//						//records_item.award_time = award_time->valueint;
//						records_item.num = num->valueint;
//						records_item.uuid = uuid->valuestring;
//						records_item.game_type = game_type->valueint;
//						records_item._id = id->valueint;
//						records_item.secret = secret->valuestring;
//						records_item.create_time = create_time->valueint;
//						//records_item.bet_info = bet_info->valuestring;
//						records_item.data = data->valuestring;
//						records_item.seed = seed->valuestring;
//						//records_item.open_time = open_time->valueint;
//						
//						vecProfitInfoRecords.push_back(records_item);
//					}
//				}
//			}
//		}
//	}
//	
//	cJSON_Delete(jsonMsg);
//	return vecProfitInfoRecords;
//}
bool GameControlBjl::InitBjlBetInfo(string jsonData, BJLBETINFO *betInfo)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (!jsonMsg)
		return false;
	bool result = InitBjlBetInfo(jsonMsg, betInfo);
	cJSON_Delete(jsonMsg);
	return result;
}
bool GameControlBjl::InitBjlBetInfo(cJSON *jsonMsg, BJLBETINFO *betInfo)
{
	if (!jsonMsg || !betInfo)
		return false;
	cJSON *p, *pp, *b, *bp, *t, *cards, *highest_cards, *public_cards, *bonus, *total_bet;
	bool result = false;
	
	if (jsonMsg->type == cJSON_Object)
	{
		if ((p = cJSON_GetObjectItem(jsonMsg, "p")) && p->type == cJSON_Number){
			betInfo->p = p->valueint;
		}
		if ((b = cJSON_GetObjectItem(jsonMsg, "b")) && b->type == cJSON_Number){
			betInfo->b = b->valueint;
		}
		if ((bp = cJSON_GetObjectItem(jsonMsg, "bp")) && bp->type == cJSON_Number){
			betInfo->bp = bp->valueint;
		}
		if ((pp = cJSON_GetObjectItem(jsonMsg, "pp")) && pp->type == cJSON_Number){
			betInfo->pp = pp->valueint;
		}
		if ((t = cJSON_GetObjectItem(jsonMsg, "t")) && t->type == cJSON_Number){
			betInfo->t = t->valueint;
		}
		if ((highest_cards = cJSON_GetObjectItem(jsonMsg, "highest_cards")) && highest_cards->type == cJSON_String){
			betInfo->highest_cards = highest_cards->valuestring;
		}
		if ((public_cards = cJSON_GetObjectItem(jsonMsg, "public_cards")) && public_cards->type == cJSON_String){
			betInfo->public_cards = public_cards->valuestring;
		}
		if ((cards = cJSON_GetObjectItem(jsonMsg, "cards")) && cards->type == cJSON_String){
			betInfo->cards = cards->valuestring;
		}
		result = true;
	}
	return result;
}
bool GameControlBjl::InitBetProfits(string jsonData, vector<BJLBETPROFIT> *pBetProfits)
{
	if (jsonData.empty() || !pBetProfits)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	bool result = false;
	int size;
	if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg))>0)
	{
		result = true;
		cJSON *item, *pos, *balance, *amount;
		BJLBETPROFIT betProfit;
		for (int i = 0; i < size; i++)
		{
			if ((item = cJSON_GetArrayItem(jsonMsg, i)) && (item->type == cJSON_Array) && (cJSON_GetArraySize(item) == 3)
				&& (pos = cJSON_GetArrayItem(item, 0)) && (pos->type == cJSON_Number)
				&& (balance = cJSON_GetArrayItem(item, 1)) && (balance->type == cJSON_Number)
				&& (amount = cJSON_GetArrayItem(item, 2)) && (amount->type == cJSON_Number))
			{
				betProfit.pos = pos->valueint;
				betProfit.balance = balance->valuedouble;
				betProfit.amount = amount->valuedouble;
				pBetProfits->push_back(betProfit);
			}
			else
			{
				result = false;
				break;
			}
		}
	}
	cJSON_Delete(jsonMsg);
	return result;
}
bool GameControlBjl::InitBetResultInfo(string jsonData, int64_t *pBalance, int *pNum, BJLBETINFO *pBetInfo)
{
	if (jsonData.empty() || !pBalance || !pNum || !pBetInfo)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	bool result = false;
	cJSON *balance, *success_bets, *game_num;
	if (jsonMsg->type == cJSON_Object
		&& (balance = cJSON_GetObjectItem(jsonMsg, "balance")) && (balance->type == cJSON_Number)
		&& (success_bets = cJSON_GetObjectItem(jsonMsg, "success_bets")) && (success_bets->type == cJSON_Array)
		&& (game_num = cJSON_GetObjectItem(jsonMsg, "game_num")) && (game_num->type == cJSON_Number))
	{
		result = true;
		*pBalance = balance->valueint;
		*pNum = game_num->valueint;
		int size = cJSON_GetArraySize(success_bets);
		cJSON *betInfo, *type, *amount;
		int subsize;
		pBetInfo->p = pBetInfo->pp = pBetInfo->b = pBetInfo->bp = pBetInfo->t = 0;
		for (int i = 0; i < size; i++)
		{
			if ((betInfo = cJSON_GetArrayItem(success_bets, i)) && (betInfo->type == cJSON_Array) && (subsize = cJSON_GetArraySize(betInfo)) == 2
				&& (type = cJSON_GetArrayItem(betInfo, 0)) && (type->type == cJSON_String)
				&& (amount = cJSON_GetArrayItem(betInfo, 1)) && (amount->type == cJSON_Number))
			{
				if (!strcmp(type->valuestring, "p"))
					pBetInfo->p += amount->valueint;
				else if (!strcmp(type->valuestring, "pp"))
					pBetInfo->pp += amount->valueint;
				else if (!strcmp(type->valuestring, "b"))
					pBetInfo->b += amount->valueint;
				else if (!strcmp(type->valuestring, "bp"))
					pBetInfo->bp += amount->valueint;
				else if (!strcmp(type->valuestring, "t"))
					pBetInfo->t += amount->valueint;
				else
				{
					//未知类型，返回出错
					result = false;
					break;
				}
			}
			else
			{
				result = false;
				break;
			}
		}
	}
	cJSON_Delete(jsonMsg);
	return result;
}
bool GameControlBjl::InitBjlRoomBet(string jsonData, map<int, BJLBETINFO*> *roomBet)
{
	if (jsonData.empty() || !roomBet)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	int size;
	bool result = false;
	if (jsonMsg->type == cJSON_Array)
	{
		size = cJSON_GetArraySize(jsonMsg);
		if (size > 0){
			cJSON *betData, *sitId, *betInfo;
			for (int i = 0; i < size; i++){
				if ((betData = cJSON_GetArrayItem(jsonMsg, i)) && (betData->type == cJSON_Array) && (cJSON_GetArraySize(betData) == 2)
					&& (sitId = cJSON_GetArrayItem(betData, 0)) && (sitId->type == cJSON_Number)
					&& (betInfo = cJSON_GetArrayItem(betData, 1)) && (betInfo->type == cJSON_Object))
				{
					BJLBETINFO *newBetInfo = new BJLBETINFO();
					if (InitBjlBetInfo(betInfo, newBetInfo))
						(*roomBet)[sitId->valueint] = newBetInfo;
					else
						delete newBetInfo;
				}
			}
		}
		result = true;
	}
	cJSON_Delete(jsonMsg);
	return result;
}
// 开始翻牌消息
bool GameControlBjl::InitFpBeginData(string jsonData, int *playerId, int *bankerId){
	if (jsonData.empty() || !playerId || !bankerId)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	cJSON *item1, *item2;
	bool result = false;
	if (jsonMsg->type == cJSON_Array && cJSON_GetArraySize(jsonMsg) == 2
		&& (item1 = cJSON_GetArrayItem(jsonMsg, 0)) && item1->type == cJSON_Number
		&& (item2 = cJSON_GetArrayItem(jsonMsg, 1)) && item2->type == cJSON_Number){
		*playerId = item1->valueint;
		*bankerId = item2->valueint;
		result = true;
	}
	cJSON_Delete(jsonMsg);
	return result;
}
// 翻牌状态切换,[0:闲开牌, 1:庄开牌, 2:闲补牌, 3:庄补牌], time:结束时间
bool GameControlBjl::InitFpStatusData(string jsonData, int *status, double *endTime){
	if (jsonData.empty() || !status || !endTime)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	cJSON *item1, *item2;
	bool result = false;
	if (jsonMsg->type == cJSON_Array && cJSON_GetArraySize(jsonMsg) == 2
		&& (item1 = cJSON_GetArrayItem(jsonMsg, 0)) && item1->type == cJSON_Number
		&& (item2 = cJSON_GetArrayItem(jsonMsg, 1)) && item2->type == cJSON_Number){
		*status = item1->valueint;
		*endTime = item2->valuedouble;
		result = true;
	}
	cJSON_Delete(jsonMsg);
	return result;
}
// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
bool GameControlBjl::InitFpSelectData(string jsonData, int *card){
	if (jsonData.empty() || !card)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	bool result = false;
	if (jsonMsg->type == cJSON_Number){
		*card = jsonMsg->valueint;
		result = true;
	}
	cJSON_Delete(jsonMsg);
	return result;
}
// 翻牌中
bool GameControlBjl::InitFpingData(string jsonData, int *x1, int *y1, int *x2, int *y2){
	if (jsonData.empty() || !x1 || !y1 || !x2 || !y2)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	cJSON *items[2], *pt0[2], *pt1[2];
	bool result = false;
	int pos = 0;
	if (jsonMsg->type == cJSON_Array && (cJSON_GetArraySize(jsonMsg) == 2)
		&& (items[0] = cJSON_GetArrayItem(jsonMsg, 0)) && items[0]->type == cJSON_Array && (cJSON_GetArraySize(items[0]) == 2)
		&& (items[1] = cJSON_GetArrayItem(jsonMsg, 1)) && items[1]->type == cJSON_Array && (cJSON_GetArraySize(items[1]) == 2)
		&& (pt0[0] = cJSON_GetArrayItem(items[0], 0)) && pt0[0]->type == cJSON_Number
		&& (pt0[1] = cJSON_GetArrayItem(items[0], 1)) && pt0[1]->type == cJSON_Number
		&& (pt1[0] = cJSON_GetArrayItem(items[1], 0)) && pt1[0]->type == cJSON_Number
		&& (pt1[1] = cJSON_GetArrayItem(items[1], 1)) && pt1[1]->type == cJSON_Number){
		*x1 = pt0[0]->valueint;
		*y1 = pt0[1]->valueint;
		*x2 = pt1[0]->valueint;
		*y2 = pt1[1]->valueint;
		result = true;
	}
	cJSON_Delete(jsonMsg);
	return result;
}
// 开牌
bool GameControlBjl::InitFpOpenData(string jsonData, int *card){
	return InitFpSelectData(jsonData, card);		//因为数据结构相同，所以使用FpSelect函数来处理
}
// 中途进入房间时，翻牌初始化信息
bool GameControlBjl::InitFpInitData(string jsonData, int *playerid, int *bankerid, int *status, double *endTime, vector<int> *cardStatus){
	if (jsonData.empty() || !playerid || !bankerid || !status || !endTime || !cardStatus)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	cJSON *items[5];
	bool result = false;
	int pos = 0;
	if (jsonMsg->type == cJSON_Array && (cJSON_GetArraySize(jsonMsg) == (sizeof(items) / sizeof(items[0])))
		&& (items[pos] = cJSON_GetArrayItem(jsonMsg, pos)) && items[pos++]->type == cJSON_Number
		&& (items[pos] = cJSON_GetArrayItem(jsonMsg, pos)) && items[pos++]->type == cJSON_Number
		&& (items[pos] = cJSON_GetArrayItem(jsonMsg, pos)) && items[pos++]->type == cJSON_Number
		&& (items[pos] = cJSON_GetArrayItem(jsonMsg, pos)) && items[pos++]->type == cJSON_Number
		&& (items[pos] = cJSON_GetArrayItem(jsonMsg, pos)) && items[pos]->type == cJSON_Array && cJSON_GetArraySize(items[pos]) == 6){
		cJSON *jsonCards = items[pos];
		cJSON *subItems[6];
		result = true;
		for (int i = 0; i < 6; i++){
			if ((subItems[i] = cJSON_GetArrayItem(jsonCards, i)) && subItems[i]->type == cJSON_Number){
				cardStatus->push_back(subItems[i]->valueint);
			}
			else{
				result = false;
				break;
			}
		}
		if (result){
			*playerid = items[0]->valueint;
			*bankerid = items[1]->valueint;
			*status = items[2]->valueint;
			*endTime = items[3]->valuedouble;
		}
	}
	cJSON_Delete(jsonMsg);
	return result;
}
bool GameControlBjl::InitSitPlayers(string jsonData, vector<BjlRoomSitInfo> *sitPlayers)
{
	if (jsonData.empty() || !sitPlayers)
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		result = true;
		int size;
		if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg)) > 0)
		{
			int playersize;
			cJSON  *player, *pos, *userid, *balance;
			for (int i = 0; i < size; i++)
			{

				if ((player = cJSON_GetArrayItem(jsonMsg, i)) && player->type == cJSON_Array && (playersize = cJSON_GetArraySize(player)) > 2
					&& (pos = cJSON_GetArrayItem(player, 0)) && pos->type == cJSON_Number
					&& (userid = cJSON_GetArrayItem(player, 1)) && userid->type == cJSON_Number
					&& (balance = cJSON_GetArrayItem(player, 2)) && balance->type == cJSON_Number)
				{
					BjlRoomSitInfo sitInfo;
					sitInfo.pos= pos->valueint;
					sitInfo.userid = userid->valueint;
					sitInfo.balance = balance->valuedouble;
					sitPlayers->push_back(sitInfo);
				}
			}
		}
		cJSON_Delete(jsonMsg);
	}
	return result;
}
bool GameControlBjl::InitRoomPlayers(string jsonData, map<int,BjlRoomPlayer*> *roomPlayers)
{
	if (jsonData.empty())
		return false;
	bool result = false;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		int size;
		if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg)) > 0)
		{
			result = true;
			int playersize;
			cJSON  *player, *id, *nickname, *avatar;
			for (int i = 0; i < size; i++)
			{

				//CCLOG("player->type == cJSON_Array  = %d",player->type == cJSON_Array);


				if ((player = cJSON_GetArrayItem(jsonMsg, i)) && player->type == cJSON_Array && (playersize = cJSON_GetArraySize(player)) == 3
					&& (id = cJSON_GetArrayItem(player, 0)) && id->type == cJSON_Number
					&& (nickname = cJSON_GetArrayItem(player, 1)) && nickname->type == cJSON_String
					&& (avatar = cJSON_GetArrayItem(player, 2)) && avatar->type == cJSON_Number)
				{
					BjlRoomPlayer *roomPlayer = new BjlRoomPlayer();
					roomPlayer->id = id->valueint;
					roomPlayer->nickname = nickname->valuestring;
					roomPlayer->avatar = avatar->valueint;
					(*roomPlayers)[roomPlayer->id] = roomPlayer;
				}
			}
		}

		cJSON_Delete(jsonMsg);
	}
	return result;
}
BetResultData* GameControlBjl::InitBetResultData(string jsonData)
{
	if (jsonData.empty())
		return nullptr;
	BetResultData *betResultData = nullptr;
	cJSON *jsonMsg;
	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		cJSON  *bonus, *next_game_num, *secret, *seed, *next_secret, *data, *is_end, *balance;
		if (jsonMsg->type == cJSON_Object
			&& (bonus = cJSON_GetObjectItem(jsonMsg, "bonus")) && bonus->type == cJSON_Number
			&& (next_game_num = cJSON_GetObjectItem(jsonMsg, "next_game_num")) && next_game_num->type == cJSON_Number
			&& (secret = cJSON_GetObjectItem(jsonMsg, "secret")) && secret->type == cJSON_String
			&& (seed = cJSON_GetObjectItem(jsonMsg, "seed")) && seed->type == cJSON_String
			&& (next_secret = cJSON_GetObjectItem(jsonMsg, "next_secret")) && next_secret->type == cJSON_String
			&& (data = cJSON_GetObjectItem(jsonMsg, "data")) && data->type == cJSON_String
			&& (is_end = cJSON_GetObjectItem(jsonMsg, "is_end")) && (is_end->type == cJSON_False || is_end->type == cJSON_True)
			&& (balance = cJSON_GetObjectItem(jsonMsg, "balance")) && balance->type == cJSON_Object)
		{
			betResultData = new BetResultData();
			betResultData->bonus = bonus->valueint;
			betResultData->next_game_num = next_game_num->valueint;
			betResultData->secret = secret->valuestring;
			betResultData->seed = seed->valuestring;
			betResultData->next_secret = next_secret->valuestring;
			betResultData->data = data->valuestring;
			betResultData->is_end = is_end->type == cJSON_True;
			betResultData->balance = cJSON_PrintUnformatted(balance);
		}
		cJSON_Delete(jsonMsg);
	}
	return betResultData;
}

//根据节点创建playerpanel类，index为头像索引,flip是否水平翻转
BjlTablePlayerPanel* BjlTablePlayerPanel::create(Node *root,int num, int index, bool flip)
{
	if (!root)
		return nullptr;
	BjlTablePlayerPanel *pRet = new BjlTablePlayerPanel();
	if (!(pRet->imgAvatar = root->getChildByName<Sprite*>("imgAvatar"))
		|| !(pRet->imgBgAmount = root->getChildByName<Sprite*>("imgBgAmount"))
		|| !(pRet->imgCm = root->getChildByName<Sprite*>("imgCm"))
		|| !(pRet->lbNickName = root->getChildByName<Text*>("lbNickName"))
		|| !(pRet->lbAmount = root->getChildByName<Text*>("lbAmount"))
		|| !(pRet->btSit = root->getChildByName<Button*>("btSit")))
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
	pRet->num = num;
	pRet->RootNode = root;
	pRet->_index = index;
	pRet->_flip = flip;
	pRet->btSit->addClickEventListener(CC_CALLBACK_1(BjlTablePlayerPanel::btSitClicked, pRet));
	pRet->_playerData=(BjlRoomPlayer*)1;		//设置为1，然后强制更新
	pRet->SetPlayer(nullptr);
	//设置大赢家中心点位置
	pRet->ptBigWinner = root->convertToWorldSpace(pRet->lbNickName->getPosition());
	return pRet;
}
void BjlTablePlayerPanel::btSitClicked(Ref *pSender)
{
	PKNotificationCenter::getInstance()->postNotification("BjlRoomPlayerSit", this);
}
bool BjlTablePlayerPanel::Empty()
{
	return _playerData == nullptr;
}
void BjlTablePlayerPanel::SetPlayer(BjlRoomPlayer *player)
{
	if (_playerData == player)
		return;
	_winAmount = 0;
	_playerData = player;
	if (_playerData == nullptr)
	{
		//无人入座,隐藏除了btSit之外，所有node
		for (auto node : RootNode->getChildren())
		{
			node->setVisible(node->getName() == "btSit");
		}
	}
	else
	{
		//有人入座,显示除了btSit之外，所有node
		for (auto node : RootNode->getChildren())
		{
			node->setVisible(node->getName() != "btSit");
		}
		//提取头像并显示
		imgAvatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(_playerData->avatar, _index));
		//显示昵称和金额
		lbNickName->setString(_playerData->nickname);
		lbAmount->setString(Comm::GetFloatShortStringFromInt64(_playerData->balance));
		//lbAmount->setString(toString(_playerData->amount));
	}
}
void BjlTablePlayerPanel::SetBigWinner(bool enable)
{
	if (enable)
	{
		//大赢家
		RootNode->setZOrder(100);
	}
	else
	{
		//恢复正常
		RootNode->setZOrder(0);
	}
}
void BjlTablePlayerPanel::SetProfit(int64_t balance)
{
	if (_playerData == nullptr)
		return;
	_winAmount = balance - _playerData->balance;			//新balance-旧balance，就是赢钱金额，负数为输
	_playerData->balance = balance;
	lbAmount->setString(Comm::GetFloatShortStringFromInt64(_playerData->balance));
}
BjlTableControl* BjlTableControl::create(Node *root)
{
	if (!root)
		return nullptr;
	auto spMask = root->getChildByName<Sprite*>("spMask");
	if (!spMask)
		return nullptr;
	BjlTableControl *pRet = new BjlTableControl();
	for (int i = 0; i < 8; i++)
	{

		// 头像索引,位置0,7索引2,位置1,6索引为1，其他索引为0
		int index;
		switch (i)
		{
		case 0:
		case 7:
			index = 2;
			break;
		case 1:
		case 6:
			index = 1;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			index = 0;
			break;
		default:
			index = -1;
			break;
		}
		BjlTablePlayerPanel *playerPanel;
		if (index == -1 || !(playerPanel = BjlTablePlayerPanel::create(root->getChildByName(StringUtils::format("panelPlayer%d", i)), i + 1, index, i > 3)))
		{
			for (auto player : pRet->_playerPanels)
			{
				delete player.second;
			}
			pRet->_playerPanels.clear();
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
		playerPanel->SetPlayer(nullptr);		//默认都没有人座
		pRet->_playerPanels[i + 1] = playerPanel;
	}
	pRet->RootNode = root;
	spMask->setZOrder(1);			//mask的zorder设置为1，以盖住其他玩家
	pRet->spMask = spMask;
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("bjl/room/PlistBigWinner.plist");
	Sprite *spBigWinner = Sprite::create();
	spBigWinner->setVisible(false);
	pRet->RootNode->addChild(spBigWinner, 1);	//bigwinner的动画zorder设置为1，以显示在mask层上
	pRet->spBigWinner = spBigWinner;
	return pRet;
}
bool BjlTableControl::SetPlayerOnSeat(int pos, BjlRoomPlayer *player)
{
	if (_playerPanels.find(pos) == _playerPanels.end())
		return false;
	_playerPanels[pos]->SetPlayer(player);
}
void BjlTableControl::SetPlayerProfit(int pos, int64_t balance)
{
	if (_playerPanels.find(pos) == _playerPanels.end())
		return;
	_playerPanels[pos]->SetProfit(balance);
}
//根据用户ID，获取座位ID，无座时，返回0
int BjlTableControl::GetSitIdFromUserId(int uid)
{
	BjlTablePlayerPanel *panelPlayer;
	for (auto panel : _playerPanels)
	{
		panelPlayer = panel.second;
		if (panelPlayer&&panelPlayer->_playerData&&panelPlayer->_playerData->id == uid)
			return panel.first;
	}
	return 0;
}
void BjlTableControl::ShowBigWinner(float dt)
{
	// find bigwinner
	if (dt <= 0.0f)
		return;
	BjlTablePlayerPanel* panelBigWinner = nullptr;
	int64_t maxAmount = 0;
	for (int i = 1; i <= 8; i++)
	{
		if (_playerPanels[i]->_winAmount > maxAmount)
		{
			maxAmount = _playerPanels[i]->_winAmount;
			panelBigWinner = _playerPanels[i];
		}
	}
	if (!panelBigWinner)
		return;			//没有大赢家
	spMask->setVisible(true);			// show mask
	panelBigWinner->RootNode->setZOrder(2);		//set winner's zorder
	// show animation
	//创建bigwinner动画
	Animation *animation = Animation::create();
	for (int i = 1; i <= 11; i++)
		animation->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(StringUtils::format("big_winner_%02d.png", i)));
	animation->setDelayPerUnit(2.0f / 11.0f);
	Animate *animate = Animate::create(animation);
	spBigWinner->setPosition(panelBigWinner->ptBigWinner);
	spBigWinner->setVisible(true);
	spBigWinner->runAction(Spawn::create(animate, Sequence::create(DelayTime::create(dt), CallFunc::create([=](){
		panelBigWinner->RootNode->setZOrder(0);
		spMask->setVisible(false);
		spBigWinner->setVisible(false);
	}), nullptr), nullptr));
    SoundControl::PlayEffect("sound/win2.mp3");
	
	if (panelBigWinner->_playerData->id == UserControl::GetInstance()->GetUserData()->user_id)
	{
		//验证完成任务 第一次成为BIG WINNER
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(3)));
	}
}
void BjlTableControl::Test()
{
	//生成随机测试
	for (int i = 0; i < 8; i++)
	{
		BjlRoomPlayer player;
		player.id = random(10000, 99999);
		player.avatar = random(1, 5);
		player.nickname = cocos2d::StringUtils::format("ID:%d", player.id);
		SetPlayerOnSeat(i + 1, &player);
	}
}
//根据node节点，创建sitchat类，dt表示聊天信息显示时长
BjlSitChat* BjlSitChat::create(int sid,Node *node, float dt)
{
	if (!node || dt <= 0)
		return nullptr;
	ImageView *imgBg;
	Text *lbMsg;
	if (!(imgBg = node->getChildByName<ImageView*>("imgBg"))
		|| !(lbMsg = node->getChildByName<Text*>("lbMsg")))
		return nullptr;
	imgBg->setScale(1.0f);		//恢复正常缩放比
	BjlSitChat *pRet = new BjlSitChat();
	pRet->_nodeRoot = node;
	pRet->_sid = sid;
	pRet->_imgBg = imgBg;
	pRet->_lbMsg = lbMsg;
	pRet->_dt = dt;
	pRet->_nodeRoot->setVisible(false);		//默认隐藏
	pRet->_lbMaxWidth = pRet->_imgBg->getSize().width - MARGIN_LEFT - MARGIN_RIGHT;
	return pRet;
}
//设置聊天信息
void BjlSitChat::setString(string msg)
{
	if (msg.empty())
		return;
	
	//设置text为变长控件，以便测试文本宽度
	_lbMsg->setTextAreaSize(Size(0, 0));
	_lbMsg->ignoreContentAdaptWithSize(true);
	_lbMsg->setString(msg);
	Size size = _lbMsg->getSize();
	if (size.width > _lbMaxWidth)
	{
		//超宽度，需要改为固定宽度文本框显示
		_lbMsg->setTextAreaSize(Size(_lbMaxWidth, 0));
		_lbMsg->ignoreContentAdaptWithSize(false);
		size = _lbMsg->getSize();
		if (size.width > _lbMaxWidth)
		{
			//需要强制换行，一般为连续英文字母
			//此方法，并不适用于中英文混排
			int i = 22;
			while (i<msg.size())
			{
				msg.insert(i, "\n");
				i += 23;
			}
			_lbMsg->setTextAreaSize(Size(0, 0));
			_lbMsg->ignoreContentAdaptWithSize(true);
			_lbMsg->setString(msg);
			_lbMsg->setTextAreaSize(Size(_lbMaxWidth, 0));
			_lbMsg->ignoreContentAdaptWithSize(false);
			size = _lbMsg->getSize();
		}
	}
	
	//根据text大小，设置背景大小
	size.width += MARGIN_LEFT + MARGIN_RIGHT;
	size.height += MARGIN_TOP + MARGIN_BOTTOM;
	_imgBg->setSize(size);
	//显示控件，并延迟调用隐藏功能
	_nodeRoot->setVisible(true);
	_nodeRoot->runAction(Sequence::create(DelayTime::create(_dt), Hide::create(), nullptr));
}
BjlSitChatControl::~BjlSitChatControl()
{
	//清理数据
	for (auto item : _bjlSitChats)
	{
		if (item.second)
			delete item.second;
	}
	_bjlSitChats.clear();
}
BjlSitChatControl* BjlSitChatControl::create(Node *node)
{
	if (!node)
		return nullptr;
	BjlSitChatControl *pRet = new BjlSitChatControl();
	for (int i = 1; i <= 8; i++)
	{
		if (!(pRet->_bjlSitChats[i] = BjlSitChat::create(i, node->getChildByName(StringUtils::format("nodeChat_%d", i)))))
		{
			delete pRet;
			pRet = nullptr;
			break;
		}
	}
	pRet->_nodeRoot = node;
	pRet->_nodeRoot->setZOrder(1);			//将zorder设置为1，以盖住玩家
	return pRet;
}
void BjlSitChatControl::ShowChat(int pos,string msg)
{
	if (_bjlSitChats.find(pos) == _bjlSitChats.end())
		return;
	_bjlSitChats[pos]->setString(msg);
}

bool GameControlBjl::SyncBet()
{

	if (_bet.size() == 0)
		return false;

	std::map<string, BjlBet> merger;

	vector <BjlBet>::iterator iter;

	for (iter = _bet.begin(); iter != _bet.end(); iter++)
	{
		BjlBet bet = *iter;
		string key = StringUtils::format("%d_%d", (int)bet.m_type, bet.m_gameNum);

		std::map<string, BjlBet>::iterator mergerIter;
		mergerIter = merger.find(key);
		if (mergerIter != merger.end())
		{
			mergerIter->second.m_amount += bet.m_amount;
		}
		else
		{
			merger.insert(std::map<string, BjlBet>::value_type(key, bet));
		}
	}

	for (auto val : merger)
	{
		bool succeed = Api::Game::mo_baccarat_bet(BjlBetTypeToString(val.second.m_type), val.second.m_amount, val.second.m_gameNum);

		if (!succeed)
			return succeed;
	}

	_bet.clear();

	return true;
}

void GameControlBjl::Bet(BjlBet bet)
{
	_bet.push_back(bet);
}

void GameControlBjl::FinishBetTask(RoomInfoBjl *info)
{
	//投注成功
	//SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BET);
	if (info->type->max_bet <= 10000){
		//验证完成任务 首次在中级房进行游戏
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(8)));
	}
	else if (info->type->max_bet <= 10000 * 10){
		//验证完成任务 首次在中级房进行游戏
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(9)));
	}
	else{
		//验证完成任务 首次在高级房进行游戏
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(10)));
	}
}