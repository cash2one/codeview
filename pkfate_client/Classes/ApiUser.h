#pragma once
namespace Api
{
	class User
	{
	public:
		User();
		~User();
		static bool Register(unsigned short login_type, const char *username, const char *password, int parentId, const char * channel);
		static bool Bind(unsigned int user_id, const char *username, const char *password);
		static bool GetUserInfo();
		static bool Login(const char* email, const char* password);		//登录
		static bool CheckNickName(const char* nickname);				//检查nickname是否可用
		static bool SetNickName(const char *nickname, int avatarid);
		static bool SetAvatar(int avatarid);
		static bool UpdatePassword(int userId, const char* oldPassword, const char* newPassword);

		static bool updateMessageStatus(int user_id,int message_id,int status);
		static bool getMessageCount(int user_id);
		static bool pageUserMessage(int user_id, int page_index, int page_siz);
		static bool sendFriendMessage(int user_id, int to_user_id, const char * content);

		static bool sendFriendInvite(int user_id, int to_user_id, int game_type, const char* game_token, const char* remark ="");

		static bool sendBindMobileTicket(int user_id,const char* mobile);
		static bool validBindMobile(int user_id,int ticket_id,const char* ticket);


		static bool GetGameNotice();


		static bool makeFriend(int user_id, int other_user_id, int action, const char* remark="");
		static bool fetchFriend(int user_id);
		static bool lookupFriend(const char*  key);

		static bool ExistUser(const char *username);

		static bool FetchOnlineUsers();
	};
}
