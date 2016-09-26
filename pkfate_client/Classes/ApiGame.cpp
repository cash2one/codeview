#include "ApiGame.h"
#include "SocketControl.h"
#include "bjlcomm.h"
#include "dzpkcomm.h"
#include "cmd.h"
#include "GuideControl.h"
using namespace Api;
Game::Game()
{
}
Game::~Game()
{
}



bool Game::mo_sng_dzpk_connect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK,MSGTYPE_DzpkGameMo_Connect, "{}");
}

bool Game::mo_sng_dzpk_disconnect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, MSGTYPE_DzpkGameMo_DisConnect, "{}");
}

bool Game::mo_sng_dzpk_login_game()
{

	//德州扑克登录游戏
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, MSGTYPE_DzpkGameMo_Login, "");
}

bool Game::mo_sng_apply_start(int apply_currency_type)
{
	
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, TP_SNG_BEGIN_APPLY, toString(apply_currency_type));
}


bool Game::mo_dzpk_enter_sng_game(int roomId)
{
	
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, MSGTYPE_DzpkGameMo_EnterRoom, toString(roomId));
}


bool Game::mo_dzpk_leave_sng_game()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, MSGTYPE_DzpkGameMo_LeaveRoom, "{}");
}

bool Game::mo_sng_buy_chip(int count)
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_SNG_DZPK, TP_SNG_BUY_CHIP, toString(count));
}



//------------------------------------------------------------------------------------------------------------------------
bool Game::mo_dzpk_connect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Connect, "{}");
}

bool Game::mo_dzpk_disconnect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_DisConnect, "{}");
}

bool Game::mo_dzpk_login_game()
{
	
	//德州扑克登录游戏
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Login, "");
}


bool Game::mo_dzpk_join_game(int roomType)
{
	//德州扑克加入游戏
	if (GuideControl::GetInstance()->checkInGuide()){
		return GuideControl::GetInstance()->sendVirtual(MSGTYPE_DZPKGAMEMO, TP_JOIN_GAME, toString(roomType));
	}
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, TP_JOIN_GAME, toString(roomType));
}

bool Game::mo_dzpk_leave()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_LeaveRoom, "");
}

bool Game::mo_dzpk_tp_bet(int round_num, int action, int amount)
{
	if (GuideControl::GetInstance()->checkInGuide()){
		return GuideControl::GetInstance()->sendVirtual(MSGTYPE_DZPKGAMEMO, TP_BET, "");
	}
	char args[BUFSIZE];
	sprintf(args, "[%d,%d,%d]", round_num, action, amount);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, TP_BET, args);
}


bool Game::mo_dzpk_chat(string msg)
{
	if (msg.empty())
		return false;
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Chat, "\"" + msg + "\"");
}
bool Game::mo_dzpk_sit(int num)
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Sit, toString(num));
}

bool Game::mo_dzpk_create_room(int type)
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, TP_CREATE_ROOM, toString(type));
}

bool Game::mo_dzpk_join_room(const char* token)
{
	
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_DZPKGAMEMO, TP_JOIN_ROOM, StringUtils::format("\"%s\"",token));
}





bool Game::sg_baccarat_begin_game(bool isNew, int roomTypeId)
{
	char args[BUFSIZE];
	sprintf(args, "{\"room_type_id\":%d,\"is_new\":%s}", roomTypeId, isNew ? "true" : "false");
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_BEGIN_GAME, args);
}
bool Game::sg_baccarat_get_game(char* gameUuid)
{
	char args[BUFSIZE];
	sprintf(args, "{\"game_uuid\":\"%s\"}", gameUuid);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_GET_GAME, args);
}
bool Game::sg_baccarat_bet_game(const char* gameUuid, int game_num, int currency_type, int b, int p, int t, int b_p, int p_p)
{
	char args[BUFSIZE];
	sprintf(args, "{\"game_uuid\":\"%s\",\"game_num\":%d,\"currency_type\":%d,\"bet_info\":{\"b\":%d,\"p\":%d,\"t\":%d,\"bp\":%d,\"pp\":%d}}", gameUuid, game_num, currency_type, b, p, t, b_p, p_p);

	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_BET_GAME, args);
}

bool Game::mo_baccarat_get_room_results(vector<int> *roomIds)
{
	if (roomIds == nullptr || roomIds->size() <= 0)
		return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomResults, "");
	else
	{
		stringstream ss;
		ss << "[";
		for (auto id : *roomIds)
		{
			ss << id << ",";
		}
		string ids = ss.str();
		ids = ids.substr(0, ids.size() - 1);
		ids += "]";
		return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomResults, ids);
	}
}

bool Game::mo_baccarat_connect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Connect, "{}");
}
bool Game::mo_baccarat_disconnect()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_DisConnect, "{}");
}
bool Game::mo_baccarat_login()
{
	
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Login, "{}");
}
bool Game::mo_baccarat_enter(int roomId)
{
	
	if (GuideControl::GetInstance()->checkInGuide()){
		return GuideControl::GetInstance()->sendVirtual(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_EnterRoom, "");
	}

	char args[BUFSIZE];
	sprintf(args, "%d", roomId);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_EnterRoom, args);
}
bool Game::mo_baccarat_leave()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_LeaveRoom, "");
}
bool Game::mo_baccarat_bet(const char* side, int amount, int num)
{
	if (GuideControl::GetInstance()->checkInGuide())
	{
		return GuideControl::GetInstance()->sendVirtual(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Bet, "");
	}

	if (!side || strcmp(side, "") == 0)
		return false;

	char args[BUFSIZE];
	sprintf(args, "{\"side\":\"%s\",\"amount\":%d,\"game_num\":%d}", side, amount, num);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Bet, args);
}
bool Game::mo_baccarat_chat(string msg)
{
	if (msg.empty())
		return false;
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Chat, "\"" + msg + "\"");
}
bool Game::mo_baccarat_sit(int num)
{
	
	if (GuideControl::GetInstance()->checkInGuide()){
		return GuideControl::GetInstance()->sendVirtual(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Sit, "");
	}

	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Sit, toString(num));
}
bool Game::mo_baccarat_fp_select(int card){
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGCMD_BJLGAMEMO::CARD_SELECT, toString(card));
}
bool Game::mo_baccarat_fp_flip(int x1, int y1, int x2, int y2){
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGCMD_BJLGAMEMO::CARD_FLIP, StringUtils::format("[[%d,%d],[%d,%d]]", x1, y1, x2, y2));
}
bool Game::mo_baccarat_fp_open(int card){
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGCMD_BJLGAMEMO::CARD_OPEN, toString(card));
}



bool Game::mo_baccarat_fp_end(){
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BJLGAMEMO, MSGCMD_BJLGAMEMO::FLIP_OVER, "");
}
bool Game::sg_baccarat_get_game_page_game_indexes(string strFrom, string strTo, int page_index, int page_size,int child) {
	if (strFrom.empty() || strTo.empty() || page_index <= 0 || page_size <= 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args, "{\"create_date\":[\"%s\",\"%s\"],\"page_index\":%d,\"page_size\":%d,\"child\":%d}", strFrom.c_str(), strTo.c_str(), page_index, page_size, child);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::PAGE_GAME_INDEXES, args);
}


bool Game::sg_baccarat_get_game_page_game_round(int page_index, int page_size) {
	if (page_index <= 0 || page_size <= 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args, "{\"page_index\":%d,\"page_size\":%d}", page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::PAGE_GAME_ROUND, args);
}

bool Game::sg_baccarat_get_page_game_info(int page_index, int page_size) {
	if (page_index <= 0 || page_size <= 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args, "{\"page_index\":%d,\"page_size\":%d}", page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::PAGE_GAME_INFO, args);
}

bool Game::sg_baccarat_get_game_SG_BACCARAT_BET_GAME(int page_index, int page_size) {
	if (page_index <= 0 || page_size <= 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args, "{\"page_index\":%d,\"page_size\":%d}", page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_BET_GAME, args);
}

bool Game::sg_baccarat_game_get_user_game_round(int round_id,int child) {
	if (round_id < 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args, "{\"round_id\":%d,\"child\":%d}", round_id, child);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GET_USER_GAME_ROUND, args);
}

bool Game::sg_bill_get_rank(unsigned short rankType){

	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, rankType, "");
}


bool Game::count_game_indexes(){

	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::COUNT_GAME_INDEXES, "");
}


bool Game::game_level_config(){
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GAME_LEVEL_CONFIG, "");
}

bool Game::get_game_stat(int user_id){
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GET_GAME_STAT, args);
}


bool Game::get_avaliable_games(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GET_AVALIABLE_GAMES, args);
}

bool Game::get_channel_avaliable_games(const char* channel)
{
	char args[BUFSIZE];
	sprintf(args, "{\"channel\":\"%s\"}", channel);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GET_CHANNEL_AVALIABLE_GAMES, args);
}



bool Game::update_game_step(int user_id, int game_type, int step)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"game_type\":%d,\"step\":%d}", user_id, game_type, step);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::UPDATE_GAME_STEP, args);
}

bool Game::get_game_step(int user_id, int game_type)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"game_type\":%d}", user_id, game_type);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_GAME, MSGCMD_GAME::GET_GAME_STEP, args);
}

bool Game::page_game_step()
{
	return false;
}



