#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
namespace Api
{
	class Game
	{
	public:
		Game();
		~Game();
		static bool sg_baccarat_begin_game(bool isNew, int roomTypeId);
		static bool sg_baccarat_get_game(char* gameUuid);
		static bool sg_baccarat_bet_game(const char* gameUuid, int game_num, int currency_type, int b, int p, int t, int b_p, int p_p);
		static bool mo_baccarat_get_room_results(vector<int> *roomIds);
		static bool mo_baccarat_connect();
		static bool mo_baccarat_disconnect();
		static bool mo_baccarat_login();
		static bool mo_baccarat_enter(int roomId);
		static bool mo_baccarat_leave();
		static bool mo_baccarat_chat(string msg);
		static bool mo_baccarat_sit(int num);
		static bool mo_baccarat_bet(const char* side, int amount, int num);
		static bool sg_baccarat_get_game_page_game_indexes(string strFrom, string strTo, int page_index, int page_size, int child);
		static bool sg_baccarat_get_game_page_game_round(int page_index, int page_size);
		static bool sg_baccarat_get_page_game_info(int page_index, int page_size);
		static bool sg_baccarat_get_game_SG_BACCARAT_BET_GAME(int page_index, int page_size);
		static bool sg_baccarat_game_get_user_game_round(int round_id,int child);
		static bool mo_baccarat_fp_select(int card);
		static bool mo_baccarat_fp_flip(int x1, int y1, int x2, int y2);
		static bool mo_baccarat_fp_open(int card);
		
		static bool mo_baccarat_fp_end();
		//排行
		static bool sg_bill_get_rank(unsigned short rankType);

		static bool mo_dzpk_connect();
		static bool mo_dzpk_disconnect();
		static bool mo_dzpk_login_game();//登入游戏大厅
		static bool mo_dzpk_join_game(int roomType);//加入游戏
		static bool mo_dzpk_leave();
		static bool mo_dzpk_chat(string msg);
		static bool mo_dzpk_sit(int num);
		static bool mo_dzpk_tp_bet(int round_num, int action, int amount);
		static bool mo_dzpk_create_room(int type);
		static bool mo_dzpk_join_room(const char* token);

		static bool count_game_indexes();

		static bool game_level_config();

		static bool get_game_stat(int user_id);


		static bool get_avaliable_games(int user_id);

		static bool get_channel_avaliable_games(const char* channel);
		
		static bool update_game_step(int user_id, int game_type, int step);
		static bool get_game_step(int user_id, int game_type);
		static bool page_game_step();



		static bool mo_sng_dzpk_connect();
		

		static bool mo_sng_dzpk_disconnect();


		static bool mo_sng_dzpk_login_game();

		static bool mo_sng_apply_start(int apply_currency_type);

		static bool mo_dzpk_enter_sng_game(int roomId);

		static bool mo_dzpk_leave_sng_game();

		static bool mo_sng_buy_chip(int count);
		
	};
}


