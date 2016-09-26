#include "MailControl.h"
#include "cJSON.h"
#include "comm.h"
#include "cmd.h"
#include "PKNotificationCenter.h"
#include "ApiGame.h"
#include "ApiUser.h"
#include "UserControl.h"
#include "LayerLoading.h"
#include "LayerMail.h"
#include "TimeControl.h"
static MailControl *m_pInstance = nullptr;

MailControl::MailControl() :view(nullptr), _current_user_id(0), current_read_message(nullptr)
{
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onGetGameNoticeCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::GAME_NOTICE), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onUpdateMessageStatusCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::UPDATE_MESSAGE_STATUS), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onGetMessageCountCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::GET_MESSAGE_COUNT), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onPageUserMessageCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::PAGE_USER_MESSAGE), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onSendFriendMessageCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::SEND_FRIEND_MESSAGE), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onSendFriendInviteCallBack), GetMsgTypeString(MSGTYPE_NOTICE, MSGCMD_NOTICE::SEND_FRIEND_INVITE), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onMailSwitch), "MailSwitch", nullptr);//监听来自视图的事件
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onReadMessage), "ReadMessage", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onMessageNotifyCallBack), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::SYS_MSG_NOTIFY), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(MailControl::onMessageNotifyCallBack), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::USER_MSG_NOTIFY), NULL);
}



void MailControl::requestMail()
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (_current_user_id == 0 || _current_user_id != user_id){

		LayerLoading::Wait();
		if (Api::User::pageUserMessage(user_id, 1, 500)){
			_current_user_id = user_id;
		}
		else{
			LayerLoading::CloseWithTip("pageUserMessage net error");
		}
	}
}

void MailControl::onMailSwitch(Ref *pSender)
{
	//暂时不做分页
	String* type = (String*)pSender;
	// 判断是否已经读取
	int mailType = atoi(type->getCString());


	if (view){
		((LayerMail*)view)->updateMailList();
	}
	
}

static bool compare(MessageData* a, MessageData* b)
{
	if (a->status != b->status){
		return a->status < b->status;
	}
	
	return a->id>b->id; 
}


void MailControl::sortMail()
{
	sort(sys_list.begin(), sys_list.end(), compare);//倒叙
	sort(user_list.begin(), user_list.end(), compare);//倒叙
}

void MailControl::updateViewList()
{
	sortMail();
	if (view != nullptr){
		((LayerMail*)view)->updateMailList();
	}
	PKNotificationCenter::getInstance()->postNotification("TriggerMailUnRead");
}


void MailControl::onReadMessage(Ref *pSender)
{
	MessageData* messageData = (MessageData*)pSender;

	if (!messageData)return;

	this->current_read_message = messageData;

	int user_id = UserControl::GetInstance()->GetUserData()->user_id;

	if (!Api::User::updateMessageStatus(user_id, messageData->id, messageData->status)){
		LayerLoading::CloseWithTip("updateMessageStatus net error");
	}
	
}


void MailControl::deleteMessage(int id)
{
	for (vector<MessageData*>::iterator i = sys_list.begin(); i != sys_list.end();)
	{
		if ((*i)->id == id)
		{
			delete *i;

			i = sys_list.erase(i);
			return;
		}
		else
		{
			i++;
		}
	}

	for (vector<MessageData*>::iterator i = user_list.begin(); i != user_list.end();)
	{
		if ((*i)->id == id)
		{
			delete *i;

			i = user_list.erase(i);
			return;
		}
		else
		{
			i++;
		}
	}

	updateViewList();
}

int MailControl::getUnReadMailCount()
{
	int count = 0;
	for (MessageData* item : sys_list){
		if (item->status == NOTICE_MSG_STATUS::UNREAD){
			count = count + 1;
		}
	}

	for (MessageData* item : user_list){
		if (item->status == NOTICE_MSG_STATUS::UNREAD){
			count = count + 1;
		}
	}
	return count;
}




void MailControl::onSendFriendMessageCallBack(Ref *pSender)
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
			if (InitSendFriendMessage(msg->data))
			{
				//更新成功，通知其他	/;p[--进程
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onSendFriendMessageCallBack", "onSendFriendMessageCallBack error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onSendFriendMessageCallBack", "onSendFriendMessageCallBack error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool MailControl::InitSendFriendMessage(std::string jsonData)
{
	return true;
}


void MailControl::onSendFriendInviteCallBack(Ref *pSender)
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
			if (IniSendFriendInvite(msg->data))
			{
				//更新成功，通知其他	/;p[--进程
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onSendFriendMessageCallBack", "onSendFriendMessageCallBack error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onSendFriendMessageCallBack", "onSendFriendMessageCallBack error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool MailControl::IniSendFriendInvite(std::string jsonData)
{
	return true;
}




void MailControl::onGetGameNoticeCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			if (InitGameNotice(msg->data))
			{
				//更新成功，通知其他进程
				if (isLoginNotice())
				{
					PKNotificationCenter::getInstance()->postNotification("TriggerShowNotice", nullptr);

					int user_id = UserControl::GetInstance()->GetUserData()->user_id;
					double serverTime = TimeControl::GetInstance()->GetServerTime();
					std::string user_key = StringUtils::format("%d_LoginNoticeTime", user_id);
					UserDefault::getInstance()->setIntegerForKey(user_key.c_str(), serverTime);   //注释测试
				}
				else{
					PKNotificationCenter::getInstance()->postNotification("TriggerRequestTask", nullptr);
				}
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onGetGameNoticeCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onGetGameNoticeCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool MailControl::InitGameNotice(std::string jsonData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	for (NoticeData* noticeData : notice_list)
	{
		delete noticeData;
	}
	notice_list.clear();

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *mailList, *count;


	if (!(mailList = cJSON_GetArrayItem(jsonMsg, 0)) || mailList->type != cJSON_Array){
		cJSON_Delete(jsonMsg);
		return false;
	}

	if (!(count = cJSON_GetArrayItem(jsonMsg, 1)) || count->type != cJSON_Number){
		cJSON_Delete(jsonMsg);
		return false;
	}

	size = cJSON_GetArraySize(mailList);
	
	cJSON *mail;
	cJSON *status, *title, *content, *create_time, *publisher_id, *id, *publisher_name;

	for (int i = 0; i < size; i++) {
		if (!(mail = cJSON_GetArrayItem(mailList, i)) || mail->type != cJSON_Object
			|| !(status = cJSON_GetObjectItem(mail, "status")) || status->type != cJSON_Number
			|| !(title = cJSON_GetObjectItem(mail, "title")) || title->type != cJSON_String
			|| !(publisher_id = cJSON_GetObjectItem(mail, "publisher_id")) || publisher_id->type != cJSON_Number
			|| !(create_time = cJSON_GetObjectItem(mail, "create_time")) || create_time->type != cJSON_String
			|| !(publisher_name = cJSON_GetObjectItem(mail, "publisher_name")) || publisher_name->type != cJSON_String
			|| !(content = cJSON_GetObjectItem(mail, "content")) || content->type != cJSON_String
			|| !(id = cJSON_GetObjectItem(mail, "id")) || id->type != cJSON_Number
			){
			cJSON_Delete(jsonMsg);
			return false;
		}


		NoticeData* noticeData = new NoticeData;
		noticeData->status = status->valueint;
		noticeData->title = title->valuestring;
		noticeData->publisher_id = publisher_id->valueint;
		noticeData->content = content->valuestring;

		noticeData->publisher_name = publisher_name->valuestring;
		noticeData->id = id->valueint;
		noticeData->create_time = create_time->valuestring;
		notice_list.push_back(noticeData);
		

	}
	cJSON_Delete(jsonMsg);
	return true;
}

vector<NoticeData*> & MailControl::getNoticeList()
{
	return notice_list;
}


void MailControl::onUpdateMessageStatusCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			if (InitUpdateMessageStatus(msg->data))
			{
				//更新成功，通知其他进程

				if (current_read_message != nullptr){
					if (current_read_message->status == NOTICE_MSG_STATUS::DELETED){
						deleteMessage(current_read_message->id);
					}
					
					updateViewList();
				}

			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onUpdateMessageStatusCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onUpdateMessageStatusCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool MailControl::InitUpdateMessageStatus(std::string jsonData)
{
	return true;
}

void MailControl::onGetMessageCountCallBack(Ref *pSender)
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
			if (InitGetMessageCount(msg->data))
			{
				//更新成功，通知其他进程
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onGetMessageCountCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			LayerLoading::CloseWithTip("get MessageCount error.");
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onGetMessageCountCallBack", "get MessageCount error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool MailControl::InitGetMessageCount(std::string jsonData)
{
	return true;
}

void MailControl::onPageUserMessageCallBack(Ref *pSender)
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
			
			if (InitPageUserMessage(msg->data))
			{
				//更新成功，通知其他进程
				updateViewList();
				//
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onPageUserMessageCallBack", "PageUserMessage error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onPageUserMessageCallBack", "get PageUserMessage error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool MailControl::InitPageUserMessage(std::string jsonData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	for (MessageData* item : sys_list){
		if (item->content){
			delete item->content;
		}
		delete item;
	}

	for (MessageData* item : user_list){
		if (item->content){
			delete item->content;
		}
		delete item;
	}

	sys_list.clear();
	user_list.clear();

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *mailList, *count;


	if (!(mailList = cJSON_GetArrayItem(jsonMsg, 0)) || mailList->type != cJSON_Array){
		cJSON_Delete(jsonMsg);
		return false;
	}

	if (!(count = cJSON_GetArrayItem(jsonMsg, 1)) || count->type != cJSON_Number){
		cJSON_Delete(jsonMsg);
		return false;
	}

	cJSON *mail;
	cJSON *status, *username, *user_id, *sender_name, *sender_id, *title, *content, *type, *create_time, *id, *sender_nickname;
	size = cJSON_GetArraySize(mailList);

	for (int i = 0; i < size; i++) {
		if (!(mail = cJSON_GetArrayItem(mailList, i)) || mail->type != cJSON_Object
			|| !(status = cJSON_GetObjectItem(mail, "status")) || status->type != cJSON_Number
			|| !(username = cJSON_GetObjectItem(mail, "username")) || username->type != cJSON_String
			|| !(user_id = cJSON_GetObjectItem(mail, "user_id")) || user_id->type != cJSON_Number
			|| !(sender_name = cJSON_GetObjectItem(mail, "sender_name")) || sender_name->type != cJSON_String
			|| !(sender_nickname = cJSON_GetObjectItem(mail, "sender_nickname")) || sender_nickname->type != cJSON_String
			|| !(sender_id = cJSON_GetObjectItem(mail, "sender_id")) || sender_id->type != cJSON_Number
			|| !(title = cJSON_GetObjectItem(mail, "title")) || title->type != cJSON_String
			|| !(content = cJSON_GetObjectItem(mail, "content")) || content->type != cJSON_String
			|| !(type = cJSON_GetObjectItem(mail, "type")) || type->type != cJSON_Number
			|| !(create_time = cJSON_GetObjectItem(mail, "create_time")) || create_time->type != cJSON_String
			|| !(id = cJSON_GetObjectItem(mail, "id")) || id->type != cJSON_Number
			){
			cJSON_Delete(jsonMsg);
			return false;
		}


		MessageData* messageData = new MessageData;
		messageData->status = status->valueint;
		messageData->username = username->valuestring;
		messageData->sender_nickname = sender_nickname->valuestring;

		messageData->user_id = user_id->valueint;
		messageData->sender_name = sender_name->valuestring;
		messageData->sender_id = sender_id->valueint;


		messageData->title = title->valuestring;

		MailData* mailData = new MailData;
		InitMailContent(content->valuestring, mailData);
		messageData->content = mailData;

		messageData->type = type->valueint;

		messageData->create_time = create_time->valuestring;
		messageData->id = id->valueint;
	
		dealMessageTitleFormat(messageData);
		

		if (messageData->type == NOTICE_MSG_TYPE::SYS_MSG){ //充值,请求好友,拒绝,删除

			sys_list.push_back(messageData);
		}
		else{
			//聊天,邀请
			user_list.push_back(messageData);
		}



	}
	cJSON_Delete(jsonMsg);
	

	return true;
}


void MailControl::dealMessageTitleFormat(MessageData* data)
{

	switch (data->content->type){
	case NoticeMsgSubType::USER_MSG_ADD_FRIEND_RSP:
	
	case NoticeMsgSubType::USER_MSG_DEL_FRIEND:
	
	case NoticeMsgSubType::USER_MSG_ADD_FRIEND_REQ:
	{
		data->title = data->content->nickname +" "+ Language::getStringByKey(data->title.c_str());
	}
		break;
	case NoticeMsgSubType::FRIEND_INVITE_GAME:
	{
		data->title = data->sender_nickname +" "+Language::getStringByKey("InviteGameTitleFormat");
	}
		break;
	case NoticeMsgSubType::FRIEND_SEND_MSG:
	{
		data->title = data->sender_nickname +" "+Language::getStringByKey("TalkTitleFormat");
	}
		break;
	}
}


void MailControl::onMessageNotifyCallBack(Ref *pSender)
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

			if (InitMessageNotifyCallBack(msg->data))
			{
				//更新成功，通知其他进程
				updateViewList();
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "MailControl::onPageUserMessageCallBack", "PageUserMessage error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "MailControl::onPageUserMessageCallBack", "get PageUserMessage error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool MailControl::InitMessageNotifyCallBack(std::string jsonData)
{
	cJSON *mail;
	if (!(mail = cJSON_Parse(jsonData.c_str())))
		return false;
	if (mail->type != cJSON_Object)
	{
		cJSON_Delete(mail);
		return false;
	}

	cJSON *status, *username, *user_id, *sender_name, *sender_id, *title, *content, *type, *create_time, *id, *sender_nickname;

	if (!(status = cJSON_GetObjectItem(mail, "status")) || status->type != cJSON_Number
		|| !(username = cJSON_GetObjectItem(mail, "username")) || username->type != cJSON_String
		|| !(user_id = cJSON_GetObjectItem(mail, "user_id")) || user_id->type != cJSON_Number
		|| !(sender_name = cJSON_GetObjectItem(mail, "sender_name")) || sender_name->type != cJSON_String
		|| !(sender_nickname = cJSON_GetObjectItem(mail, "sender_nickname")) || sender_nickname->type != cJSON_String
		|| !(sender_id = cJSON_GetObjectItem(mail, "sender_id")) || sender_id->type != cJSON_Number
		|| !(title = cJSON_GetObjectItem(mail, "title")) || title->type != cJSON_String
		|| !(content = cJSON_GetObjectItem(mail, "content")) || content->type != cJSON_String
		|| !(type = cJSON_GetObjectItem(mail, "type")) || type->type != cJSON_Number
		|| !(create_time = cJSON_GetObjectItem(mail, "create_time")) || create_time->type != cJSON_String
		|| !(id = cJSON_GetObjectItem(mail, "id")) || id->type != cJSON_Number
		){
		cJSON_Delete(mail);
		return false;
	}


	MessageData* messageData = new MessageData;
	messageData->status = status->valueint;
	messageData->username = username->valuestring;
	messageData->sender_nickname = sender_nickname->valuestring;

	messageData->user_id = user_id->valueint;
	messageData->sender_name = sender_name->valuestring;
	messageData->sender_id = sender_id->valueint;

	messageData->title = title->valuestring;

	MailData* mailData = new MailData;
	InitMailContent(content->valuestring, mailData);
	messageData->content = mailData;

	messageData->type = type->valueint;

	messageData->create_time = create_time->valuestring;
	messageData->id = id->valueint;

	dealMessageTitleFormat(messageData);

	if (mailData->type == NoticeMsgSubType::USER_MSG_ADD_FRIEND_RSP
		|| mailData->type == NoticeMsgSubType::USER_MSG_DEL_FRIEND){
		PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", nullptr); //刷新好友
	}

	if (messageData->type == NOTICE_MSG_TYPE::SYS_MSG){
		sys_list.push_back(messageData);
	}
	else{
		user_list.push_back(messageData);
	}

	cJSON_Delete(mail);

	return true;
}


bool  MailControl::InitMailContent(std::string jsonData, MailData* mailData)
{
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Object)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *content, *user_id, *nickname, *avatar, *type,*game_type,*token;

	if ( !(content = cJSON_GetObjectItem(jsonMsg, "content")) || content->type != cJSON_String
		|| !(type = cJSON_GetObjectItem(jsonMsg, "type")) || type->type != cJSON_Number
	)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{



		mailData->content = content->valuestring;
		mailData->type = type->valueint;
		
		if ((user_id = cJSON_GetObjectItem(jsonMsg, "user_id")) && user_id->type == cJSON_Number){
			mailData->user_id = user_id->valueint;
		}
		

		if ((avatar = cJSON_GetObjectItem(jsonMsg, "avatar")) && avatar->type == cJSON_Number){
			mailData->avatar = avatar->valueint;
		}

		if ((nickname = cJSON_GetObjectItem(jsonMsg, "nickname")) && nickname->type == cJSON_String){
			mailData->nickname = nickname->valuestring;
		}

		if ((game_type = cJSON_GetObjectItem(jsonMsg, "game_type")) && game_type->type == cJSON_Number){
			mailData->game_type = game_type->valueint;
		}

		if ((token = cJSON_GetObjectItem(jsonMsg, "token")) && token->type == cJSON_String){
			mailData->token = token->valuestring;
		}
	}

	return true;
}



bool MailControl::isLoginNotice()
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	std::string user_key = StringUtils::format("%d_LoginNoticeTime", user_id);
	long loginAwardTime = UserDefault::getInstance()->getIntegerForKey(user_key.c_str());

	if (loginAwardTime <= 0)
	{
		return true;
	}

	time_t serverTime = TimeControl::GetInstance()->GetServerTime();
	struct tm *ptm_st = gmtime(&serverTime);
	if (ptm_st == nullptr) return false;

	char st[100] = { 0 };
	memset(st, 0x0, 100);
	strftime(st, sizeof(st), "%d", ptm_st);

	time_t lat = loginAwardTime;
	struct tm *ptm_lat = gmtime(&lat);
	char tmp_lat[100] = { 0 };
	memset(tmp_lat, 0x0, 100);
	strftime(tmp_lat, sizeof(tmp_lat), "%d", ptm_lat);

	CCLOG("from last loginAward  time:%f hour", (serverTime - loginAwardTime) / 3600);

	if (serverTime - loginAwardTime < 24 * 3600)
	{
		int s_d = String::create(st)->intValue();
		int l_d = String::create(tmp_lat)->intValue();
		if (serverTime >= loginAwardTime &&  s_d - l_d >= 1)
		{
			//std::string intervalTurn_key = StringUtils::format("%d_IntervalTurn", user_id);
			//UserDefault::getInstance()->setIntegerForKey(intervalTurn_key.c_str(), 0);//登录奖励的时候重置
			return true;
		}
	}
	else
	{
		return true;
	}
	return false;
}



vector<MessageData*> &  MailControl::getMailList(int type)
{
	if (type == NOTICE_MSG_TYPE::SYS_MSG){
		return sys_list;
	}
	else{
		return user_list;
	}
}

//请求公告
void  MailControl::requestNotice()
{
	if (!Api::User::GetGameNotice())
	{
		LayerLoading::CloseWithTip("GetGameNotice net error");
	}
	
}


void MailControl::setMailView(Node* view)
{
	this->view = view;
}

MailControl::~MailControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);

	for (MessageData* item : sys_list){
		if (item->content){
			delete item->content;
		}
		delete item;
	}

	for (MessageData* item : user_list){
		if (item->content){
			delete item->content;
		}
		delete item;
	}

	sys_list.clear();
	user_list.clear();

	for (NoticeData* noticeData : notice_list)
	{
		delete noticeData;
	}
	notice_list.clear();

	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
MailControl* MailControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new MailControl();
	}

	return m_pInstance;
}

