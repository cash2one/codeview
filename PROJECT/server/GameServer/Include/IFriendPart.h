#ifndef __THINGSERVER_IFRIEND_PART__
#define __THINGSERVER_IFRIEND_PART__

#include "IThingPart.h"
#include "UniqueIDGenerator.h"

//好友的信息
struct SFriendBasicInfo
{
	SFriendBasicInfo(){
		memset(this, 0, sizeof(*this));
	}
	UINT64		 m_uidFriend;							//好友的UID值
	char		 m_szFriendName[THING_NAME_LEN];		//好友名字
	INT8		 m_Level;								//好友等级
	INT8		 m_Sex;									//好友性别
	char		 m_szSynName[THING_NAME_LEN];			//好友帮派名称
	UINT32		 m_RelationNum;							//和好友的好友度
	enFriendType m_enFriendType;						//单方好友还是双方好友
	bool		 m_bOneCity;							//是否同城	
	UINT16		 m_Facade;								//外观
	TTitleID	 m_TitleID;								//称号ID
	UINT8		 m_vipLevel;							//vip等级
	INT32		 m_DuoBaoLevel;							//夺宝等级
	INT32		 m_SynWarLevel;							//帮战等级
	UINT32		 m_CombatAbility;						//战斗力
	
};

struct IFriendPart : public IThingPart
{
	//加好友
	virtual bool	AddFriend(const UID & uidFriend) = 0;

	//删除好友
	virtual bool	DeleteFriend(const UID & uidFriend) = 0;

	//得到与此玩家的好友度
	virtual UINT32	GetRelationNum(const UID & uidFriend) = 0;

	//增加好友度	szEventDescript1是对于我的事件描述，szEventDescript2是对于好友的事件描述
	virtual void	AddRelationNum(const UID & uidFriend, INT32 AddRelationNum, const char * szEventDescript1, const char * szEventDescript2) = 0;

	//增加好友度(包括减少),根据增加方式来决定增加多少好友度
	virtual void	AddRelationNum(const UID & uidFriend, const char * szFriendName, enAddRelationNumType AddRelationNumType, INT8 nHours = 0/*需要时间的用，单位：小时*/) = 0;

	//更新好友度   因为好友度是双方共同的属性，所以也会修改另一方的好友度
	virtual void	UpdateFriend(const UID & uidFriend, UINT32 nRelateionNum) = 0;

	//查看好友
	virtual void	ViewFriend() = 0;

	//检测是否好友
	virtual bool	IsFriend(const UID & uid_Actor) = 0;

	//加改变好友度事件,szDescript为事件描述
	virtual void	AddFriendEnvent(const UID & uidFriend, UINT32 uAddRelationNum, const char * szDescript) = 0;

	//删除改变好友度事件(只保存一天记录)
	virtual void	DeleteFriendEnvent() = 0;

	//查看改变好友度事件
	virtual void	ViewFriendEnvent(UID & uidFriend) = 0;

	//加到好友信息标签里
	virtual void	AddToFriendMsgList(const UID & uidSrcUser) = 0;

	//查看好友信息标签里的信息
	virtual void	ViewFriendMsg() = 0;

	//设置好友对自己的好友度数值
	virtual void	SetRelationNum(const UID & uidFriend, UINT32 uRelationNum) = 0;

	//同步好友对自己的好友数据
	virtual void	SynFriendInfo(const UID & uidFriend, enFriendType FriendType, UINT8 RelationNum) = 0;

	//查看同城的在线玩家
	virtual void	ViewOneCityOnlineUser()	= 0;

	//造访好友
	virtual void	VisitFriend(const UID & uidFriend) = 0;

	virtual	void	OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) = 0;

	//得到好友名字
	virtual const char * GetFriendName(UID uidFriend) = 0;

	//得到好友信息
	virtual const SFriendBasicInfo * GetFriendInfo(UID uidFriend) = 0;

	//邮件点击选择收信人按钮
	virtual void  ClickMailFriend() = 0;

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing() = 0;

	//查看人物信息
	virtual void ViewUserInfo(UID uidUser) = 0;

	//离开查看其它玩家人物信息界面
	virtual void LeaveViewUserInfo(UID uidUser) = 0;

	//删除好友信息
	virtual void DeleteFriendMsg(UID uidUser) = 0;
};

#endif
