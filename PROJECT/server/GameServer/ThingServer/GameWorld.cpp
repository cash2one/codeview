

#include "GameWorld.h"
#include "ThingServer.h"
#include "IGameServer.h"
#include "IConfigServer.h"
#include "Building.h"
#include "Goods.h"
#include "Equipment.h"
#include "XDateTime.h"
#include "GodSword.h"
#include "Talisman.h"
#include "CommonGoods.h"
#include "Portal.h"
#include "ITalk.h"
#include "EquipPart.h"
#include "MagicPart.h"
#include "StatusPart.h"
#include "CombatPart.h"
#include "ActorBasicPart.h"
#include "CDTimerPart.h"
#include "AddAllUserMailNum.h"
#include <vector>


GameWorld::GameWorld()
{
}

GameWorld::~GameWorld()
{
}

void GameWorld::Release()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_GameWorld,this);

	delete this;
}

 bool GameWorld::Create()
 {
	 UniqueIDGeneratorService::Init(g_pGameServer->GetServerID(),g_pGameServer->GetGameWorldID());

	 g_pGameServer->GetTimeAxis()->SetTimer(enGameWorldTimer_DestroyScene,this,60 * 1000,"GameWorld::Create");

	 return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_GameWorld,this);;
 }

 //收到MSG_ROOT消息
 void GameWorld::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib) 
 {
	 	 typedef  void (GameWorld::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enGameWorldCmd_Max]=
	 {
		0,
		0,
		0,
		0,
		0,
		0,		
		& GameWorld::OnEnterPortal,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		& GameWorld::UnLoadEmployee,
	 };

	 if(nCmd>=enGameWorldCmd_Max || 0==s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
 }

 void GameWorld::OnEnterPortal(IActor *pActor,UINT8 nCmd, IBuffer & ib)
 {
	 CS_EnterPortal_Req Req;

	 ib >> Req;

	 if(ib.Error())
	{
		TRACE("<error> %s : %d Line 打开建筑请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	 IThing * pThing = this->GetThing(Req.m_uidPortal);
	 if(pThing == 0)
	 {
		 //队长把队员拉回摆怪场景时，会删除战斗场景的传送门，这时会找不到场景
		 SC_EnterPortal_Rsp Rsp;
		 Rsp.m_Result = enGameWorldRetCode_ErrNoPortal;

		 OBuffer1k ob;
		 ob << GameWorldHeader(enGameWorldCmd_EnterPortal,sizeof(Rsp)) << Rsp;
		 pActor->SendData(ob.TakeOsb());
		 return ;
	 }

	 if(pThing->GetThingClass() != enThing_Class_Portal)
	 {
		  TRACE("<error> %s : %d Line 类型错误，该Thing不是传送门!", __FUNCTION__,__LINE__);
		 return ;
	 }

	 IPortal * pPortal = (IPortal *)pThing;

	 pPortal->OnClicked(pActor);

 }

//解雇招募角色
void GameWorld::UnLoadEmployee(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_UnLoadEmployee_Req UnLoadEmployee_Req;
	ib >> UnLoadEmployee_Req;
	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	SC_UnLoadEmployee_Rsp UnLoadEmployee_Rsp;

	pActor->UnLoadEmployee(UnLoadEmployee_Req.m_EmployeeUid, UnLoadEmployee_Rsp.UnLoadEmployeeRetCode);

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_UnLoadEmployee,sizeof(UnLoadEmployee_Rsp)) << UnLoadEmployee_Rsp ;
	pActor->SendData(ob.TakeOsb());
}

 //IDBProxyClientSink接口
void GameWorld::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	}

	switch(ReqCmd)
	{
		case enDBCmd_InsertMailInfo:
			{
				HandleWriteSystemMail(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}
}

///根据UID，查询IThing
IThing	*	GameWorld::GetThing(const UID & uid)
{
	return m_ThingContainer.GetThing(uid);
}

IGoods *    GameWorld::GetGoods(UID uidGoods)
{
	IThing * pThing = GetThing(uidGoods);
	if(pThing==0 || pThing->GetThingClass() != enThing_Class_Goods)
	{
		return 0;
	}
	return (IGoods*)pThing;
}

IEquipment *     GameWorld::GetEquipment(UID uidGoods)
{
	IGoods * pGoods = GetGoods(uidGoods);
	if(pGoods==0 || pGoods->GetGoodsClass() != enGoodsCategory_Equip)
	{
		return 0;
	}
	return (IEquipment*)pGoods;
}


 IGodSword *  GameWorld::GetGodSword(UID uidGoods)
 {
	IGoods * pGoods = GetGoods(uidGoods);
	if(pGoods==0 || pGoods->GetGoodsClass() != enGoodsCategory_GodSword)
	{
		return 0;
	}
	return (IGodSword*)pGoods;
 }

ITalisman * GameWorld::GetTalisman(UID uidGoods)
{
	IGoods * pGoods = GetGoods(uidGoods);
	if(pGoods==0 || pGoods->GetGoodsClass() != enGoodsCategory_Talisman)
	{
		return 0;
	}
	return (ITalisman*)pGoods;
}

//创建招募角色
IActor *  GameWorld::CreateEmploy(const SBuild_Employee & Build_Employee)
{
	if( !Build_Employee.IsOK()){
		return 0;
	}

	if( !UID(Build_Employee.m_pActorBasicData->uid).IsValid()){
		TRACE("<error> %s : %d 行　创建招募角色失败,UID无效值", __FUNCTION__,__LINE__);
		return 0;
	}

	IActor * pMaster = g_pGameServer->GetGameWorld()->FindActor(UID(Build_Employee.m_pActorBasicData->uidMaster));
	if( 0 == pMaster){
		TRACE("<error> %s : %d 行 创建招募角色，主角获取失败", __FUNCTION__,__LINE__);
		return 0;
	}

	Actor * pEmployee = new Actor;
	if (NULL == pEmployee){
		return 0;
	}

	if ( !pEmployee->OnSetDBContext(Build_Employee.m_pActorBasicData, SIZE_OF(SDB_Get_ActorBasicData_Rsp())) ){		
		return 0;
	}

	if ( !pEmployee->Create()){	
		return NULL;
	}

	pEmployee->RecalculateProp(false);	//计算属性

	TSceneID SecenID = pMaster->GetCrtProp(enCrtProp_ActorMainSceneID);

	pEmployee->SetCrtProp(enCrtProp_ActorMainSceneID,SecenID.m_id);
	pEmployee->SetCrtProp(enCrtProp_SceneID, SecenID.m_id);
	pEmployee->SetCrtProp(enCrtProp_ActorLastSceneID, SecenID.m_id);

	//基本
	IThingPart*  pThingPart = new ActorBasicPart();
	if( pThingPart->Create(pEmployee, 0, 0) == false)
	{
		delete pEmployee;
		return 0;
	}
	pEmployee->AddPart(pThingPart);

	
	//冷却时间
	pThingPart = new CDTimerPart();
	if( pThingPart->Create(pEmployee, (void *)&Build_Employee.m_CDTimerDBData, Build_Employee.m_CDTimerDBData.m_Len) == false){
		delete pEmployee;
		return 0;
	}
	pEmployee->AddPart(pThingPart);


	//状态part
	pThingPart = new StatusPart();
	if(pThingPart->Create(pEmployee,(void *)&Build_Employee.m_StatusPart,Build_Employee.m_StatusPart.m_Len) == false)
	{
		delete pEmployee;
		return 0;
	}
	pEmployee->AddPart(pThingPart);

	//装备
	pThingPart = new EquipPart();
	if( pThingPart->Create(pEmployee, Build_Employee.m_pEquipPanel, sizeof(SDBEquipPanel)) == false){
		delete pEmployee;
		return 0;	
	}
	pEmployee->AddPart(pThingPart);



	//法术栏
	pThingPart = new MagicPart();
	if( pThingPart->Create(pEmployee, Build_Employee.m_pMagicPanelData, sizeof(SDBMagicPanelData)) == false){
		delete pEmployee;
		return 0;	
	}
	pEmployee->AddPart(pThingPart);
	
	//战斗
	pThingPart = new CombatPart();
	if( pThingPart->Create(pEmployee, Build_Employee.m_pCombatData, sizeof(SDBCombatData)) == false){
		delete pEmployee;
		return 0;
	}
	pEmployee->AddPart(pThingPart);

	pEmployee->RecalculateProp(false);	//计算属性

	m_ThingContainer.AddThing(pEmployee);

	//发数据给客户端
	pEmployee->NoticClientCreatePrivateThing(pEmployee->GetUID());
		
	for(int k = enThingPart_Crt_Basic; k < enThingPart_Crt_Max; ++k)
	{
		if(k == enThingPart_Actor_Packet)
		{
			continue;  //雇佣用人员和主角共用背包，所以不需要再发
		}
		IThingPart* pThingPart = pEmployee->GetPart((enThingPart)k);
		if(pThingPart){
			pThingPart->InitPrivateClient();
		}
	}

	return pEmployee;
}


IMonster *  GameWorld::FindMonster(const UID& uid)
{
	IThing * pThing = GetThing(uid);
	if(pThing==0 || pThing->GetThingClass() != enThing_Class_Monster)
	{
		return 0;
	}
	return (IMonster*)pThing;
}

//在游戏世界里创建一个Thing,
IThing*	GameWorld::CreateThing(enThing_Class enThingClass,TSceneID SceneID, const char * buf, int len, UINT32 flag)
{
	switch(enThingClass)
	{
	case enThing_Class_Actor :
		return CreateActor(SceneID,buf,len,flag);
		break;
	case enThing_Class_Monster:
		return CreateMonster(SceneID,buf,len,flag);
		break;
	case enThing_Class_Building:
		return CreateBuilding(SceneID,buf,len,flag);
		break;
	case enThing_Class_Goods:
		return CreateGoods(SceneID,buf,len,flag);
	case enThing_Class_Portal:
		return CreatePortal(SceneID,buf,len,flag);
	default:
		break;
	}

	return 0;
}


//写系统邮件		RecvUserUID为收件人的UID
void	GameWorld::WriteSystemMail(const SWriteSystemData & SystemMail)
{
	SDB_Insert_MailData_Req Req;

	Req.Mail_Type			  = enMailType_System;

	Req.uid_SendUser		  = UID().ToUint64();		//系统邮件用无效值 
	Req.Time				  = time(0);
	Req.Money				  = SystemMail.m_Money;
	Req.Stone				  = SystemMail.m_Stone;
	Req.Ticket				  = SystemMail.m_Ticket;
	Req.PolyNimbus			  = SystemMail.m_PolyNimbus;	//聚灵气
	Req.uid_User			  = SystemMail.m_DestUID.ToUint64();
	strncpy(Req.ThemeText, SystemMail.m_szThemeText, sizeof(Req.ThemeText));
	strncpy(Req.ContentText, SystemMail.m_szContentText, sizeof(Req.ContentText));

	Req.ThemeText[DESCRIPT_LEN_50 - 1] = '\0';
	Req.ContentText[DESCRIPT_LEN_300 - 1] = '\0';

	// fly add	20121106

	strncpy(Req.Name_SendUser, g_pGameServer->GetGameWorld()->GetLanguageStr(10009), sizeof(Req.Name_SendUser));
	//strncpy(Req.Name_SendUser, "系统", sizeof(Req.Name_SendUser));
	memcpy(&Req.GoodsData, SystemMail.m_SDBGoodsData, sizeof(SystemMail.m_SDBGoodsData));

	OBuffer1k ob;
	ob << Req;

	UINT32 UserID = Req.uid_User;

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(SystemMail.m_DestUID);
	if( 0 != pActor){
		UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);
	}

	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_InsertMailInfo, ob.TakeOsb(), this, 0);
}

//写系统邮件		RecvUserUID为收件人的UID
void	GameWorld::WriteSystemMail(const SWriteSystemData & SystemMail, std::vector<IGoods *> & vecGoods)
{
	SDB_Insert_MailData_Req Req;

	OBuffer1k ob;

	Req.Mail_Type			  = enMailType_System;

	Req.uid_SendUser		  = UID().ToUint64();		//系统邮件用无效值 
	Req.Time				  = time(0);
	Req.Money				  = SystemMail.m_Money;
	Req.Stone				  = SystemMail.m_Stone;
	Req.Ticket				  = SystemMail.m_Ticket;
	Req.PolyNimbus			  = SystemMail.m_PolyNimbus;	//聚灵气
	Req.uid_User			  = SystemMail.m_DestUID.ToUint64();
	strncpy(Req.ThemeText, SystemMail.m_szThemeText, sizeof(Req.ThemeText));
	strncpy(Req.ContentText, SystemMail.m_szContentText, sizeof(Req.ContentText));

	Req.ThemeText[DESCRIPT_LEN_50 - 1] = '\0';
	Req.ContentText[DESCRIPT_LEN_600 - 1] = '\0';

	strncpy(Req.Name_SendUser, g_pGameServer->GetGameWorld()->GetLanguageStr(10009), sizeof(Req.Name_SendUser));

	SDBGoodsData * pGoodsData = (SDBGoodsData *)&Req.GoodsData;

	int GoodsNum = 0;

	for ( int i = 0; i <  vecGoods.size() && GoodsNum < MAX_MAIL_GOODS_NUM; ++i )
	{
		IGoods * pGoods = vecGoods[i];

		if ( 0 == pGoods )
			continue;

		SDB_Save_GoodsReq GoodsDB;
		int nLen = sizeof(GoodsDB);

		pGoods->OnGetDBContext(&GoodsDB, nLen);

		pGoodsData->m_uidGoods = UID(GoodsDB.m_uidGoods);
		pGoodsData->m_GoodsID  = GoodsDB.m_GoodsID;
		pGoodsData->m_CreateTime = GoodsDB.m_CreateTime;
		pGoodsData->m_Number   = GoodsDB.m_Number;
		pGoodsData->m_Binded   = GoodsDB.m_Binded;

		memcpy(&pGoodsData->m_TalismanProp, &GoodsDB.GoodsData, sizeof(pGoodsData->m_TalismanProp));

		++GoodsNum;

		if ( MAX_MAIL_GOODS_NUM <= GoodsNum ){

			ob.Reset();
			ob << Req;
			g_pGameServer->GetDBProxyClient()->Request(Req.uid_User , enDBCmd_InsertMailInfo, ob.TakeOsb(), this, 0);

			memset(&Req.GoodsData, 0, sizeof(Req.GoodsData));
			pGoodsData = (SDBGoodsData *)&Req.GoodsData;

			GoodsNum = 0;

		} else {
			++pGoodsData;
		}

		//销毁物品
		g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
	}

	if ( GoodsNum > 0 || vecGoods.size() == 0 ){
		ob.Reset();
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(Req.uid_User , enDBCmd_InsertMailInfo, ob.TakeOsb(), this, 0);	
	}
}

//访问世界指定类事物
void	GameWorld::VisitWorldThing(enThing_Class cls, IVisitWorldThing & VisitThing)
{
	m_ThingContainer.Visit(cls, VisitThing);
}


//全服公告
void	GameWorld::WorldNotice(IActor * pActor, const char * pszNoticeContext)
{
	ITalk * pTalk = g_pGameServer->GetRelationServer()->GetTalk();
	if( 0 == pTalk){
		return;
	}

	pTalk->WorldNotice(pActor, pszNoticeContext);
}

//世界频道的系统消息 (pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
void	GameWorld::WorldSystemMsg(const char * pszMsgContext, IActor * pActor, enTalkMsgType enMsgType)
{
	ITalk * pTalk = g_pGameServer->GetRelationServer()->GetTalk();
	if( 0 == pTalk){
		return;
	}

	if(enMsgType == enTalkMsgType_UpLevel){
	
		pTalk->WorldSystemMsg(pszMsgContext, pActor,enMsgType);

	}else{

		pTalk->WorldSystemMsg(pszMsgContext, pActor);

	}
	
}

//删除场景
bool	GameWorld::DeleteGameScene(IGameScene* pGameScene)
{
	if( 0 == pGameScene){
		return false;
	}

	this->RemoveGameScene(pGameScene);

	delete pGameScene;
	
	return true;
}

//删除场景
bool	GameWorld::DeleteGameScene(TSceneID SceneID)
{
	if( !SceneID.IsValid()){
		return false;
	}

	GameScene * pGameScene = this->GetGameScene(SceneID);

	if( pGameScene){

		this->RemoveGameScene(pGameScene);

		delete pGameScene;
	}

	return true;
}

//弹出框提示
void	GameWorld::TipPopUpBox(IActor * pActor, enGameWorldRetCode GameWorldRetCode)
{
	SC_Tip_PopUpBox Tip_PopUpBox;

	Tip_PopUpBox.GameWorldRetCode = GameWorldRetCode;

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_TipBox,sizeof(Tip_PopUpBox)) << Tip_PopUpBox ;
	pActor->SendData(ob.TakeOsb());	
}

//通过UserID获取人物
IActor * GameWorld::GetUserByUserID(TUserID UserID)
{
	return m_ThingContainer.GetUserByUserID(UserID);
}

//给所有玩家写系统邮件
void	GameWorld::WriteAllUserSysMail(const SWriteSystemData & SystemMail)
{
	SDB_WriteAllUserSysMail_Req Req;

	Req.uid_SendUser = UID().ToUint64();

	// fly add	20121106

	strncpy(Req.Name_SendUser, g_pGameServer->GetGameWorld()->GetLanguageStr(10009), sizeof(Req.Name_SendUser));
	//strncpy(Req.Name_SendUser, "系统", sizeof(Req.Name_SendUser));
	Req.Mail_Type = enMailType_System;
	Req.Stone	  = SystemMail.m_Stone;
	Req.Money	  = SystemMail.m_Money;
	Req.Ticket	  = SystemMail.m_Ticket;
	Req.PolyNimbus	  = SystemMail.m_PolyNimbus;	//聚灵气
	strncpy(Req.ThemeText, SystemMail.m_szThemeText, sizeof(Req.ThemeText));
	strncpy(Req.ContentText, SystemMail.m_szContentText, sizeof(Req.ContentText));
	Req.Time	  = CURRENT_TIME();
	memcpy(&Req.GoodsData, &SystemMail.m_SDBGoodsData, sizeof(Req.GoodsData));

	UINT32 UserID = Req.Time;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Insert_AllUserSysMail, ob.TakeOsb(), 0, 0);

	//增加所有在线玩家的邮件数量
	AddAllUserMailNum AddMailNum;

	m_ThingContainer.Visit(enThing_Class_Actor, AddMailNum);
}

//记录仙石使用日志
void	GameWorld::Save_GodStoneLog(TUserID UserID,INT32 Num,INT32 Aftergodstone,char * pszDesc,enGodStoneChanel chanel,enGodStoneType type)
{
	SDB_Save_GodStoneLog_Req DBReq;

	DBReq.m_Aftergodstone = Aftergodstone;
	DBReq.m_Chanel = (UINT8)chanel;
	DBReq.m_ServerID = g_pGameServer->GetServerID();
	DBReq.m_Type	 = (UINT8)type;
	DBReq.m_UserID	 = UserID;
	DBReq.m_Vargodstone = Num;

	strncpy(DBReq.m_Description, pszDesc, sizeof(DBReq.m_Description));

	//DBReq.m_Description[DESCRIPT_LEN_300 - 1] = '\0';

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Save_GodStoneLog, ob.TakeOsb(), 0, 0);
}


IActor * GameWorld::CreateActor(TSceneID SceneID, const char * buf, int len, UINT32 flag)
{
	UID uid;
	if(THING_CREATE_FLAG_DB_DATA & flag)
	{
		uid = UniqueIDGeneratorService::GenerateUID(enThing_Class_Actor);
	}

	IGameScene * pGameScene = GetGameScene(SceneID);

	if(0==pGameScene)
	{
		//场景不存在，则创建
		  pGameScene = CreateGameSceneBySceneID(SceneID);
		  if(pGameScene == NULL)
		  {			
			return 0;
		  }
	}

	IActor * pActor = m_ActorBuilder.BuildActor(SceneID,uid,buf,len,flag);

	if(pActor==0)
	{
	    delete pGameScene;
		return false;
	}

	m_ThingContainer.AddThing(pActor);

	
	return pActor;

}

	//创建怪物
IMonster *   GameWorld::CreateMonster(SCreateMonsterContext & MonsterCnt)
{
	SMonsterPrivateData PrivateData;

	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MonsterCnt.MonsterID);
	if(pMonsterCnfg == 0)
	{
		TRACE("%s : %d Line 不能获取怪物配置信息 monsterid=%d",__FUNCTION__,__LINE__,MonsterCnt.MonsterID);
		return 0;
	}

	PrivateData.m_MonsterID = MonsterCnt.MonsterID;
	PrivateData.m_Avoid     = pMonsterCnfg->m_Avoid;
	PrivateData.m_Blood     = pMonsterCnfg->m_Blood;
	PrivateData.m_MonsterLayer     = 0;
	PrivateData.m_MonsterMagic     = pMonsterCnfg->m_MagicValue;
	PrivateData.m_MonsterSwordkee  = pMonsterCnfg->m_Swordkee;
	PrivateData.m_Level     = pMonsterCnfg->m_Level;
	PrivateData.m_Shield    = pMonsterCnfg->m_Shield;
	PrivateData.m_Spirit    = pMonsterCnfg->m_Spirit;

	if ( MonsterCnt.uid.IsValid() )
	{
		PrivateData.m_uid = MonsterCnt.uid;
	}
	else
	{
		PrivateData.m_uid     = UniqueIDGeneratorService::GenerateUID(enThing_Class_Monster);
	}

	// fly add	20121106
	strncpy(PrivateData.m_szName, GetLanguageStr(pMonsterCnfg->m_MonsterNameLangID),sizeof(PrivateData.m_szName));
	//strncpy(PrivateData.m_szName, pMonsterCnfg->m_szName,sizeof(PrivateData.m_szName));
	PrivateData.m_nDir = MonsterCnt.nDir;
	PrivateData.m_ptX = MonsterCnt.ptLoc.x;
	PrivateData.m_ptY = MonsterCnt.ptLoc.y;
	

	SBuild_Monster Build_Monster;
	Build_Monster.nPrivateLen = sizeof(SMonsterPrivateData);
	Build_Monster.pPrivateData = (char*)&PrivateData;

	IMonster * pMonster = CreateMonster(MonsterCnt.SceneID,(char*)&Build_Monster,sizeof(Build_Monster),THING_CREATE_FLAG_NULL);

		if(pMonster == 0)
		{
			TRACE("<error> %s : %d line 创建怪物失败 monsterid = %d!",__FUNCTION__,__LINE__,MonsterCnt.MonsterID);
		}
		else
		{
				pMonster->SetCrtProp(enCrtProp_MonsterLineup,MonsterCnt.m_nLineup);
				pMonster->SetCrtProp(enCrtProp_MonsterCombatIndex,MonsterCnt.m_CombatIndex);
				pMonster->SetCrtProp(enCrtProp_MonsterDropID,MonsterCnt.m_DropID);
				pMonster->SetCrtProp(enCrtProp_MonsterDropIDHard,MonsterCnt.m_DropIDHard);
				pMonster->SetCrtProp(enCrtProp_MonsterCombatIndexHard,MonsterCnt.m_CombatIndexHard);
				pMonster->SetCrtProp(enCrtProp_MonsterDropIDTeam,MonsterCnt.m_DropIDTeam);
				pMonster->SetCrtProp(enCrtProp_MonsterCombatIndexTeam,MonsterCnt.m_CombatIndexTeam);
				pMonster->SetCrtProp(enCrtProp_MonsterCanReplace,MonsterCnt.m_bCanReplace);
		}


	return pMonster;
}

IMonster * GameWorld::CreateMonster(TSceneID SceneID, const char * buf, int len, UINT32 flag)
{
	UID uid;
	if(!(THING_CREATE_FLAG_DB_DATA & flag))
	{
		uid = UniqueIDGeneratorService::GenerateUID(enThing_Class_Monster);
	}

	IMonster * pMonster = m_MonsterBuilder.BuildMonster(SceneID,uid,buf,len,flag);

	if(pMonster == 0)
	{		
		return 0;
	}

	m_ThingContainer.AddThing(pMonster);

	IGameScene * pGameScene = GetGameScene(SceneID);

	if(0!=pGameScene)
	{
		pGameScene->EnterScene(pMonster);
	}
		
	
	return pMonster;
}

//创建建筑
IBuilding *  GameWorld::CreateBuilding(SCreateBuildingContext & BuildingCnt)
{
	SBuildingPrivateData PrivateData ;

	PrivateData.m_BuildingType = BuildingCnt.m_BuildingType;
	PrivateData.m_uidOwner = BuildingCnt.m_uidOwner;
	strncpy(PrivateData.m_szOwnerName,BuildingCnt.m_szOwnerName,sizeof(PrivateData.m_szOwnerName));
	PrivateData.m_Uid = UniqueIDGeneratorService::GenerateUID(enThing_Class_Building);
	PrivateData.m_SceneID = BuildingCnt.SceneID;

	return CreateBuilding(BuildingCnt.SceneID,(char*)&PrivateData,sizeof(SBuildingPrivateData),THING_CREATE_FLAG_NULL);
}

//创建传送门
IPortal * GameWorld::CreatePortal(const SCreatePortalContext & PortalCnt)
{
	SPortalPrivateData PrivateData;
	PrivateData.m_uidPortal = UniqueIDGeneratorService::GenerateUID(enThing_Class_Portal);
	PrivateData.m_PortalID = PortalCnt.m_PortalID;
	PrivateData.m_SceneID = PortalCnt.m_SceneID;
	PrivateData.m_ptX = PortalCnt.m_ptX;
	PrivateData.m_ptY = PortalCnt.m_ptY;

	return CreatePortal( PortalCnt.m_SceneID,(char*)&PrivateData,sizeof(PrivateData),THING_CREATE_FLAG_NULL);
}

//创建建筑

IBuilding * GameWorld::CreateBuilding(enBuildingType BuildingType)
{
	if(BuildingType <ResOutputBuildingType_Max)
	{
		return new ResOutputBuilding();
	}
	else if(BuildingType <enBuildingType_Max)
	{
		return new Building();
	}

	return 0;
}

//写系统邮件的回调函数
void	GameWorld::HandleWriteSystemMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Insert_MailData_Rsp Rsp;
	RspIb >> RspHeader >> OutParam >> Rsp;

	if(RspIb.Error())
	{
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	OBuffer1k ob;

	if(enMailRetCode_WriteError == OutParam.retCode){
		//邮件入库失败
		return;
	}

	//增加邮件数量
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Rsp.uid_User));
	if( 0 != pActor){
		IMailPart * pMailPart = pActor->GetMailPart();
		if( 0 != pMailPart){
			pMailPart->AddMailNum(1);
		}
	}
}

//传送门
IPortal *  GameWorld::CreatePortal(TSceneID SceneID, const char * buf, int len,UINT32 flag)
{
	IPortal * pPortal = 0;

	if(!(THING_CREATE_FLAG_DB_DATA & flag))
	{	
		pPortal = new Portal();

		if(pPortal->OnSetPrivateContext(buf,len)==false)
	    {
		  delete pPortal;
		  pPortal = 0;
	    }
	}
	else
	{
		NULL;
	}
	
	

	if(pPortal==0 || pPortal->Create()==false)
	{
		delete pPortal;
		pPortal = 0;
		return 0;
	}

	m_ThingContainer.AddThing(pPortal);

	IGameScene * pGameScene = GetGameScene(SceneID);

	if(0!=pGameScene)
	{
		pGameScene->EnterScene(pPortal);
	}
	else
	{
		TRACE("<error> %s : %d line 场景不存在!",__FUNCTION__,__LINE__);
	}

	return pPortal;
}


IBuilding * GameWorld::CreateBuilding(TSceneID SceneID, const char * buf, int len,UINT32 flag)
{
	IBuilding * pBuilding = 0;

	if(!(THING_CREATE_FLAG_DB_DATA & flag))
	{	
		SBuildingPrivateData * pPrivateData = (SBuildingPrivateData *)buf;

		pBuilding = CreateBuilding((enBuildingType)pPrivateData->m_BuildingType);

		if(pBuilding == 0)
		{
			return 0;
		}

		if(pBuilding->OnSetPrivateContext(buf,len)==false)
	    {
		  delete pBuilding;
		  pBuilding = 0;
	    }
	}
	else
	{
		SDB_Get_BuildingData_Rsp * pBuildingData = (SDB_Get_BuildingData_Rsp *)buf;

		pBuilding = CreateBuilding((enBuildingType)pBuildingData->m_BuildingType);

		if(pBuilding == 0)
		{
			return 0;
		}

		if(pBuilding->OnSetDBContext(buf,len)==false)
	    {
		  delete pBuilding;
		  pBuilding = 0;
	    }
	}
	
	

	if(pBuilding==0 || pBuilding->Create()==false)
	{
		delete pBuilding;
		pBuilding = 0;
		return 0;
	}

	m_ThingContainer.AddThing(pBuilding);

	IGameScene * pGameScene = GetGameScene(SceneID);

	if(0!=pGameScene)
	{
		pGameScene->EnterScene(pBuilding);		
	}
	else
	{
		TRACE("<error> %s : %d 场景不存在!",__FUNCTION__,__LINE__);
	}

	return pBuilding;
}

//创建物品
IGoods * GameWorld::CreateGoods(TSceneID SceneID, const char * buf, int len,UINT32 flag)
{
	IGoods * pGoods = 0;

	if(!(THING_CREATE_FLAG_DB_DATA & flag))
	{	
		SGoodsPrivateData * pPrivateData = (SGoodsPrivateData *)buf;

		pGoods = __CreateGoods(pPrivateData->m_GoodsID);

		if(pGoods == 0)
		{
			return 0;
		}

		if(pGoods->OnSetPrivateContext(buf,len)==false)
	    {
		  delete pGoods;
		  pGoods = 0;
	    }
	}
	else
	{
		SDBGoodsData * pDBGoodsData = (SDBGoodsData *)buf;

		pGoods = __CreateGoods(pDBGoodsData->m_GoodsID);

		if(pGoods == 0)
		{
			return 0;
		}

		if(pGoods->OnSetDBContext(buf,len)==false)
	    {
		  delete pGoods;
		  pGoods = 0;
	    }
	}
	
	

	if(pGoods->Create()==false)
	{
		delete pGoods;
		pGoods = 0;
	}

	m_ThingContainer.AddThing(pGoods);

	return pGoods;
}

IGoods * GameWorld::__CreateGoods(TGoodsID GoodsID)
{
	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	if(pGoodsCnfg == 0)
	{
		TRACE("<error> %s : %d Line 不能找到物品配置信息 GoodsID=%d",__FUNCTION__,__LINE__,GoodsID);
		return 0;
	}

	switch(pGoodsCnfg->m_GoodsClass)
	{
	case enGoodsCategory_Common:
		return new CommonGoods();
		break;
	case enGoodsCategory_Equip:
		return new Equipment();
		break;
	case enGoodsCategory_GodSword:
		return new GodSword();
	case enGoodsCategory_Talisman:
		return new Talisman();
	default:
		break;
	}

	return 0;
}

//创建物品
IGoods * GameWorld::CreateGoods(const SCreateGoodsContext & GoodsCnt )
{
	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsCnt.m_GoodsID);

	if(pGoodsCnfg == 0)
	{
		TRACE("<error> %s : %d Line 不能找到物品配置信息 GoodsID=%d",__FUNCTION__,__LINE__,GoodsCnt.m_GoodsID);
		return 0;
	}

	if(GoodsCnt.m_Number>pGoodsCnfg->m_PileNum)
	{
		TRACE("<error> %s : %d Line 创建物品失败，数量超出最大叠加数 GoodsID=%d Number = %d",__FUNCTION__,__LINE__,GoodsCnt.m_GoodsID,GoodsCnt.m_Number);
		return 0;
	}

	SGoodsPrivateData PrivateData;
	PrivateData.m_GoodsClass = pGoodsCnfg->m_GoodsClass;
	PrivateData.m_Binded = GoodsCnt.m_Binded;
	PrivateData.m_CreateTime = CURRENT_TIME();
	PrivateData.m_GoodsID = GoodsCnt.m_GoodsID;
	PrivateData.m_Number = GoodsCnt.m_Number;
	PrivateData.m_uidGoods = UniqueIDGeneratorService::GenerateUID(enThing_Class_Goods);

	if(pGoodsCnfg->m_GoodsClass == enGoodsCategory_Equip)
	{
			PrivateData.m_EquipProp.m_Star = 0;
	        memset(PrivateData.m_EquipProp.m_GemGoodsID,0,sizeof(PrivateData.m_EquipProp.m_GemGoodsID));
	}
	else if(pGoodsCnfg->m_GoodsClass == enGoodsCategory_GodSword)
	{
		PrivateData.m_GodSwordProp.m_Nimbus = 0;
		PrivateData.m_GodSwordProp.m_SecretLevel = 0;
		PrivateData.m_GodSwordProp.m_SwordLevel = 1;

	}
	else if(pGoodsCnfg->m_GoodsClass == enGoodsCategory_Talisman)
	{
		PrivateData.m_TalismanProp.m_BeginGestateTime = 0;
		PrivateData.m_TalismanProp.m_EnterNum = 0;
		PrivateData.m_TalismanProp.m_GestateGoodsID = INVALID_GOODS_ID;
		//PrivateData.m_TalismanProp.m_LastEnterTime = 0;
		PrivateData.m_TalismanProp.m_MagicLevel = 1;
		PrivateData.m_TalismanProp.m_QualityPoint = 0;
		PrivateData.m_TalismanProp.m_GhostGoodsID = INVALID_GOODS_ID;

	}
	else if(pGoodsCnfg->m_GoodsClass == enGoodsCategory_Common)
	{
		if(pGoodsCnfg->m_SubClass == enGoodsSecondType_Common && pGoodsCnfg->m_ThirdClass == enGoodsThreeType_Ghost)
			PrivateData.m_CommonProp.m_Ghost.m_GhostLevel = 1;

	}

	return CreateGoods(INVALID_SCENE_ID,(char*)&PrivateData,sizeof(PrivateData),THING_CREATE_FLAG_NULL);
}

std::vector<IGoods *> GameWorld::CreateGoods(TGoodsID GoodsID, UINT32 nNum, bool bBinded)
{
	std::vector<IGoods *> vectGoods;

	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	if(pGoodsCnfg == 0)
	{
		TRACE("<error> %s : %d Line 不能找到物品配置信息 GoodsID=%d",__FUNCTION__,__LINE__,GoodsID);
		return vectGoods;
	}

	SCreateGoodsContext GoodsCnt;
	GoodsCnt.m_GoodsID = GoodsID;
	GoodsCnt.m_Binded  = bBinded;

	while(nNum > 0)
	{
		if( nNum > pGoodsCnfg->m_PileNum){
			GoodsCnt.m_Number = pGoodsCnfg->m_PileNum;
			nNum -= pGoodsCnfg->m_PileNum;
		}else{
			GoodsCnt.m_Number = nNum;	
			nNum = 0;
		}

		IGoods * pGoods = this->CreateGoods(GoodsCnt);
		if( 0 != pGoods){
			vectGoods.push_back(pGoods);
		}
	}

	return vectGoods;
}

//从游戏世界里删除Thing
void	GameWorld::DestroyThing(const UID & uid)
{
	
	IThing * pThing = m_ThingContainer.GetThing(uid);

	if(0==pThing)
	{
		return;
	}

		//移除
	m_ThingContainer.RemoveThing(uid);	

	//发事件出去
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThingDestroy);
	SS_ThingDestroyContext Context;
	Context.m_uidThing = uid;

	pThing->OnEvent(msgID,&Context,SIZE_OF(Context));

	pThing->Release();	
}



//通过UID取得角色
IActor*				GameWorld::FindActor(const UID& uid)
{
	IThing * pThing = m_ThingContainer.GetThing(uid);
	if(pThing==0)
	{
		return 0;
	}
	else if(pThing->GetThingClass()!= enThing_Class_Actor)
	{
		TRACE("<error> %s : %d line 该Thing不是玩家 uid=%s",__FUNCTION__,__LINE__,uid.ToString());
		return 0;
	}
	return (IActor*)pThing;
}


void GameWorld::AddGameScene(GameScene* pGameScene)
{ 
	TSceneID sceneID = pGameScene->GetSceneID();

	//注删场景
	m_scenes[sceneID] = pGameScene;
	
}


void GameWorld::RemoveGameScene(IGameScene* pGameScene)
{
	TSceneID sceneID = pGameScene->GetSceneID();

	//移除场景
	m_scenes.erase(sceneID); 
}

TSceneID GameWorld::GetNextSceneID(const TMapID mapid)
{
	
	const TServerID svrID = g_pGameServer->GetServerID()%4;

	const UINT32 MaxNid = 1048575;  //2^20

	UINT32 nid = m_sidMap[mapid];
	UINT32 old_nid = nid;
	TSceneID sid;
	do 
	{
		sid.From(svrID, mapid, nid);
		if(++nid>MaxNid)
		{
			nid = 0;
		}
		if(old_nid == nid)
		{
			TRACE("<error> %s : %d line 没有可用的场景ID,mapid=%d",__FUNCTION__,__LINE__,mapid);
			return INVALID_SCENE_ID;			
		}
	}while(GetGameScene(sid));

	m_sidMap[mapid] = nid;

	return sid;
}

//////////////////////////////////////////////////////////////////////////
GameScene*	GameWorld::GetGameScene(const TSceneID & sid)
{
	GameSceneMap::iterator it = FindScene(sid);
	return it == m_scenes.end() ? NULL : it->second;
}

GameWorld::GameSceneMap::iterator	GameWorld::FindScene(TSceneID sid)
{
	GameSceneMap::iterator it = m_scenes.find(sid);
	return it;
}

IGameScene*   GameWorld::CreateGameSceneByMapID(TMapID mapid,INT32 CreateNpcIndex, bool bDelNoneUser)
{	
	return CreateGameSceneBySceneID(GetNextSceneID(mapid),CreateNpcIndex, bDelNoneUser);
}

IGameScene*      	GameWorld::CreateGameSceneBySceneID(TSceneID sceneid,INT32 CreateNpcIndex, bool bDelNoneUser)
{
	GameScene*	pGameScene = GetGameScene(sceneid);
	if(pGameScene == 0)
	{
		pGameScene = new GameScene;

	   if(pGameScene->Create(this,sceneid,CreateNpcIndex, bDelNoneUser)==false)
	   {
		  delete pGameScene;
		  return 0;
	    }
	}

	return pGameScene;
}



//获得主城场景ID
TSceneID GameWorld::GetMainSceneID()
{
	TMapID MapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MainMapID;
	TSceneID SceneID = GetNextSceneID(MapID);
	return SceneID;
}

//创建招募角色
IActor *	GameWorld::CreateEmploy(TEmployeeID EmployeeID, IActor * pActor)
{
	CreateEmployee CreateEmploy(EmployeeID, pActor);
	return CreateEmploy.CreateEmploy();
}

//获取语言字串	fly add 20121105
const char * GameWorld::GetLanguageStr(INT32 LanguageID)	
{

	const SLanguageTypeCnfg * pLanguageTypeConfig = g_pGameServer->GetConfigServer()->GetLanguageTypeCnfg(LanguageID);
	if( 0 == pLanguageTypeConfig){
		TRACE("<error> %s ; %d 行 获取语言类型配置数据出错!!语言ID = %d", __FUNCTION__, __LINE__, LanguageID);
		return "Error";
	}
	return pLanguageTypeConfig->m_strEnglish.c_str();
}

//收入无效场景中，等待回收
void		GameWorld::Push_InvalidScene(TSceneID SceneID)
{
	m_vecInvalidScene.push_back(SceneID);
}

void		GameWorld::OnTimer(UINT32 timerID)
{
	if ( enGameWorldTimer_DestroyScene == timerID ){
		//定时垃圾回收场景
		for ( int i = 0; i < m_vecInvalidScene.size(); ++i )
		{
			IGameScene * pGameScene = this->GetGameScene(m_vecInvalidScene[i]);

			if ( 0 == pGameScene )
				continue;

			this->DeleteGameScene(pGameScene);
		}

		m_vecInvalidScene.clear();
	}
}
