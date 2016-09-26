#pragma once
#include "cocos2d.h"
USING_NS_CC;


class ShareControl :public Ref
{
public:
	static ShareControl* GetInstance();

    void initSDK();
	  //授权
    void auth(cocos2d::Ref* pSender);
    
    //是否授权
    bool isAuthValid(cocos2d::Ref* pSender);
    
    //取消授权
    void cancelAuth(cocos2d::Ref* pSender);
    
    //客户端是否安装
    bool isClientValid(cocos2d::Ref* pSender);
    
    //获取用户信息
    void getUserInfo(cocos2d::Ref* pSender);
    
    //获取授权信息
    void getAuthInfo(cocos2d::Ref* pSender);
    
    //直接分享
    void shareContent(cocos2d::Ref* pSender);
    
    //一键分享
    void oneKeyShareContent(cocos2d::Ref* pSender);
    
    //分享菜单
    void showShareMenu(cocos2d::Ref* pSender);
    
    //显示分享编辑页面
    void showShareView(cocos2d::Ref* pSender);
    
    //获取好友列表
    void getFriendList(cocos2d::Ref* pSender);
    
    //加为好友
    void addFriend(cocos2d::Ref* pSender);


private:
	
	ShareControl();
	~ShareControl();

};


