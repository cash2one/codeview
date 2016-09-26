#pragma once
#pragma execution_character_set("utf-8")


#define min(a, b) (((a)<(b))?(a):(b))
#define BUFSIZE 1024
#include "cocos2d.h"
#include "ui/UIButton.h"
#include "LayerMessageTip.h"
#include "cJSON.h"

USING_NS_CC;
enum ACCOUNTTYPE{ NONE=-1,HOST, TEMP, THIRD };
//enum GAMETYPE{ BJL, DZPK, SGJ, CZD, MORE };
enum GAMETYPE {
	NO = 0,
	BJL = 16,
	DZPK = 64,
	DZPK1 = 65,
	SGJ,
	CZD,
	MORE,
};
typedef const std::function<void(EventCustom*)> ccFuncEventCustom;


typedef struct structGameInfo
{
	GAMETYPE type;
	char name[32];
	char bgImgUrl[32];
	structGameInfo(GAMETYPE _type,const char *_name,const char *_imgurl)
	{
		type=_type;
		strncpy(name, _name, 32);
		strncpy(bgImgUrl, _imgurl, 32);
	}
} GAMEINFO;
//Bjl room data struct
typedef struct structBjlRoomInfo
{
	int id;
	char name[20];
	float min;
	float max;
	int count;
	int countMax;
} BjlRoomInfo;
static const char* toString(int64_t d)
{
	static char str[32];
	memset(str, 0, 32);
	sprintf(str, "%ld", d);
	return str;
}
static const char* DoubleToString(double f)
{
	static char str[32];
	memset(str, 0, 32);
	sprintf(str, "%.2lf", f);
	return str;
}
static void EnableButton(ui::Button *bt,bool enable)
{
	if (bt)
	{
		bt->setEnabled(enable);
		bt->setBright(enable);
	}
}


typedef struct structSocketData
{
	unsigned short type;
	unsigned short typesub;
	unsigned short code;
	unsigned short len;
	std::string content;
}SOCKETDATA;
//根据消息类型和子类型，获取对应的16进制字符串，做为消息的名称
static const char* GetMsgTypeString(unsigned short type, unsigned short typesub)
{
	// 翻牌类消息，使用同一个消息名称
	if (type == 1000 && typesub >= 1017 && typesub <= 1023)
		return "BjlFpMsg";
	static char str[10];
	memset(str, 0, 10);
	sprintf(str, "%04x%04x", type, typesub);
	return str;
}
//==================== socket
class RECVMSG
{
public:
	unsigned short type;
	unsigned short typesub;
	unsigned short code;
	unsigned short len;
	std::string data;
	static RECVMSG* create(SOCKETDATA *socketData);			//解析数据包
    void retain() {
        assert(_count > 0);
        _count++;
    };
    void release() {
        assert(_count > 0);
        _count--;
        if (_count == 0) {
            delete this;
        }
    };
    int _count=1;
};
//字符串UUID转换为64位代码
unsigned long long Str2Uuid(const char* str);
unsigned char HexChar2Char(const char* str);
char* Char322Char64(unsigned char* char32);		//将char32字节数组转换为64个字符的字符串

int SHA256(unsigned char *d, unsigned char *s, unsigned long n);	// 生成密文

static std::string GetFileNameFromPath(std::string path)
{
	if (path.empty())
		return "";
	int pos = path.find_last_of('/');
	if (pos == path.npos)
		return "";
	return path.substr(pos + 1, path.size() - pos - 1);
}
//return type:const char*  
#define LocalizedCStringByKey(key) Language::getStringByKey(key)  

static CCDictionary *zhDic = nullptr;
static CCDictionary *enDic = nullptr;

static Array* filterWords = nullptr;

class Language{
private:

public:

	static const char* getStringByKey(const char * key)
	{
		//获取当前系统环境语言类型
		std::string langType = UserDefault::getInstance()->getStringForKey("LangType");
		//字典  

		std::string keyStr = key;

		if (langType == "zh")
		{
			//根据语言选择不同的属性表
			if (zhDic == nullptr){
				zhDic = CCDictionary::createWithContentsOfFile("lang/zh.plist");
				zhDic->retain();
			}
			return (zhDic->valueForKey(keyStr))->getCString();
		}
		else
		{
			if (enDic == nullptr){
				enDic = CCDictionary::createWithContentsOfFile("lang/en.plist");
				enDic->retain();
			}
			return (enDic->valueForKey(keyStr))->getCString();
		}

		//返回key对应的value  

	}

	static const bool isNeedFitler(string& key)
	{
		//if (UserDefault::getInstance()->getStringForKey("LangType") == "en")//英文不处理敏感词汇
		//{
		//	return false;
		//}

		if (key.empty()) return "";

		if (filterWords == nullptr){
			String* content = String::createWithContentsOfFile("static/filter.json");
			filterWords = content->componentsSeparatedByString("|");
			filterWords->retain();
		}
		Ref* obj = nullptr;
		String* word = nullptr;
		int len = 0;
		string t_word;
		string::size_type   pos(0);
		CCARRAY_FOREACH(filterWords, obj)
		{
			word = (String*)obj;
			len = word->length();
			string t_word = word->getCString();
			if (!t_word.empty())
			{
				if ((pos = key.find(t_word))!= string::npos)
				{
					return true;
				}
				
			}

		}
		return false;
	}



public:
    static const char* Lang()
    {
        return UserDefault::getInstance()->getStringForKey("LangType")=="zh"?"zh":"en";
    }
};

static string&   replaceAll(string&   str, const   string&   old_value, const   string&   new_value)
{
	while (true)   {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

class Comm
{
public:
	static const time_t TIMEZONEBIAS;
	static time_t InitTimeZoneBias();
	static std::string GetResDir();
	static std::string GetShortStringFromInt64(long long num);
	static std::string GetStringFromInt64(long long num);
	static std::string GetFloatShortStringFromInt64(long long num);
	static std::string GetGameTypeStringFromInt64(long long num);
	static std::string GetFormatThousandsSeparatorFromInt64(long long  num);
	static std::string GetLocalTimeStrngFromUTCString(std::string strUTC);
	static std::string GetUTCStringFromLocalTimeStrng(std::string strLocalTime);
	static std::string DeleteCharcter(string source, string del);
	static bool RegexIsMatch(const char* str, const char* reg);
};

/** @def CREATE_CFUNC(__TYPE__)
* Define a create function for a c object
*
* @param __TYPE__  class type to add create(), such as class.
*/
#define CREATE_C_FUNC(__TYPE__) \
static __TYPE__* create() \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->init()) \
	    { \
        return pRet; \
	    } \
	    else \
    { \
        delete pRet; \
        pRet = NULL; \
        return NULL; \
    } \
}


static void Tips(string msg)
{
	MessageTip* tip = MessageTip::create();

	tip->setTip(msg);

	auto scene = Director::getInstance()->getRunningScene();

	if (scene != nullptr){
		scene->addChild(tip, 1009);
	}
	tip->setAnchorPoint(ccp(0.5,0.5));
	Size visibleSize = Director::getInstance()->getVisibleSize();
	tip->setPosition(ccp(visibleSize.width / 2, 100));
	tip->setCascadeOpacityEnabled(true);
	tip->setOpacity(0);
	tip->runAction(Sequence::create(Spawn::create(EaseExponentialOut::create(MoveBy::create(0.8f, ccp(0, 500))), FadeIn::create(0.8f), nullptr),
		DelayTime::create(1.5f), 
		Spawn::create(EaseExponentialIn::create(MoveBy::create(1.0f, ccp(0, 400))), FadeOut::create(1.0f), nullptr),
		[=](){
		      tip->removeFromParent();
	    },
		nullptr));

}

static void ShowTip(std::string msg, std::function<void()> confirmCallback, std::function<void()> cancelCallback = nullptr,bool isOnlyOk = false)
{
	LayerMessageTip * tip = LayerMessageTip::create();

	tip->showTip(msg, confirmCallback, cancelCallback,isOnlyOk);

	auto scene = Director::getInstance()->getRunningScene();

	if (scene!=nullptr){
		scene->addChild(tip, 1008);
	}
}

static void OpenWithAlert(Node* node)
{
	if (node == nullptr) return;
	node->setScale(0.1f);
	node->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1)));
}

static void CloseWithAction(Node* node)
{
	if (node == nullptr) return;
	node->setCascadeOpacityEnabled(true);
	node->runAction(Sequence::create(
		Spawn::create(EaseBackIn::create(ScaleTo::create(0.2f, 0.2f)),
		FadeOut::create(0.2f), nullptr),
		CallFunc::create([=]{node->removeFromParent(); }), nullptr));
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS||CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ProxyInterface.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#else		 
// do nothings
#endif
static char* Game_Channel=nullptr;
static char* Game_Version=nullptr;


static void bridgeCall( char* json, char *returnStr = nullptr)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS||CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    const char* str = ProxyInterface::bridgeCall(json);
	if(returnStr!=nullptr){
		strcpy(returnStr, str);
	}
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo minfo;
	bool isHave = JniHelper::getStaticMethodInfo(minfo,/*JniMethodInfo的引用*/
		"org/cocos2dx/cpp/AppActivity",/*类的路径*/
		"bridgeCall",/*函数名*/
		"(Ljava/lang/String;)Ljava/lang/String;");/*函数类型简写*/

	if (isHave)
	{
		jstring jmsg = minfo.env->NewStringUTF(json);
		//CallStaticObjectMethod调用java函数，并把返回值赋值给activityObj
		jobject jobj = minfo.env->CallStaticObjectMethod(minfo.classID, minfo.methodID, jmsg);
		jstring jresult = (jstring)jobj;  
		const char* str = (minfo.env)->GetStringUTFChars(jresult, 0);
		if(returnStr!=nullptr){
			strcpy(returnStr, str);
		}
	
		(minfo.env)->ReleaseStringUTFChars(jresult, str);
		minfo.env->DeleteLocalRef(jmsg);
	}
	else
	{
		//(Ljava/lang/String;I)V
		CCLOG(" jni error");
	}

#else
	CCLOG("not support platform , will return nullptr");
	// do nothings
#endif

}

static char* GetAppVersion()
{
    if(Game_Version) return Game_Version;
	char* method = "{\"methodId\":21}";

	char result[BUFSIZ];
    
	bridgeCall(method, result);

	if (result == nullptr) return "";

	cJSON *item;
	if (!(item = cJSON_Parse(result))){
		return "";
	}
	cJSON *status,*version;
	if (!(status = cJSON_GetObjectItem(item, "status")) || status->type != cJSON_String
		|| !(version = cJSON_GetObjectItem(item, "version")) || version->type != cJSON_String){
		cJSON_Delete(item);
		return "";
	}
    if(!Game_Version){
        Game_Version= new char[20];
    }
    
    strcpy(Game_Version, version->valuestring);
    cJSON_Delete(item);
    return Game_Version;
}


static char* GetRegisterChannel()
{
    if(Game_Channel) return Game_Channel;
	char* method = "{\"methodId\":22}";

	char result[BUFSIZ];
	bridgeCall(method, result);

	if (result == nullptr) return "test";
	cJSON *item;
	if (!(item = cJSON_Parse(result))){
		return "test";
	}
	cJSON *status,  *channel;
	if (!(status = cJSON_GetObjectItem(item, "status")) || status->type != cJSON_String
		|| !(channel = cJSON_GetObjectItem(item, "channel")) || channel->type != cJSON_String){
		cJSON_Delete(item);
		return "test";
	}
	if (!Game_Channel){
		Game_Channel = new char[20];
	}
	strcpy(Game_Channel, channel->valuestring);
	cJSON_Delete(item);
	return Game_Channel;
}


static const char* GetChannelIap()
{
	string channel = GetRegisterChannel();
	string iap = replaceAll(channel, "iap_", "iap-");//后端需求哎
	return iap.c_str();
}

static void HideLogo()
{
	char* method = "{\"methodId\":23}";

	bridgeCall(method);
}