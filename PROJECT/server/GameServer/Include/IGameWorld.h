
#ifndef __THINGSERVER_IGAMEWORLD_H__
#define __THINGSERVER_IGAMEWORLD_H__

#include "IThing.h"
#include "DSystem.h"
#include "DBProtocol.h"
#include "IMailPart.h"
#include "GameFrameCmd.h"

struct IActor;
struct ISession;
struct IGameScene;
struct IMonster;
struct IBuilding;
struct IGoods;
struct IPortal;
class ThingContainer;
struct IEquipment;
struct ITalisman;
struct IGodSword;
struct IVisitWorldThing;

//游戏对象创建标志
#define THING_CREATE_FLAG_NULL			0x0
#define THING_CREATE_FLAG_DB_DATA		0x01	//为TRUE表示是使用DB保存的数据创建物品，否则表示是使用私有数据

//新的UID设计
//1.全世界唯一

#define CLIENT_USE_UID_SERVERID INVALID_SERVER_ID			//客户端产生UID的场景服ID,服务器不会用这个ID
#define CLIENT_USE_UID_WORLDID  0x1FF		//客户端产生UID的世界ID,服务器不会用这个ID



template<>
struct std::hash<UID>
{
	UINT32 operator()(const UID & uid) const
	{
		//取m_passTime的低10位与m_objectSN合成一个32位比较不集中的key
		return (UINT32)(((uid.m_passTime & 0x3FF) << 22) | uid.m_objectSN); 
	}
};


struct SDB_Get_ActorBasicData_Rsp;
struct SDB_Get_BuildingData_Rsp;
struct SDB_Get_FuMoDongData_Rsp;
struct SDB_Get_TrainingHallData_Rsp;
struct SDB_Get_GatherGodHouseData_Rsp; 
struct SDB_Get_ActorBasicData_Rsp;
struct SDB_Get_XiuLianData_Rsp;
struct SDBFuBenData;


//资源part数据
struct SBuildingPart
{
	SBuildingPart()
	{
		m_pData = 0;
		m_Len = 0;
	}
	char * m_pData;
	INT32  m_Len;
};

//状态数据
struct SStatusPart
{
	SStatusPart(){
		m_pData = 0;
		m_Len	= 0;
	}
	char * m_pData;
	INT32  m_Len;
};

//任务数据
struct STaskPart
{
	STaskPart(){
		m_pData = 0;
		m_Len = 0;
		m_bNewUser = false;
	}
	char * m_pData;
	INT32  m_Len;
	bool   m_bNewUser;
};

//成就数据
struct SChengJiuDBData
{
	SChengJiuDBData() : m_pData(0),m_Len(0)
	{
	}

	char * m_pData;
	INT32  m_Len;
};

//冷却时间
struct SCDTimerDBData
{
	SCDTimerDBData(){
		m_pData = 0;
		m_Len   = 0;
	}
	
	char * m_pData;
	INT32  m_Len;
};

//创建招募角色数据结构
struct SBuild_Employee
{
	SBuild_Employee() : m_pActorBasicData(0),m_pEquipPanel(0),m_pMagicPanelData(0) 
	{ 
		m_StatusPart.m_Len = 0;  
		m_StatusPart.m_pData = 0;
		m_pCombatData = 0;
	}

	~SBuild_Employee() { this->Release(); }

	SDB_Get_ActorBasicData_Rsp	   * m_pActorBasicData;							//玩家的基本数据
	SDBEquipPanel				   * m_pEquipPanel;								//装备栏数据
	SDBMagicPanelData			   * m_pMagicPanelData;							//法术栏数据
	SStatusPart					   m_StatusPart;								//状态数据
	SDBCombatData				   * m_pCombatData;								//战斗数据
	SCDTimerDBData				   m_CDTimerDBData;								//冷却时间数据

	void Release()
	{
		if( 0 != m_pActorBasicData){
			delete m_pActorBasicData;
			m_pActorBasicData = 0;
		}

		if( 0 != m_pEquipPanel){
			delete m_pEquipPanel;
			m_pEquipPanel = 0;
		}

		if( 0 != m_pMagicPanelData){
			delete m_pMagicPanelData;
			m_pMagicPanelData = 0;
		}

		if( 0 != m_StatusPart.m_pData){
			delete [] m_StatusPart.m_pData;
			m_StatusPart.m_pData = 0;
		}

		if( 0 != m_pCombatData){
			delete m_pCombatData;
			m_pCombatData = 0;
		}

		if( 0 != m_CDTimerDBData.m_pData){
			delete [] m_CDTimerDBData.m_pData;
			m_CDTimerDBData.m_pData = 0;
		}
	}

	bool IsOK() const
	{
		if( 0 != m_pActorBasicData && 0 != m_pEquipPanel && 0 != m_pMagicPanelData && 0 != m_StatusPart.m_pData && 0 != m_pCombatData && 0 != m_CDTimerDBData.m_pData){
			return true;
		}

		return false;
	}
};

//创建玩家的数据结构
struct SBuild_Actor
{

	enIdentityStatus   m_IdentityStatus; //认证状态
	UINT32             m_OnlineTime; //在线时长

	ISession	                   * m_pSession;
	char                           *m_pBuildingData;								//资源产出建筑	
	INT32                           m_BuildingLen;
	SDB_Get_FuMoDongData_Rsp	   * m_pFuMoDongData;								//伏魔洞数据
	SDB_Get_TrainingHallData_Rsp   * m_pTrainingHall;								//练功堂数据
	SDB_Get_GatherGodHouseData_Rsp * m_pGatherGodHouseData;							//聚仙楼数据				  
	char						   * m_pXiuLianData;								//修炼数据
	INT32							m_XiuLianDataSize;								//修炼数据长度
	SDB_Get_ActorBasicData_Rsp	   * m_pActorBasicData;								//玩家的基本数据
	SDBFuBenData				   * m_pFuBenData;									//副本数据
	SDB_Get_GodSwordShopData_Rsp   * m_pGodSwordShopData;							//剑冢数据
	SDBPacketData				   m_PacketData;									//背包数据
	SDBEquipPanel				   * m_pEquipPanel;									//装备栏数据
	SDBMagicPanelData			   * m_pMagicPanelData;								//法术栏数据
	SDBSynMagicPanelData		   * m_pSynMagicPanelData;							//帮派技能数据
	SDBCombatData				   * m_pCombatData;									//战斗数据
	SStatusPart					   m_StatusPart;									//状态数据
	SDB_Get_DouFaPartInfo_Rsp	   * m_pDouFaPart;									//斗法数据
	STaskPart					   m_TaskPart;										//任务数据
	SChengJiuDBData				   m_ChengJiuData;									//成就数据
	char                           *m_pActivityData;                                //活动数据
	INT32                           m_ActivityDataLen;                              //活动数据长度
	SCDTimerDBData				   m_CDTimerDBData;									//冷却时间数据

	SDB_SynPart					   * m_pSynPart;									//帮派PART数据

	SDB_TalismanPart			   * m_pTalismanPart;								//法宝PART数据

	bool                           m_bInitCreate;									//是否是初次创建的角色，如是，需要赠送装备及背包物品



	UINT8							 m_nEmployNum;									//拥有招募角色的数量
	SBuild_Employee				   * m_EmployBuildData[MAX_EMPLOY_NUM];				//招募角色的创建数据

	bool							m_bUserBuildOk;									//玩家角色是否创建好了
	bool							m_bEmployeeOk;									//招募角色是否创建好了

	SBuild_Actor()
	{
		memset(this, 0, sizeof(*this));
	}

	void Release()
	{
	
		if( m_pFuMoDongData){
			delete m_pFuMoDongData;
			m_pFuMoDongData = 0;
		}
		if( m_pTrainingHall){
			delete m_pTrainingHall;
			m_pTrainingHall = 0;
		}
		if( m_pGatherGodHouseData){
			delete m_pGatherGodHouseData;
			m_pGatherGodHouseData = 0;
		}
		if( m_pXiuLianData){
			delete [] m_pXiuLianData;
			m_pXiuLianData = 0;
		}
		if( m_pActorBasicData){
			delete m_pActorBasicData;
			m_pActorBasicData = 0;
		}
		if( m_pFuBenData){
			delete m_pFuBenData;
			m_pFuBenData = 0;
		}
		if( m_pGodSwordShopData){
			delete m_pGodSwordShopData;
			m_pGodSwordShopData = 0;
		}
		if( m_PacketData.m_pData){
			delete [] m_PacketData.m_pData;
			m_PacketData.m_pData = 0;
		}
		if( m_pEquipPanel){
			delete m_pEquipPanel;
			m_pEquipPanel = 0;
		}
		if( m_pMagicPanelData){
			delete m_pMagicPanelData;
			m_pMagicPanelData = 0;
		}
		if( m_pSynMagicPanelData){
			delete m_pSynMagicPanelData;
			m_pSynMagicPanelData = 0;
		}
		if( m_pCombatData){
			delete m_pCombatData;
			m_pCombatData = 0;
		}
		if( m_StatusPart.m_pData){
			delete [] m_StatusPart.m_pData;
			m_StatusPart.m_pData = 0;
		}

		if( m_pDouFaPart){
			delete m_pDouFaPart;
			m_pDouFaPart = 0;
		}

		if( m_TaskPart.m_pData){
			delete [] m_TaskPart.m_pData;
			m_TaskPart.m_pData = 0;
		}

		if( m_ChengJiuData.m_pData){
			delete [] m_ChengJiuData.m_pData;
			m_ChengJiuData.m_pData = 0;
		}

		if(m_pBuildingData)
		{
			delete [] m_pBuildingData;
			m_pBuildingData = 0;
		}

		if(m_pActivityData)
		{
			delete [] m_pActivityData;
			m_pActivityData = 0;
		}

		if( 0 != m_CDTimerDBData.m_pData){
			delete [] m_CDTimerDBData.m_pData;
			m_CDTimerDBData.m_pData = 0;
		}

		if ( 0 != m_pSynPart ){
			delete m_pSynPart;
			m_pSynPart = 0;
		}

		if ( 0 != m_pTalismanPart ){
			delete m_pTalismanPart;
			m_pTalismanPart = 0;
		}
		
		for( int k = 0; k < m_nEmployNum && k < MAX_EMPLOY_NUM; ++k)
		{
			delete m_EmployBuildData[k];
			m_EmployBuildData[k] = 0;
		}
	}

	~SBuild_Actor()
	{
		 Release();
	}

	bool IsOK()
	{
		if( 0 == m_pActorBasicData ||  0 == m_pCombatData || 0 == m_pEquipPanel || 0 == m_pBuildingData ||
			0 == m_pFuBenData || 0 == m_pFuMoDongData || 0 == m_pGatherGodHouseData || 0 == m_pGodSwordShopData ||
			0 == m_pMagicPanelData || 0 == m_PacketData.m_pData || 0 == m_pSynMagicPanelData || 0 == m_pTrainingHall || 
			0 == m_pXiuLianData || 0 == m_StatusPart.m_pData || 0 == m_pDouFaPart || 0 == m_TaskPart.m_pData ||
			0 == m_ChengJiuData.m_pData ||	0 == m_pActivityData || 0 == m_CDTimerDBData.m_pData || 0 == m_pSynPart || 0 == m_pTalismanPart)
		{
			return false;
		}


		for( int k = 0; k < m_nEmployNum && k < MAX_EMPLOY_NUM; ++k)
		{
			if(m_EmployBuildData[k]==0 || m_EmployBuildData[k]->IsOK() == false)
			{
				return false;
			}
		}

		return true;
	}
};


// 创建怪物的数据结构
struct SBuild_Monster
{
	const char *		pPrivateData;							// 私有数据
	UINT32				nPrivateLen;							// 私有数据长度
	
	SBuild_Monster()
	{
		memset(this, 0, sizeof(*this));		
	}
};

//创建怪物的输入结构
struct SCreateMonsterContext
{
	SCreateMonsterContext()
	{
		MEM_ZERO(this);
	}

	UID								uid;					//UID
	TMonsterID						MonsterID;				//怪物ID
	char							nDir;					//方向
	TSceneID                        SceneID;                //场景
	XPoint							ptLoc;					//位置
	UINT8                           m_nLineup;              //阵营
	TMapID                          m_CombatIndex;          //普通战斗索引
	UINT16                          m_DropID;               //普通掉落ID
	UINT16							m_DropIDHard;			//困难掉落ID
	UINT32							m_CombatIndexHard;		//困难战斗索引
	UINT16							m_DropIDTeam;			//组队掉落ID
	UINT32							m_CombatIndexTeam;		//组队战斗索引
	UINT8							m_bCanReplace;			//是否可被替换

};

//创建角色(非玩家)的输入结构
struct SCreateActorContext
{
	TActorID						ActorID;								//角色ID
	TLevel							nLevel;									//等级
	char							nDir;									//方向
	TSceneID						SceneID;								//场景
	XPoint							ptLoc;									//位置
	UID								uidMaster;								//主人ID

};

//创建建筑输入结构
struct SCreateBuildingContext
{
	UINT8          m_BuildingType;       //建筑类型

	UID           m_uidOwner; //建筑所属的主人
	TInt8Array<THING_NAME_LEN>  m_szOwnerName;  //建筑所属主人的姓名
	TSceneID                        SceneID;                                //场景

};

//创建物品
struct SCreateGoodsContext
{
	SCreateGoodsContext()
	{
		MEM_ZERO(this);
	}
	TGoodsID    m_GoodsID;   //GoodsID
	UINT8       m_Number;     //数量
	bool        m_Binded;    //是否已绑定
};

//创建传送门
struct SCreatePortalContext
{
	TPortalID          m_PortalID;
	TSceneID           m_SceneID;        //所在场景
	UINT16             m_ptX;           //X坐标
	UINT16             m_ptY;           //Y坐标
};

struct IGameWorld
{
	virtual void Release() = 0;

	virtual bool Create() = 0;

	///根据UID，查询IThing
    virtual	IThing	*	GetThing(const UID & uid)=0;

    //在游戏世界里创建一个Thing,
    virtual	IThing*				CreateThing(enThing_Class enThingClass,TSceneID SceneID, const char * buf, int len, UINT32 flag=0)=0;
    //从游戏世界里删除Thing
    virtual	void				DestroyThing(const UID & uid)=0;

	//创建怪物
	virtual IMonster *  CreateMonster(SCreateMonsterContext & MonsterCnt)=0;

	//创建物品
	virtual IGoods * CreateGoods(const SCreateGoodsContext & GoodsCnt ) = 0;

	virtual std::vector<IGoods *> CreateGoods(TGoodsID GoodsID, UINT32 nNum, bool bBinded) = 0;

	//创建传送门
	virtual IPortal * CreatePortal(const SCreatePortalContext & PortalCnt) = 0;


	//通过UID取得角色
	virtual IActor*				FindActor(const UID& uid) = 0;

	virtual IMonster *          FindMonster(const UID& uid) = 0;

	  //查询游戏场景
    virtual	IGameScene *		GetGameScene(const TSceneID & sid)=0;

	//mapid 地图ID, CreateNpcIndex从配置文件中该索引位置开始，创建怪物
	virtual IGameScene*      	CreateGameSceneByMapID(TMapID mapid,INT32 CreateNpcIndex=0, bool bDelNoneUser = false) = 0;

	virtual IGameScene*      	CreateGameSceneBySceneID(TSceneID sceneid,INT32 CreateNpcIndex=0, bool bDelNoneUser = false) = 0;

	//获得主城场景ID
	virtual TSceneID GetMainSceneID() = 0;

	//创建建筑
	virtual IBuilding * CreateBuilding(SCreateBuildingContext & BuildingCnt)=0;

	virtual IGoods *    GetGoods(UID uidGoods) = 0;

	virtual IEquipment *    GetEquipment(UID uidGoods) = 0;

	virtual IGodSword * GetGodSword(UID uidGoods) = 0;

	virtual ITalisman * GetTalisman(UID uidGoods) = 0;

	//创建招募角色
	virtual IActor *    CreateEmploy(const SBuild_Employee & Build_Employee) = 0;

	//创建招募角色
	virtual IActor *	CreateEmploy(TEmployeeID EmployeeID, IActor * pActor) = 0;

	//写系统邮件		RecvUserUID为收件人的UID
	virtual void		WriteSystemMail(const SWriteSystemData & SystemMail) = 0;

	//写系统邮件
	virtual void		WriteSystemMail(const SWriteSystemData & SystemMail, std::vector<IGoods *> & vecGoods) = 0;

	//访问世界指定类事物
	virtual void		VisitWorldThing(enThing_Class cls, IVisitWorldThing & VisitThing) = 0;

	//全服公告
	virtual void		WorldNotice(IActor * pActor, const char * pszNoticeContext) = 0;

	//世界频道的系统消息 (pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
	virtual void		WorldSystemMsg(const char * pszMsgContext, IActor * pActor = 0, enTalkMsgType enMsgType = enTalkMsgType_System) = 0;

	//删除场景
	virtual bool		DeleteGameScene(IGameScene* pGameScene) = 0;

	//删除场景
	virtual bool		DeleteGameScene(TSceneID SceneID) = 0;

	//弹出框提示
	virtual void		TipPopUpBox(IActor * pActor, enGameWorldRetCode GameWorldRetCode) = 0;

	//通过UserID获取人物
	virtual IActor *	GetUserByUserID(TUserID UserID) = 0;

	//给所有玩家写系统邮件
	virtual void		WriteAllUserSysMail(const SWriteSystemData & SystemMail) = 0;

	//记录仙石使用日志
	virtual void		Save_GodStoneLog(TUserID UserID,INT32 Num,INT32 Aftergodstone,char * pszDesc,enGodStoneChanel chanel = enGodStoneChanel_Game,enGodStoneType type = enGodStoneType_Use) = 0;

	//获取语言字串
	virtual const char * GetLanguageStr(INT32 LanguageID) = 0;	//fly add 20121106

	//收入无效场景中，等待回收
	virtual void		Push_InvalidScene(TSceneID SceneID) = 0;
};



#endif
