
#ifndef __THINGSERVER_STATUSPART_H__
#define __THINGSERVER_STATUSPART_H__

#include <hash_map>
#include "IStatusPart.h"
#include <list>
#include "DBProtocol.h"
#include "IConfigServer.h"

struct IActor;
struct Status;
struct IEffect;
struct ICreature;

//状态组状态个数满时的处理方式
enum enFullCalType
{	
	enFullCalType_ReplaceMinPriority,	//替换低优先级的那个效果
	enFullCalType_ReplaceMinEndTime,	//替换结束时间最早的那个效果
	enFullCalType_NoReplace,			//不替换
};	


class StatusPart : public IStatusPart
{

public:
	StatusPart();
	virtual ~StatusPart();

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
	//增加一个状态
	virtual bool AddStatus(TStatusID lID, UID uidCreator,std::vector<UINT8> & vectDeleteStatusType, TMagicID MagicID = INVALID_MAGIC_ID,INT32 StatusRoundNum=0);

	//增加一个状态,通过使用物品得到的   bOk是否是点击确认使用
	virtual bool AddStatus_UseGoods(TStatusID lID, UID uidCreator, SAddStatus & AddStatus);

	//删除状态
	virtual bool RemoveStatus(TStatusID lStatusID, UID uidCreator = UID());

	//根据状态ID进行查询
	virtual IStatus* FindStatus(TStatusID lStatusID);

	//效果控制接口！直接给生物加效果，生物销毁的时候会自动销毁效果，外部不要控制效果的生命周期！！！！
	virtual	bool	AddEffect(TEffectID effID);

	virtual	void	RemoveEffect(TEffectID effID);

		//获得生物已有状态中优先级最高的一个状态类型
	virtual const SStatusTypeCnfg * GetMaxPriorityStatusTypeCnfg();

	//打开人物面板，显示状态
	virtual void	ShowStatusOpenUserPanel();

	//检测是否要通知客户端取消效果公告
	virtual void Check_CancelViewEffectMsg(TStatusID lStatusID);

	//检测是否要显示公告，要的话并显示
	virtual void Check_ViewEffectMsg(TStatusID lStatusID);

	//得到对所有角色共用的状态
	virtual std::vector<StatusShowInfo> GetAllActorStatus();

	//使用已有的状态
	virtual void UseAlreadyHaveStatus(IStatus * pStatus);

private:
	//从数据库获取状态数据创建
	bool __CreateStatus(SDB_Get_StatusData_Rsp Rsp);

	//按组处理(物品使用)    bOk是否是点击确认使用
	bool __GroupHandle_UseGoods(const SStatusCnfg* pStatusInfo, UID uidCreator, SAddStatus & AddStatus);

	//给玩家加上一种新状态
	bool __AddNewStatus(const SStatusCnfg* pStatusInfo, UID uidCreator,TMagicID MagicID,INT32 StatusRoundNum=0 );

	//发送同步状态消息
	void __SendSycStatus(IStatus * pStatus);

	//发送增加玩家状态消息
	void __SendAddStatus(IStatus * pStatus);

	bool __StartStatus(IStatus * pStatus,UID uidCreator,TMagicID MagicID,INT32  StatusRoundNum);

	//得到相同状态组的状态
	void __GetSameGroupStatus(const std::vector<TStatusGroupID> & vectGroupID, std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus);

	//组处理
	bool __CalStatusGroup(const SStatusCnfg* pStatusInfo, std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus, bool & bCreateNewStatus, std::vector<UINT8> & vectDeleteStatusType);

	//得到组中最低优先级的状态下标
	int  __GetMinPriorityStatusGroup(const std::vector<IStatus *> & vectStatus);

	//删除状态组中的状态
	void __DeleteStatus_Group(std::map<TStatusGroupID, std::vector<IStatus *>> & mapGroupStatus, IStatus * pStatus);

private:
	typedef std::hash_map<TStatusID, IStatus*>			StatusMap;

	StatusMap	m_status;						//所有状态 

	typedef		std::list<IEffect*>	EffectList;	//不通过状态而直接增加的效果！！
	EffectList	m_effectList;

	ICreature * m_pActor;

};




#endif

