#ifndef __THINGSERVER_TEAMPART_H__
#define __THINGSERVER_TEAMPART_H__

#include "ITeamPart.h"

class TeamPart : public ITeamPart
{
public:
	TeamPart();

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
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

public:
	//当前是否有组队
	virtual	bool	IsHaveTeam();

	//得到队友
	virtual IActor *GetTeamMember();

	//得到队长
	virtual IActor *GetTeamLeader();

	//邀请组队
	virtual bool	InviteTeam(UID uidUser, enCreateTeamType type);

	//同意组队
	virtual void	AgreeTeam(UID uidUser);

	//拒绝组队
	virtual void	RefuseTeam(UID uidUser);

	//退出队伍
	virtual bool	QuitTeam();

	//创建队伍
	virtual UINT8	CreateTeam(UID uidUser, bool bTeamLeader);

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing();

	//设置是否在快速组队
	virtual void	SetIsInFastTeam(bool bInFastTeam);

	//得到是否在快速组队
	virtual bool	GetIsInFastTeam();

public:

	virtual void	SetTeamData(UID uidUser, bool bLeader);

	virtual void	ClearTeamData(bool bMeQuit, bool bSendMsg = true);

private:
	void			__QuitTeam();

	//检测是否能组队
	UINT8			__CheckCreateTeam(UID uidUser);

	//创建队伍
	void			__CreateTeam(IActor * pFriend, bool bLeader);

	//取消等待
	void			__QuitWait();

	//设置默认队伍阵形
	bool			__SetDefaultTeamLineup(UID uidLeader, UID uidMember);

private:
	IActor * m_pActor;
	
	UID		 m_TeamMember;

	bool	 m_bTeamLeader; //是否是队长

	bool	 m_bInFastTeam;	//是否在快速组队中
};


#endif
