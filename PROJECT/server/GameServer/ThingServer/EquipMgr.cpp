
#include "EquipMgr.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "EquipPart.h"
#include "IActor.h"
#include "IConfigServer.h"
#include "RandomService.h"
#include "IPacketPart.h"
#include "IMagicPart.h"
#include "ICombatPart.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "DMsgSubAction.h"
#include "ISynMagicPart.h"
#include "stdio.h"
#include <sstream>
#include "IGoodsServer.h"
#include "IStatusPart.h"
#include "IGodSword.h"


EquipMgr::EquipMgr()
{
}

EquipMgr::~EquipMgr()
{
}

bool EquipMgr::Create()
{
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Equip,this);
}

void EquipMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Equip,this);
}

//收到MSG_ROOT消息
void EquipMgr::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	typedef  void (EquipMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enEquipCmd_Max]=
	{
		NULL,
		NULL,
		& EquipMgr::RemoveEquip,	
		& EquipMgr::EquipStronger,
		& EquipMgr::EquipAutoStronger,
		& EquipMgr::EquipInlay,

		& EquipMgr::OpenMagic,
		& EquipMgr::AddMagic,
		& EquipMgr::RemoveMagic,
		NULL,
		& EquipMgr::UpgradeMagic,

		&EquipMgr::LineupSync,
		&EquipMgr::OnJoinBattle,				
		&EquipMgr::AddLayerLevel,
		&EquipMgr::OpenSynMagicPanel,
		&EquipMgr::ViewActorStatus,
		&EquipMgr::DeleteStatus,

		&EquipMgr::RemoveGem,
		&EquipMgr::ReplaceGem,
		&EquipMgr::StrongInheritance,

		&EquipMgr::OpenFacadePanel,      //打开外观栏
		&EquipMgr::SetShowFacade, //设置显示外观

		
	};

	if(nCmd>=enEquipCmd_Max || 0==s_funcProc[nCmd])
	{
		TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);
} 


//移除装备
void EquipMgr::RemoveEquip(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	
	CS_RemoveEquip_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pEmployee = pActor;

	if(pActor->GetUID() != Req.m_uidActor)
	{
		pEmployee = pActor->GetEmployee(Req.m_uidActor);

		if(pEmployee==0)
		{
			TRACE("<error> %s : %d Line 不存在的雇佣人员",__FUNCTION__,__LINE__ );
			return ;
		}
	}

	IEquipPart * pEquipPart = pEmployee->GetEquipPart();
	if(pEquipPart==0)
	{
		return ;
	}

	pEquipPart->RemoveEquip(Req.m_uidEquip);
}

//装备自动强化
void EquipMgr::__EquipAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req)
{
	SC_EquipAutoStronger_Rsp Rsp;

	IEquipment * pEquipment = (IEquipment *)pGoods;

	OBuffer1k ob;

	//检测是否满足条件
	this->__Check_CanEquipAutoStronger(pActor, pEquipment, Req, Rsp.m_Result);

	if( Rsp.m_Result != enEquipRetCode_OK){

		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_StarLevel,starLevel);

	EquipStrongerRet StrongerRet;

	//记录下消耗的原料和数量: 物品ID,数量,物品ID,数量.....
	std::vector<UINT16> vectMaterial;
	
	//强化总次数
	int TotalNum = 0;

	while( starLevel < Req.m_starLevel)
	{
		INT32 Result = this->__StrongerEquip(pActor,pEquipment,Req.m_bUseLockStone);

		if( Result != enEquipRetCode_OK && Result != enEquipRetCode_ProbFaild && Result != enEquipRetCode_NoMaterial){
			Rsp.m_Result  = (enEquipRetCode)Result;
			break;
		}

		std::string UseDesc;	//使用描述

		std::ostringstream os;

		if( Result == enEquipRetCode_NoMaterial){
			//少原料
						
			// fly add	20121106

			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10061) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);
			//os << "强化石不足，强化次数" << TotalNum << "次，消耗";		

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				// fly add	20121106
				os << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
				//os << (char *)pGoodsCnfg->m_szName << '*' << (int)vectMaterial[i + 1];
			}

			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}

		++TotalNum;

		INT32 stronglevel = 0;
		pEquipment->GetPropNum(enGoodsProp_StarLevel,stronglevel);

		if( Result == enEquipRetCode_ProbFaild){
			++stronglevel;
		}

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( stronglevel * 10 + enStrongerType_Equip);
		if( 0 == pStrongerCnfg){
			Rsp.m_Result = enEquipRetCode_LevelLimit;
			break;
		}

		StrongerRet.m_GoodsID = pStrongerCnfg->m_gidMaterial;

		bool bExist = false;
		for( int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( pStrongerCnfg->m_gidMaterial == vectMaterial[n]){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bExist = true;
				break;
			}
		}

		if( !bExist){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		StrongerRet.m_GoodsNum += pStrongerCnfg->m_MaterialNum;

		StrongerRet.m_StrongerNum++;

		Req.m_StrongerNum--;
		
		if( Result == enEquipRetCode_OK){				
			++starLevel;

			const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(StrongerRet.m_GoodsID);
			if( 0 == pGoodsCnfg){
				continue;
			}

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << starLevel<< g_pGameServer->GetGameWorld()->GetLanguageStr(10064) << StrongerRet.m_StrongerNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062) << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID)<< '*' << (int)StrongerRet.m_GoodsNum; 
			//os << "强化" << starLevel<< "级成功，强化次数" << StrongerRet.m_StrongerNum << "次，消耗" << (char *)pGoodsCnfg->m_szName << '*' << (int)StrongerRet.m_GoodsNum;

			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());

			StrongerRet.m_GoodsID     = INVALID_GOODS_ID;
			StrongerRet.m_GoodsNum    = 0;
			StrongerRet.m_StrongerNum = 0;

			////如果是未绑定的，变成绑定
			//int bBind = false;

			//pGoods->GetPropNum(enGoodsProp_Bind, bBind);

			//if ( !bBind )
			//{
			//	pGoods->SetPropNum(pActor, enGoodsProp_Bind, 1);
			//}
		}

		if( starLevel != Req.m_starLevel && Req.m_StrongerNum <= 0){
			//强化次数用完了
			std::string strStrongNum;	//使用描述
			std::ostringstream osStrongNum;
						
			// fly add	20121106

			osStrongNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << (int)Req.m_starLevel << g_pGameServer->GetGameWorld()->GetLanguageStr(10065) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);
			//osStrongNum << "强化" << (int)Req.m_starLevel << "级失败，强化次数" << TotalNum << "次，消耗";			

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				// fly add	20121106

				osStrongNum << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
				//osStrongNum << (char *)pGoodsCnfg->m_szName << '*' << (int)vectMaterial[i + 1];
			}

			strStrongNum = osStrongNum.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + strStrongNum.length() + 1) << Rsp;
			ob.Push(strStrongNum.c_str(), strStrongNum.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}
	}

	if(  Rsp.m_Result != enEquipRetCode_OK && Rsp.m_Result != enEquipRetCode_ProbFaild && Rsp.m_Result != enEquipRetCode_NoMaterial){
		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}
}

//法宝自动强化
void  EquipMgr::__TalismanAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req)
{
	SC_EquipAutoStronger_Rsp Rsp;

	IEquipment * pEquipment = (IEquipment *)pGoods;

	OBuffer1k ob;

	//检测是否满足条件
	this->__Check_CanTalismanAutoStronger(pActor, pEquipment, Req, Rsp.m_Result);

	if( Rsp.m_Result != enEquipRetCode_OK){

		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_MagicLevel,starLevel);

	bool bUseLockStone = Req.m_bUseLockStone;

	EquipStrongerRet StrongerRet;

	//记录下消耗的原料和数量: 物品ID,数量,物品ID,数量.....
	std::vector<UINT16> vectMaterial;
	
	//强化总次数
	UINT16 TotalNum = 0;

	while(starLevel < Req.m_starLevel)
	{
		INT32 Result = this->__StrongerTalisman(pActor,pEquipment,bUseLockStone);

		if( Result != enEquipRetCode_OK && Result != enEquipRetCode_ProbFaild && Result != enEquipRetCode_NoMaterial){
			Rsp.m_Result  = (enEquipRetCode)Result;
			break;
		}

		std::string UseDesc;	//使用描述

		std::ostringstream os;

		if( Result == enEquipRetCode_NoMaterial){
			//少原料
						
			// fly add	20121106

			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10061) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);	
			//os << "强化石不足，强化次数" << TotalNum << "次，消耗";		

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				// fly add	20121106
				os << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
				//os << (char *)pGoodsCnfg->m_szName << '*' << (int)vectMaterial[i + 1];
			}

			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}

		++TotalNum;

		INT32 stronglevel = 0;
		pEquipment->GetPropNum(enGoodsProp_MagicLevel,stronglevel);

		if( Result == enEquipRetCode_ProbFaild){
			++stronglevel;
		}

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( stronglevel * 10 + enStrongerType_Talisman);
		if( 0 == pStrongerCnfg){
			Rsp.m_Result = enEquipRetCode_LevelLimit;
			break;
		}

		StrongerRet.m_GoodsID = pStrongerCnfg->m_gidMaterial;

		bool bExist = false;
		for( int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( pStrongerCnfg->m_gidMaterial == vectMaterial[n]){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bExist = true;
				break;
			}
		}

		if( !bExist){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		StrongerRet.m_GoodsNum += pStrongerCnfg->m_MaterialNum;

		StrongerRet.m_StrongerNum++;

		Req.m_StrongerNum--;

		if( Result == enEquipRetCode_OK){		
			++starLevel;

			const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(StrongerRet.m_GoodsID);
			if( 0 == pGoodsCnfg){
				continue;
			}

			// fly add	20121106
	
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << starLevel<<  g_pGameServer->GetGameWorld()->GetLanguageStr(10064)<<StrongerRet.m_StrongerNum<< g_pGameServer->GetGameWorld()->GetLanguageStr(10062)<< (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)StrongerRet.m_GoodsNum;
			//os << "强化" << starLevel<< "级成功，强化次数" << StrongerRet.m_StrongerNum << "次，消耗" << (char *)pGoodsCnfg->m_szName << '*' << (int)StrongerRet.m_GoodsNum;

			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());

			StrongerRet.m_GoodsID     = INVALID_GOODS_ID;
			StrongerRet.m_GoodsNum    = 0;
			StrongerRet.m_StrongerNum = 0;

			////如果是未绑定的，变成绑定
			//int bBind = false;

			//pGoods->GetPropNum(enGoodsProp_Bind, bBind);

			//if ( !bBind )
			//{
			//	pGoods->SetPropNum(pActor, enGoodsProp_Bind, 1);
			//}

		}

		if( Req.m_StrongerNum <= 0 && starLevel != Req.m_starLevel){
			//强化次数用完了
			std::string strStrongNum;	//使用描述
			std::ostringstream osStrongNum;
						
			// fly add	20121106
		
			osStrongNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << (int)Req.m_starLevel <<g_pGameServer->GetGameWorld()->GetLanguageStr(10065) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);
			//osStrongNum << "强化" << (int)Req.m_starLevel << "级失败，强化次数" << TotalNum << "次，消耗";

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				// fly add	20121106
				osStrongNum << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
				//osStrongNum << (char *)pGoodsCnfg->m_szName << '*' << (int)vectMaterial[i + 1];
			}

			strStrongNum = osStrongNum.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + strStrongNum.length() + 1) << Rsp;
			ob.Push(strStrongNum.c_str(), strStrongNum.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}
	}

	if(  Rsp.m_Result != enEquipRetCode_OK && Rsp.m_Result != enEquipRetCode_ProbFaild && Rsp.m_Result != enEquipRetCode_NoMaterial){
		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}
}

//仙剑自动强化
void  EquipMgr::__GodSwordAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req)
{
	SC_EquipAutoStronger_Rsp Rsp;

	IEquipment * pEquipment = (IEquipment *)pGoods;

	OBuffer1k ob;

	//检测是否满足条件
	this->__Check_CanGodSwordAutoStronger(pActor, pEquipment, Req, Rsp.m_Result);

	if( Rsp.m_Result != enEquipRetCode_OK){

		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_SwordLevel,starLevel);

	bool bUseLockStone = Req.m_bUseLockStone;

	EquipStrongerRet StrongerRet;

	//记录下消耗的原料和数量: 物品ID,数量,物品ID,数量.....
	std::vector<UINT16> vectMaterial;
	
	//强化总次数
	UINT8 TotalNum = 0;

	while(starLevel < Req.m_starLevel)
	{
		INT32 Result = this->__StrongerGodSword(pActor,pEquipment,bUseLockStone);

		if( Result != enEquipRetCode_OK && Result != enEquipRetCode_ProbFaild && Result != enEquipRetCode_NoMaterial){
			Rsp.m_Result  = (enEquipRetCode)Result;
			break;
		}

		std::string UseDesc;	//使用描述

		std::ostringstream os;

		if( Result == enEquipRetCode_NoMaterial){
			//少原料
						
			// fly add	20121106

			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10061) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);	
			//os << "强化石不足，强化次数" << TotalNum << "次，消耗";		

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				// fly add	20121106
				os << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
				//os << (char *)pGoodsCnfg->m_szName << '*' << (int)vectMaterial[i + 1];
			}

			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}

		++TotalNum;

		INT32 stronglevel = 0;
		pEquipment->GetPropNum(enGoodsProp_SwordLevel,stronglevel);

		if( Result == enEquipRetCode_ProbFaild){
			++stronglevel;
		}

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( stronglevel * 10 + enStrongerType_GodSword);
		if( 0 == pStrongerCnfg){
			Rsp.m_Result = enEquipRetCode_LevelLimit;
			break;
		}

		StrongerRet.m_GoodsID = pStrongerCnfg->m_gidMaterial;

		bool bExist = false;
		for( int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( pStrongerCnfg->m_gidMaterial == vectMaterial[n]){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bExist = true;
				break;
			}
		}

		if( !bExist){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		StrongerRet.m_GoodsNum += pStrongerCnfg->m_MaterialNum;

		StrongerRet.m_StrongerNum++;

		Req.m_StrongerNum--;

		if( Result == enEquipRetCode_OK){		
			++starLevel;

			const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(StrongerRet.m_GoodsID);
			if( 0 == pGoodsCnfg){
				continue;
			}
	
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << starLevel<<  g_pGameServer->GetGameWorld()->GetLanguageStr(10064)<<StrongerRet.m_StrongerNum<< g_pGameServer->GetGameWorld()->GetLanguageStr(10062)<< (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)StrongerRet.m_GoodsNum;
			
			UseDesc = os.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + UseDesc.length() + 1) << Rsp;
			ob.Push(UseDesc.c_str(), UseDesc.length() + 1);

			pActor->SendData(ob.TakeOsb());

			StrongerRet.m_GoodsID     = INVALID_GOODS_ID;
			StrongerRet.m_GoodsNum    = 0;
			StrongerRet.m_StrongerNum = 0;

			////如果是未绑定的，变成绑定
			//int bBind = false;

			//pGoods->GetPropNum(enGoodsProp_Bind, bBind);

			//if ( !bBind )
			//{
			//	pGoods->SetPropNum(pActor, enGoodsProp_Bind, 1);
			//}

		}

		if( Req.m_StrongerNum <= 0 && starLevel != Req.m_starLevel){
			//强化次数用完了
			std::string strStrongNum;	//使用描述
			std::ostringstream osStrongNum;					
		
			osStrongNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10063) << (int)Req.m_starLevel <<g_pGameServer->GetGameWorld()->GetLanguageStr(10065) << TotalNum << g_pGameServer->GetGameWorld()->GetLanguageStr(10062);

			for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
			{
				const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectMaterial[i]);
				if( 0 == pGoodsCnfg){
					continue;
				}

				osStrongNum << (char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID) << '*' << (int)vectMaterial[i + 1];
			}

			strStrongNum = osStrongNum.str();

			ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp) + strStrongNum.length() + 1) << Rsp;
			ob.Push(strStrongNum.c_str(), strStrongNum.length() + 1);

			pActor->SendData(ob.TakeOsb());
			return;
		}
	}

	if(  Rsp.m_Result != enEquipRetCode_OK && Rsp.m_Result != enEquipRetCode_ProbFaild && Rsp.m_Result != enEquipRetCode_NoMaterial){
		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}
}


INT32 EquipMgr::__StrongerEquip(IActor* pActor,IEquipment * pEquipment,bool bAddExtender)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enEquipRetCode_Err;
	}

	int starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_StarLevel,starLevel);

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if( starLevel >= ConfigParam.m_EquipMaxStarLevel ){
		return enEquipRetCode_LevelLimit;
	}

	INT32 nextLevel = starLevel+1;
	const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( nextLevel * 10 + enStrongerType_Equip);
	if( 0 == pStrongerCnfg){
		TRACE("<error> %s : %d line 找不到强化配置信息,nextLevel=%d",__FUNCTION__,__LINE__,nextLevel);
		return enEquipRetCode_LevelLimit;
	}

	if( false == pPacketPart->HaveGoods(pStrongerCnfg->m_gidMaterial,pStrongerCnfg->m_MaterialNum)){
		//少原料
		return enEquipRetCode_NoMaterial;
	}

	if( true == bAddExtender && !pPacketPart->HaveGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	const UINT32 base_num = 1000; //基数

	//成功率
	INT32 SuccessRate = 0;

	//获取随机数
	INT32 nRandom = 0;

	if( !pActor->GetUseFlag(enUseFlag_First_EquipStronger)){
		//第一次强化必然成功
		SuccessRate = base_num;
		//设成第一次强化必成功使用过了
		pActor->SetUseFlag(enUseFlag_First_EquipStronger, true);
	}else{
		SuccessRate = pStrongerCnfg->m_SuccessRate;
		if( bAddExtender){
			SuccessRate+=pStrongerCnfg->m_AddSuccessRate;
		}
		
		//帮派福利
		ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(pActor->GetUID()); 
		if( 0 != pSyndicate){
			//百分比转化为千分比需要*10
			SuccessRate += pSyndicate->GetWelfareValue(enWelfare_StrongRandom)*10;
		}

		SuccessRate += pActor->GetVipValue(enVipType_AddStrongerRate) * 10;
		
		//获取随机数
		nRandom = RandomService::GetRandom()%base_num;
	}

	//扣原料
	if( pPacketPart->DestroyGoods(pStrongerCnfg->m_gidMaterial,pStrongerCnfg->m_MaterialNum)==false){
		return enEquipRetCode_NoMaterial;
	}

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidMaterial,UID(),pStrongerCnfg->m_MaterialNum,"强化装备扣原料");

	if( bAddExtender && !pPacketPart->DestroyGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	if( bAddExtender){
		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidExtender,UID(),1,"强化装备扣幸运石");
	}

	if( nRandom < SuccessRate){
		//增加星级
		pEquipment->SetPropNum(pActor,enGoodsProp_StarLevel,nextLevel);
		
		//检测装备是否在装备栏上，是的话给玩家加属性
		this->__CheckInEquip_AddStrongerProp(pActor, pEquipment, nextLevel - 1, nextLevel);

		//发布事件
		SS_EquipStronger EquipStronger;
		EquipStronger.m_Category = pEquipment->GetGoodsClass();
		EquipStronger.m_LevelStronger = nextLevel;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EquipStronger);
			
		pActor->OnEvent(msgID,&EquipStronger,sizeof(EquipStronger));
	}else{
		//失败
		return enEquipRetCode_ProbFaild;
	}
	return enEquipRetCode_OK;
}

//强化法宝
INT32 EquipMgr::__StrongerTalisman(IActor* pActor,IEquipment * pEquipment,bool bAddExtender)
{
	//装备是否存在于背包中
	IPacketPart * pPacketPart = pActor->GetPacketPart();

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_MagicLevel,starLevel);

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if( starLevel >= ConfigParam.m_TalismanMaxStarLevel ){
		return enEquipRetCode_LevelLimit;
	}

	INT32 nextLevel = starLevel+1;
	const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( nextLevel * 10 + enStrongerType_Talisman);
	if( 0 == pStrongerCnfg){
		TRACE("<error> %s : %d line 找不到强化配置信息,nextLevel=%d",__FUNCTION__,__LINE__,nextLevel);
		return enEquipRetCode_LevelLimit;
	}

	if( false == pPacketPart->HaveGoods(pStrongerCnfg->m_gidMaterial, pStrongerCnfg->m_MaterialNum)){
		//少原料
		return enEquipRetCode_NoMaterial;
	}

	if( true == bAddExtender && !pPacketPart->HaveGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	const UINT32 base_num = 1000; //基数

	//成功率
	INT32 SuccessRate = 0;

	//获取随机数
	INT32 nRandom = 0;

	if( !pActor->GetUseFlag(enUseFlag_First_TalismanStronger)){
		//第一次强化必然成功
		SuccessRate = base_num;
		//设成第一次强化必成功使用过了
		pActor->SetUseFlag(enUseFlag_First_TalismanStronger, true);
	}else{
		SuccessRate = pStrongerCnfg->m_SuccessRate;
		if( bAddExtender){
			SuccessRate += pStrongerCnfg->m_AddSuccessRate;
		}

		//帮派福利
		ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(pActor->GetUID()); 
		if( 0 != pSyndicate){
			//百分比转化为千分比需要*10
			SuccessRate += pSyndicate->GetWelfareValue(enWelfare_TalismanStrongRandom) * 10;
		}

		//获取随机数
		nRandom = RandomService::GetRandom()%base_num;
	}

	//扣原料
	if( false == pPacketPart->DestroyGoods(pStrongerCnfg->m_gidMaterial,pStrongerCnfg->m_MaterialNum)){
		return enEquipRetCode_NoMaterial;
	}

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidMaterial,UID(),pStrongerCnfg->m_MaterialNum,"强化装备扣材料");

	if( bAddExtender && !pPacketPart->DestroyGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	if( bAddExtender){
		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidExtender,UID(),1,"强化法宝扣幸运石");
	}

	if(nRandom < SuccessRate){
		//增加星级
		pEquipment->SetPropNum(pActor,enGoodsProp_MagicLevel,nextLevel);

		//发布事件
		SS_EquipStronger EquipStronger;
		EquipStronger.m_Category = pEquipment->GetGoodsClass();
		EquipStronger.m_LevelStronger = nextLevel;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EquipStronger);

		pActor->OnEvent(msgID,&EquipStronger,sizeof(EquipStronger));
	}else{
		//失败
		return enEquipRetCode_ProbFaild;
	}

	return enEquipRetCode_OK;
}

//强化仙剑
INT32 EquipMgr::__StrongerGodSword(IActor* pActor,IEquipment * pEquipment,bool bAddExtender)
{
	//仙剑是否存在于背包中
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	IGodSword * pGodSword = (IGodSword *)pEquipment;
	INT32 nCurLevel = 0;
	pEquipment->GetPropNum(enGoodsProp_SwordLevel,nCurLevel);
	//上限
	INT32 nLevelUp = pGodSword->GetGoodsCnfg()->m_AvoidOrSwordLvMax;		
	if(nCurLevel >= nLevelUp)
	{
		return enEquipRetCode_LevelLimit;
	}

	INT32 nextLevel = nCurLevel+1;
	const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( nextLevel * 10 + enStrongerType_GodSword);
	if( 0 == pStrongerCnfg){
		TRACE("<error> %s : %d line 找不到强化配置信息,nextLevel=%d",__FUNCTION__,__LINE__,nextLevel);
		return enEquipRetCode_LevelLimit;
	}

	if( false == pPacketPart->HaveGoods(pStrongerCnfg->m_gidMaterial, pStrongerCnfg->m_MaterialNum)){
		//少原料
		return enEquipRetCode_NoMaterial;
	}

	if( true == bAddExtender && !pPacketPart->HaveGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	const UINT32 base_num = 1000; //基数

	//成功率
	INT32 SuccessRate = 0;

	//获取随机数
	INT32 nRandom = 0;

	if( !pActor->GetUseFlag(enUseFlag_First_GodSwordStronger)){
		//第一次强化必然成功
		SuccessRate = base_num;
		//设成第一次强化必成功使用过了
		pActor->SetUseFlag(enUseFlag_First_GodSwordStronger, true);
	}else{
		SuccessRate = pStrongerCnfg->m_SuccessRate;
		if( bAddExtender){
			SuccessRate += pStrongerCnfg->m_AddSuccessRate;
		}

/*		//帮派福利
		ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(pActor->GetUID()); 
		if( 0 != pSyndicate){
			//百分比转化为千分比需要*10
			SuccessRate += pSyndicate->GetWelfareValue(enWelfare_TalismanStrongRandom) * 10;
		}
*/
		//获取随机数
		nRandom = RandomService::GetRandom()%base_num;
	}

	//扣原料
	if( false == pPacketPart->DestroyGoods(pStrongerCnfg->m_gidMaterial,pStrongerCnfg->m_MaterialNum)){
		return enEquipRetCode_NoMaterial;
	}

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidMaterial,UID(),pStrongerCnfg->m_MaterialNum,"强化仙剑扣材料");

	if( bAddExtender && !pPacketPart->DestroyGoods(pStrongerCnfg->m_gidExtender,1)){
		return enEquipRetCode_NoExtender;
	}

	if( bAddExtender){
		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,pStrongerCnfg->m_gidExtender,UID(),1,"强化仙剑扣幸运石");
	}

	if(nRandom < SuccessRate){
		//增加等级		
		if(pGodSword->UpgradeSword(pActor) == false)
		{
			return enEquipRetCode_ProbFaild;
		}
		
		//pEquipment->SetPropNum(pActor,enGoodsProp_SwordLevel,nextLevel);

		//发布事件
		SS_EquipStronger EquipStronger;
		EquipStronger.m_Category = pEquipment->GetGoodsClass();
		EquipStronger.m_LevelStronger = nextLevel;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EquipStronger);

		pActor->OnEvent(msgID,&EquipStronger,sizeof(EquipStronger));
	}else{
		//失败
		return enEquipRetCode_ProbFaild;
	}

	return enEquipRetCode_OK;
}



//装备强化
void EquipMgr::EquipStronger(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_EquipStronger_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_TargetActor);
	if( 0 == pTargetActor){
		return;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	IGoods * pGoods = this->__GetStrongerGoods(pActor, Req.m_uidEquip);
	if( 0 == pGoods){
		return;
	}

	SC_EquipStronger_Rsp Rsp;

	if( pGoods->GetGoodsClass() == enGoodsCategory_Equip){
		Rsp.m_Result = (enEquipRetCode)__StrongerEquip(pActor,(IEquipment *)pGoods,Req.m_bUseLockStone);
	}else if( pGoods->GetGoodsClass() == enGoodsCategory_Talisman){
		Rsp.m_Result = (enEquipRetCode)__StrongerTalisman(pActor,(IEquipment *)pGoods,Req.m_bUseLockStone);
	}
	else if(pGoods->GetGoodsClass() == enGoodsCategory_GodSword)
	{
		Rsp.m_Result = (enEquipRetCode)__StrongerGodSword(pActor,(IEquipment *)pGoods,Req.m_bUseLockStone);
	}
	else{
		Rsp.m_Result = enEquipRetCode_UnEquip;
	}

	if (Rsp.m_Result == enEquipRetCode_OK){
		
		pPacketPart->SaveGoodsToDB(pGoods);

		////如果是未绑定的，变成绑定
		//int bBind = false;

		//pGoods->GetPropNum(enGoodsProp_Bind, bBind);

		//if ( !bBind )
		//{
		//	pGoods->SetPropNum(pActor, enGoodsProp_Bind, 1);
		//}
	}
	
	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_StrongerEquip,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

}

//装备自动强化
void EquipMgr::EquipAutoStronger(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_EquipAutoStronger_Req Req;

	ib >> Req;

	if(ib.Error()){
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_TargetActor);
	if( 0 == pTargetActor){
		return;
	}

	IGoods * pGoods = this->__GetStrongerGoods(pActor, Req.m_uidEquip);
	if( 0 == pGoods){
		return;
	}

	if(pGoods->GetGoodsClass() == enGoodsCategory_Equip){
		//装备强化
		this->__EquipAutoStronger(pActor, (IEquipment *)pGoods, Req);
	}else if(pGoods->GetGoodsClass() == enGoodsCategory_Talisman){
		//法宝强化
		this->__TalismanAutoStronger(pActor, (IEquipment *)pGoods, Req);
	}
	else if(pGoods->GetGoodsClass() == enGoodsCategory_GodSword)
	{
		//仙剑强化
		this->__GodSwordAutoStronger(pActor, (IEquipment *)pGoods, Req);
	}
	else{
		SC_EquipAutoStronger_Rsp Rsp;
		Rsp.m_Result = enEquipRetCode_UnEquip;

		OBuffer1k ob;
		ob << Equip_Header(enEquipCmd_AutoStronger,sizeof(Rsp)) << Rsp;

		pActor->SendData(ob.TakeOsb());
	}
}

//装备镶嵌
void EquipMgr::EquipInlay(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{

	CS_EquipInlay_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_EquipInlay_Rsp Rsp;

	Rsp.m_Result = enEquipRetCode_OK;

	//装备是否存在于背包中
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	IGoods * pGoods = this->__GetStrongerGoods(pActor, Req.m_uidEquip);
	if( 0 == pGoods){
		return;
	}

	IGoods * pGemGoods = pPacketPart->GetGoods(Req.m_uidGem);
	if( 0 == pGemGoods){
		return;
	}

	if(pGoods==0)
	{
		Rsp.m_Result = enEquipRetCode_NoEquip;
	}
	else if(pGoods->GetGoodsClass()!=enGoodsCategory_Equip)
	{
		Rsp.m_Result = enEquipRetCode_UnEquip;
	}
	else if(pGemGoods == 0)
	{
		Rsp.m_Result = enEquipRetCode_NoMaterial;
	}
	else
	{
		//先保存下该ID,下面宝石销毁后，还需要用该ID
		TGoodsID gidGem = pGemGoods->GetGoodsID();

		IEquipment * pEquipment = (IEquipment *)pGoods;

		//星级
		INT32 starLevel = 0;
		pEquipment->GetPropNum(enGoodsProp_StarLevel,starLevel);

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg(starLevel * 10 + enStrongerType_Equip);

		int OldGemID = INVALID_GOODS_ID;
		pEquipment->GetPropNum((enGoodsProp)(enGoodsProp_Gem_ID1+Req.m_Hole), OldGemID);

		if(pStrongerCnfg == 0 || Req.m_Hole >= pStrongerCnfg->m_HoleNum)
		{
			Rsp.m_Result = enEquipRetCode_NoHole;
		}
		else if ( OldGemID != INVALID_GOODS_ID )
		{
			Rsp.m_Result = enEquipRetCode_ExistHave;
		}
		else if(pPacketPart->DestroyGoods(gidGem)==false)
		{
			Rsp.m_Result = enEquipRetCode_NoMaterial;
		}
		else
		{
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Stronger,gidGem,UID(),1,"装备镶嵌扣除宝石");

			//成功
			pEquipment->SetPropNum(pActor,(enGoodsProp)(enGoodsProp_Gem_ID1+Req.m_Hole),gidGem);

			int NewGemID = 0;
			pEquipment->GetPropNum((enGoodsProp)(enGoodsProp_Gem_ID1+Req.m_Hole), NewGemID);

			pPacketPart->SaveGoodsToDB(pGoods);

			//镶嵌成功后，如果装备在玩家身上，则把属性加上去
			this->__CheckInEquip_AddInlayProp(pActor, pEquipment, OldGemID, NewGemID);

			////如果是未绑定的，变成绑定
			//int bBind = false;

			//pEquipment->GetPropNum(enGoodsProp_Bind, bBind);

			//if ( !bBind )
			//{
			//	pEquipment->SetPropNum(pActor, enGoodsProp_Bind, 1);
			//}

			//发布事件
			SS_EquipInlay EquipInlay;
			EquipInlay.m_Category = pGoods->GetGoodsClass();

			IEquipment * pEquipment = (IEquipment *)pGoods;

			int nCount = 0;

			int tmp = 0;

			pEquipment->GetPropNum(enGoodsProp_Gem_ID1, tmp);
			if( tmp != 0){
				++nCount;
				tmp = 0;
			}

			pEquipment->GetPropNum(enGoodsProp_Gem_ID2, tmp);
			if( tmp != 0){
				++nCount;
				tmp = 0;
			}

			pEquipment->GetPropNum(enGoodsProp_Gem_ID3, tmp);
			if( tmp != 0){
				++nCount;
			}

			EquipInlay.m_NumInlay = nCount;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EquipInlay);
			pActor->OnEvent(msgID,&EquipInlay,sizeof(EquipInlay));
		}
	}

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_Inlay,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

}

//打开法术栏
void EquipMgr::OpenMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMagicPart * pMagicPart = pActor->GetMagicPart();
	if(pMagicPart==0)
	{
		return ;
	}
	
	pMagicPart->OpenMagicPanel();
}

//加载法术
void EquipMgr::AddMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	
	CS_EquipAddMagic_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pEmployee = pActor;

	if(pActor->GetUID() != Req.m_uidActor)
	{
		pEmployee = pActor->GetEmployee(Req.m_uidActor);

		if(pEmployee==0)
		{
			TRACE("<error> %s : %d Line 不存在的雇佣人员",__FUNCTION__,__LINE__ );
			return ;
		}
	}

	IMagicPart * pMagicPart = pEmployee->GetMagicPart();
	if(pMagicPart==0)
	{
		return ;
	}

	pMagicPart->LoadMagic(Req.m_MagicID,Req.m_Pos);

}

//移除法术
void EquipMgr::RemoveMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{

	CS_EquipRemoveMagic_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pEmployee = pActor;

	if(pActor->GetUID() != Req.m_uidActor)
	{
		pEmployee = pActor->GetEmployee(Req.m_uidActor);

		if(pEmployee==0)
		{
			TRACE("<error> %s : %d Line 不存在的雇佣人员",__FUNCTION__,__LINE__ );
			return ;
		}
	}

	IMagicPart * pMagicPart = pEmployee->GetMagicPart();
	if(pMagicPart==0)
	{
		return ;
	}

	pMagicPart->UnloadMagic(Req.m_MagicID,Req.m_Pos);
}

//升级法术
void EquipMgr::UpgradeMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	
	CS_EquipUpgradeMagic_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pEmployee = pActor;

	if(pActor->GetUID() != Req.m_uidActor)
	{
		pEmployee = pActor->GetEmployee(Req.m_uidActor);

		if(pEmployee==0)
		{
			TRACE("<error> %s : %d Line 不存在的雇佣人员",__FUNCTION__,__LINE__ );
			return ;
		}
	}

	IMagicPart * pMagicPart = pEmployee->GetMagicPart();
	if(pMagicPart==0)
	{
		return ;
	}

	pMagicPart->UpgradeMagic(Req.m_MagicID);
}

	//同步了阵形
void EquipMgr::LineupSync(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if(pCombatPart==0)
	{
		return ;
	}

	pCombatPart->LineupSync();
}

//设置参战
void EquipMgr::OnJoinBattle(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if(pCombatPart==0)
	{
		return ;
	}

	CS_EquipJoinBattle_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pCombatPart->SetJoinBattleActor(Req);

}





//升级境界
void EquipMgr::AddLayerLevel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_AddLayerLevel_Req Req;
	ib >> Req;

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(UID(Req.uid_Actor));
	if( 0 == pTargetActor){
		return;
	}

	const SActorLayerCnfg * pLayerCnfoOld = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorLayer));
	if( 0 == pLayerCnfoOld){
		return;
	}



	SC_AddLayerLevel_Rsp  Rsp;

	const SActorLayerCnfg * pLayerCnfoNew = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorLayer) + 1);
	if( 0 == pLayerCnfoNew){
		//已到达最高境界
		Rsp.m_RetCode = enEquipRetCode_ErrMaxLayer;
	}else if( OPEN_MAX_LAYER < (pTargetActor->GetCrtProp(enCrtProp_ActorLayer) + 1)){
		//目前此级别境界尚未开放
		Rsp.m_RetCode = enEquipRetCode_ErrNotOpenLevel;	
	}else if( pTargetActor->GetCrtProp(enCrtProp_Level) < pLayerCnfoNew->m_NeedLevel){
		//玩家等级不足
		Rsp.m_RetCode = enEquipRetCode_ErrLevel;
	}else if( pTargetActor->GetCrtProp(enCrtProp_ActorNimbus) < pLayerCnfoNew->m_AddLevelDecNimbusNum){
		//玩家灵气不足
		Rsp.m_RetCode = enEquipRetCode_ErrNimbus;
	}else{
		if( false == pTargetActor->SetCrtProp(enCrtProp_ActorLayer, pTargetActor->GetCrtProp(enCrtProp_ActorLayer) + 1)){
			return;
		}

		//扣除灵气
		pTargetActor->AddCrtPropNum(enCrtProp_ActorNimbus, -pLayerCnfoNew->m_AddLevelDecNimbusNum);

		pTargetActor->AddCrtPropNum(enCrtProp_ActorNimbusSpeed, pLayerCnfoNew->m_NimbusSpeed - pLayerCnfoOld->m_NimbusSpeed);

		pTargetActor->AddCrtPropNum(enCrtProp_ActorNimbusUp, pLayerCnfoNew->m_NimbusUp - pLayerCnfoOld->m_NimbusUp);

		//最大资质判断
		INT16 MaxAptitude = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxAptitude;

		INT32 nAddAptitude = pLayerCnfoNew->m_AddAptitude - pLayerCnfoOld->m_AddAptitude;

		if ( (pTargetActor->GetCrtProp(enCrtProp_ActorAptitude) + nAddAptitude) > MaxAptitude){

			nAddAptitude = MaxAptitude - pTargetActor->GetCrtProp(enCrtProp_ActorAptitude);
		}

		pTargetActor->AddCrtPropNum(enCrtProp_ActorSpiritBasic, pLayerCnfoNew->m_AddSpirit - pLayerCnfoOld->m_AddSpirit);

		pTargetActor->AddCrtPropNum(enCrtProp_ActorShieldBasic, pLayerCnfoNew->m_AddShield - pLayerCnfoOld->m_AddShield);

		pTargetActor->AddCrtPropNum(enCrtProp_ActorBloodUpBasic, pLayerCnfoNew->m_AddBlood - pLayerCnfoOld->m_AddBlood);

		pTargetActor->AddCrtPropNum(enCrtProp_ActorAvoidBasic, pLayerCnfoNew->m_AddAvoid - pLayerCnfoOld->m_AddAvoid);

		//这个必须最后加，资质改变会重新计算，已经包括了新资质
		pTargetActor->AddCrtPropNum(enCrtProp_ActorAptitude, nAddAptitude);

		//发布事件
		SS_UpLayer UpLayer;
		UpLayer.m_LayerLevel = pTargetActor->GetCrtProp(enCrtProp_ActorLayer);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UpLayer);
		pActor->OnEvent(msgID,&UpLayer,sizeof(UpLayer));
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_AddLayerLevel,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());
}

//得到要强化的物品
IGoods * EquipMgr::__GetStrongerGoods(IActor * pActor, UID uidGoods)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return 0;
	}

	IGoods * pGoods = pPacketPart->GetGoods(uidGoods);
	if( 0 == pGoods){
		//背包中没有
		//先在主角装备栏中查找
		IEquipPart * pEquipPart = pActor->GetEquipPart();
		if( 0 == pEquipPart){
			return 0;
		}

		pGoods = pEquipPart->GetEquipment(uidGoods);
		if( 0 != pGoods){
			return pGoods;
		}

		//从招募角色的装备栏中查找
		for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
		{
			IActor * pEmployee = pActor->GetEmployee(i);
			if( 0 == pEmployee){
				continue;
			}

			IEquipPart * pTmpEquipPart = pEmployee->GetEquipPart();
			if( 0 == pTmpEquipPart){
				continue;
			}

			pGoods = pTmpEquipPart->GetEquipment(uidGoods);
			if( 0 != pGoods){
				return pGoods;
			}
		}

		if( 0 == pGoods){
			TRACE("<error> %s : %d 行 找不到要强化的装备", __FUNCTION__, __LINE__);
			return 0;
		}
	}

	return pGoods;
}

//打开帮派技能
void EquipMgr::OpenSynMagicPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ISynMagicPart * pSynMagicPart = pActor->GetSynMagicPart();
	if( 0 == pSynMagicPart){
		return;
	}

	pSynMagicPart->OpenSynMagicPanel();
}

//查看人物状态
void EquipMgr::ViewActorStatus(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewActorStatus_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidActor);
	if( 0 == pTargetActor){
		return;
	}

	IStatusPart * pStatusPart = (IStatusPart *)pTargetActor->GetPart(enThingPart_Crt_Status);
	if( 0 == pStatusPart){
		return;
	}

	pStatusPart->ShowStatusOpenUserPanel();
}

//删除状态
void EquipMgr::DeleteStatus(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_DeleteStatus_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 获取客户端数据长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidActor);
	if( 0 == pTargetActor){
		return;
	}

	const SStatusCnfg * pStatusCnfg = g_pGameServer->GetConfigServer()->GetStatusCnfg(Req.m_StatusID);
	if( 0 == pStatusCnfg){
		TRACE("<error> %s : %d 行　获取不到状态的配置信息！！,状态ID= %d ", __FUNCTION__, __LINE__, Req.m_StatusID);
		return;
	}

	IStatusPart * pStatusPart = 0;

	if( 1 == pStatusCnfg->m_bAllActor){
		//对所有角色共用的状态，放在主角身上
		pStatusPart = (IStatusPart *)pActor->GetPart(enThingPart_Crt_Status);
	}else if( 0 == pStatusCnfg->m_bAllActor){
		//
		pStatusPart = (IStatusPart *)pTargetActor->GetPart(enThingPart_Crt_Status);
	}

	if( 0 == pStatusPart){
		return;
	}

	pStatusPart->Check_CancelViewEffectMsg(Req.m_StatusID);

	pStatusPart->RemoveStatus(Req.m_StatusID);

	SC_DeleteStatus_Rsp Rsp;
	Rsp.m_Result = enEquipRetCode_OK;

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_DeleteStatus,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//检测装备是否可以自动强化
void  EquipMgr::__Check_CanEquipAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode)
{
	EquipRetCode = enEquipRetCode_OK;

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_StarLevel,starLevel);

	if( Req.m_starLevel <= starLevel){
		EquipRetCode = enEquipRetCode_ErrTargetLevelLow;
		return;
	}

	if( Req.m_starLevel - starLevel > Req.m_StrongerNum){
		EquipRetCode = enEquipRetCode_NumLimit;
		return;
	}

	const SEquipStrongerCnfg * pCurStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( (starLevel + 1) * 10 + enStrongerType_Equip);
	if( 0 == pCurStrongerCnfg){
		//已是最大级别，不能再强化了
		EquipRetCode = enEquipRetCode_LevelLimit;
		return;
	}

	const SEquipStrongerCnfg * pTargetStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( Req.m_starLevel * 10 + enStrongerType_Equip);
	if( 0 == pTargetStrongerCnfg){
		//超出最大可强化等级
		EquipRetCode = enEquipRetCode_MaxStrongLvOver;
		return;
	}

	//需要的材料和数量:物品ID,数量,物品ID,数量....
	std::vector<UINT16> vectMaterial;
	//需要的幸运石和数量
	std::vector<UINT16> vectExtender;

	for( int i = starLevel + 1; i <= Req.m_starLevel; ++i){

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( i * 10 + enStrongerType_Equip);
		if( 0 == pStrongerCnfg){
			TRACE("<error> %s : %d 行, 找不到强化配置信息！！,强化等级=%d", __FUNCTION__, __LINE__, i);
			return;
		}

		bool bMaterial = false;
		bool bExtender = false;

		for(int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( vectMaterial[n] == pStrongerCnfg->m_gidMaterial){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bMaterial = true;
				break;
			}
		}

		for(int n = 0; n + 1 < vectExtender.size(); n += 2)
		{
			if( vectExtender[n] == pStrongerCnfg->m_gidExtender){
				vectExtender[n + 1] += 1;
				bExtender = true;
				break;
			}
		}

		if( !bMaterial){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		if( !bExtender){
			vectExtender.push_back(pStrongerCnfg->m_gidExtender);
			vectExtender.push_back(1);
		}
	}

	for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
	{
		if( false == pPacketPart->HaveGoods(vectMaterial[i],vectMaterial[i + 1])){
			//原料不足以强化至目标等级
			EquipRetCode = enEquipRetCode_MaterialLimit;
			return;
		}		
	}

	if( Req.m_bUseLockStone){
		for( int i = 0; i + 1 < vectExtender.size(); i += 2)
		{
			if( false == pPacketPart->HaveGoods(vectExtender[i], vectExtender[i + 1])){
				//寒冰石ID
				const TGoodsID HanBinShiID = 10716;

				if( vectExtender[i] == HanBinShiID){
					//寒冰石不足
					EquipRetCode = enEquipRetCode_HanBinShiLimit;
					return;
				}else{
					EquipRetCode = enEquipRetCode_NoExtender;
					return;
				}
			}
		}
	}
}

//检测法宝是否可以自动强化
void  EquipMgr::__Check_CanTalismanAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode)
{
	EquipRetCode = enEquipRetCode_OK;

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_MagicLevel,starLevel);

	if( Req.m_starLevel <= starLevel){
		EquipRetCode = enEquipRetCode_ErrTargetLevelLow;
		return;
	}

	if( Req.m_starLevel - starLevel > Req.m_StrongerNum){
		EquipRetCode = enEquipRetCode_NumLimit;
		return;
	}

	const SEquipStrongerCnfg * pCurStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( (starLevel + 1) * 10 + enStrongerType_Talisman);
	if( 0 == pCurStrongerCnfg){
		//已是最大级别，不能再强化了
		EquipRetCode = enEquipRetCode_LevelLimit;
		return;
	}

	const SEquipStrongerCnfg * pTargetStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( Req.m_starLevel * 10 + enStrongerType_Talisman);
	if( 0 == pTargetStrongerCnfg){
		//超出最大可强化等级
		EquipRetCode = enEquipRetCode_MaxStrongLvOver;
		return;
	}

	//需要的材料和数量:物品ID,数量,物品ID,数量....
	std::vector<UINT16> vectMaterial;
	//需要的幸运石和数量
	std::vector<UINT16> vectExtender;

	for( int i = starLevel + 1; i <= Req.m_starLevel; ++i){

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( i * 10 + enStrongerType_Talisman);
		if( 0 == pStrongerCnfg){
			TRACE("<error> %s : %d 行, 找不到强化配置信息！！,强化等级=%d", __FUNCTION__, __LINE__, i);
			return;
		}

		bool bMaterial = false;
		bool bExtender = false;

		for(int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( vectMaterial[n] == pStrongerCnfg->m_gidMaterial){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bMaterial = true;
				break;
			}
		}

		for(int n = 0; n + 1 < vectExtender.size(); n += 2)
		{
			if( vectExtender[n] == pStrongerCnfg->m_gidExtender){
				vectExtender[n + 1] += 1;
				bExtender = true;
				break;
			}
		}

		if( !bMaterial){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		if( !bExtender){
			vectExtender.push_back(pStrongerCnfg->m_gidExtender);
			vectExtender.push_back(1);
		}
	}

	for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
	{
		if( false == pPacketPart->HaveGoods(vectMaterial[i],vectMaterial[i + 1])){
			//原料不足以强化至目标等级
			EquipRetCode = enEquipRetCode_MaterialLimit;
			return;
		}		
	}

	if( Req.m_bUseLockStone){
		for( int i = 0; i + 1 < vectExtender.size(); i += 2)
		{
			if( false == pPacketPart->HaveGoods(vectExtender[i], vectExtender[i + 1])){
				//烈焱石ID
				const TGoodsID LieYanShiID = 10717;

				if( vectExtender[i] == LieYanShiID){
					//烈焱石不足
					EquipRetCode = enEquipRetCode_HanBinShiLimit;
					return;
				}else{
					EquipRetCode = enEquipRetCode_NoExtender;
					return;
				}
			}
		}
	}
}

//检测仙剑是否可以自动强化
void  EquipMgr::__Check_CanGodSwordAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode)
{
	EquipRetCode = enEquipRetCode_OK;

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	INT32 starLevel = 0;

	pEquipment->GetPropNum(enGoodsProp_SwordLevel,starLevel);

	if( Req.m_starLevel <= starLevel){
		EquipRetCode = enEquipRetCode_ErrTargetLevelLow;
		return;
	}

	if( Req.m_starLevel - starLevel > Req.m_StrongerNum){
		EquipRetCode = enEquipRetCode_NumLimit;
		return;
	}

	const SEquipStrongerCnfg * pCurStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( (starLevel + 1) * 10 + enStrongerType_GodSword);
	if( 0 == pCurStrongerCnfg){
		//已是最大级别，不能再强化了
		EquipRetCode = enEquipRetCode_LevelLimit;
		return;
	}

	const SEquipStrongerCnfg * pTargetStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( Req.m_starLevel * 10 + enStrongerType_GodSword);
	if( 0 == pTargetStrongerCnfg){
		//超出最大可强化等级
		EquipRetCode = enEquipRetCode_MaxStrongLvOver;
		return;
	}

	//需要的材料和数量:物品ID,数量,物品ID,数量....
	std::vector<UINT16> vectMaterial;
	//需要的幸运石和数量
	std::vector<UINT16> vectExtender;

	for( int i = starLevel + 1; i <= Req.m_starLevel; ++i){

		const SEquipStrongerCnfg * pStrongerCnfg = g_pGameServer->GetConfigServer()->GetEquipStrongerCnfg( i * 10 + enStrongerType_GodSword);
		if( 0 == pStrongerCnfg){
			TRACE("<error> %s : %d 行, 找不到强化配置信息！！,强化等级=%d", __FUNCTION__, __LINE__, i);
			return;
		}

		bool bMaterial = false;
		bool bExtender = false;

		for(int n = 0; n + 1 < vectMaterial.size(); n += 2)
		{
			if( vectMaterial[n] == pStrongerCnfg->m_gidMaterial){
				vectMaterial[n + 1] += pStrongerCnfg->m_MaterialNum;
				bMaterial = true;
				break;
			}
		}

		for(int n = 0; n + 1 < vectExtender.size(); n += 2)
		{
			if( vectExtender[n] == pStrongerCnfg->m_gidExtender){
				vectExtender[n + 1] += 1;
				bExtender = true;
				break;
			}
		}

		if( !bMaterial){
			vectMaterial.push_back(pStrongerCnfg->m_gidMaterial);
			vectMaterial.push_back(pStrongerCnfg->m_MaterialNum);
		}

		if( !bExtender){
			vectExtender.push_back(pStrongerCnfg->m_gidExtender);
			vectExtender.push_back(1);
		}
	}

	for( int i = 0; i + 1 < vectMaterial.size(); i += 2)
	{
		if( false == pPacketPart->HaveGoods(vectMaterial[i],vectMaterial[i + 1])){
			//原料不足以强化至目标等级
			EquipRetCode = enEquipRetCode_MaterialLimit;
			return;
		}		
	}

	if( Req.m_bUseLockStone){
		for( int i = 0; i + 1 < vectExtender.size(); i += 2)
		{
			if( false == pPacketPart->HaveGoods(vectExtender[i], vectExtender[i + 1])){
				//烈焱石ID
				const TGoodsID LieYanShiID = 10717;

				if( vectExtender[i] == LieYanShiID){
					//烈焱石不足
					EquipRetCode = enEquipRetCode_HanBinShiLimit;
					return;
				}else{
					EquipRetCode = enEquipRetCode_NoExtender;
					return;
				}
			}
		}
	}
}

//强化成功后，如果装备在玩家身上，则把属性加上去
void  EquipMgr::__CheckInEquip_AddStrongerProp(IActor *pActor,IEquipment * pEquipment, UINT8 OldStarLevel, UINT8 NewStarLevel)
{
	//if( OldStarLevel >= NewStarLevel){
	//	return;
	//}

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
void  EquipMgr::__CheckInEquip_AddInlayProp(IActor *pActor,IEquipment * pEquipment, TGoodsID OldGemID, TGoodsID NewGemID)
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

//宝石摘除
void EquipMgr::RemoveGem(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_RemoveGem_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidEquip);

	if ( 0 == pGoods){
		
		TRACE("<error> %s : %d Line 宝石摘除,找不到装备", __FUNCTION__, __LINE__);
		return;
	}

	if ( pGoods->GetGoodsClass() != enGoodsCategory_Equip){
		
		TRACE("<error> %s : %d Line 不是装备,物品ID = %d", __FUNCTION__, __LINE__, pGoods->GetGoodsID());
		return;
	}

	IEquipment * pEquipment = (IEquipment *)pGoods;

	if ( Req.m_index + enGoodsProp_Gem_ID1 > enGoodsProp_Gem_ID3){
		
		TRACE("<error> %s : %d Line 超出宝石槽上限,index = %d",  __FUNCTION__, __LINE__, Req.m_index);
		return;
	}

	int GemID = INVALID_GOODS_ID;
	pEquipment->GetPropNum((enGoodsProp)(Req.m_index + enGoodsProp_Gem_ID1), GemID);

	if ( GemID == INVALID_GOODS_ID){
		
		TRACE("<error> %s ; %d Line 该位置没有宝石,index = %d", __FUNCTION__, __LINE__, Req.m_index);
		return;
	}

	SC_RemoveGem_Rsp Rsp;

	Rsp.m_Result = (enEquipRetCode)this->__RemoveGem(pActor, pEquipment, GemID, Req.m_index);

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_RemoveGem,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//宝石摘除,返回结果码
UINT8  EquipMgr::__RemoveGem(IActor * pActor, IEquipment * pEquipment, TGoodsID OldGemID, UINT8 index)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if ( 0 == pPacketPart){
		return enEquipRetCode_Err;
	}

	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	bool bVipFree = pActor->GetVipValue(enVipType_bFreeRemoveGem);

	if ( !bVipFree && pActor->GetCrtProp(enCrtProp_ActorStone) < ServerConfigParam.m_RemoveGemStoneNum){
		
		//灵石不足，无法摘除
		return enEquipRetCode_NoStrone_ReGem;
	}else if ( !pPacketPart->CanAddGoods(OldGemID, 1)){
		
		//背包已满，请清理背包
		return enEquipRetCode_PacketFull;
	}else {
		if ( !bVipFree){
			pActor->AddCrtPropNum(enCrtProp_ActorStone, -ServerConfigParam.m_RemoveGemStoneNum);
		}

		pPacketPart->AddGoods(OldGemID, 1, true);

		pEquipment->SetPropNum(pActor, (enGoodsProp)(index + enGoodsProp_Gem_ID1), INVALID_GOODS_ID);

		//若在装备栏，则改变属性
		this->__CheckInEquip_AddInlayProp(pActor, pEquipment, OldGemID, INVALID_GOODS_ID);
	}

	return enEquipRetCode_OK;
}

//宝石替换
void EquipMgr::ReplaceGem(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ReplaceGem_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidEquip);

	if ( 0 == pGoods){
		
		TRACE("<error> %s : %d Line 宝石摘除,找不到装备", __FUNCTION__, __LINE__);
		return;
	}

	IGoods * pGem = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidGem);
	if ( 0 == pGem){
		
		TRACE("<error> %s : %d Line 宝石摘除,找不到新宝石", __FUNCTION__, __LINE__);
		return;
	}

	if ( pGoods->GetGoodsClass() != enGoodsCategory_Equip){
		
		TRACE("<error> %s : %d Line 不是装备,物品ID = %d", __FUNCTION__, __LINE__, pGoods->GetGoodsID());
		return;
	}

	IEquipment * pEquipment = (IEquipment *)pGoods;

	if ( Req.m_index + enGoodsProp_Gem_ID1 > enGoodsProp_Gem_ID3){
		
		TRACE("<error> %s : %d Line 超出宝石槽上限,index = %d",  __FUNCTION__, __LINE__, Req.m_index);
		return;
	}

	int OldGemID = INVALID_GOODS_ID;
	pEquipment->GetPropNum((enGoodsProp)(Req.m_index + enGoodsProp_Gem_ID1), OldGemID);

	if ( OldGemID == INVALID_GOODS_ID){
		
		TRACE("<error> %s ; %d Line 该位置没有宝石可被替换,index = %d", __FUNCTION__, __LINE__, Req.m_index);
		return;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if ( 0 == pPacketPart){
		return;
	}

	SC_ReplaceGem_Rsp Rsp;

	//第一步,先宝石摘除
	Rsp.m_Result = (enEquipRetCode)this->__RemoveGem(pActor, pEquipment, OldGemID, Req.m_index);

	if ( Rsp.m_Result == enEquipRetCode_OK){
		
		//第二步,再镶嵌上新宝石
		TGoodsID NewGemID = pGem->GetGoodsID();

		//先删除宝石
		if ( !pPacketPart->DestroyGoods(pGem->GetUID(), 1)){
			
			TRACE("<error> %s : %d Line 删除宝石失败!!", __FUNCTION__, __LINE__);
			return;
		}

		pGem = 0;

		pEquipment->SetPropNum(pActor,(enGoodsProp)(enGoodsProp_Gem_ID1+Req.m_index),NewGemID);

		//镶嵌成功后，如果装备在玩家身上，则把属性加上去
		this->__CheckInEquip_AddInlayProp(pActor, pEquipment, 0, NewGemID);
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_ReplaceGem,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//强化传承
void EquipMgr::StrongInheritance(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_StrongInheritance_Req Req;

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

	SC_StrongInheritance_Rsp Rsp;

	Rsp.m_Result = enEquipRetCode_OK;

	if ( Req.m_Type == InheritanceType_Money){

		Rsp.m_Result = (enEquipRetCode)this->__MoneyInheritance(pActor, (IEquipment *)pSrcGoods, (IEquipment *)pDesGoods);

	}
/*	else if ( Req.m_Type == InheritanceType_stone)
	{

		Rsp.m_Result = (enEquipRetCode)this->__StoneInheritance(pActor, (IEquipment *)pSrcGoods, (IEquipment *)pDesGoods);

	}*/
	else if ( Req.m_Type == InheritanceType_Ordinary)
	{

		Rsp.m_Result = (enEquipRetCode)this->__OrdinaryInheritance(pActor, (IEquipment *)pSrcGoods, (IEquipment *)pDesGoods);

	}
	else
	{
		
		TRACE("<error> %s ; %d Line 客户端数据有误！！传承方式为%d", __FUNCTION__, __LINE__, Req.m_Type);
		return;
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_StrongInheritance,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//支付仙石传承
UINT8  EquipMgr::__MoneyInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment)
{
	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;
	IGodSword * pSrcGodSword = (IGodSword *)pSrcEquipment;
	IGodSword * pDesGodSword = (IGodSword *)pDesEquipment;
	if ( pActor->GetCrtProp(enCrtProp_ActorMoney) < ServerConfigParam.m_MoneyInheritMoneyNum){
		
		return enEquipRetCode_NoMoney;
	}

	if ( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ServerConfigParam.m_OrdinaryInheritPolyNimbusNum){
		//聚灵气不足
		return enEquipRetCode_NoPolyNimbus;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
			}

			pDesGodSword->UpgradeSword(pActor, SrcLevel - DesLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	default:
		TRACE("<error> %s : %d Line 强化传承中的物品类型有错!!,类型1:%d,类型2:%d", __FUNCTION__, __LINE__, SrcGoodsClass,DesGoodsClass);
		return enEquipRetCode_Err;
	}
	

	return enEquipRetCode_OK;
}

//支付灵石传承
UINT8  EquipMgr::__StoneInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment)
{
	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( pActor->GetCrtProp(enCrtProp_ActorStone) < ServerConfigParam.m_StoneInheritStoneNum){
		
		return enEquipRetCode_NoStone_Inherit;
	}

	pActor->AddCrtPropNum(enCrtProp_ActorStone, -ServerConfigParam.m_StoneInheritStoneNum); 

	enGoodsCategory SrcGoodsClass = pSrcEquipment->GetGoodsClass();

	enGoodsCategory DesGoodsClass = pDesEquipment->GetGoodsClass();

	int SrcLevel = 0;

	int DesLevel = 0;

	INT8 RealLevel = g_pGameServer->GetConfigServer()->RandGetStrongInheritLeve(pSrcEquipment, DesGoodsClass);

	//装备稳定强化等级开始
	UINT8 nEquipNoRandLevel = ServerConfigParam.m_EquipInheritNoRandLevel;

	//法宝稳定强化等级开始
	UINT8 nTalismanNoRandLevel = ServerConfigParam.m_TalismanInheritNoRandLevel;

	//装备星级从0开始，法宝从1开始
	switch( SrcGoodsClass * 10 + DesGoodsClass)
	{
	case enGoodsCategory_Equip * 10 + enGoodsCategory_Talisman:
		{
			pSrcEquipment->GetPropNum(enGoodsProp_StarLevel, SrcLevel);
			pDesEquipment->GetPropNum(enGoodsProp_MagicLevel, DesLevel);

			if ( SrcLevel + 1 <= DesLevel){
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
			}

			if ( SrcLevel >= nTalismanNoRandLevel){
				
				RealLevel = RealLevel + SrcLevel - nTalismanNoRandLevel + 1;
			}

			pDesEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, RealLevel);
			pSrcEquipment->SetPropNum(pActor, enGoodsProp_MagicLevel, 1);
		}
		break;
	default:
		TRACE("<error> %s : %d Line 强化传承中的物品类型有错!!,类型1:%d,类型2:%d", __FUNCTION__, __LINE__, SrcGoodsClass,DesGoodsClass);
		return enEquipRetCode_Err;
	}
	
	return enEquipRetCode_OK;
}

//普通传承
UINT8  EquipMgr::__OrdinaryInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment)
{
	IGodSword * pSrcGodSword = (IGodSword *)pSrcEquipment;
	IGodSword * pDesGodSword = (IGodSword *)pDesEquipment;
	const SGameServerConfigParam & ServerConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( pActor->GetCrtProp(enCrtProp_ActorPolyNimbus) < ServerConfigParam.m_OrdinaryInheritPolyNimbusNum){
		//聚灵气不足
		return enEquipRetCode_NoPolyNimbus;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
				
				return enEquipRetCode_LevelLow;
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
		return enEquipRetCode_Err;
	}
	
	return enEquipRetCode_OK;
}

//
void   EquipMgr::ClearStartLvAndGem(IActor * pActor, IEquipment * pEquipment)
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

void EquipMgr::OpenFacadePanel(IActor *pActor,UINT8 nCmd, IBuffer & ib)      //打开外观栏
{

	CS_OpenFacade_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端数据长度有误",__FUNCTION__,__LINE__);
		return;
	}

	IActor * pTarget = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidActor);

	if(pTarget==0)
	{
		TRACE("<error> %s : %d line 请求外观的玩家[UID:%s]不存在",__FUNCTION__,__LINE__,Req.m_uidActor.ToString());
		return;
	}


	IEquipPart * pEquipPart = pTarget->GetEquipPart();

	if(pEquipPart==0)
	{
		return;
	}

	pEquipPart->OpenFacadePanel();
}


void EquipMgr::SetShowFacade(IActor *pActor,UINT8 nCmd, IBuffer & ib) //设置显示外观
{
	CS_SetFacade_Req Req;
	ib >> Req;
	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息包长度有误!",__FUNCTION__,__LINE__);
	}

	IActor * pTarget = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidActor);

	if(pTarget==0)
	{
		return;
	}

	IEquipPart * pEquipPart = pTarget->GetEquipPart();

	if(pEquipPart==0)
	{
		return;
	}

	pEquipPart->SetShowFacade(Req.m_FacadeID);
}



