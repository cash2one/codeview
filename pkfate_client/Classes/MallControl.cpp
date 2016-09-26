#include "MallControl.h"
#include "cJSON.h"
#include "comm.h"
#include "cmd.h"
#include "UserControl.h"
#include "BillControl.h"
#include "LayerMall.h"
#include "LayerLoading.h"
#include "ApiBill.h"
#include "PKNotificationCenter.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "ProxyInterface.h"
#define CREATE_ORDER_URL	"https://pay.pkfate.com/create_order"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//#define CREATE_ORDER_URL	"https://pay.pkfate.com/create_order"
#define CREATE_ORDER_URL	"http://pay.pkfate.cn/create_order"

#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#else		 
// do nothings
#define CREATE_ORDER_URL	""
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)  
#include <regex>
#else
#include <regex.h>  
#endif


static MallControl *m_pInstance = nullptr;
MallControl::MallControl() :_mallView(nullptr)
{

}

bool initGetAllCoodsData(std::string strJsonProductArray, std::vector<SProductConversion> &_vec)
{
	cJSON *jsonProductArray, *jsonProduct, *jsonCostAmount,
		*jsonCostCurrencyType, *jsonGoodsAmount, *jsonGoodsCurrencyType, 
		*jsonID, *jsonLevelRequire, *jsonName;

	if (!(jsonProductArray = cJSON_Parse(strJsonProductArray.c_str())))
	if (jsonProductArray->type != cJSON_Array)
	{
		cJSON_Delete(jsonProductArray);
		return false;
	}

	_vec.clear();

	int sizeProductArray = cJSON_GetArraySize(jsonProductArray);
	for (int i = 0; i < sizeProductArray; i++)
	{
		cJSON *jsonProduct = cJSON_GetArrayItem(jsonProductArray, i);
		if (!jsonProduct || jsonProduct->type != cJSON_Object)
			return false;

		jsonCostAmount = cJSON_GetObjectItem(jsonProduct, "cost_amount");
		jsonCostCurrencyType = cJSON_GetObjectItem(jsonProduct, "cost_currency_type");
		jsonGoodsAmount = cJSON_GetObjectItem(jsonProduct, "goods_amount");
		jsonGoodsCurrencyType = cJSON_GetObjectItem(jsonProduct, "goods_currency_type");
		jsonID = cJSON_GetObjectItem(jsonProduct, "id");
		jsonLevelRequire = cJSON_GetObjectItem(jsonProduct, "level_require");
		jsonName = cJSON_GetObjectItem(jsonProduct, "name");
		if (!jsonCostAmount || !jsonCostCurrencyType || !jsonGoodsAmount ||
			!jsonGoodsCurrencyType || !jsonID||!jsonLevelRequire || !jsonName ||
			jsonCostAmount->type != cJSON_Number || jsonCostCurrencyType->type != cJSON_Number ||
			jsonGoodsAmount->type != cJSON_Number || jsonGoodsCurrencyType->type != cJSON_Number ||
			jsonID->type != cJSON_Number || jsonLevelRequire->type != cJSON_Number || jsonName->type != cJSON_String)
			return false;

		SProductConversion productConversion;
		productConversion.dCostAmount = jsonCostAmount->valuedouble;
		productConversion.byCostCurrencyType = jsonCostCurrencyType->valueint;
		productConversion.nGoodsAmount = jsonGoodsAmount->valueint;
		productConversion.byGoodsCurrencyType = jsonGoodsCurrencyType->valueint;
		productConversion.nId = jsonGoodsCurrencyType->valueint;
		productConversion.nLevelRequire = jsonLevelRequire->valueint;
		productConversion.strName = jsonName->valuestring;
		_vec.push_back(productConversion);
	}
	return true;
}

void MallControl::createOrder(const char* payData)
{


	// request url
	// 组合参数 用户参数 和 productId 拼接url

	//string url = StringUtils::format()
	/*	curl->onDone = CC_CALLBACK_1(LayerMall::onCreateOrderCallBack, this);
	curl->GetUrlAsync(CREATE_ORDER_URL);
	CurlControl *curl = new CurlControl();*/

	HttpRequest* request = new (std::nothrow) HttpRequest();
	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/json; charset=utf-8");
	request->setHeaders(headers);
	request->setUrl(CREATE_ORDER_URL);
	
	request->setRequestType(HttpRequest::Type::POST);
	request->setResponseCallback(CC_CALLBACK_2(MallControl::onCreateOrderCallBack, this));
	
	log("postData:%s", payData);
	request->setRequestData(payData, strlen(payData));
	request->setTag("POST immediate createOrder");
	HttpClient::getInstance()->sendImmediate(request);
	HttpClient::getInstance()->setTimeoutForConnect(10);
	HttpClient::getInstance()->setTimeoutForRead(8);
	//HttpClient::getInstance()->setSSLVerification("/usr/local/share/curl/curl-ca-bundle.crt");
	request->release();
}



void MallControl::onCreateOrderCallBack(HttpClient *sender, HttpResponse *response)
{
	if (!response){
		return; // donothings
	}

	response->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (!response)
		{
			messageTip("createOrder fail", true);
			return;
		}
		long statusCode = response->getResponseCode();
		log("response code: %ld", statusCode);
		if (!response->isSucceed())
		{
			messageTip("createOrder fail", true);
			log("response failed");
			return;
		}
		std::vector<char>* buffer = response->getResponseData();
		std::string temp(buffer->begin(), buffer->end());
		CCString* responseString = CCString::create(temp);
		log("response : %s", responseString->getCString());
		response->release();
		initOrderData(responseString->getCString());
	});

}


void MallControl::initOrderData(string responseString)
{
	cJSON *item;
	if (!(item = cJSON_Parse(responseString.c_str()))){
		messageTip("createOrder fail", true);
		return;
	}
	cJSON *order_id, *callback, *amount, *code, *ext, *waresId, *quantity;
	if (!(order_id = cJSON_GetObjectItem(item, "order_id")) || order_id->type != cJSON_Number
		|| !(callback = cJSON_GetObjectItem(item, "callback")) || callback->type != cJSON_String
		|| !(code = cJSON_GetObjectItem(item, "code")) || code->type != cJSON_Number

		){

		cJSON_Delete(item);
		messageTip("order data from server is error", true);
		return;
	}



	int orderCode = code->valueint;// do what？
	if (orderCode != 0){
		messageTip("order data from server is error", true);
		return;
	}

	PayData* payData = new PayData();
	payData->gameOrderId = toString(order_id->valueint);//订单ID 竟然是数字 e..
	payData->notifyUrl = callback->valuestring;
	
	

	if ((ext = cJSON_GetObjectItem(item, "private_info")) && ext->type == cJSON_String){
		payData->ext = ext->valuestring;
	}
	else{
		payData->ext = payData->gameOrderId;
	}

	if ((waresId = cJSON_GetObjectItem(item, "waresId")) && waresId->type == cJSON_String){
		payData->waresId = waresId->valuestring;
	}
	else{
		payData->waresId = string(_productId);// 事实上要由服务端传
	}

	if ((quantity = cJSON_GetObjectItem(item, "quantity")) && quantity->type == cJSON_Number){
		payData->quantity = quantity->valueint;
	}
	else{
		payData->quantity = 1;// 事实上要由服务端传
	}

	/*if ((amount = cJSON_GetObjectItem(item, "amount")) && amount->type == cJSON_Number){
		payData->price = amount->valuedouble;
	}*/


	ProductData* product = GetProduct(payData->waresId);
	payData->productName = StringUtils::format("%d%s",product->coin,Language::getStringByKey("Coin"));
	payData->price = product->price;

	cJSON_Delete(item);
	char payJsonStr[BUFSIZE * 2];
	char* payJsonFormat = "{\"methodId\":7,\"gameOrderId\":\"%s\",\"waresId\":\"%s\",\"quantity\":%d,\"notifyUrl\":\"%s\",\"privateInfo\":\"%s\",\"price\":%f,\"productName\":\"%s\"}";
	sprintf(payJsonStr, payJsonFormat, (payData->gameOrderId).c_str(), (payData->waresId).c_str(), payData->quantity, (payData->notifyUrl).c_str(), (payData->ext).c_str(), payData->price, (payData->productName).c_str());

	log("pay:%s", payJsonStr);
	startPay(payJsonStr);
	delete payData;
}





void MallControl::startPay(char* payJsonInfo)
{


#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	ProxyInterface::call(payJsonInfo, 1);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	clearTip();//
	JniMethodInfo minfo;
	bool isHave = JniHelper::getStaticMethodInfo(minfo,/*JniMethodInfo的引用*/
		"org/cocos2dx/cpp/AppActivity",/*类的路径*/
		"jniCall",/*函数名*/
		"(Ljava/lang/String;I)V");/*函数类型简写*/
	
	if (isHave)
	{
		jstring jmsg = minfo.env->NewStringUTF(payJsonInfo);
		//CallStaticObjectMethod调用java函数，并把返回值赋值给activityObj
		minfo.env->CallStaticVoidMethod(minfo.classID, minfo.methodID,jmsg,1);
	}
	else
	{
		//(Ljava/lang/String;I)V
	}
	
	

#else		 
	// do nothings
#endif

}

bool MallControl::Init()
{


	float priceList[6];//金额
	int coinList[6] = { 5000, 25500, 52000, 106000, 270000, 550000 }; //coin数

	//To obtain language types
	std::string langType = UserDefault::getInstance()->getStringForKey("LangType");
	if (langType == "zh")
	{
		priceList[0] = 6;
		priceList[1] = 30;
		//priceList[0] = 0.1;
		//priceList[1] = 0.5;
		priceList[2] = 68;
		priceList[3] = 128;
		priceList[4] = 328;
		priceList[5] = 648;
	}
	else if (langType == "en")
	{
		priceList[0] = 0.99f;
		priceList[1] = 4.99f; 
		priceList[2] = 9.99f;
		priceList[3] = 19.99f;
		priceList[4] = 49.99f;
		priceList[5] = 99.99f;
	}

	//for fixed data
#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

	

	string productIds[6] = { "t_1", "t_5", "t_10", "t_20", "t_50", "t_100" };//产品ID ，苹果商城设置，根据需要最好是字符串，兼容各类平台
	
#else
	string productIds[6] = { "T1", "T5", "T10", "T20", "T50", "T100" };//产品ID ，苹果商城设置，根据需要最好是字符串，兼容各类平台
#endif
	

	for (int i = 0, length = sizeof(coinList) / sizeof(coinList[0]); i < length; i++)
	{
		ProductData *data = new ProductData();
		data->coin = coinList[i];
		data->price = priceList[i];
		data->productId = productIds[i];
		_productData[i + 1] = data;
	}
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MallControl::UpdateBalance), "UpdateBalanceMall", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MallControl::onPayResult), "PayResult", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MallControl::onSendPay), "SendPay", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MallControl::onGetAllCoodsCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_ALL_GOODS), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MallControl::onGetAllCoodsNotification), "GetAllCoodsNotification", NULL);

	return true;
}

void MallControl::onPayResult(Ref *ref)
{
	String *resultStr = (String*)ref;

	cJSON *item;
	if (!(item = cJSON_Parse(resultStr->getCString()))){
		messageTip("onPayResult fail", true);
		return;
	}
	cJSON *status, *error;
	if (!(status = cJSON_GetObjectItem(item, "status")) || status->type != cJSON_String){
		cJSON_Delete(item);
		return;
	}

	int payStatus = atoi(status->valuestring);

	if (payStatus == 0){
		//messageTip("pay success!", true);
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(2)));//验证完成任务  第一次充值筹码
		auto product = GetProduct(_productId);
		if (product){
			Tips(StringUtils::format(Language::getStringByKey("AddChipsTip"),product->coin));
		}
		Api::Bill::GET_BALANCE();		//每次进入时，刷新用户余额
		clearTip();
		return;
	}
	else if (payStatus == 2000){
		clearTip();
		return;
	}
	string payError = "pay failed";
	if ((error = cJSON_GetObjectItem(item, "error")) && error->type == cJSON_String){
		payError = error->valuestring;
	}
	log("payResult  code:%d,msg:%s", payStatus, payError.c_str());
	messageTip(payError, true);
	cJSON_Delete(item);


}

void MallControl::onSendPay(Ref *sender)
{
	messageTip("", false);

	String *data = (String*)sender;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

	UserData* userData = UserControl::GetInstance()->GetUserData();
	const string productId = data->getCString();
	_productId = string(productId);
	int userId = userData->user_id;
	const string token = userData->token;
	const char* iap = GetRegisterChannel();
	char postData[BUFSIZE];


	sprintf(postData, "{\"user_id\":%d,\"token\":\"%s\",\"waresid\":\"%s\",\"quantity\":%d,\"iap\":\"%s\"}", userId, token.c_str(), productId.c_str(), 1, iap);
	createOrder(postData);

    
	

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)


	UserData* userData = UserControl::GetInstance()->GetUserData();
	const string productId = data->getCString();
	_productId = string(productId);
	int userId = userData->user_id;
	const string token = userData->token;
	const char* iap = GetChannelIap();
	

	char postData[BUFSIZE];
	sprintf(postData, "{\"user_id\":%d,\"token\":\"%s\",\"waresid\":\"%s\",\"quantity\":%d,\"iap\":\"%s\"}", userId, token.c_str(), productId.c_str(), 1, iap);
	createOrder(postData);


#else	


	messageTip("platform is not support!", true);
	// do nothings
#endif
}



void MallControl::UpdateBalance(Ref *ref)
{
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (_mallView){
			//更新成功，通知其他进程
			
			((LayerMall*)_mallView)->UpdateBalance(ref);
		}
	});
}

void MallControl::setMallView(Node* view)
{
	_mallView = view;
}

MallControl::~MallControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_productData.size()>0)
	{
		for (auto data : _productData)
		{
			delete data.second;
		}
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
MallControl* MallControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new MallControl();
		m_pInstance->Init();
	}
	return m_pInstance;
}
ProductData* MallControl::GetProduct(string productId)
{
	

	for (auto item : _productData){
		if (item.second->productId == productId){
			return item.second;
		}
	}

	return nullptr;
}

map<int, ProductData*> MallControl::GetProducts()
{
	return _productData;
}

std::vector<SProductConversion>& MallControl::getProductConversion()
{
	return _vecProductConversion;
}

void MallControl::messageTip(string msg, bool cancelVisible)
{
	if (!_mallView){
		return;
	}
	LayerLoading *layerLoading = (LayerLoading *)_mallView->getChildByName("LayerLoading");
	if (!layerLoading){
		layerLoading = LayerLoading::create();
		layerLoading->SetTimeout(35);
		layerLoading->setName("LayerLoading");
		_mallView->addChild(layerLoading);
	}

	if (!msg.empty() && msg.size()>0){
		layerLoading->SetString(msg);
	}
	layerLoading->btCancel->setVisible(cancelVisible);
}

void MallControl::clearTip()
{
	if (!_mallView){
		return;
	}
	auto layerLoading = _mallView->getChildByName("LayerLoading");
	if (layerLoading){
		layerLoading->removeFromParent();
	}
}

void MallControl::onGetAllCoodsCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;

	if (msg->code != 0 || !initGetAllCoodsData(msg->data, _vecProductConversion))
	{
		CCLOG("[%s]:%s\t%s", "MallControl::onGetAllCoodsCallBack", "onGetAllCoodsCallBack error.", msg->data.c_str());
		return;
	}

	LayerMall *pLayout = dynamic_cast<LayerMall*>(_mallView);
	if (pLayout)
	{
		pLayout->refreshProductConversion();
	}
}

void MallControl::onGetAllCoodsNotification(Ref *pSender)
{
	if (!Api::Bill::GetAllGoods())
	{
		//todo
	}
}