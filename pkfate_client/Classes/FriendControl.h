#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
enum FriendAction{
	FRIEND_ADD_REQUEST = 0,
	FRIEND_ADD_RESPONSE_OK = 1,
	FRIEND_ADD_RESPONSE_DENY = 2,
	FRIEND_DEL = 3,
};




struct FriendData{
	int user_id;
	string nickname;
	int avatar;
};


class FriendControl :public Ref
{
public:
	static FriendControl* GetInstance();
	void setFriendView(Node* view);
	std::vector<FriendData*>& getFriendList();
	bool isFriend(int user_id);

	FriendData* getFriendData(int user_id);

	void requestFriend();
	void fetchOnlineUsers();

private:
	FriendControl();
	~FriendControl();

	void onFetchFriendCallBack(Ref *pSender);
	bool InitFetchFriend(std::string jsonData);

	void onMakeFriendCallBack(Ref *pSender);
	bool InitMakeFriend(std::string jsonData);

	void onLookupFriendCallBack(Ref *pSender);
	bool InitLookupFriend(std::string jsonData, FriendData* friendData);

	void onTriggleGetFriendList(Ref *pSender);
	void onTriggleLookupFriend(Ref *pSender);
	void onTriggleAddFriend(Ref *pSender);
	void onTriggleAgreeAddFriend(Ref *pSender);
	void onTriggleDenyFriend(Ref *pSender);
	
	void onTriggleDelFriend(Ref *pSender);
	void onTriggleSendPhoneCode(Ref *pSender);
	void onTriggleBindPhone(Ref *pSender);

	void onSendBindMobileTicket(Ref*pSender);
	bool InitSendBindMobile(std::string jsonData);

	void onValidBindMobile(Ref*pSender);
	bool InitValidBindMobile(std::string jsonData);

	void delRefreshView(int user_id);
    
	void onFetchOnlineUsersCallBack(Ref*pSender);
	bool InitFetchOnlineUsersData(string jsonData, std::vector<FriendData*> &vecFriendData);

private:
	int ticket_id;
	Node * view;
	string mobile;
	std::vector<FriendData*> friendList;
	std::vector<FriendData*> _vecStrangerList;
};


