#include "ApiUser.h"
#include "SocketControl.h"
#include "cmd.h"

using namespace Api;
User::User()
{
}
User::~User()
{
}
/*
bool User::Register(char* email)
{
if (email == nullptr)
return false;

//-----  添加邮箱验证功能

//-----  生成 json msg
cJSON *jsonMsg, *jsonData, *jsonArgs;
jsonMsg = cJSON_CreateObject();
if (!jsonMsg)
{
CCLOG("[User.Register]:CJSON create error : [%s]", cJSON_GetErrorPtr());
return false;
}
cJSON_AddStringToObject(jsonMsg, "type", "user.regist");
cJSON_AddItemToObject(jsonMsg, "data", jsonData = cJSON_CreateObject());
cJSON_AddItemToObject(jsonData, "args", jsonArgs = cJSON_CreateArray());
cJSON_AddItemToArray(jsonArgs, cJSON_CreateString(email));
cJSON_AddItemToArray(jsonArgs, cJSON_CreateNull());
char *msg = cJSON_PrintUnformatted(jsonMsg);
if (msg)
{
SocketControl::GetInstance()->Send(msg);
free(msg);
}
cJSON_Delete(jsonMsg);
}*/
bool User::Register(unsigned short login_type, const char *username, const char *password, int parentId,const char * channel)
{
	char args[BUFSIZE];
	sprintf(args, "{\"login_type\":%d,\"username\":\"%s\",\"password\":\"%s\",\"parent_id\":%d,\"register_channel\":\"%s\"}", login_type, username == NULL ? "" : username, password == NULL ? "" : password, parentId, channel);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::REGISTER, args);
}


bool User::Bind(unsigned int user_id, const char *username, const char *password)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"username\":\"%s\",\"password\":\"%s\"}", user_id, username, password);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::BIND, args);
}

bool User::GetUserInfo()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, GET_USER_INFO, "");
}
bool User::Login(const char* email, const char* password)
{
	char args[BUFSIZE];
	sprintf(args, "{\"username\":\"%s\",\"password\":\"%s\"}", email, password);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::LOGIN, args);
}
//检查nickname是否可用
bool User::CheckNickName(const char* nickname)
{
	char args[BUFSIZE];
	sprintf(args, "{\"nickname\":\"%s\"}", nickname);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::CHECK_NICKNAME, args);
}
//设置昵称与头像
bool User::SetNickName(const char *nickname, int avatarid)
{
	char args[BUFSIZE];
	sprintf(args, "{\"nickname\":\"%s\",\"avatar_id\":%d}", nickname == NULL ? "" : nickname, avatarid);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::SET_NICKNAME, args);
}
//设置头像
bool User::SetAvatar(int avatarid)
{
	char args[BUFSIZE];
	sprintf(args, "{\"avatar_id\":%d}", avatarid);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::SET_AVATAR, args);
}

//设置头像
bool User::UpdatePassword(int userId, const char* oldPassword, const char* newPassword)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"old_password\":\"%s\",\"new_password\":\"%s\"}", userId,oldPassword,newPassword);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::UPDATE_PASSWORD, args);
}

bool User::updateMessageStatus(int user_id, int message_id, int status)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"message_id\":%d,\"status\":%d}", user_id, message_id, status);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::UPDATE_MESSAGE_STATUS, args);
}


bool User::getMessageCount(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::GET_MESSAGE_COUNT, args);
}


bool User::pageUserMessage(int user_id,int page_index,int page_size)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"page_index\":%d,\"page_size\":%d}", user_id, page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::PAGE_USER_MESSAGE, args);
}


bool User::sendFriendMessage(int user_id, int to_user_id, const char * content)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"to_user_id\":%d,\"content\":\"%s\"}", user_id, to_user_id, content);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::SEND_FRIEND_MESSAGE, args);
}

bool User::sendFriendInvite(int user_id, int to_user_id, int game_type, const char* game_token, const char* remark)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"to_user_id\":%d,\"game_type\":%d,\"game_token\":\"%s\",\"remark\":\"%s\"}", user_id, to_user_id, game_type, game_token, remark);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::SEND_FRIEND_INVITE, args);
}

bool User::GetGameNotice()
{
	char args[BUFSIZE];
	sprintf(args, "{\"page_index\":%d,\"page_size\":%d}", 1, 5);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_NOTICE, MSGCMD_NOTICE::GAME_NOTICE, args);
}

bool User::makeFriend(int user_id, int other_user_id, int action,const char* remark)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"other_user_id\":%d,\"action\":%d,\"remark\":\"%s\"}", user_id, other_user_id, action, remark);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::MAKE_FRIEND, args);
}

bool User::lookupFriend(const char*  key)
{

	char args[BUFSIZE];
	sprintf(args, "{\"nickname\":\"%s\"}", key);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::LOOKUP_USER, args);
}

bool User::fetchFriend(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::FETCH_FRIEND, args);
}

bool User::sendBindMobileTicket(int user_id, const char* mobile)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"mobile\":\"%s\"}", user_id,mobile);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::SEND_BLIND_MOBILE_TICKET, args);
}

bool User::validBindMobile(int user_id, int ticket_id, const char* ticket)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"ticket_id\":%d,\"ticket\":\"%s\"}", user_id, ticket_id,ticket);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::VALID_BIND_MOBILE, args);
}

bool User::ExistUser(const char *username)
{
	char args[BUFSIZE];
	sprintf(args, "{\"username\":\"%s\"}", username);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_USER, MSGCMD_USER::EXIST_USER, args);
}

bool User::FetchOnlineUsers()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TIME, MSGCMD_TIME::FETCH_ONLINE_USERS, "");
}