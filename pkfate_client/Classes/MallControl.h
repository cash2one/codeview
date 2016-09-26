#pragma once
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpResponse.h"
USING_NS_CC;
using namespace network;
using namespace std;

struct ProductData{
	int coin;
	float price;
	string productId;
};

struct PayData{
	string gameOrderId;
	string notifyUrl;
	string waresId;
	string ext;
	string productName;
	int quantity = 1;
	float price = 0;
};

struct SProductConversion
{
	double dCostAmount;
	unsigned char byCostCurrencyType;
	int nGoodsAmount;
	unsigned char byGoodsCurrencyType;
	int nId;
	int nLevelRequire;
	std::string strName;
};

enum ConversionGoodsType
{
	SILVER_CARD,
	GOLD_CARD,
};



class MallControl :public Ref
{
public:
	static MallControl* GetInstance();
	bool Init();		//
	ProductData* GetProduct(string productId);
	map<int, ProductData*> GetProducts();
	std::vector<SProductConversion>& getProductConversion();
	void createOrder(const char* payData);
	void startPay(char* payJsonInfo);
	void onSendPay(Ref *sender);
	void setMallView(Node* view);
	void UpdateBalance(Ref *ref);//更新balance
	void onPayResult(Ref *ref);//支付结果

	void messageTip(string msg = "", bool cancelVisible = false);// loading message
	void clearTip();//clear loading message

	void onGetAllCoodsCallBack(Ref *pSender);

	void onGetAllCoodsNotification(Ref *pSender);
private:

	Node* _mallView;
	string _productId;
	MallControl();
	~MallControl();
	map<int, ProductData*> _productData;
	std::vector<SProductConversion> _vecProductConversion;
	void onCreateOrderCallBack(HttpClient *sender, HttpResponse *respons);
	void initOrderData(string responseString);
};