#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
enum NOTICE_MSG_TYPE
{
	SYS_MSG = 1,
	USER_MSG = 2,
};

enum NOTICE_MSG_STATUS
{
	DELETED = 0,
	UNREAD = 1,
	READED = 2,
};


enum NoticeMsgSubType
{
	BILL_MSG = 1,

	USER_MSG_ADD_FRIEND_REQ = 101,
	USER_MSG_ADD_FRIEND_RSP = 102,
	USER_MSG_DEL_FRIEND = 103,

	FRIEND_SEND_MSG = 201,
	FRIEND_INVITE_GAME = 202,
};

struct NoticeData{
	int status;
	string title;
	string content;
	string create_time;
	int publisher_id;
	string publisher_name;
	int id;
};

struct MailData{
	string content;
	int user_id;
	string nickname;
	int avatar;
	int type;
	string token;//dz邀请
	int game_type;//dz游戏类型
};

struct MessageData:Ref
{
	int status;
	std::string username;
	int user_id;

	std::string sender_name;
	std::string sender_nickname;
	int sender_id;

	std::string title;
	MailData* content;

	short type;

	std::string create_time;

	int id;
};


class MailControl :public Ref
{
public:
	static MailControl* GetInstance();
	void setMailView(Node* view);
	void requestNotice();

	vector<MessageData*> & getMailList(int type);
	vector<NoticeData*> & getNoticeList();
	bool isLoginNotice();

	void dealMessageTitleFormat(MessageData* data);
    
	int getUnReadMailCount();

	void requestMail();

	void sortMail();

	void deleteMessage(int id);

private:

	MessageData* current_read_message;

	void  updateViewList();

	vector<MessageData*> sys_list;
	vector<MessageData*> user_list;

	vector<NoticeData*> notice_list;
	int _current_user_id;
	Node* view;
	MailControl();
	~MailControl();

	void onGetGameNoticeCallBack(Ref *pSender);
	bool InitGameNotice(std::string jsonData);

	void onUpdateMessageStatusCallBack(Ref *pSender);
	bool InitUpdateMessageStatus(std::string jsonData);

	void onGetMessageCountCallBack(Ref *pSender);
	bool InitGetMessageCount(std::string jsonData);

	void onPageUserMessageCallBack(Ref *pSender);
	bool InitPageUserMessage(std::string jsonData);

	bool InitMailContent(std::string jsonData, MailData* mailData);

	void onSendFriendMessageCallBack(Ref *pSender);
	bool InitSendFriendMessage(std::string jsonData);

	void onSendFriendInviteCallBack(Ref *pSender);
	bool IniSendFriendInvite(std::string jsonData);
	
	void onMessageNotifyCallBack(Ref *pSender);
	bool InitMessageNotifyCallBack(std::string jsonData);
	
	
	void onReadMessage(Ref *pSender);

	void onMailSwitch(Ref *pSender);
};


