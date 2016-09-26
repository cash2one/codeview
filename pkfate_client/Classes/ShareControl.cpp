#include "ShareControl.h"
#include "comm.h"

#include "PKNotificationCenter.h"

#include "LayerLoading.h"
#include <sstream>
#include "C2DXShareSDK.h"

USING_NS_CC;

using namespace std;
using namespace cn::sharesdk;

static ShareControl *m_pInstance = nullptr;

ShareControl::ShareControl()
{
	// register callback func
}


ShareControl::~ShareControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);

	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
ShareControl* ShareControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new ShareControl();
    }
		
	return m_pInstance;
}

void ShareControl::initSDK()
{
	   //设置平台配置
    //Platforms
     __Dictionary *totalDict = __Dictionary::create();
    
    // //新浪微博
    __Dictionary *sinaWeiboConf= __Dictionary::create();
    sinaWeiboConf->setObject(__String::create("568898243"), "app_key");
    sinaWeiboConf->setObject(__String::create("38a4f8204cc784f81f9f0daaf31e02e3"), "app_secret");
    sinaWeiboConf->setObject(__String::create("http://www.sharesdk.cn"), "redirect_uri");
    stringstream sina;
    sina << cn::sharesdk::C2DXPlatTypeSinaWeibo;
    totalDict->setObject(sinaWeiboConf, sina.str());
    
    //微信
    __Dictionary *wechatConf = __Dictionary::create();
    wechatConf->setObject(__String::create("wx4868b35061f87885"), "app_id");
    wechatConf->setObject(__String::create("64020361b8ec4c99936c0e3999a9f249"), "app_secret");
    stringstream wechat;
    wechat << cn::sharesdk::C2DXPlatTypeWechatPlatform;
    totalDict->setObject(wechatConf, wechat.str());
    
    //QQ
    __Dictionary *qqConf = __Dictionary::create();
    qqConf->setObject(__String::create("100371282"), "app_id");
    qqConf->setObject(__String::create("aed9b0303e3ed1e27bae87c33761161d"), "app_key");
    stringstream qq;
    qq << cn::sharesdk::C2DXPlatTypeQQPlatform;
    totalDict->setObject(qqConf, qq.str());
    
    //腾讯微博
    __Dictionary *tencentConf = __Dictionary::create();
    tencentConf->setObject(__String::create("801307650"), "app_key");
    tencentConf->setObject(__String::create("ae36f4ee3946e1cbb98d6965b0b2ff5c"), "app_secret");
    tencentConf->setObject(__String::create("http://www.sharesdk.cn"), "redirect_uri");
    stringstream tencent;
    tencent << cn::sharesdk::C2DXPlatTypeTencentWeibo;
    totalDict->setObject(tencentConf, tencent.str());
    
    //Facebook
    __Dictionary *fbConf = __Dictionary::create();
    fbConf->setObject(__String::create("107704292745179"), "api_key");
    fbConf->setObject(__String::create("38053202e1a5fe26c80c753071f0b573"), "app_secret");
    stringstream facebook;
    facebook << cn::sharesdk::C2DXPlatTypeFacebook;
    totalDict->setObject(fbConf, facebook.str());
    
    //Twitter
    __Dictionary *twConf = __Dictionary::create();
    twConf->setObject(__String::create("LRBM0H75rWrU9gNHvlEAA2aOy"), "consumer_key");
    twConf->setObject(__String::create("gbeWsZvA9ELJSdoBzJ5oLKX0TU09UOwrzdGfo9Tg7DjyGuMe8G"), "consumer_secret");
    twConf->setObject(__String::create("http://www.mob.com"), "redirect_uri");
    stringstream twitter;
    twitter << cn::sharesdk::C2DXPlatTypeTwitter;
    totalDict->setObject(twConf, twitter.str());
    
    //Mail
    __Dictionary *mailConf = __Dictionary::create();
    stringstream mail;
    mail << cn::sharesdk::C2DXPlatTypeMail;
    totalDict->setObject(mailConf, mail.str());
    
    cn::sharesdk::C2DXShareSDK::registerAppAndSetPlatformConfig("8e3320a36606", totalDict);
}



//分享回调
void shareContentResultHandler(int seqId, cn::sharesdk::C2DXResponseState state, cn::sharesdk::C2DXPlatType platType, __Dictionary *result)
{
    switch (state)
    {
        case cn::sharesdk::C2DXResponseStateSuccess:
        {
            log("Success");
        }
            break;
        case cn::sharesdk::C2DXResponseStateFail:
        {
            log("Fail");
            //回调错误信息
			if (result == NULL) {
				break;
			}
            __Array *allKeys = result->allKeys();
            allKeys->retain();
            for (int i = 0; i < allKeys-> count(); i++)
            {
                __String *key = (__String*)allKeys->getObjectAtIndex(i);
                Ref *obj = result->objectForKey(key->getCString());
                
                log("key = %s", key -> getCString());
                if (dynamic_cast<__String *>(obj))
                {
                    log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                }
                else if (dynamic_cast<__Integer *>(obj))
                {
                    log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                }
                else if (dynamic_cast<__Double *>(obj))
                {
                    log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                }
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateCancel:
        {
            log("Cancel");
        }
            break;
        default:
            break;
    }
}

//授权回调
void authResultHandler(int seqId, cn::sharesdk::C2DXResponseState state, cn::sharesdk::C2DXPlatType platType, __Dictionary *result)
{
    switch (state)
    {
        case cn::sharesdk::C2DXResponseStateSuccess:
        {
            log("Success");
            
            //输出信息
            try
            {
				if (result == NULL) {
					break;
				}
                __Array *allKeys = result -> allKeys();
                allKeys->retain();
                for (int i = 0; i < allKeys -> count(); i++)
                {
                    __String *key = (__String *)allKeys -> getObjectAtIndex(i);
                    Ref *obj = result -> objectForKey(key -> getCString());
                    
                    log("key = %s", key -> getCString());
                    if (dynamic_cast<__String *>(obj))
                    {
                        log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                    }
                    else if (dynamic_cast<__Integer *>(obj))
                    {
                        log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                    }
                    else if (dynamic_cast<__Double *>(obj))
                    {
                        log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                    }
                }
                allKeys->release();
            }
            catch(...)
            {
                log("==============error");
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateFail:
        {
            log("Fail");
			if (result == NULL) {
				break;
			}
            //回调错误信息
            __Array *allKeys = result->allKeys();
            allKeys->retain();
            for (int i = 0; i < allKeys-> count(); i++)
            {
                __String *key = (__String*)allKeys->getObjectAtIndex(i);
                Ref *obj = result->objectForKey(key->getCString());
                
                log("key = %s", key -> getCString());
                if (dynamic_cast<__String *>(obj))
                {
                    log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                }
                else if (dynamic_cast<__Integer *>(obj))
                {
                    log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                }
                else if (dynamic_cast<__Double *>(obj))
                {
                    log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                }
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateCancel:
        {
            log("Cancel");
        }
            break;
        default:
            break;
    }
}

//获取用户信息结果回调
void getUserResultHandler(int reqID, C2DXResponseState state, C2DXPlatType platType, __Dictionary *result)
{
    switch (state)
    {
        case cn::sharesdk::C2DXResponseStateSuccess:
        {
            log("Success");
            
            //输出信息
            try
            {
				if (result == NULL) {
					break;
				}
                __Array *allKeys = result -> allKeys();
                allKeys->retain();
                for (int i = 0; i < allKeys -> count(); i++)
                {
                    __String *key = (__String *)allKeys -> getObjectAtIndex(i);
                    Ref *obj = result -> objectForKey(key -> getCString());
                    
                    log("key = %s", key -> getCString());
                    if (dynamic_cast<__String *>(obj))
                    {
                        log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                    }
                    else if (dynamic_cast<__Integer *>(obj))
                    {
                        log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                    }
                    else if (dynamic_cast<__Double *>(obj))
                    {
                        log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                    }
                }
                allKeys->release();
            }
            catch(...)
            {
                log("==============error");
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateFail:
        {
            log("Fail");
            //回调错误信息
			if (result == NULL) {
				break;
			}
            __Array *allKeys = result->allKeys();
            allKeys->retain();
            for (int i = 0; i < allKeys-> count(); i++)
            {
                __String *key = (__String*)allKeys->getObjectAtIndex(i);
                Ref *obj = result->objectForKey(key->getCString());
                
                log("key = %s", key -> getCString());
                if (dynamic_cast<__String *>(obj))
                {
                    log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                }
                else if (dynamic_cast<__Integer *>(obj))
                {
                    log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                }
                else if (dynamic_cast<__Double *>(obj))
                {
                    log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                }
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateCancel:
        {
            log("Cancel");
        }
            break;
        default:
            break;
    }
}

void getFriendListResultHandler(int reqID, C2DXResponseState state, C2DXPlatType platType, __Dictionary *result)
{
    switch (state)
    {
        case cn::sharesdk::C2DXResponseStateSuccess:
        {
            log("Success");
            
            //输出信息
            try
            {
				if (result == NULL) {
					break;
				}
				
                __Array *allKeys = result -> allKeys();
                allKeys->retain();
                for (int i = 0; i < allKeys -> count(); i++)
                {
                    __String *key = (__String *)allKeys -> getObjectAtIndex(i);
                    Ref *obj = result -> objectForKey(key -> getCString());
                    
                    log("key = %s", key -> getCString());
                    if (dynamic_cast<__String *>(obj))
                    {
                        log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                    }
                    else if (dynamic_cast<__Integer *>(obj))
                    {
                        log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                    }
                    else if (dynamic_cast<__Double *>(obj))
                    {
                        log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                    }
                }
                allKeys->release();
            }
            catch(...)
            {
                log("==============error");
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateFail:
        {
            log("Fail");
            //回调错误信息
			if (result == NULL) {
				break;
			}
            __Array *allKeys = result->allKeys();
            allKeys->retain();
            for (int i = 0; i < allKeys-> count(); i++)
            {
                __String *key = (__String*)allKeys->getObjectAtIndex(i);
                Ref *obj = result->objectForKey(key->getCString());
                
                log("key = %s", key -> getCString());
                if (dynamic_cast<__String *>(obj))
                {
                    log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                }
                else if (dynamic_cast<__Integer *>(obj))
                {
                    log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                }
                else if (dynamic_cast<__Double *>(obj))
                {
                    log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                }
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateCancel:
        {
            log("Cancel");
        }
            break;
        default:
            break;
    }
}

void addFriendResultHandler(int reqID, C2DXResponseState state, C2DXPlatType platType, __Dictionary *result)
{
    switch (state)
    {
        case cn::sharesdk::C2DXResponseStateSuccess:
        {
            log("Success");
        }
            break;
        case cn::sharesdk::C2DXResponseStateFail:
        {
            log("Fail");
            //回调错误信息
			if (result == NULL) {
				break;
			}
            __Array *allKeys = result->allKeys();
            allKeys->retain();
            for (int i = 0; i < allKeys-> count(); i++)
            {
                __String *key = (__String*)allKeys->getObjectAtIndex(i);
                Ref *obj = result->objectForKey(key->getCString());
                
                log("key = %s", key -> getCString());
                if (dynamic_cast<__String *>(obj))
                {
                    log("value = %s", dynamic_cast<__String *>(obj) -> getCString());
                }
                else if (dynamic_cast<__Integer *>(obj))
                {
                    log("value = %d", dynamic_cast<__Integer *>(obj) -> getValue());
                }
                else if (dynamic_cast<__Double *>(obj))
                {
                    log("value = %f", dynamic_cast<__Double *>(obj) -> getValue());
                }
            }
        }
            break;
        case cn::sharesdk::C2DXResponseStateCancel:
        {
            log("Cancel");
        }
            break;
        default:
            break;
    }
}

void ShareControl::showShareMenu(cocos2d::Ref* pSender)
{    
    __Dictionary *content = __Dictionary::create();
    content -> setObject(__String::create("先进的加密算法保证你的公平，杜绝猫腻冤家牌，一起来PKFATE！"), "text");
    content -> setObject(__String::create("ShareControl.png"), "image");
    content -> setObject(__String::create("pkfate加密游戏"), "title");
    content -> setObject(__String::create("http://www.pkfate.com"), "url");
    content -> setObject(__String::createWithFormat("%d", cn::sharesdk::C2DXContentTypeAuto), "type");
    
//可以自定义分享平台，如果平台传入NULL，此时显示所有初始化的平台
    C2DXArray *platforms = C2DXArray::create();
    __Integer *sina = new __Integer(cn::sharesdk::C2DXPlatTypeSinaWeibo);
    __Integer *tencent = new __Integer(cn::sharesdk::C2DXPlatTypeTencentWeibo);
    __Integer *wechat = new __Integer(cn::sharesdk::C2DXPlatTypeWechatPlatform);
    __Integer *qq = new __Integer(cn::sharesdk::C2DXPlatTypeQQPlatform);
    __Integer *fb = new __Integer(cn::sharesdk::C2DXPlatTypeFacebook);
    __Integer *tw = new __Integer(cn::sharesdk::C2DXPlatTypeTwitter);
    __Integer *mail = new __Integer(cn::sharesdk::C2DXPlatTypeMail);
    __Integer *sms = new __Integer(cn::sharesdk::C2DXPlatTypeSMS);

    platforms->addObject(sina);
    platforms->addObject(tencent);
    platforms->addObject(wechat);
    platforms->addObject(qq);
    platforms->addObject(fb);
    platforms->addObject(tw);
    platforms->addObject(mail);
    platforms->addObject(sms);
    C2DXShareSDK::showShareMenu(platforms,content,100,100,shareContentResultHandler);
}

void ShareControl::auth(cocos2d::Ref* pSender)
{
    C2DXShareSDK::authorize(cn::sharesdk::C2DXPlatTypeSinaWeibo, authResultHandler);
}

bool ShareControl::isAuthValid(cocos2d::Ref *pSender)
{
    bool isAuthValid = C2DXShareSDK::isAuthorizedValid(cn::sharesdk::C2DXPlatTypeSinaWeibo);
    log("isAuthValid: %i",isAuthValid);
    return isAuthValid;
}

void ShareControl::cancelAuth(cocos2d::Ref *pSender)
{
    C2DXShareSDK::cancelAuthorize(cn::sharesdk::C2DXPlatTypeSinaWeibo);
}

bool ShareControl::isClientValid(cocos2d::Ref *pSender)
{
    bool isClientValid = C2DXShareSDK::isClientValid(cn::sharesdk::C2DXPlatTypeSinaWeibo);
    log("isAuthValid: %i",isClientValid);
    return isClientValid;
}

void ShareControl::getUserInfo(cocos2d::Ref *pSender)
{
    C2DXShareSDK::getUserInfo(cn::sharesdk::C2DXPlatTypeSinaWeibo, getUserResultHandler);
}

void ShareControl::getAuthInfo(cocos2d::Ref *pSender)
{
    log("Use getUserInfo method instead.");
}

void ShareControl::shareContent(cocos2d::Ref *pSender)
{    
    //分享内容
    __Dictionary *content = __Dictionary::create();
    content -> setObject(__String::create("分享文本"), "text");
    content -> setObject(__String::create("ShareControl.png"), "image");
    content -> setObject(__String::create("测试标题"), "title");
    content -> setObject(__String::create("http://www.mob.com"), "url");
    content -> setObject(__String::createWithFormat("%d", cn::sharesdk::C2DXContentTypeImage), "type");
    
    C2DXShareSDK::shareContent(cn::sharesdk::C2DXPlatTypeSinaWeibo, content, shareContentResultHandler);
}

void ShareControl::oneKeyShareContent(cocos2d::Ref *pSender)
{    
    //分享内容
    __Dictionary *content = __Dictionary::create();
    content -> setObject(__String::create("分享文本"), "text");
    content -> setObject(__String::create("ShareControl.png"), "image");
    content -> setObject(__String::create("测试标题"), "title");
    content -> setObject(__String::create("http://www.mob.com"), "url");
    content -> setObject(__String::createWithFormat("%d", cn::sharesdk::C2DXContentTypeImage), "type");
    
    //分享平台
    C2DXArray *platforms = C2DXArray::create();
    __Integer *sina = new __Integer(cn::sharesdk::C2DXPlatTypeSinaWeibo);
    platforms->addObject(sina);
    __Integer *tencent = new __Integer(cn::sharesdk::C2DXPlatTypeTencentWeibo);
    platforms->addObject(tencent);
    
    //一键分享
    C2DXShareSDK::oneKeyShareContent(platforms, content, shareContentResultHandler);
}

void ShareControl::showShareView(cocos2d::Ref *pSender)
{    
    //分享内容
    __Dictionary *content = __Dictionary::create();
    content -> setObject(__String::create("分享文本"), "text");
    content -> setObject(__String::create("ShareControl.png"), "image");
    content -> setObject(__String::create("测试标题"), "title");
    content -> setObject(__String::create("http://www.mob.com"), "url");
    content -> setObject(__String::createWithFormat("%d", cn::sharesdk::C2DXContentTypeImage), "type");
    
    C2DXShareSDK::showShareView(cn::sharesdk::C2DXPlatTypeSinaWeibo, content, shareContentResultHandler);
}

void ShareControl::getFriendList(cocos2d::Ref *pSender)
{    
    C2DXShareSDK::getFriendList(cn::sharesdk::C2DXPlatTypeSinaWeibo, 5, 1, getFriendListResultHandler);
}

void ShareControl::addFriend(cocos2d::Ref *pSender)
{    
    C2DXShareSDK::addFriend(cn::sharesdk::C2DXPlatTypeTencentWeibo, "ShareSDK", addFriendResultHandler);
}




