#ifndef __THINGSERVER_CREATEEMPLOYEE_CPP__
#define __THINGSERVER_CREATEEMPLOYEE_CPP__

#include "IActor.h"
#include "CreateEmployee.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IConfigServer.h"
#include "DMsgSubAction.h"
#include "IGameScene.h"
#include "ICombatPart.h"

CreateEmployee::CreateEmployee(TEmployeeID EmployeeID, IActor * pMaster)
{
	m_EmployeeID    = EmployeeID;
	m_pMaster		= pMaster;
}

CreateEmployee::~CreateEmployee()
{
}

//创建招募角色
IActor * CreateEmployee::CreateEmploy()
{
	const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(m_EmployeeID);
	if( 0 == pEmployeeDataCnfg){
		TRACE("<error> %s:%d 招募角色配置文件获取失败,EmployeeID = %d", __FUNCTION__, __LINE__, m_EmployeeID);
		return 0;
	}

	m_BuildEmployee.m_pActorBasicData = new SDB_Get_ActorBasicData_Rsp();
	m_BuildEmployee.m_pActorBasicData->UserID			= 0;

	// fly add	20121106
	strncpy(m_BuildEmployee.m_pActorBasicData->Name, g_pGameServer->GetGameWorld()->GetLanguageStr(pEmployeeDataCnfg->m_NameLanguageID), sizeof(m_BuildEmployee.m_pActorBasicData->Name));
	//strncpy(m_BuildEmployee.m_pActorBasicData->Name, pEmployeeDataCnfg->m_szName, sizeof(m_BuildEmployee.m_pActorBasicData->Name));
	m_BuildEmployee.m_pActorBasicData->Level			= pEmployeeDataCnfg->m_Level;
	m_BuildEmployee.m_pActorBasicData->Spirit			= pEmployeeDataCnfg->m_Spirit;
	m_BuildEmployee.m_pActorBasicData->Shield			= pEmployeeDataCnfg->m_Shield;
	m_BuildEmployee.m_pActorBasicData->ActorBloodUp		= pEmployeeDataCnfg->m_BloodUp;
	m_BuildEmployee.m_pActorBasicData->Avoid			= pEmployeeDataCnfg->m_Avoid;
	m_BuildEmployee.m_pActorBasicData->ActorExp			= pEmployeeDataCnfg->m_Exp;
	m_BuildEmployee.m_pActorBasicData->ActorLayer		= pEmployeeDataCnfg->m_Layer;
	m_BuildEmployee.m_pActorBasicData->ActorNimbus		= pEmployeeDataCnfg->m_Nimbus;
	m_BuildEmployee.m_pActorBasicData->ActorAptitude	= pEmployeeDataCnfg->m_Aptitude;
	m_BuildEmployee.m_pActorBasicData->ActorSex			= pEmployeeDataCnfg->m_Sex % enCrtSex_Max;
	m_BuildEmployee.m_pActorBasicData->uid				= (UniqueIDGeneratorService::GenerateUID(enThing_Class_Actor)).ToUint64();
	m_BuildEmployee.m_pActorBasicData->ActorFacade		= pEmployeeDataCnfg->m_ResID;
	m_BuildEmployee.m_pActorBasicData->uidMaster		= m_pMaster->GetUID().ToUint64();
	m_BuildEmployee.m_pActorBasicData->CityID			= m_pMaster->GetCrtProp(enCrtProp_ActorCityID);
	m_BuildEmployee.m_pActorBasicData->ActorMoney		= 0;
	m_BuildEmployee.m_pActorBasicData->ActorStone		= 0;
	m_BuildEmployee.m_pActorBasicData->ActorTicket		= 0;	
	m_BuildEmployee.m_pActorBasicData->Dir				= 0;
	m_BuildEmployee.m_pActorBasicData->ptX				= 0;
	m_BuildEmployee.m_pActorBasicData->ptY				= 0;
	m_BuildEmployee.m_pActorBasicData->ActorNimbusSpeed = pEmployeeDataCnfg->m_NimbusSpeed;		
	m_BuildEmployee.m_pActorBasicData->GodSwordNimbus	= 0;
	m_BuildEmployee.m_pActorBasicData->VipLevel			= 0;
	m_BuildEmployee.m_pActorBasicData->Recharge			= 0;
	m_BuildEmployee.m_pActorBasicData->GoldDamageLv		= 0;
	m_BuildEmployee.m_pActorBasicData->WoodDamageLv		= 0;
	m_BuildEmployee.m_pActorBasicData->WaterDamageLv	= 0;
	m_BuildEmployee.m_pActorBasicData->FireDamageLv		= 0;
	m_BuildEmployee.m_pActorBasicData->SoilDamageLv		= 0;
	m_BuildEmployee.m_pActorBasicData->CritLv			= 0;
	m_BuildEmployee.m_pActorBasicData->TenacityLv		= 0;
	m_BuildEmployee.m_pActorBasicData->HitLv			= 0;
	m_BuildEmployee.m_pActorBasicData->DodgeLv			= 0;
	m_BuildEmployee.m_pActorBasicData->MagicCDLv		= 0;
	m_BuildEmployee.m_pActorBasicData->Crit				= pEmployeeDataCnfg->m_Crit;
	m_BuildEmployee.m_pActorBasicData->Tenacity			= pEmployeeDataCnfg->m_Tenacity;
	m_BuildEmployee.m_pActorBasicData->Hit				= pEmployeeDataCnfg->m_Hit;
	m_BuildEmployee.m_pActorBasicData->Dodge			= pEmployeeDataCnfg->m_Dodge;
	m_BuildEmployee.m_pActorBasicData->MagicCD			= pEmployeeDataCnfg->m_MagicCD;
	m_BuildEmployee.m_pActorBasicData->GoldDamage		= pEmployeeDataCnfg->m_GoldDamage;
	m_BuildEmployee.m_pActorBasicData->WoodDamage		= pEmployeeDataCnfg->m_WoodDamage;
	m_BuildEmployee.m_pActorBasicData->WaterDamage		= pEmployeeDataCnfg->m_WaterDamage;
	m_BuildEmployee.m_pActorBasicData->FireDamage		= pEmployeeDataCnfg->m_FireDamage;
	m_BuildEmployee.m_pActorBasicData->SoilDamage		= pEmployeeDataCnfg->m_SoilDamage;


	//创建PART数据
	this->CreateEquipPanelData();
	this->CreateMagicPanelData();
	this->CreateStatusData();
	this->CreateCombatData();
	this->CreateCDTimerData();

	//检测并创建角色
	return CheckBuildEployee();
}

//创建装备
void	CreateEmployee::CreateEquipPanelData()
{
	m_BuildEmployee.m_pEquipPanel = new SDBEquipPanel();
}

//创建法术栏
void	CreateEmployee::CreateMagicPanelData()
{
	m_BuildEmployee.m_pMagicPanelData = new SDBMagicPanelData();
}


//创建状态数据
void	CreateEmployee::CreateStatusData()
{
	m_BuildEmployee.m_StatusPart.m_pData	= new char[sizeof(SDB_Get_StatusNum_Rsp)];
	m_BuildEmployee.m_StatusPart.m_Len		= sizeof(SDB_Get_StatusNum_Rsp);
	*(SDB_Get_StatusNum_Rsp *)m_BuildEmployee.m_StatusPart.m_pData = SDB_Get_StatusNum_Rsp();
}

//创建战斗数据
void	CreateEmployee::CreateCombatData()
{
	m_BuildEmployee.m_pCombatData = new SDBCombatData();

	for( int i = 0; i < MAX_LINEUP_POS_NUM; ++i)
	{
		m_BuildEmployee.m_pCombatData->m_uidLineup[i] = UID();
	}
}

//创建冷却时间数据
void	CreateEmployee::CreateCDTimerData()
{
	m_BuildEmployee.m_CDTimerDBData.m_pData = new char[SIZE_OF(SDB_CDTimerData())];
	m_BuildEmployee.m_CDTimerDBData.m_Len   = SIZE_OF(SDB_CDTimerData());
	*(SDB_CDTimerData *)m_BuildEmployee.m_CDTimerDBData.m_pData = SDB_CDTimerData();
}

//检测是否可以创建招募角色，可以的话创建招募角色
IActor *	CreateEmployee::CheckBuildEployee()
{
	if( m_BuildEmployee.IsOK() == true){
		//创建角色
		TSceneID SecenID = m_pMaster->GetCrtProp(enCrtProp_ActorMainSceneID);
		IThing * pThing = g_pGameServer->GetGameWorld()->CreateEmploy(m_BuildEmployee);
		if(pThing==0)
		{
			TRACE("<error> %s : %d 创建招募角色失败!",__FUNCTION__,__LINE__);
			return 0;
		}

		IActor * pEmployee = (IActor *)pThing;

		//加入招募角色
		if( false == m_pMaster->SetEmployee(-1, pEmployee)){
			g_pGameServer->GetGameWorld()->DestroyThing(pEmployee->GetUID());
			return false;
		}

		m_pMaster->NoticClientCreatePrivateThing(pEmployee->GetUID());

		for(int i=enThingPart_Crt_Basic; i<enThingPart_Crt_Max;i++)
		{
			IThingPart* pThingPart = pEmployee->GetPart((enThingPart)i);
			if(pThingPart)
			{
				pThingPart->InitPrivateClient();
			}
		}
		
		m_BuildEmployee.Release();
		//////////////////////////////////////

		ICombatPart * pCombatPart = m_pMaster->GetCombatPart();
		if( 0 == pCombatPart){
			return pEmployee;
		}

		int nCount = 0;
		for( int i = 0; i < MAX_EMPLOY_NUM; ++i){
			if( m_pMaster->GetEmployee(i)){
				++nCount;
			}
		}

		if( nCount < MAX_COMBAT_ACTOR_NUM){
			//当招募角色小于5人时,自动切换参战状态
			pCombatPart->TrySetJoinBattle(pEmployee->GetUID());
		}

		//发布事件
		SS_CreateEmployee CreateEmployee;
		CreateEmployee.m_EmployeeID = m_EmployeeID;
		CreateEmployee.m_Index = nCount;

		if( m_EmployeeID == 1001 || m_EmployeeID == 1002 || 
				m_EmployeeID == 1003 || m_EmployeeID == 1004)
		{
			CreateEmployee.m_bChangQi = true;
		}

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_CreateEmployee);
		m_pMaster->OnEvent(msgID,&CreateEmployee,sizeof(CreateEmployee));

		return pEmployee;
	}

	return 0;
}


#endif
