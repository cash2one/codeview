#include "FriendControl.h"
#include "cJSON.h"
#include "comm.h"
#include "cmd.h"
#include "PKNotificationCenter.h"
#include "ApiGame.h"
#include "ApiUser.h"
#include "LayerLoading.h"
#include "UserControl.h"
#include "LayerFriend.h"
static FriendControl *m_pInstance = nullptr;

FriendControl::FriendControl() :ticket_id(-1), mobile(""),
	view(nullptr)
{
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onFetchFriendCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::FETCH_FRIEND), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onMakeFriendCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::MAKE_FRIEND), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onLookupFriendCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOOKUP_USER), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleGetFriendList), "TriggleGetFriendList", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleLookupFriend), "TriggleLookupFriend", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleAddFriend), "TriggleAddFriend", nullptr);//监听来自视图的事件
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleAgreeAddFriend), "TriggleAgreeAddFriend", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleDelFriend), "TriggleDelFriend", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleDenyFriend), "TriggleDenyFriend", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onSendBindMobileTicket), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::SEND_BLIND_MOBILE_TICKET), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onValidBindMobile), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::VALID_BIND_MOBILE), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleSendPhoneCode), "TriggleSendPhoneCode", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onTriggleBindPhone), "TriggleBindPhone", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(FriendControl::onFetchOnlineUsersCallBack), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::FETCH_ONLINE_USERS), NULL);
}



void FriendControl::onTriggleSendPhoneCode(Ref *pSender)
{
	String* phoneStr = (String*)pSender;
	if (!phoneStr) return;
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	LayerLoading::Wait();

	this->mobile = phoneStr->getCString();

	if (!Api::User::sendBindMobileTicket(user_id, mobile.c_str())){
		LayerLoading::CloseWithTip("sendBindMobileTicket net error");
	}
}


void FriendControl::onTriggleBindPhone(Ref *pSender)
{
	if (ticket_id == -1){
		LayerLoading::CloseWithTip("phone code error");
		return;
	} 

	String* codeStr = (String*)pSender;
	if (!codeStr) return;

	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	LayerLoading::Wait();

	if (!Api::User::validBindMobile(user_id, ticket_id, codeStr->getCString())){
		LayerLoading::CloseWithTip("validBindMobile net error");
	}
}


void FriendControl::onSendBindMobileTicket(Ref*pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (InitSendBindMobile(msg->data))
			{
				//更新成功，通知其他进程

				ticket_id = atoi(msg->data.c_str());
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "FriendControl::onBindMobileTicket", "onBindMobileTicket error.", msg->data.c_str());
			}
		}
		else
		{
			LayerLoading::CloseWithTip("phone error");
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "FriendControl::onBindMobileTicket", "onBindMobileTicket error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool FriendControl::InitSendBindMobile(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	
	return true;
}

void FriendControl::onValidBindMobile(Ref*pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		

		if (msg->code == 0)
		{
			if (InitValidBindMobile(msg->data))
			{
				//更新成功，通知其他进程
				UserControl::GetInstance()->GetUserData()->mobile = mobile;
				if (view){
					((LayerFriend*)view)->refreshInvitePanel();
				}
				
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "FriendControl::onValidBindMobile", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "FriendControl::onValidBindMobile", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool FriendControl::InitValidBindMobile(std::string jsonData)
{
	/*if (jsonData.empty()) //返回数据null
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}*/

	return true;
}


FriendControl::~FriendControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);

	for (FriendData* friendData : friendList)
	{
		delete friendData;
	}
	friendList.clear();

	for (FriendData* friendData : _vecStrangerList)
	{
		delete friendData;
	}
	_vecStrangerList.clear();

	if (m_pInstance != nullptr)
	{
		
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
FriendControl* FriendControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new FriendControl();
		
    }
		
	return m_pInstance;
}


void FriendControl::onTriggleAddFriend(Ref *pSender)
{

	Array* array = (Array*)pSender;
	String* toId = (String*)array->objectAtIndex(0);
	String* remark = (String*)array->objectAtIndex(1);
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::makeFriend(user_id, toId->intValue(), FriendAction::FRIEND_ADD_REQUEST,remark->getCString())){
		LayerLoading::CloseWithTip("AddFriend error");
	}

}

void FriendControl::onTriggleAgreeAddFriend(Ref *pSender)
{

	Array* array = (Array*)pSender;
	String* toId = (String*)array->objectAtIndex(0);
	String* remark = (String*)array->objectAtIndex(1);
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::makeFriend(user_id, toId->intValue(), FriendAction::FRIEND_ADD_RESPONSE_OK, remark->getCString())){
		LayerLoading::CloseWithTip("  AgreeAddFriend error");
	}

}


void FriendControl::onTriggleDenyFriend(Ref *pSender)
{

	Array* array = (Array*)pSender;
	String* toId = (String*)array->objectAtIndex(0);
	String* remark = (String*)array->objectAtIndex(1);
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::makeFriend(user_id, toId->intValue(), FriendAction::FRIEND_ADD_RESPONSE_DENY, remark->getCString())){
		LayerLoading::CloseWithTip(" agree DenyFriend error");
	}

}



void FriendControl::requestFriend()
{
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::fetchFriend(user_id))
	{
		LayerLoading::CloseWithTip("GetGameNotice net error");
	}
}


void FriendControl::onTriggleDelFriend(Ref *pSender)
{
	String* key = (String*)pSender;
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::makeFriend(user_id, key->intValue(), FriendAction::FRIEND_DEL)){
		LayerLoading::CloseWithTip("delFriend error");
	}
}

void FriendControl::onTriggleLookupFriend(Ref *pSender)
{
	String* key = (String*)pSender;

	LayerLoading::Wait();
	if (!Api::User::lookupFriend(key->getCString())){
		LayerLoading::CloseWithTip("lookupFriendnet error");
	}
}

void FriendControl::onTriggleGetFriendList(Ref *pSender)
{
	LayerLoading::Wait();
	if (!Api::User::fetchFriend(UserControl::GetInstance()->GetUserData()->user_id)){
		LayerLoading::CloseWithTip("fetch friend net error");
	}
}

void FriendControl::onFetchFriendCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (InitFetchFriend(msg->data))
			{
				PKNotificationCenter::getInstance()->postNotification("updateFriendList", nullptr); //第一次 默认切换
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onFetchFriendCallBack", "onFetchFriendCallBack error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onFetchFriendCallBack", "onFetchFriendCallBack error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏

		if (friendList.size() <= 0 && dynamic_cast<LayerFriend*>(view))
		{
			fetchOnlineUsers();
		}
	});
}

bool FriendControl::InitFetchFriend(std::string jsonData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	for (FriendData* friendData : friendList)
	{
		delete friendData;
	}
	friendList.clear();

	cJSON *friend_,  *user_id, *nickname, *avatar;
	int size = cJSON_GetArraySize(jsonMsg);

	for (int i = 0; i < size; i++) {
		if (!(friend_ = cJSON_GetArrayItem(jsonMsg, i)) || friend_->type != cJSON_Object
			|| !(avatar = cJSON_GetObjectItem(friend_, "avatar")) || avatar->type != cJSON_Number
			|| !(nickname = cJSON_GetObjectItem(friend_, "nickname")) || nickname->type != cJSON_String
			|| !(user_id = cJSON_GetObjectItem(friend_, "user_id")) || user_id->type != cJSON_Number
			){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			FriendData* friendData = new FriendData;
			friendData->avatar = avatar->valueint;
			friendData->nickname = nickname->valuestring;
			friendData->user_id = user_id->valueint;
			friendList.push_back(friendData);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}

void FriendControl::onMakeFriendCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			
			if (InitMakeFriend(msg->data))
			{
				//更新成功，通知其他进程
				//PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", nullptr); //刷新好友
				//
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onMakeFriendCallBack", "onMakeFriendCallBack.", msg->data.c_str());
			}
		}
		else
		{
			if (msg->code == 2018){
				LayerLoading::CloseWithTip("don`t not add Friend again");
			}
			else{
				//失败
				CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onMakeFriendCallBack", "onMakeFriendCallBackerror.", msg->code, msg->data.c_str());
			}
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool FriendControl::InitMakeFriend(std::string jsonData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Object)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}


	cJSON *other_user_id, *action;

	if (!(other_user_id = cJSON_GetObjectItem(jsonMsg, "other_user_id")) || other_user_id->type != cJSON_Number
		|| !(action = cJSON_GetObjectItem(jsonMsg, "action")) || action->type != cJSON_Number
		)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{

		int s_action = action->valueint;
		int s_user_id = other_user_id->valueint;

		if (s_action == FriendAction::FRIEND_DEL){
			delRefreshView(s_user_id);
		}
		else if (s_action == FriendAction::FRIEND_ADD_REQUEST){
			LayerLoading::CloseWithTip(Language::getStringByKey("YourFriendAskSended"));//您的好友请求已经发送成功
		}

	}
	cJSON_Delete(jsonMsg);

	return true;
}

void FriendControl::delRefreshView( int user_id)
{
	
	for (vector<FriendData*>::iterator i = friendList.begin(); i != friendList.end();)
	{
		if ((*i)->user_id == user_id)
		{
			delete *i;

			i = friendList.erase(i);
			break;
		}
		else
		{
			i++;
		}
	}

	if (view){
		((LayerFriend*)view)->updateFriendList(friendList);
	}
}

void FriendControl::onLookupFriendCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			FriendData* friendData = new FriendData;
			if (InitLookupFriend(msg->data, friendData))
			{
				_vecStrangerList.insert(_vecStrangerList.begin(), friendData);
				if (view){
					((LayerFriend*)view)->updateFriendList(_vecStrangerList);
				}
			}
			else
			{
				LayerLoading::CloseWithTip("can not find friend ");
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onLookupFriendCallBack", "update onLookupFriendCallBack error.", msg->data.c_str());
			}

			delete friendData;
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onLookupFriendCallBack", "onLookupFriendCallBack error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool FriendControl::InitLookupFriend(std::string jsonData,FriendData* friendData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Object)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	
	cJSON *user_id, *nickname, *avatar;

	if(!(user_id = cJSON_GetObjectItem(jsonMsg, "user_id")) || user_id->type != cJSON_Number
		|| !(nickname = cJSON_GetObjectItem(jsonMsg, "nickname")) || nickname->type != cJSON_String
		|| !(avatar = cJSON_GetObjectItem(jsonMsg, "avatar")) || avatar->type != cJSON_Number
	)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{
		
		friendData->avatar = avatar->valueint;
		friendData->nickname = nickname->valuestring;
		friendData->user_id = user_id->valueint;
		

	}
	cJSON_Delete(jsonMsg);
	return true;
}

bool FriendControl::isFriend(int user_id)
{
	for (FriendData* friendData : friendList)
	{
		if (friendData->user_id == user_id){
			return true;
		}
	}
	return false;
}

FriendData* FriendControl::getFriendData(int user_id)
{
	for (FriendData* friendData : friendList)
	{
		if (friendData->user_id == user_id){
			return friendData;
		}
	}
	return nullptr;
}

std::vector<FriendData*>& FriendControl::getFriendList()
{
	return friendList;
}


void FriendControl::setFriendView(Node* view)
{
	this->view = view;
}

void FriendControl::onFetchOnlineUsersCallBack(Ref*pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();

	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			for (FriendData* friendData : _vecStrangerList)
			{
				delete friendData;
			}
			_vecStrangerList.clear();

			if (!InitFetchOnlineUsersData(msg->data, _vecStrangerList))
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "FriendControl::onFetchOnlineUsersCallBack", "update onFetchOnlineUsersCallBack error.", msg->data.c_str());
			}

			if (view)
			{
				((LayerFriend*)view)->updateFriendList(_vecStrangerList);
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onLookupFriendCallBack", "onLookupFriendCallBack error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void FriendControl::fetchOnlineUsers()
{
	if (!Api::User::FetchOnlineUsers())
	{
		LayerLoading::CloseWithTip("AddFriend error");
	}
}

bool FriendControl::InitFetchOnlineUsersData(string jsonData, vector<FriendData*> &vecFriendData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())) || jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return NULL;
	}

	int array_size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < array_size; i++)
	{
		FriendData *friendData = new FriendData();
		cJSON *content = cJSON_GetArrayItem(jsonMsg, i);

		cJSON *user_id, *nickname, *avatar;
		if (!(user_id = cJSON_GetArrayItem(content, 0)) || user_id->type != cJSON_Number
			|| !(avatar = cJSON_GetArrayItem(content, 1)) || avatar->type != cJSON_Number
			|| !(nickname = cJSON_GetArrayItem(content, 2)) || nickname->type != cJSON_String
			)
			continue;

		friendData->avatar = avatar->valueint;
		friendData->nickname = nickname->valuestring;
		friendData->user_id = user_id->valueint;

		vecFriendData.push_back(friendData);
	}

	cJSON_Delete(jsonMsg);
	return false;
}