
#include "DBProtocol.h"

#include "ActorBuilder.h"
#include "GameWorld.h"
#include "ResOutputPart.h"
#include "FuMoDongPart.h"
#include "TrainingHallPart.h"
#include "PacketPart.h"
#include "EquipPart.h"
#include "GatherGodHousePart.h"
#include "XiuLianPart.h"
#include "FriendPart.h"
#include "MailPart.h"
#include "MagicPart.h"
#include "StatusPart.h"
#include "CombatPart.h"
#include "FuBenPart.h"
#include "SynMagicPart.h"
#include "GodSwordShopPart.h"
#include "ThingServer.h"
#include "ActorBasicPart.h"
#include "TalismanPart.h"
#include "DouFaPart.h"
#include "TaskPart.h"
#include "ChengJiuPart.h"
#include "ActivityPart.h"
#include "XDateTime.h"
#include "CDTimerPart.h"
#include "TeamPart.h"
#include "SynPart.h"

Actor* ActorBuilder::BuildActor(TSceneID sceneID, const UID& uid, const char *buf, int len,UINT32 flag)
{
	if (NULL == buf || len != SIZE_OF(SBuild_Actor()))
	{
		return NULL;
	}

	SBuild_Actor *	pBuildActor = (SBuild_Actor *) buf;

	IThingPart *	CrtPartArray[enThingPart_Crt_Max];
	//memset(CrtPartArray, 0, MEM_SIZE(CrtPartArray));
	MEM_ZERO(CrtPartArray);

	//可以new一块大内存，以免每次创建人物时，都要new内存
	Actor *pActor = new Actor;
	if (NULL == pActor){
		return 0;
	}

	

	if ( !pActor->OnSetDBContext(pBuildActor->m_pActorBasicData, SIZE_OF(SDB_Get_ActorBasicData_Rsp())) ){		
		return 0;
	}

	if(!pActor->GetUID().IsValid())
	{
	   pActor->SetUID(uid);
	}
	
	if( !pActor->SetEmployUID(pBuildActor, SIZE_OF(SBuild_Actor()))){
		return 0;
	}

	if ( !pActor->Create() )
	{	
		return NULL;
	}

	pActor->RecalculateProp(false);	//计算属性

	pActor->SetCrtProp(enCrtProp_ActorMainSceneID,sceneID.m_id);
	pActor->SetCrtProp(enCrtProp_SceneID, sceneID.m_id);
	pActor->SetCrtProp(enCrtProp_ActorLastSceneID, sceneID.m_id);
	pActor->SetCrtProp(enCrtProp_SceneID, sceneID.m_id);
	pActor->SetCrtProp(enCrtProp_ActorExpRate, 1000);

	pActor->SetCrtProp(enCrtProp_ActorLogin,CURRENT_TIME());
	pActor->SetCrtProp(enCrtProp_ActorIdentity,pBuildActor->m_IdentityStatus);
	pActor->SetCrtProp(enCrtProp_ActorOnlineTimes,pBuildActor->m_OnlineTime);

	IThingPart*  pThingPart = new ActorBasicPart();
	if( pThingPart->Create(pActor, 0, 0) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);
	
	
	//状态part(先创建，因为会影响其它PART资源的获取)
	 pThingPart = new StatusPart();
	 if(pThingPart->Create(pActor,&pBuildActor->m_StatusPart,pBuildActor->m_StatusPart.m_Len) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//资源产出ThingPart
  pThingPart = new ResOutputPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pBuildingData, pBuildActor->m_BuildingLen) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//伏魔洞
	 pThingPart = new FuMoDongPart();
	 if(pThingPart->Create(pActor, pBuildActor->m_pFuMoDongData, SIZE_OF(SDB_Get_FuMoDongData_Rsp())) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//练功堂
	 pThingPart = new TrainingHallPart();
	 if(pThingPart->Create(pActor, pBuildActor->m_pTrainingHall, SIZE_OF(SDB_Get_TrainingHallData_Rsp())) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//聚仙楼
	pThingPart = new GatherGodHousePart();
	if(pThingPart->Create(pActor, pBuildActor->m_pGatherGodHouseData, SIZE_OF(SDB_Get_GatherGodHouseData_Rsp())) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//好友
	pThingPart = new FriendPart();
	if(pThingPart->Create(pActor, 0, 0) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//修炼
	pThingPart = new XiuLianPart();
	if(pThingPart->Create(pActor, pBuildActor->m_pXiuLianData,pBuildActor->m_XiuLianDataSize) == false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//邮件
	pThingPart = new MailPart();
	if(pThingPart->Create(pActor, 0, 0) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);


	//剑冢
	pThingPart = new GodSwordShopPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pGodSwordShopData, sizeof(SDB_Get_GodSwordShopData_Rsp)) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//背包
	pThingPart = new PacketPart();
	if( pThingPart->Create(pActor, &pBuildActor->m_PacketData, pBuildActor->m_PacketData.m_nLen) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//装备
	pThingPart = new EquipPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pEquipPanel, sizeof(SDBEquipPanel)) == false){
		delete pActor;
		return 0;	
	}
	pActor->AddPart(pThingPart);

	//副本part
	 pThingPart = new FuBenPart();
	 if(pThingPart->Create(pActor,pBuildActor->m_pFuBenData,SIZE_OF(SDBFuBenData()))==false)
	{
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//法术栏
	pThingPart = new MagicPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pMagicPanelData, sizeof(SDBMagicPanelData)) == false){
		delete pActor;
		return 0;	
	}
	pActor->AddPart(pThingPart);

	//帮派技能
	pThingPart = new SynMagicPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pSynMagicPanelData, sizeof(SDBSynMagicPanelData)) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//战斗
	pThingPart = new CombatPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pCombatData, sizeof(SDBCombatData)) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//法宝PART
	pThingPart = new TalismanPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pTalismanPart, sizeof(SDB_TalismanPart)) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//任务
	pThingPart = new TaskPart();
	if( pThingPart->Create(pActor, &pBuildActor->m_TaskPart, pBuildActor->m_TaskPart.m_Len) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//成就
	pThingPart = new ChengJiuPart();
	if( pThingPart->Create(pActor, &pBuildActor->m_ChengJiuData, pBuildActor->m_ChengJiuData.m_Len) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);


		//活动
	pThingPart = new ActivityPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pActivityData, pBuildActor->m_ActivityDataLen) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//冷却时间
	pThingPart = new CDTimerPart();
	if( pThingPart->Create(pActor, &pBuildActor->m_CDTimerDBData, pBuildActor->m_CDTimerDBData.m_Len) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//斗法PART
	pThingPart = new DouFaPart();
	if( pThingPart->Create(pActor, pBuildActor->m_pDouFaPart, sizeof(SDB_Get_DouFaPartInfo_Rsp)) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//组队PART
	pThingPart = new TeamPart();
	if( pThingPart->Create(pActor, 0, 0) == false){
		delete pActor;
		return 0;
	}
	pActor->AddPart(pThingPart);

	//帮派PART
	pThingPart = new SynPart();

	if ( pThingPart->Create(pActor, pBuildActor->m_pSynPart, sizeof(SDB_SynPart)) == false){
		delete pActor;
		return 0;
	}
	
	pActor->AddPart(pThingPart);

	//把帮派技能所加的效果加给玩家
	ISynMagicPart * pSynMagicPart = pActor->GetSynMagicPart();
	if( 0 == pSynMagicPart){
		delete pActor;
		return 0;
	}

	pSynMagicPart->UserSynMagicEffect();

	//计算属性
	pActor->RecalculateProp(false);	


	return pActor;
}

ActorBuilder::ActorBuilder()
{
}

ActorBuilder::~ActorBuilder()
{
}
