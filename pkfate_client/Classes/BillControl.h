#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;

enum CurrencyType{
	GOLD = 1,
	SILVER = 2,
	DIAMOND = 3,
	RMB = 4,
	CHIP = 0,
};

struct BalanceData{
	
	string update_time;
	double balance;
	int user_id;
	int currency_type;
	string create_time;
	//double bonus;
	//double tax;
	//double inpour;
	//double freeze;
	//double commission;
	//double outpour;
	//
	//double deposit;
	//double withdraw;
	//double rebate;
	//double bet;
};

class BillControl :public Ref
{
public:
	static BillControl* GetInstance();
	BalanceData *GetBalanceData(int currency_type);
	int GetBalanceStateAmount(string key);
	bool InitBalanceData(std::string jsonData);		//从服务端返回的json数据，生成balance data
	unsigned int getBetLevel(double bet);
	unsigned int getNextBetLevelPercent(double bet);
	void requestCardBalance();
	vector<int64_t> _levelConfigs;
	void requestBalance();
private:
	BillControl();
	~BillControl();
	map<int,BalanceData*> _balanceData;
	map<string, int> _mapBalanceState;
	void onGetBalanceCallBack(Ref *pSender);
	void onGetLevelConfigCallBack(Ref *pSender);
	bool InitLevelConfigData(std::string jsonData);
	void onListBalanceCallBack(Ref *pSender);
	void onBuyGoodsCallBack(Ref *pSender);
	void onGetBalanceStateCallBack(Ref *pSender);

	void onListBalanceNotification(Ref *pSender);
	void onGetBalanceNotification(Ref *pSender);
	void onGetBalanceGoldCardNotification(Ref *pSender);
	void onGetBalanceSilverCardNotification(Ref *pSender);
	void onGetBalanceDiamondCardNotification(Ref *pSender);
	void onGetBalanceRMBNotification(Ref *pSender);
	void onGetBalanceStateNotification(Ref *pSender);
};