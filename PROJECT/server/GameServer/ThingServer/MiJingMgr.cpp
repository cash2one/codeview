
#include "MiJingMgr.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "IGoodsServer.h"
#include "IGodSword.h"
#include "ITalisman.h"
#include "ICommonGoods.h"
#include "ITalismanPart.h"



MiJingMgr::MiJingMgr()
{
}

MiJingMgr::~MiJingMgr()
{
}

bool MiJingMgr::Create()
{

	//当玩家登陆后,下发所有产出装备的配置信息
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_ActorLogin);
	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return false;
	}
	pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"MiJingMgr::Create");
	//m_pActor->SubscribeEvent(msgID,this,"MiJingMgr::Create");

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_MiJing,this);
}

void MiJingMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_MiJing,this);
}

//收到MSG_ROOT消息
void MiJingMgr::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	typedef  void (MiJingMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enMiJingCmd_Max]=
	{

		&MiJingMgr::StrongInheritance,
		&MiJingMgr::AddGhost,
		&MiJingMgr::RemoveGhost,
		&MiJingMgr::UpGhost,
		&MiJingMgr::CreditMagicUp,
		&MiJingMgr::EquipMake,
		&MiJingMgr::DamageUp,
		&MiJingMgr::OpenMiJing,
		&MiJingMgr::ReplaceGhost,
		


		
	};

	if(nCmd>=enEquipCmd_Max || 0==s_funcProc[nCmd])
	{
		TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);
} 


//打开秘境界面
void  MiJingMgr::OpenMiJing(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{


	SC_OpenMiJing_Rsp  Rsp;

	Rsp.m_GodSwordNimbus	= pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus);
	Rsp.m_MaxGodSwordNimbus = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;

	 OBuffer1k ob;
	 ob << MiJing_Header(enMiJingCmd_OpenMiJing,SIZE_OF(Rsp)) << Rsp;
	 pActor->SendData(ob.TakeOsb());
}


//强化传承
void MiJingMgr::StrongInheritance(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_StrongInheritanceMJ_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pSrcGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_SrcEquip);
	
	if ( 0 == pSrcGoods){
		
		TRACE("<error> %s : %d Line 强化传承，找不到传承物品！！uid = %s", __FUNCTION__, __LINE__, Req.m_SrcEquip.ToString());
		return;
	}

	if ( pSrcGoods->GetGoodsClass() != enGoodsCategory_Equip && pSrcGoods->GetGoodsClass() != enGoodsCategory_Talisman && pSrcGoods->GetGoodsClass() != enGoodsCategory_GodSword){
		
		TRACE("<error> %s : %d Line 强化传承，传承物品不是装备或者法宝或仙剑，物品ID = %d", __FUNCTION__, __LINE__, pSrcGoods->GetGoodsID());
		return;
	}

	IGoods * pDesGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_DesEquip);
	
	if ( 0 == pDesGoods){
		
		TRACE("<error> %s : %d Line 强化传承，找不到被传承物品！！uid = %s", __FUNCTION__, __LINE__, Req.m_DesEquip.ToString());
		return;
	}

	if ( pDesGoods->GetGoodsClass() != enGoodsCategory_Equip && pDesGoods->GetGoodsClass() != enGoodsCategory_Talisman && pDesGoods->GetGoodsClass() != enGoodsCategory_GodSword){
		
		TRACE("<error> %s : %d Line 强化传承，被传承物品不是装备或者法宝或仙剑，物品ID = %d", __FUNCTION__, __LINE__, pDesGoods->GetGoodsID());
		return;
	}

	SC_StrongInheritanceMJ_Rsp Rsp;

	Rsp.m_Result = enMiJingRetCode_OK;

	if ( Req.m_Type == InheritanceTypeMJ_Money)
	{
		Rsp.m_Result = (enMiJingRetCode)this->__MoneyInheritance(pActor, (IEquipment *)pSrcGoods, (IEquipment *)pDesGoods);
	}
	else if ( Req.m_Type == InheritanceTypeMJ_Ordinary)
	{
		Rsp.m_Result = (enMiJingRetCode)this->__OrdinaryInheritance(pActor, (IEquipment *)pSrcGoods, (IEquipment *)pDesGoods);
	}
	else
	{
		TRACE("<error> %s ; %d Line 客户端数据有误！！传承方式为%d", __FUNCTION__, __LINE__, Req.m_Type);
		return;
	}

	OBuffer1k ob;
	ob << MiJing_Header(enMiJingCmd_StrongInheritance,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//支付仙石传承
UINT8  MiJingMgr::__MoneyInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment)
{
	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;
	IGodSword * pSrcGodSword = (IGodSword *)pSrcEquipment;
	IGodSword * pDesGodSword = (IGodSword *)pDesEquipment;
	if ( pActor->GetCrtProp(enCrtProp_ActorMoney) < ServerConfigParam.m_MoneyInheritMoneyNum){
		
		return enMiJingRetCode_NoMoney;
	}

	if ( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ServerConfigParam.m_OrdinaryInheritPolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}
	
	pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -ServerConfigParam.m_MoneyInheritPolyNimbusNum);
	pActor->AddCrtPropNum(enCrtProp_ActorMoney, -ServerConfigParam.m_MoneyInheritMoneyNum);

	g_pGameServer->GetGameWorld()->Save_GodStoneLog(pActor->GetCrtProp(enCrtProp_ActorUserID), ServerConfigParam.m_MoneyInheritMoneyNum, pActor->GetCrtProp(enCrtProp_ActorMoney), "支付仙石传承");
	
	enGoodsCategory SrcGoodsClass = pSrcEquipment->GetGoodsClass();

	enGoodsCategory DesGoodsClass = pDesEquipment->GetGoodsClass();

	int SrcLevel = 0;

	int DesLevel = 0;


	//装备星级从0开始，法宝、仙剑从1开始
	switch( SrcGoodsClass * 10 + DesGoodsClass)
	{
	case enGoodsCategory_Equip * 10 + enGoodsCategory_Talisman:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel + 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, SrcLevel + 1);
			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_Equip:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel - 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, SrcLevel - 1);

			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, SrcLevel - 1);

			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	case enGoodsCategory_Equip * 10 + enGoodsCategory_Equip:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, SrcLevel);

			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, SrcLevel);

			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_Talisman:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, SrcLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_GodSword:
		{
			//仙->仙
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}
			
			pDesGodSword->UpgradeSword(pActor,SrcLevel - DesLevel);
			pSrcGodSword->UpgradeSword(pActor,-(SrcLevel - 1));

		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_Equip:
		{
			//仙->装
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, SrcLevel - 1);
			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, SrcLevel - 1);
			pSrcGodSword->UpgradeSword(pActor, -(SrcLevel - 1));
		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_Talisman:
		{
			//仙->法
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, SrcLevel);
			pSrcGodSword->UpgradeSword(pActor, -(SrcLevel - 1));
		}
		break;
	case enGoodsCategory_Equip * 10 + enGoodsCategory_GodSword:
		{
			//装->仙
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel + 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesGodSword->UpgradeSword(pActor,SrcLevel + 1 - DesLevel);
			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_GodSword:
		{
			//法->仙
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			pDesGodSword->UpgradeSword(pActor, SrcLevel - DesLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	default:
		TRACE("<error> %s : %d Line 强化传承中的物品类型有错!!,类型1:%d,类型2:%d", __FUNCTION__, __LINE__, SrcGoodsClass,DesGoodsClass);
		return enMiJingRetCode_ErrEquip;
	}
	

	return enMiJingRetCode_OK;
}


//普通传承
UINT8  MiJingMgr::__OrdinaryInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment)
{
	IGodSword * pSrcGodSword = (IGodSword *)pSrcEquipment;
	IGodSword * pDesGodSword = (IGodSword *)pDesEquipment;
	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ServerConfigParam.m_OrdinaryInheritPolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}

	pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -ServerConfigParam.m_OrdinaryInheritPolyNimbusNum); 

	enGoodsCategory SrcGoodsClass = pSrcEquipment->GetGoodsClass();

	enGoodsCategory DesGoodsClass = pDesEquipment->GetGoodsClass();

	int SrcLevel = 0;

	int DesLevel = 0;

	INT8 RealLevel = g_pGameServer->GetConfigServer()->RandGetStrongInheritLeve(pSrcEquipment, DesGoodsClass);

	//装备稳定强化等级开始
	UINT8 nEquipNoRandLevel = ServerConfigParam.m_EquipInheritNoRandLevel;

	//法宝、仙剑稳定强化等级开始
	UINT8 nTalismanNoRandLevel = ServerConfigParam.m_TalismanInheritNoRandLevel;

	//装备星级从0开始，法宝、仙剑从1开始
	switch( SrcGoodsClass * 10 + DesGoodsClass)
	{
	case enGoodsCategory_Equip * 10 + enGoodsCategory_Talisman:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel + 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			if ( SrcLevel >= nEquipNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nEquipNoRandLevel + 1;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, RealLevel);
			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_Equip:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel - 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, RealLevel);

			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, RealLevel);

			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	case enGoodsCategory_Equip * 10 + enGoodsCategory_Equip:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			if ( SrcLevel >= nEquipNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nEquipNoRandLevel + 1;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, RealLevel);

			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, RealLevel);

			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_Talisman:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, RealLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_GodSword:
		{
			//仙->仙
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}

			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}
			pDesGodSword->UpgradeSword(pActor,RealLevel - DesLevel);
			pSrcGodSword->UpgradeSword(pActor,-(SrcLevel - 1));

		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_Equip:
		{
			//仙->装
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_StarLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}
			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}
			pDesEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, RealLevel);
			this->__CheckInEquip_AddStrongerProp(pActor, pDesEquipment, DesLevel, RealLevel);
			pSrcGodSword->UpgradeSword(pActor, -(SrcLevel - 1));
		}
		break;
	case enGoodsCategory_GodSword * 10 + enGoodsCategory_Talisman:
		{
			//仙->法
			pSrcGodSword->GetPropNum(enGoodsProp_SwordLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}
			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}
			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, RealLevel);
			pSrcGodSword->UpgradeSword(pActor, -(SrcLevel - 1));
		}
		break;
	case enGoodsCategory_Equip * 10 + enGoodsCategory_GodSword:
		{
			//装->仙
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel + 1 <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}
			if ( SrcLevel >= nEquipNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nEquipNoRandLevel + 1;
			}
			pDesGodSword->UpgradeSword(pActor,RealLevel - DesLevel);
			this->ClearStartLvAndGem(pActor, pSrcEquipment);
		}
		break;
	case enGoodsCategory_Talisman * 10 + enGoodsCategory_GodSword:
		{
			//法->仙
			pSrcEquipment->GetPropNum(enGoodsProp_MagicLevel, SrcLevel);
			pDesGodSword->GetPropNum(enGoodsProp_SwordLevel, DesLevel);

			if ( SrcLevel <= DesLevel){
				
				return enMiJingRetCode_EquipLevelLow;
			}
			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}
			pDesGodSword->UpgradeSword(pActor, RealLevel - DesLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;

	default:
		TRACE("<error> %s : %d Line 强化传承中的物品类型有错!!,类型1:%d,类型2:%d", __FUNCTION__, __LINE__, SrcGoodsClass,DesGoodsClass);
		return enMiJingRetCode_ErrEquip;
	}
	
	return enMiJingRetCode_OK;
}

//清除装备星级
void   MiJingMgr::ClearStartLvAndGem(IActor * pActor, IEquipment * pEquipment)
{
	enGoodsCategory GoodsClass = pEquipment->GetGoodsClass();

	switch (GoodsClass)
	{
	case enGoodsCategory_Equip:
		{
			int OldStarLevel = 0;

			pEquipment->GetPropNum(enGoodsProp_StarLevel, OldStarLevel);

			pEquipment->SetPropNum(pActor, enGoodsProp_StarLevel, 0);

			this->__CheckInEquip_AddStrongerProp(pActor, pEquipment, OldStarLevel, 0);

			for ( int i = enGoodsProp_Gem_ID1; i <= enGoodsProp_Gem_ID3; ++i)
			{
				int GemID = INVALID_GOODS_ID;

				pEquipment->GetPropNum((enGoodsProp)i, GemID);

				if ( GemID == INVALID_GOODS_ID){
					
					continue;
				}

				//若在装备栏，则改变属性
				this->__CheckInEquip_AddInlayProp(pActor, pEquipment, GemID, INVALID_GOODS_ID);

				pEquipment->SetPropNum(pActor, (enGoodsProp)i, 0);
			}
		}
		break;
	case enGoodsCategory_Talisman:
		{
			pEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	}
}

//强化成功后，如果装备在玩家身上，则把属性加上去
void  MiJingMgr::__CheckInEquip_AddStrongerProp(IActor *pActor,IEquipment * pEquipment, UINT8 OldStarLevel, UINT8 NewStarLevel)
{


	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	if( enGoodsCategory_Equip != pEquipment->GetGoodsClass()){
		return;
	}

	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pEquipment->GetGoodsID());
	if( 0 == pGoodsCnfg){
		TRACE("<error> %s : %d 行 获取物品配置信息出错！！物品ID＝%d", __FUNCTION__, __LINE__, pEquipment->GetGoodsID());
		return;
	}

	//装备在哪个角色身上
	IActor * pTargetActor = g_pGameServer->GetGoodsServer()->GetActorOnEquip(pActor, pEquipment);
	if( 0 == pTargetActor){
		//不在角色身上
		return;
	}

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//增加的比例值
	if( NewStarLevel > ConfigParam.m_vectEquipStrongPropRate.size() || OldStarLevel > ConfigParam.m_vectEquipStrongPropRate.size()){
		return;
	}

	INT32 NewRate = 0;

	if ( NewStarLevel > 0){
		
		NewRate = ConfigParam.m_vectEquipStrongPropRate[NewStarLevel-1];
	}

	INT32 OldRate = 0;

	if ( OldStarLevel > 0){

		OldRate = ConfigParam.m_vectEquipStrongPropRate[OldStarLevel-1];
	}

	for(int i = 0 ; i<enEquipProp_Max; i++)
	{
		INT32 Value = g_pThingServer->GetEquipProp(pEquipment->GetGoodsID(),(enEquipProp)i);
		if(Value >0)
		{
			INT32 NewValue = Value * (float)NewRate/100 + 0.99999;

			INT32 OldValue = Value * (float)OldRate/100 + 0.99999;

            pTargetActor->AddCrtPropNum(s_EquipPropMapActorProp[i],NewValue - OldValue);
		}
	}
}

//镶嵌成功后，如果装备在玩家身上，则把属性加上去
void  MiJingMgr::__CheckInEquip_AddInlayProp(IActor *pActor,IEquipment * pEquipment, TGoodsID OldGemID, TGoodsID NewGemID)
{
	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if ( 0 == pEquipPart){

		return;
	}

	if ( enGoodsCategory_Equip != pEquipment->GetGoodsClass()){

		return;
	}

	IActor * pTargetActor = g_pGameServer->GetGoodsServer()->GetActorOnEquip(pActor, pEquipment);
	if ( 0 == pTargetActor){

		//不在角色装备栏上
		return;
	}

	//扣除旧宝石加的属性.
	const SGoodsCnfg * pOldGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(OldGemID);
	if ( 0 != pOldGoodsCnfg){

		if ( pOldGoodsCnfg->m_BloodOrSwordkee >= enEquipProp_Max){
			
			return;
		}

		pTargetActor->AddCrtPropNum(s_EquipPropMapActorProp[(enEquipProp)pOldGoodsCnfg->m_BloodOrSwordkee],-pOldGoodsCnfg->m_AvoidOrSwordLvMax);
	}

	//加上新镶嵌的宝石属性
	const SGoodsCnfg * pNewGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(NewGemID);
	if( 0 != pNewGoodsCnfg){

		if ( pNewGoodsCnfg->m_BloodOrSwordkee >= enEquipProp_Max){
			
			return;
		}

		pTargetActor->AddCrtPropNum(s_EquipPropMapActorProp[(enEquipProp)pNewGoodsCnfg->m_BloodOrSwordkee],pNewGoodsCnfg->m_AvoidOrSwordLvMax);
	}
}




//附灵成功后，如果法宝在玩家身上，则把属性加上去
void  MiJingMgr::__CheckInTalisman_AddGhostProp(IActor *pActor,ITalisman * pTalisman, UINT8 OldLevel, TGoodsID OldGhostID, UINT8 NewLevel, TGoodsID NewGhostID)
{
	//ITalisman * pTalisman = pActor->GetTalismanPart();
	//if ( 0 == pEquipPart){

	//	return;
	//}

	if ( enGoodsCategory_Talisman != pTalisman->GetGoodsClass()){

		return;
	}

	IActor * pTargetActor = g_pGameServer->GetGoodsServer()->GetActorOnEquip(pActor, pTalisman);
	if ( 0 == pTargetActor){

		//不在角色装备栏上
		return;
	}

	//扣除旧灵件加的属性.
	const SUpGhostCnfg * pOldUpGhost = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(OldGhostID,OldLevel);
	if ( 0 != pOldUpGhost){

		if ( pOldUpGhost->m_Type >= enGhostProp_Max){
			
			return;
		}

		pTargetActor->AddCrtPropNum(s_GhostPropMapActorProp[(enGhostProp)pOldUpGhost->m_Type], -pOldUpGhost->m_Addprop);

	}


	//加上新灵件属性
	const SUpGhostCnfg * pNewUpGhost = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(NewGhostID,NewLevel);
	if( 0 != pNewUpGhost){

		if ( pNewUpGhost->m_Type >= enGhostProp_Max){
			
			return;
		}

		pTargetActor->AddCrtPropNum(s_GhostPropMapActorProp[(enGhostProp)pNewUpGhost->m_Type], pNewUpGhost->m_Addprop);

	}
}


//法宝附灵
void MiJingMgr::AddGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{

	CS_AddGhost_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_AddGhost_Rsp Rsp;

	Rsp.m_Result = enMiJingRetCode_OK;

	//法宝是否存在于背包中
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidTalisman);
	if( 0 == pGoods){
		return;
	}

	IGoods * pGhostGoods = pPacketPart->GetGoods(Req.m_uidGhost);
	if( 0 == pGhostGoods){
		return;
	}

	const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if(pGoods==0)
	{
		Rsp.m_Result = enMiJingRetCode_NoTalisman;
	}
	else if(pGoods->GetGoodsClass() != enGoodsCategory_Talisman)
	{
		Rsp.m_Result = enMiJingRetCode_NotTalisman;
	}
	else if(pGhostGoods->GetGoodsClass() != enGoodsCategory_Common)
	{
		Rsp.m_Result = enMiJingRetCode_NotGhost;
	}
	else if(pGhostGoods->GetGoodsCnfg()->m_ThirdClass != enGoodsThreeType_Ghost)
	{
		Rsp.m_Result = enMiJingRetCode_NotGhost;
	}
	else if(pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ConfigParam.m_PolyNimbusAddGhost)
	{
		Rsp.m_Result = enMiJingRetCode_NoPolyNimbus;
	}
	else
	{
		//先保存下该ID,下面灵件销毁后，还需要用该ID
		TGoodsID gidGhost = pGhostGoods->GetGoodsID();

		ICommonGoods * pCommonGoods = (ICommonGoods * )pGhostGoods;

		const SGhostProp * pGhostProp = (const SGhostProp *)pCommonGoods->GetExtendData();

		//保存等级
		UINT8 Level = pGhostProp->m_GhostLevel;

		ITalisman * pTalisman = (ITalisman *)pGoods;
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -ConfigParam.m_PolyNimbusAddGhost);
		if(pPacketPart->DestroyGoods(gidGhost)==false)
		{
			Rsp.m_Result = enMiJingRetCode_NotGhost;
		}
		else
		{
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,gidGhost,UID(),1,"法宝附灵扣除灵件");

			//int OldGhostID = 0;
			//pTalisman->GetPropNum(enGoodsProp_GhostGoodsID, OldGhostID);

			//成功
			pTalisman->SetPropNum(pActor,enGoodsProp_GhostGoodsID,gidGhost);

			//等级
			pTalisman->SetPropNum(pActor,enGoodsProp_GhostLevel,Level);

			int NewGhostID = 0;
			pTalisman->GetPropNum(enGoodsProp_GhostGoodsID, NewGhostID);

			pPacketPart->SaveGoodsToDB(pGoods);

			//附灵成功后，如果法宝在玩家身上，则把属性加上去
			this->__CheckInTalisman_AddGhostProp(pActor, pTalisman, 0,INVALID_GOODS_ID,Level, NewGhostID);

			//发布事件
			SS_TalismanAddGhost TalismanAddGhost;
			TalismanAddGhost.m_Category = pGoods->GetGoodsClass();

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TalismanAddGhost);
			pActor->OnEvent(msgID,&TalismanAddGhost,sizeof(TalismanAddGhost));
		}
	}

	OBuffer1k ob;

	ob << MiJing_Header(enMiJingCmd_AddGhost,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

}

//灵件摘除
void MiJingMgr::RemoveGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_RemoveGhost_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidTalisman);

	if ( 0 == pGoods){
		
		TRACE("<error> %s : %d Line 灵件摘除,找不到法宝", __FUNCTION__, __LINE__);
		return;
	}

	if ( pGoods->GetGoodsClass() != enGoodsCategory_Talisman){
		
		TRACE("<error> %s : %d Line 不是法宝,物品ID = %d", __FUNCTION__, __LINE__, pGoods->GetGoodsID());
		return;
	}

	ITalisman * pTalisman = (ITalisman *)pGoods;



	int GhostID = INVALID_GOODS_ID;
	pTalisman->GetPropNum(enGoodsProp_GhostGoodsID, GhostID);

	if ( GhostID == INVALID_GOODS_ID){
		
		TRACE("<error> %s ; %d Line 该法宝没有灵件,", __FUNCTION__, __LINE__);
		return;
	}

	SC_RemoveGhost_Rsp Rsp;

	Rsp.m_Result = (enMiJingRetCode)this->__RemoveGhost(pActor, pTalisman, GhostID);

	OBuffer1k ob;
	ob << MiJing_Header(enMiJingCmd_RemoveGhost,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//灵件摘除,返回结果码
UINT8  MiJingMgr::__RemoveGhost(IActor * pActor, ITalisman * pTalisman, TGoodsID OldGhostID)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if ( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}
	if ( !pPacketPart->CanAddGoods(OldGhostID, 1)){
		
		//背包已满，请清理背包
		return enMiJingRetCode_NoSpace;
	}else {

		SCreateGoodsContext CreateGoodsCnt;
		CreateGoodsCnt.m_Binded = true;
		CreateGoodsCnt.m_GoodsID = OldGhostID;
		CreateGoodsCnt.m_Number = 1;
		IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(CreateGoodsCnt);

		if(pGoods == 0)
		{
			return enMiJingRetCode_NotGhost;
		}

		int Level = 0;
		pTalisman->GetPropNum(enGoodsProp_GhostLevel,Level);

		SGhostProp GhostProp;
		GhostProp.m_GhostLevel =  Level;

		ICommonGoods * pCommonGoods = (ICommonGoods *)pGoods;

		pCommonGoods->SetExtendLen(&GhostProp,sizeof(GhostProp));

		//pPacketPart->AddGoods(OldGhostID,1,true);
		pPacketPart->AddGoods(pGoods->GetUID());

		pTalisman->SetPropNum(pActor, enGoodsProp_GhostGoodsID, INVALID_GOODS_ID);

		pTalisman->SetPropNum(pActor, enGoodsProp_GhostLevel, 0);

		//若在装备栏，则改变属性
		this->__CheckInTalisman_AddGhostProp(pActor, pTalisman, Level, OldGhostID, 0, INVALID_GOODS_ID);
	}

	return  enMiJingRetCode_OK;
}


//灵件替换
void MiJingMgr::ReplaceGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ReplaceGhost_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidTalisman);

	if ( 0 == pGoods){
		
		TRACE("<error> %s : %d Line 灵件替换,找不到法宝", __FUNCTION__, __LINE__);
		return;
	}

	IGoods * pGhost = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidGhost);
	if ( 0 == pGhost){
		
		TRACE("<error> %s : %d Line 灵件替换,找不到新灵件", __FUNCTION__, __LINE__);
		return;
	}

	if ( pGoods->GetGoodsClass() != enGoodsCategory_Talisman){
		
		TRACE("<error> %s : %d Line 不是法宝,物品ID = %d", __FUNCTION__, __LINE__, pGoods->GetGoodsID());
		return;
	}

	ITalisman * pTalisman = (ITalisman *)pGoods;

	int OldGhostID = INVALID_GOODS_ID;
	pTalisman->GetPropNum(enGoodsProp_GhostGoodsID, OldGhostID);

	if ( OldGhostID == INVALID_GOODS_ID){
		
		TRACE("<error> %s ; %d Line 该法宝没有灵件可被替换", __FUNCTION__, __LINE__);
		return;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if ( 0 == pPacketPart){
		return;
	}

	const SGameConfigParam & ConfigParam =  g_pGameServer->GetConfigServer()->GetGameConfigParam();


	SC_ReplaceGhost_Rsp Rsp;

	if(pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ConfigParam.m_PolyNimbusAddGhost)
	{
		Rsp.m_Result = enMiJingRetCode_NoPolyNimbus;
	}else{
		//第一步,先灵件摘除
		Rsp.m_Result = (enMiJingRetCode)this->__RemoveGhost(pActor, pTalisman, OldGhostID);	
	}


	if ( Rsp.m_Result == enMiJingRetCode_OK){
		
		//第二步,再附上新灵件
		TGoodsID NewGhostID = pGhost->GetGoodsID();

		//等级
		ICommonGoods * pCommonGoods = (ICommonGoods *)pGhost;
		SGhostProp * pGhostProp = (SGhostProp * )pCommonGoods->GetExtendData();
		int Level = pGhostProp->m_GhostLevel;

		//先删除灵件
		if ( !pPacketPart->DestroyGoods(pGhost->GetUID(), 1)){
			
			TRACE("<error> %s : %d Line 删除灵件失败!!", __FUNCTION__, __LINE__);
			return;
		}

		pGhost = 0;

		pTalisman->SetPropNum(pActor,enGoodsProp_GhostGoodsID,NewGhostID);

		pTalisman->SetPropNum(pActor,enGoodsProp_GhostLevel,Level);

		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -ConfigParam.m_PolyNimbusAddGhost);

		//附灵成功后，如果法宝在玩家身上，则把属性加上去
		this->__CheckInTalisman_AddGhostProp(pActor, pTalisman, 0,INVALID_GOODS_ID,Level, NewGhostID);
	}

	OBuffer1k ob;
	ob << MiJing_Header(enMiJingCmd_ReplaceGhost,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//灵件升级
void MiJingMgr::UpGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_UpGhost_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(Req.m_uidGoods);
	if(pGoods == 0)
	{
		return ;
	}

	TGoodsID GoodsID = pGoods->GetGoodsID();

	ICommonGoods * pCommonGoods = (ICommonGoods *)pGoods;

	SC_UpGhost_Rsp Rsp;

	//const SGhostProp * pGhostProp = (SGhostProp *)pCommonGoods->GetExtendData();

	//int starLevel = pGhostProp->m_GhostLevel;

	//const SUpGhostCnfg * pUpGhost = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(GoodsID,starLevel);
	//if( 0 == pUpGhost){
	//	TRACE("<warning> %s : %d 行，灵件升级配置找不到！！　物品ID　＝　%d", __FUNCTION__,__LINE__, GoodsID);
	//	return ;
	//}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return ;
	}

	Rsp.m_RetCode = (enMiJingRetCode)__UpGhost(pActor,pCommonGoods);

	//if(starLevel == 20){
	//	if(pUpGhost->m_NextGhostID == 0){
	//		//已达最高等级
	//		Rsp.m_RetCode = enMiJingRetCode_ErrMaxLevel;
	//	}else if( pActor->GetCrtProp(enCrtProp_GhostSoul) < pUpGhost->m_GhostSoul){
	//		//灵魄不足
	//		Rsp.m_RetCode = enMiJingRetCode_NoGhostSoul;

	//	}else if(!pPacketPart->HaveGoods(pUpGhost->m_Material, pUpGhost->m_MaterialNum)){
	//		//材料不足
	//		Rsp.m_RetCode = enMiJingRetCode_NoMaterial;

	//	}else if ( pPacketPart->GetSpace() < 1){
	//		//背包空间不足
	//		Rsp.m_RetCode = enMiJingRetCode_NoSpace;

	//	}else{
	//		Rsp.m_RetCode = (enMiJingRetCode)__UpGhost(pActor,pCommonGoods);

	//	}
	//}else if(starLevel == 80){
	//	//已达最高等级
	//	Rsp.m_RetCode = enMiJingRetCode_ErrMaxLevel;
	//}else{
	//	if( pActor->GetCrtProp(enCrtProp_GhostSoul) < pUpGhost->m_GhostSoul){
	//		//灵魄不足
	//		Rsp.m_RetCode = enMiJingRetCode_NoGhostSoul;

	//	}else if ( pPacketPart->GetSpace() < 1){
	//		//背包空间不足
	//		Rsp.m_RetCode = enMiJingRetCode_NoSpace;

	//	}else{
	//		Rsp.m_RetCode = (enMiJingRetCode)__UpGhost(pActor,pCommonGoods);
	//	}	
	//}
	
	OBuffer1k ob;

	ob << MiJing_Header(enMiJingCmd_UpGhost,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());
}

//灵件升级结果码
INT32 MiJingMgr::__UpGhost(IActor* pActor,ICommonGoods * pCommonGoods)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}
	const SGhostProp * pGhostProp = (SGhostProp *)pCommonGoods->GetExtendData();

	int starLevel = pGhostProp->m_GhostLevel;

	const SUpGhostCnfg * pUpGhost = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(pCommonGoods->GetGoodsID(),starLevel);

	if(pUpGhost == 0)
	{
		TRACE("<warning> %s : %d 行，灵件升级配置找不到！！　物品ID　＝　%d", __FUNCTION__,__LINE__, pCommonGoods->GetGoodsID());
		return enMiJingRetCode_NotTalisman;
	}


	if(pUpGhost->m_NextGhostID == 0){
		//已达最高等级
		return enMiJingRetCode_ErrMaxLevel;

	}else if( pActor->GetCrtProp(enCrtProp_GhostSoul) < pUpGhost->m_GhostSoul){
		//灵魄不足
		return enMiJingRetCode_NoGhostSoul;

	}else if ( pPacketPart->GetSpace() < 1){
		//背包空间不足
		return enMiJingRetCode_NoSpace;

	}

	if(starLevel == 20){
		if(!pPacketPart->HaveGoods(pUpGhost->m_Material, pUpGhost->m_MaterialNum)){
				//材料不足
				return enMiJingRetCode_NoMaterial;

			}
	}

	if(pUpGhost->m_GhostID == pUpGhost->m_NextGhostID){
	

	   SGhostProp GhostProp;

	   GhostProp.m_GhostLevel = starLevel+1;

	   pCommonGoods->SetExtendLen(&GhostProp,sizeof(GhostProp));

	   pActor->NoticClientUpdateThing(pCommonGoods->GetUID(),SGhostProp::enPropID_GhostLevel,GhostProp.m_GhostLevel);

		
		//扣除灵魄
		pActor->AddCrtPropNum(enCrtProp_GhostSoul, -pUpGhost->m_GhostSoul);

	}else{
			//每20级变为另一种灵件
			SCreateGoodsContext CreateGoodsCnt;
			CreateGoodsCnt.m_GoodsID = pUpGhost->m_NextGhostID;
			CreateGoodsCnt.m_Number = 1;
			CreateGoodsCnt.m_Binded = true;
			IGoods * pNewGoods = g_pGameServer->GetGameWorld()->CreateGoods(CreateGoodsCnt);
			ICommonGoods * pNewCommonGoods = (ICommonGoods *)pNewGoods;

			IPacketPart * pPacketPart = pActor->GetPacketPart();

			if(false == pPacketPart->AddGoods(pNewGoods->GetUID())){
				return enMiJingRetCode_NoSpace;
			}
			else
			{			
				g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Compose,pUpGhost->m_NextGhostID,UID(),1,"灵件升级");
				//灵件升级
				pNewCommonGoods->SetPropNum(pActor,enGoodsProp_GhostLevel, starLevel + 1);
				//扣除
				pActor->AddCrtPropNum(enCrtProp_GhostSoul, -pUpGhost->m_GhostSoul);

				pPacketPart->DestroyGoods(pUpGhost->m_Material, pUpGhost->m_MaterialNum);
		
				g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pUpGhost->m_Material,UID(),pUpGhost->m_MaterialNum,"灵件升级扣除材料");
				
				UID uidGoods = pCommonGoods->GetUID();
				pPacketPart->DestroyGoods(uidGoods);
				g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pUpGhost->m_GhostID,uidGoods,1,"灵件扣除材料");
			}
	}

	//发事件
	SS_UpGhost Ghostup;

	Ghostup.Level = starLevel + 1;

	UINT32 msgID2 = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpGhost);
	pActor->OnEvent(msgID2,&Ghostup,sizeof(Ghostup));

	return enMiJingRetCode_OK;
}

//爆击升级
INT32 MiJingMgr::CritLevelUp(IActor* pActor,IActor * pTargetActor)
{
	
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SCreditMagicCnfg * pCreditMagicOld = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorCritLv),enMagicType_Crit);
	if( 0 == pCreditMagicOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SCreditMagicCnfg * pCreditMagicNew = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorCritLv) + 1,enMagicType_Crit);
	if( 0 == pCreditMagicNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorCredit) < pCreditMagicNew->m_CreditNum){
		//声望不足
		return enMiJingRetCode_NoCredit;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pCreditMagicNew->m_PolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else{
		//技能升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_ActorCritLv, pTargetActor->GetCrtProp(enCrtProp_ActorCritLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_ActorCrit,pCreditMagicNew->m_AddMagicNum-pCreditMagicOld->m_AddMagicNum);


		//扣除声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, -pCreditMagicNew->m_CreditNum);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pCreditMagicNew->m_PolyNimbusNum);

		//发布事件
		SS_UpCreditMagicLv UpCreditMagicLv;
		UpCreditMagicLv.m_CreditMagicLv = pTargetActor->GetCrtProp(enCrtProp_ActorCritLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpCreditMagicLv);
		pActor->OnEvent(msgID,&UpCreditMagicLv,sizeof(UpCreditMagicLv));
	}

	return enMiJingRetCode_OK;	
}

//坚韧升级
INT32 MiJingMgr::TenacityLevelUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SCreditMagicCnfg * pCreditMagicOld = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorTenacityLv),enMagicType_Tenacity);
	if( 0 == pCreditMagicOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SCreditMagicCnfg * pCreditMagicNew = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorTenacityLv) + 1,enMagicType_Tenacity);
	if( 0 == pCreditMagicNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorCredit) < pCreditMagicNew->m_CreditNum){
		//声望不足
		return enMiJingRetCode_NoCredit;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pCreditMagicNew->m_PolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else{
		//技能升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_ActorTenacityLv, pTargetActor->GetCrtProp(enCrtProp_ActorTenacityLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_ActorTenacity,pCreditMagicNew->m_AddMagicNum-pCreditMagicOld->m_AddMagicNum);

		//扣除声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, -pCreditMagicNew->m_CreditNum);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pCreditMagicNew->m_PolyNimbusNum);

		//发布事件
		SS_UpCreditMagicLv UpCreditMagicLv;
		UpCreditMagicLv.m_CreditMagicLv = pTargetActor->GetCrtProp(enCrtProp_ActorTenacityLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpCreditMagicLv);
		pActor->OnEvent(msgID,&UpCreditMagicLv,sizeof(UpCreditMagicLv));
	}

	return enMiJingRetCode_OK;		
}

//命中升级
INT32 MiJingMgr::HitLevelUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SCreditMagicCnfg * pCreditMagicOld = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorHitLv),enMagicType_Hit);
	if( 0 == pCreditMagicOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SCreditMagicCnfg * pCreditMagicNew = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorHitLv) + 1,enMagicType_Hit);
	if( 0 == pCreditMagicNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorCredit) < pCreditMagicNew->m_CreditNum){
		//声望不足
		return enMiJingRetCode_NoCredit;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pCreditMagicNew->m_PolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else{
		//技能升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_ActorHitLv, pTargetActor->GetCrtProp(enCrtProp_ActorHitLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_ActorHit,pCreditMagicNew->m_AddMagicNum-pCreditMagicOld->m_AddMagicNum);

		//扣除声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, -pCreditMagicNew->m_CreditNum);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pCreditMagicNew->m_PolyNimbusNum);

		//发布事件
		SS_UpCreditMagicLv UpCreditMagicLv;
		UpCreditMagicLv.m_CreditMagicLv = pTargetActor->GetCrtProp(enCrtProp_ActorHitLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpCreditMagicLv);
		pActor->OnEvent(msgID,&UpCreditMagicLv,sizeof(UpCreditMagicLv));
	}

	return enMiJingRetCode_OK;
}

//回避升级
INT32 MiJingMgr::DodgeLevelUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SCreditMagicCnfg * pCreditMagicOld = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorDodgeLv),enMagicType_Dodge);
	if( 0 == pCreditMagicOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SCreditMagicCnfg * pCreditMagicNew = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorDodgeLv) + 1,enMagicType_Dodge);
	if( 0 == pCreditMagicNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorCredit) < pCreditMagicNew->m_CreditNum){
		//声望不足
		return enMiJingRetCode_NoCredit;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pCreditMagicNew->m_PolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else{
		//技能升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_ActorDodgeLv, pTargetActor->GetCrtProp(enCrtProp_ActorDodgeLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_ActorDodge,pCreditMagicNew->m_AddMagicNum-pCreditMagicOld->m_AddMagicNum);

		//扣除声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, -pCreditMagicNew->m_CreditNum);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pCreditMagicNew->m_PolyNimbusNum);

		//发布事件
		SS_UpCreditMagicLv UpCreditMagicLv;
		UpCreditMagicLv.m_CreditMagicLv = pTargetActor->GetCrtProp(enCrtProp_ActorDodgeLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpCreditMagicLv);
		pActor->OnEvent(msgID,&UpCreditMagicLv,sizeof(UpCreditMagicLv));
	}

	return enMiJingRetCode_OK;
}

//法术回复升级
INT32 MiJingMgr::MagicCDLevelUp(IActor* pActor,IActor * pTargetActor)
{
	
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SCreditMagicCnfg * pCreditMagicOld = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_MagicCDLv),enMagicType_MagicCD);
	if( 0 == pCreditMagicOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SCreditMagicCnfg * pCreditMagicNew = g_pGameServer->GetConfigServer()->GetCreditMagicCnfg(pTargetActor->GetCrtProp(enCrtProp_MagicCDLv) + 1,enMagicType_MagicCD);
	if( 0 == pCreditMagicNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorCredit) < pCreditMagicNew->m_CreditNum){
		//声望不足
		return enMiJingRetCode_NoCredit;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pCreditMagicNew->m_PolyNimbusNum){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else{
		//技能升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_MagicCDLv, pTargetActor->GetCrtProp(enCrtProp_MagicCDLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}


		pTargetActor->AddCrtPropNum(enCrtProp_MagicCD,pCreditMagicNew->m_AddMagicNum-pCreditMagicOld->m_AddMagicNum);

		//扣除声望
		pActor->AddCrtPropNum(enCrtProp_ActorCredit, -pCreditMagicNew->m_CreditNum);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pCreditMagicNew->m_PolyNimbusNum);

		//发布事件
		SS_UpCreditMagicLv UpCreditMagicLv;
		UpCreditMagicLv.m_CreditMagicLv = pTargetActor->GetCrtProp(enCrtProp_MagicCDLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpCreditMagicLv);
		pActor->OnEvent(msgID,&UpCreditMagicLv,sizeof(UpCreditMagicLv));
	}

	return enMiJingRetCode_OK;
}

//升级声望技能
void MiJingMgr::CreditMagicUp(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_CreditMagicUp_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.uid_Actor);
	if( 0 == pTargetActor){
		return;
	}


	SC_CreditMagicUp_Rsp Rsp;

	if( Req.m_MagicType == enMagicType_Crit)
	{
		Rsp.m_RetCode = (enMiJingRetCode)CritLevelUp(pActor,pTargetActor);
	}
	else if( Req.m_MagicType == enMagicType_Tenacity)
	{
		Rsp.m_RetCode = (enMiJingRetCode)TenacityLevelUp(pActor,pTargetActor);
	}
	else if(Req.m_MagicType == enMagicType_Hit)
	{
		Rsp.m_RetCode = (enMiJingRetCode)HitLevelUp(pActor,pTargetActor);
	}
	else if( Req.m_MagicType == enMagicType_Dodge)
	{
		Rsp.m_RetCode = (enMiJingRetCode)DodgeLevelUp(pActor,pTargetActor);
	}
	else if(Req.m_MagicType == enMagicType_MagicCD)
	{
		Rsp.m_RetCode = (enMiJingRetCode)MagicCDLevelUp(pActor,pTargetActor);
	}
	else
	{
		Rsp.m_RetCode = enMiJingRetCode_ErrType;
	}

	OBuffer1k ob;

	ob << MiJing_Header(enMiJingCmd_UpCreditSkill,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());	
}



//金剑诀升级
INT32 MiJingMgr::GoldDamageUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SDamageLevelCnfg * pDamageLevelOld = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_GoldDamageLv),enGoldDamage);
	if( 0 == pDamageLevelOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SDamageLevelCnfg * pDamageLevelNew = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_GoldDamageLv) + 1,enGoldDamage);
	if( 0 == pDamageLevelNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus) < pDamageLevelNew->m_SwordNimbus){
		//仙剑灵气不足
		return enMiJingRetCode_NoNimbus;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pDamageLevelNew->m_PolyNimbus){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else if( !pPacketPart->HaveGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1])){
		//剑魂碎片不足
		return enMiJingRetCode_NoGoods;
	}else{
		//剑诀升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_GoldDamageLv, pTargetActor->GetCrtProp(enCrtProp_GoldDamageLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_GoldDamage,pDamageLevelNew->m_Damage - pDamageLevelOld->m_Damage);

		//扣除仙剑灵气
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pDamageLevelNew->m_SwordNimbus);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pDamageLevelNew->m_PolyNimbus);

		pPacketPart->DestroyGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1]);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pDamageLevelNew->m_vectGoods[0],UID(),pDamageLevelNew->m_vectGoods[1],"剑诀升级消耗材料");

		//发布事件
		SS_UpDamageLv UpDamageLv;
		UpDamageLv.m_DamageLevel = pTargetActor->GetCrtProp(enCrtProp_GoldDamageLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpDamageLv);
		pActor->OnEvent(msgID,&UpDamageLv,sizeof(UpDamageLv));
	}

	return enMiJingRetCode_OK;	
}

//木剑诀升级
INT32 MiJingMgr::WoodDamageUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SDamageLevelCnfg * pDamageLevelOld = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_WoodDamageLv),enWoodDamage);
	if( 0 == pDamageLevelOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SDamageLevelCnfg * pDamageLevelNew = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_WoodDamageLv) + 1,enWoodDamage);
	if( 0 == pDamageLevelNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus) < pDamageLevelNew->m_SwordNimbus){
		//仙剑灵气不足
		return enMiJingRetCode_NoNimbus;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pDamageLevelNew->m_PolyNimbus){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else if( !pPacketPart->HaveGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1])){
		//剑魂碎片不足
		return enMiJingRetCode_NoGoods;
	}else{
		//剑诀升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_WoodDamageLv, pTargetActor->GetCrtProp(enCrtProp_WoodDamageLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_WoodDamage,pDamageLevelNew->m_Damage - pDamageLevelOld->m_Damage);

		//扣除仙剑灵气
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pDamageLevelNew->m_SwordNimbus);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pDamageLevelNew->m_PolyNimbus);

		pPacketPart->DestroyGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1]);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pDamageLevelNew->m_vectGoods[0],UID(),pDamageLevelNew->m_vectGoods[1],"剑诀升级消耗材料");

		//发布事件
		SS_UpDamageLv UpDamageLv;
		UpDamageLv.m_DamageLevel = pTargetActor->GetCrtProp(enCrtProp_WoodDamageLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpDamageLv);
		pActor->OnEvent(msgID,&UpDamageLv,sizeof(UpDamageLv));
	}

	return enMiJingRetCode_OK;
}

//水剑诀升级
INT32 MiJingMgr::WaterDamageUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SDamageLevelCnfg * pDamageLevelOld = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_WaterDamageLv),enWaterDamage);
	if( 0 == pDamageLevelOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SDamageLevelCnfg * pDamageLevelNew = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_WaterDamageLv) + 1,enWaterDamage);
	if( 0 == pDamageLevelNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus) < pDamageLevelNew->m_SwordNimbus){
		//仙剑灵气不足
		return enMiJingRetCode_NoNimbus;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pDamageLevelNew->m_PolyNimbus){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else if( !pPacketPart->HaveGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1])){
		//剑魂碎片不足
		return enMiJingRetCode_NoGoods;
	}else{
		//剑诀升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_WaterDamageLv, pTargetActor->GetCrtProp(enCrtProp_WaterDamageLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_WaterDamage,pDamageLevelNew->m_Damage - pDamageLevelOld->m_Damage);

		//扣除仙剑灵气
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pDamageLevelNew->m_SwordNimbus);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pDamageLevelNew->m_PolyNimbus);

		pPacketPart->DestroyGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1]);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pDamageLevelNew->m_vectGoods[0],UID(),pDamageLevelNew->m_vectGoods[1],"剑诀升级消耗材料");

		//发布事件
		SS_UpDamageLv UpDamageLv;
		UpDamageLv.m_DamageLevel = pTargetActor->GetCrtProp(enCrtProp_WaterDamageLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpDamageLv);
		pActor->OnEvent(msgID,&UpDamageLv,sizeof(UpDamageLv));
	}

	return enMiJingRetCode_OK;
}

//火剑诀升级
INT32 MiJingMgr::FireDamageUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SDamageLevelCnfg * pDamageLevelOld = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_FireDamageLv),enFireDamage);
	if( 0 == pDamageLevelOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SDamageLevelCnfg * pDamageLevelNew = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_FireDamageLv) + 1,enFireDamage);
	if( 0 == pDamageLevelNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus) < pDamageLevelNew->m_SwordNimbus){
		//仙剑灵气不足
		return enMiJingRetCode_NoNimbus;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pDamageLevelNew->m_PolyNimbus){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else if( !pPacketPart->HaveGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1])){
		//剑魂碎片不足
		return enMiJingRetCode_NoGoods;
	}else{
		//剑诀升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_FireDamageLv, pTargetActor->GetCrtProp(enCrtProp_FireDamageLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_FireDamage,pDamageLevelNew->m_Damage - pDamageLevelOld->m_Damage);

		//扣除仙剑灵气
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pDamageLevelNew->m_SwordNimbus);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pDamageLevelNew->m_PolyNimbus);

		pPacketPart->DestroyGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1]);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pDamageLevelNew->m_vectGoods[0],UID(),pDamageLevelNew->m_vectGoods[1],"剑诀升级消耗材料");

		//发布事件
		SS_UpDamageLv UpDamageLv;
		UpDamageLv.m_DamageLevel = pTargetActor->GetCrtProp(enCrtProp_FireDamageLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpDamageLv);
		pActor->OnEvent(msgID,&UpDamageLv,sizeof(UpDamageLv));
	}

	return enMiJingRetCode_OK;
}

//土剑诀升级
INT32 MiJingMgr::SoilDamageUp(IActor* pActor,IActor * pTargetActor)
{

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enMiJingRetCode_ErrPacket;
	}

	const SDamageLevelCnfg * pDamageLevelOld = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_SoilDamageLv),enSoilDamage);
	if( 0 == pDamageLevelOld){
		return enMiJingRetCode_ErrCnfg;
	}

	const SDamageLevelCnfg * pDamageLevelNew = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pTargetActor->GetCrtProp(enCrtProp_SoilDamageLv) + 1,enSoilDamage);
	if( 0 == pDamageLevelNew){
		//已到达最高等级
		return enMiJingRetCode_ErrMaxLevel;
	}else if( pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus) < pDamageLevelNew->m_SwordNimbus){
		//仙剑灵气不足
		return enMiJingRetCode_NoNimbus;	
	}else if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pDamageLevelNew->m_PolyNimbus){
		//聚灵气不足
		return enMiJingRetCode_NoPolyNimbus;
	}else if( !pPacketPart->HaveGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1])){
		//剑魂碎片不足
		return enMiJingRetCode_NoGoods;
	}else{
		//剑诀升级
		if(false == pTargetActor->SetCrtProp(enCrtProp_SoilDamageLv, pTargetActor->GetCrtProp(enCrtProp_SoilDamageLv) + 1)){
			return enMiJingRetCode_UpFail;
			
		}

		pTargetActor->AddCrtPropNum(enCrtProp_SoilDamage,pDamageLevelNew->m_Damage - pDamageLevelOld->m_Damage);

		//扣除仙剑灵气
		pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pDamageLevelNew->m_SwordNimbus);

		//扣除聚灵气
		pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pDamageLevelNew->m_PolyNimbus);

		pPacketPart->DestroyGoods(pDamageLevelNew->m_vectGoods[0], pDamageLevelNew->m_vectGoods[1]);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pDamageLevelNew->m_vectGoods[0],UID(),pDamageLevelNew->m_vectGoods[1],"剑诀升级消耗材料");

		//发布事件
		SS_UpDamageLv UpDamageLv;
		UpDamageLv.m_DamageLevel = pTargetActor->GetCrtProp(enCrtProp_SoilDamageLv);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpDamageLv);
		pActor->OnEvent(msgID,&UpDamageLv,sizeof(UpDamageLv));
	}

	return enMiJingRetCode_OK;
}

//剑诀升级
void MiJingMgr::DamageUp(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_DamageUp_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.uid_Actor);
	if( 0 == pTargetActor){
		return;
	}

	UINT8	SecretLevel = 0;

	SC_DamageUp_Rsp Rsp;

	if( Req.m_DamageCategory == enGoldDamage)
	{
		Rsp.m_RetCode = (enMiJingRetCode)GoldDamageUp(pActor,pTargetActor);

		SecretLevel = pActor->GetCrtProp(enCrtProp_GoldDamageLv);
	}
	else if( Req.m_DamageCategory == enWoodDamage)
	{
		Rsp.m_RetCode = (enMiJingRetCode)WoodDamageUp(pActor,pTargetActor);

		SecretLevel = pActor->GetCrtProp(enCrtProp_WoodDamageLv);
	}
	else if(Req.m_DamageCategory == enWaterDamage)
	{
		Rsp.m_RetCode = (enMiJingRetCode)WaterDamageUp(pActor,pTargetActor);

		SecretLevel = pActor->GetCrtProp(enCrtProp_WaterDamageLv);
	}
	else if( Req.m_DamageCategory == enFireDamage)
	{
		Rsp.m_RetCode = (enMiJingRetCode)FireDamageUp(pActor,pTargetActor);

		SecretLevel = pActor->GetCrtProp(enCrtProp_FireDamageLv);
	}
	else if(Req.m_DamageCategory == enSoilDamage)
	{
		Rsp.m_RetCode = (enMiJingRetCode)SoilDamageUp(pActor,pTargetActor);

		SecretLevel = pActor->GetCrtProp(enCrtProp_SoilDamageLv);
	}
	else
	{
		Rsp.m_RetCode = enMiJingRetCode_ErrType;
	}

	OBuffer1k ob;

	ob << MiJing_Header(enMiJingCmd_UpSwordMigic,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

	if ( enMiJingRetCode_OK == Rsp.m_RetCode )
	{
		SS_UserUpSecretLevel UpSecret;

		UpSecret.Level = SecretLevel;

		UINT32 msgID2 = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UserUpSecretLevel);
		pActor->OnEvent(msgID2,&UpSecret,sizeof(UpSecret));
	}
}


//装备制作
void MiJingMgr::EquipMake(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_EquipMake_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(Req.m_uidEquip);
	if(pGoods == 0)
	{
		return ;
	}

	TGoodsID GoodsID = pGoods->GetGoodsID();

	IEquipment * pEquipment = (IEquipment *)pGoods;
	
	int StarLevel = 0;
	pEquipment->GetPropNum(enGoodsProp_StarLevel,StarLevel);

	SC_UpGhost_Rsp Rsp;

	const SEquipMakeCnfg * pEquipMake = g_pGameServer->GetConfigServer()->GetEquipMakeCnfg(GoodsID);
	if( 0 == pEquipMake){
		TRACE("<warning> %s : %d 行，装备制作配置表找不到！！　物品ID　＝　%d", __FUNCTION__,__LINE__, GoodsID);
		return ;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return ;
	}



	if( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < pEquipMake->m_PolyNimbus){
		//灵魄不足
		Rsp.m_RetCode = enMiJingRetCode_NoPolyNimbus;

	}else if(!pPacketPart->HaveGoods(pEquipMake->m_MapGoodsID, pEquipMake->m_MapNum)){
		//图纸不足
		Rsp.m_RetCode = enMiJingRetCode_NoMap;

	}else if ( pPacketPart->GetSpace() < 1){
		//背包空间不足
		Rsp.m_RetCode = enMiJingRetCode_NoSpace;

	}else{


		//移除
		//pPacketPart->RemoveGoods(Req.m_uidMap);

		//给玩家制作出的装备
		SCreateGoodsContext CreateGoodsCnt;
		CreateGoodsCnt.m_GoodsID = pEquipMake->m_NewGoodsID;
		CreateGoodsCnt.m_Number = 1;
		CreateGoodsCnt.m_Binded = true;
		IGoods * pNewGoods = g_pGameServer->GetGameWorld()->CreateGoods(CreateGoodsCnt);
		IEquipment * pNewEquipment = (IEquipment *)pNewGoods;
		int GemID = 0;
		pNewEquipment->SetPropNum(pActor,enGoodsProp_StarLevel,StarLevel);
		for(int i = 0; i < MAX_INLAY_NUM; i++)
		{
			pEquipment->GetPropNum((enGoodsProp)(enGoodsProp_Gem_ID1 + i), GemID);
			pNewEquipment->SetPropNum(pActor,(enGoodsProp)(enGoodsProp_Gem_ID1 + i),GemID);	
		}

		if(false == pPacketPart->AddGoods(pNewEquipment->GetUID()))
			return;
		else
		{
		
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Compose,pEquipMake->m_NewGoodsID,UID(),1,"制作装备");

			//扣除
			pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -pEquipMake->m_PolyNimbus);

			pPacketPart->DestroyGoods(pEquipMake->m_MapGoodsID, pEquipMake->m_MapNum);
			//g_pGameServer->GetGameWorld()->DestroyThing(Req.m_uidMap);
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pEquipMake->m_MapGoodsID,UID(),pEquipMake->m_MapNum,"装备制作扣除材料");



			//装备在哪个角色身上
			IActor * pTargetActor = g_pGameServer->GetGoodsServer()->GetActorOnEquip(pActor, pEquipment);
			if( 0 != pTargetActor)
			{
				pPacketPart->Equip(pNewEquipment->GetUID(),Req.m_uidActor,Req.m_Pos);
			}

			
			//pPacketPart->DestroyGoods(pEquipMake->m_GoodsID, 1);
			pPacketPart->DestroyGoods(Req.m_uidEquip);
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pEquipMake->m_GoodsID,Req.m_uidEquip,1,"装备制作扣除材料");

		}
		Rsp.m_RetCode = enMiJingRetCode_OK;
	}
	OBuffer1k ob;

	ob << MiJing_Header(enMiJingCmd_EquipMake,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());
}


void MiJingMgr::OnEvent(XEventData & EventData)
{
	SS_ActoreCreateContext * pCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;
	if( 0 == pCreateContext){
		return;
	}

	IActor * pCreater = g_pGameServer->GetGameWorld()->FindActor(UID(pCreateContext->m_uidActor));
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_ActorLogin);

	const std::map<UINT16, SEquipMakeCnfg> * pAllEquipMakeCnfg = g_pGameServer->GetConfigServer()->GetAllEquipMakeCnfg();
	if ( 0 == pAllEquipMakeCnfg ){
		return ;
	}


	const std::hash_map<TGoodsID,	std::vector<SUpGhostCnfg>> * pAllUpGhostCnfg = g_pGameServer->GetConfigServer()->GetAllUpGhostCnfg();
	if ( 0 == pAllUpGhostCnfg ){
		return;
	}

	const std::hash_map<TGoodsID,	SGoodsComposeCnfg> * pAllGoodsComposeCnfg = g_pGameServer->GetConfigServer()->GetAllGoodsComposeCnfg();
	if ( 0 == pAllGoodsComposeCnfg ){
		return;
	}

	if( EventData.m_MsgID == msgID)
	{
		std::map<UINT16, SEquipMakeCnfg>::const_iterator iter = pAllEquipMakeCnfg->begin();

		for ( ; iter != pAllEquipMakeCnfg->end();++iter )
		{
			SEquipMakeCnfg  EMCnfg = iter->second;
			pCreater->SendGoodsCnfg(EMCnfg.m_NewGoodsID);
			pCreater->SendGoodsCnfg(EMCnfg.m_MapGoodsID);
		}


		std::hash_map<TGoodsID,	std::vector<SUpGhostCnfg>>::const_iterator iter1 = (*pAllUpGhostCnfg).begin();

		for ( ; iter1 != (*pAllUpGhostCnfg).end(); ++iter1 )
		{
			const std::vector<SUpGhostCnfg> & vectUGCnfg = iter1->second;
			for ( int i = 0; i < vectUGCnfg.size(); ++i )
			{
				
				if(vectUGCnfg[i].m_Material != 0){

					pCreater->SendGoodsCnfg(vectUGCnfg[i].m_Material);

				}
			}
			
		}

		std::hash_map<TGoodsID, SGoodsComposeCnfg>::const_iterator iter2 = pAllGoodsComposeCnfg->begin();

		for ( ; iter2 != pAllGoodsComposeCnfg->end(); ++iter2 )
		{
			SGoodsComposeCnfg  GCCnfg = iter2->second;

			pCreater->SendGoodsCnfg(GCCnfg.m_GoodsID);
			for(int i = 0; i + 1 < GCCnfg.m_vectMaterial.size(); i += 2)
			{
				pCreater->SendGoodsCnfg(GCCnfg.m_vectMaterial[i]);
			}

		}

	}
}
