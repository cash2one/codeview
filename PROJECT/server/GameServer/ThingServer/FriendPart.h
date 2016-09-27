#ifndef __THINGSERVER_FRIEND_PART_H__
#define __THINGSERVER_FRIEND_PART_H__

#include "IFriendPart.h"
#include <vector>
#include <map>
#include <string>
#include "IEventServer.h"

struct IActor;


//好友信息里的玩家信息
struct SFriendMsgInfo
{
	SFriendMsgInfo(){
		memset(this, 0, sizeof(*this));
	}
	char		m_szSrcUserName[THING_NAME_LEN];
	INT8		m_Level;
	INT8		m_Sex;
	char		m_szSrcUserSynName[THING_NAME_LEN];
	bool		m_bOneCity;
	UINT16		m_Facade;
	TTitleID	m_TitleID;
	INT32		m_DuoBaoLevel;
	INT32		m_SynWarLevel;
	UINT32		m_CombatAbility;


};

struct SFriendEnventData
{
	UINT64	uid_friend;						//好友UID值
	char	szDescript[DESCRIPT_LEN_100];	//事件描述
	UINT64	uTime;							//事件发生的时间
};

typedef std::vector<SFriendEnventData> VECT_FRIENDEVENT;

struct SFriendData
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>		m_mapFriendData;	//好友信息列表				
	std::map<UID, SFriendMsgInfo>						m_mapFriendMsg;		//好友信息，是好友的信息标签中的信息				
	std::map<UINT64/*好友的UID值*/,	VECT_FRIENDEVENT>	m_mapFriendEnvent;	//改变好友度的事件
};

class FriendPart : public IFriendPart, public IEventListener
{
public:
	FriendPart(void);
	virtual ~FriendPart(void);

public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功 
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void);

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata);

public:
	//加好友
	virtual bool	AddFriend(const UID & uidFriend);

	//删除好友
	virtual bool	DeleteFriend(const UID & uidFriend);

	//得到与此玩家的好友度
	virtual UINT32	GetRelationNum(const UID & uidFriend);

	//增加好友度	szEventDescript1是对于我的事件描述，szEventDescript2是对于好友的事件描述
	virtual void	AddRelationNum(const UID & uidFriend, INT32 AddRelationNum, const char * szEventDescript1, const char * szEventDescript2);

	//增加好友度(包括减少),根据增加方式来决定增加多少好友度
	virtual void	AddRelationNum(const UID & uidFriend, const char * szFriendName, enAddRelationNumType AddRelationNumType, INT8 nHours = 0/*需要时间的用，单位：小时*/); 

	//更新好友度     因为好友度是双方共同的属性，所以也会修改另一方的好友度
	virtual void	UpdateFriend(const UID & uidFriend, UINT32 nRelateionNum);

	//查看好友
	virtual void	ViewFriend();

	//检测是否好友
	virtual bool	IsFriend(const UID & uid_Actor);

	//加载好友信息
	virtual void	LoadFriendInfo();

	//加改变好友度事件,szDescript为事件描述
	virtual void	AddFriendEnvent(const UID & uidFriend, UINT32 uAddRelationNum, const char * szDescript);

	//删除改变好友度事件(只保存一天记录)
	virtual void	DeleteFriendEnvent();

	//查看改变好友度事件
	virtual void	ViewFriendEnvent(UID & uidFriend);
	
	//加到好友信息标签里
	virtual void	AddToFriendMsgList(const UID & uidSrcUser);

	//查看好友信息标签里的信息
	virtual void	ViewFriendMsg();

	//设置好友对自己的好友度数值
	virtual void	SetRelationNum(const UID & uidFriend, UINT32 uRelationNum);

	//同步好友对自己的好友数据
	virtual void	SynFriendInfo(const UID & uidFriend, enFriendType FriendType, UINT8 RelationNum);

	//造访好友
	virtual void	VisitFriend(const UID & uidFriend);

	virtual void	OnEvent(XEventData & EventData);

	//结束拜访好友
	virtual void	EndVisitFriend();

	//查看同城的在线玩家
	virtual void	ViewOneCityOnlineUser();

	//得到好友名字
	virtual const char * GetFriendName(UID uidFriend);

	//得到好友信息
	virtual const SFriendBasicInfo * GetFriendInfo(UID uidFriend);

	//邮件点击选择收信人按钮
	virtual void  ClickMailFriend();

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing();

	//查看人物信息
	virtual void ViewUserInfo(UID uidUser);

	//离开查看其它玩家人物信息界面
	virtual void LeaveViewUserInfo(UID uidUser);

	//删除好友信息
	virtual void DeleteFriendMsg(UID uidUser);


private:
	//检测好友数据是否初始化过
	bool	IsInitFriendData();

	//把初始化过的标志设成true
	void	SetInitTure();

	//从好友信息列表中删除
	void	__DeleteFriendMsgList(const UID & uidSrcUser);

	//访问好友处理流程
	bool	BeginVisitFriend(IActor * pFriend);

	//加载玩家的好友信息
	void	HandleLoadFriendInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//加好友时的回调，用于调整双方的好友度相同
	void	HandleAddFriendBack(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//增加好友度(包括减少),的数据库回调
	void	HandleAddRelationNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	OnEvent_VisitFriend(UID uidFriend);

	void	OnEvent_ViewUserInfo(UID uidUser);

	void	SendUserInfo(IActor * pTargetActor);

	void	__InsertFriendEvent_ToDB(const UID & uidUser, const UID & uidFriend, UINT32 uAddRelationNum, const char * szDescript);

	//取消拜访,bBackMainScene为是否退回主场景
	void	CancelVisitFriend(bool bBackMainScene = true);

private:
	IActor*		 m_pActor;

	SFriendData	 m_FriendData;

	INT16		 m_FriendInitRelationNum;	//加好友时的初始好友值 

	INT16		 m_MaxFriendNum;			//最多有几个好友

	INT16		 m_MaxFriendMsgNum;			//最多有几条好友信息(此信息为好友的信息标签中的信息)

	INT32		 m_FriendEnventRecordTime;	//好友度改变事件记录保存时间(单位秒,目前是一天)

	bool		 m_bLoadData;				//数据是否已经加载过

	UID			 m_uidVisitFriend;			//要拜访的好友UID

	UID			 m_uidViewUserInfo;			//查看人物信息
};


#endif
