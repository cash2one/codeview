#pragma once
#include "cocos2d.h"
#include "comm.h"
USING_NS_CC;
using namespace std;

struct UserData{
	std::string username;
	short status;
	int user_id;
	std::string last_login_time;
	std::string zone;
	std::string rights;
	std::string mobile;
	int parent_id;
	std::string create_time;
	int avatar;
	int children_num;
	short login_type;
	std::string nickname;
	std::string email;
	std::string login_ip;
	std::string token;
	int level; //级别
	float expRate;//下一级经验比列
};

struct GameData{
	int bet_amount;
	int bet_amount_rank;
	int bonus;
	int bonus_rank;
	int game_type;
	int total_round;
	int win_round;
	int valid_bet_amount;
	int win_rate_rank;
};

class UserControl :public Ref
{
public:
	static UserControl* GetInstance();
	bool InitUserData(std::string jsonData);		//从服务端login返回的json数据，生成userdata
	UserData *GetUserData();
	void requestGames();
	void requestGameIndex();

	bool isValidPhone();
	void setInfoView(Node* view);
	std::vector<GAMEINFO*> & getGameInfos();

private:

	Node* infoView;

	UserControl();
	~UserControl();
	UserData *_userData;
	std::vector<int> levelIndexs;

	std::vector<GAMEINFO*> gameInfos;

	GAMEINFO* getGameInfoByKey(string key);

	void onGetUserInfoCallBack(Ref *pSender);
	void onGetGameIndexCallBack(Ref *pSender);


	void onGetGameLevelConfigCallBack(Ref *pSender);
	bool InitLevelConfig(std::string jsonData);
	
	int getIndexsByLevel(int level);
	int getLevelByIndexs(int indexs);

	void onGetGameStatCallBack(Ref*pSender);
	bool InitGetGameStat(std::string jsonData, map<int, GameData*> &tempMap);

	
	void onGetAvaliableGameCallBack(Ref*pSender);
	bool InitGetAvaliableGame(std::string jsonData);

	void onRepeatLoginCallBack(Ref* pSender);
	bool InitRepeatLogin(std::string jsonData);

	void onTriggerLookFriend(Ref*pSender);

	void onBalanceUpdateCallBack(Ref* pSender);
	bool InitBalanceUpdate(std::string jsonData);


}; 


