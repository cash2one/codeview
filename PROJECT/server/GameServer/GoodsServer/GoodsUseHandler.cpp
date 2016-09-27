
#include "GoodsUseHandler.h"
#include "GameSrvProtocol.h"
#include "IConfigServer.h"
#include "IMagicPart.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "IGoods.h"
#include "IGameWorld.h"
#include "GoodsServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicateMember.h"
#include "IFriendPart.h"
#include "IBasicService.h"
#include "ICDTime.h"
#include "IStatusPart.h"
#include "DSystem.h"
#include "RandomService.h"
#include "DMsgSubAction.h"
#include "stdio.h"
#include <sstream>

GoodsUseHandler::GoodsUseHandler()
{
	
}

GoodsUseHandler::~GoodsUseHandler()
{
}

bool GoodsUseHandler::Create()
{	
	return true;
}


GOODSUSE_FUNC GoodsUseHandler::GetHandleFunc(enGoodsUseHandlerType Type)
{
	if(Type >= enGoodsUseHandlerType_Max)
	{
		return 0;
	}

	return s_FuncArray[Type];
}



//学习法术书
INT32 API_StudyMagicBook(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=MagicID
	if(pGoodsUseCnfg->m_vectParam.size()<1)
	{
		return enPacketRetCode_ErrParam;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidTarget);
	if( 0 == pTargetActor){
		return enPacketRetCode_NoActor;
	}

	TMagicID MagicID = pGoodsUseCnfg->m_vectParam[0];

	IMagicPart * pMagicPart = pTargetActor->GetMagicPart();

	if( 0 != pMagicPart->GetMagic(MagicID)){
		return enPacketRetCode_ErrExistMagic;
	}

	if( 0 == pMagicPart->StudyMagic(MagicID))
	{
		return enPacketRetCode_ErrStudyMagic;
	}

	// fly add	20121106

	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	//发布事件
	SS_StudyMagic StudyMagic;
	StudyMagic.m_MagicID	= pGoodsUseCnfg->m_vectParam[0];
	StudyMagic.m_MagicLevel = pGoods->GetGoodsCnfg()->m_UsedLevel;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_StudyMagic);
	pActor->OnEvent(msgID,&StudyMagic,sizeof(StudyMagic));

	return enPacketRetCode_OK;
}

//打开礼包
INT32 API_OpenGiftPecket(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[]={GoodsID1,Num1,GoodsID2,Num2,...};
	IPacketPart * pPacketPart = pActor->GetPacketPart();

	std::vector<SAddGoodsInfo> vect(pGoodsUseCnfg->m_vectParam.size()/2);

	//先判断包背容量
	for(int i=0; i+1<pGoodsUseCnfg->m_vectParam.size(); i += 2)
	{
		SAddGoodsInfo Info;
		Info.m_bBinded = true;
		Info.m_GoodsID = pGoodsUseCnfg->m_vectParam[i];
		Info.m_nNum = pGoodsUseCnfg->m_vectParam[i+1];
		vect[i/2] = Info;
	}

	if(vect.empty())
	{
		return enPacketRetCode_ErrParam;
	}

	if(pPacketPart->CanAddGoods(&vect[0],vect.size())==false)
	{
		return enPacketRetCode_NoSpace;
	}

	//给给予物品
	for(int i=0; i < vect.size();i++)
	{
		if(pPacketPart->AddGoods(vect[i].m_GoodsID,vect[i].m_nNum,true)==false)
		{
			return enPacketRetCode_NoSpace;
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Other,vect[i].m_GoodsID,UID(),vect[i].m_nNum,"打开礼包获得物品");
	}

	// fly add	20121106

	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//法宝邀请令
INT32 API_OpenTalismanInvitation(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	return enPacketRetCode_OK;
}

//人物卡
INT32 API_OpenActorCard(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	if( pGoodsUseCnfg->m_vectParam.size() < 1){
		return enPacketRetCode_ErrParam;
	}

	TActorID EmployeeID = pGoodsUseCnfg->m_vectParam[0];

	const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(EmployeeID);

	if( 0 == pEmployeeDataCnfg){
		TRACE("<error> %s : %d 行 获取招募角色配置信息出错！！招募角色ID = %d", __FUNCTION__, __LINE__, EmployeeID);
		return 0;
	}

	//检测是否超出可雇用上限
	UINT8 nCount = 0;

	for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = pActor->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		++nCount;


		// fly add	20121106
		if( 0 == strcmp(pEmployee->GetName(), (const char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pEmployeeDataCnfg->m_NameLanguageID))){
			//已有相同名字招募角色
			return enPacketRetCode_ErrName;
		}						
//		if( 0 == strcmp(pEmployee->GetName(), (const char *)pEmployeeDataCnfg->m_szName)){
			//已有相同名字招募角色
//			return enPacketRetCode_ErrName;
//		}
	}

	//if( nCount >= MAX_EMPLOY_NUM){
	if ( nCount >= g_pGameServer->GetConfigServer()->GetCanEmployNum(pActor->GetCrtProp(enCrtProp_Level)) ){
		return enPacketRetCode_ErrFullEmployee;
	}

	IActor * pEmployee = g_pGameServer->GetGameWorld()->CreateEmploy(EmployeeID, pActor);
	if( 0 == pEmployee){
		TRACE("<error> %s:%d 创建招募角色失败,EmployeeID = %d", __FUNCTION__, __LINE__, EmployeeID);
		return enPacketRetCode_ErrCreateEmployee;
	}

	// fly add	20121106

	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//资源卡
INT32 API_OpenResourceCard(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=资源类型，m_vectParam[1]=资源数量;
	if(pGoodsUseCnfg->m_vectParam.size()<2)
	{
		return enPacketRetCode_ErrParam;
	}

	//资源类型
	enResourceType Type = (enResourceType)pGoodsUseCnfg->m_vectParam[0];

	if(Type>=enResourceType_Max)
	{
		return enPacketRetCode_ErrParam;
	}

	//资源数量
	INT32  Value = pGoodsUseCnfg->m_vectParam[1] * UseNum;


	if(pActor->AddCrtPropNum(s_ResourceTypeMapCrtProp[Type],Value)==false)
	{
		return enPacketRetCode_ErrAddResource;
	}

	// fly add	20121106
	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);	
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//增加人物属性
INT32 API_AddActorProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=人物属性，m_vectParam[1]=属性值,m_vectParam[2]=增加概率;

	if(pGoodsUseCnfg->m_vectParam.size() < 3)
	{
		return enPacketRetCode_ErrParam;
	}

	//资源类型
	enCrtProp PropID = (enCrtProp)pGoodsUseCnfg->m_vectParam[0];

	if( enCrtProp_Actor_End == PropID)
	{
		return enPacketRetCode_ErrParam;
	}

	//目标
	IActor * pTarget = g_pGameServer->GetGameWorld()->FindActor(uidTarget);

	if(pTarget == 0)
	{
		return enPacketRetCode_NoTarget;
	}

	//增加概率(1000来算)
	INT32 nRandom = RandomService::GetRandom() % 1000;
	if( nRandom > pGoodsUseCnfg->m_vectParam[2]){
		std::ostringstream os;

		// fly add	20121106
		os << g_pGameServer->GetGameWorld()->GetLanguageStr(10066) << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10067);
		//os << "增加" << pGoodsUseCnfg->m_UseDesc << "失败!!";
		strUserDesc = os.str();

		return enPacketRetCode_OK;
	}

	std::ostringstream os;
	INT32 nHaveNum = UseNum;								//可以使用的物品数量
	UseNum = 0;												//实际使用的物品数量
	INT32  nPerValue = pGoodsUseCnfg->m_vectParam[1];		//每个的资源数量
	INT32  Value = 0;										//实际加的资源数量

	switch(PropID)
	{
	case enCrtProp_ActorAptitude:
		{
			//最大资质判断
			INT16 MaxAptitude = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxAptitude;
			if( pTarget->GetCrtProp(enCrtProp_ActorAptitude) >= MaxAptitude){
				return enPacketRetCode_ErrFullAptitude;
			}

			for( ; UseNum < nHaveNum; ++UseNum)
			{
				INT32 nAptitude = pTarget->GetCrtProp(enCrtProp_ActorAptitude);
				if( nAptitude >= MaxAptitude){
					break;
				}

				nPerValue = (nAptitude + nPerValue) > MaxAptitude ? (MaxAptitude - nAptitude) : nPerValue;
			
				if( pTarget->AddCrtPropNum(enCrtProp_ForeverAptitude,nPerValue) == false){
					return enPacketRetCode_ErrActorProp;
				}

				Value += nPerValue;
			}

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTarget->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) <<  g_pGameServer->GetGameWorld()->GetLanguageStr(10069)<< (Value / 1000.0f);
			//os << "使用成功," << pTarget->GetName() << pGoodsUseCnfg->m_UseDesc << "资质提升" << (Value / 1000.0f);
		}
		break;
	case enCrtProp_ActorExp:
		{
			//等级上限判断
			const SActorLevelCnfg * pActorNextLevel = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(pTarget->GetCrtProp(enCrtProp_Level) + 1);
			if( 0 == pActorNextLevel){
				//已达等级上限
				return enPacketRetCode_ErrLevelUp;
			}

			for( ; UseNum < nHaveNum; ++UseNum)
			{
				const SActorLevelCnfg * pActorNextLevel = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(pTarget->GetCrtProp(enCrtProp_Level) + 1);
				if( 0 == pActorNextLevel){
					//已达等级上限
					break;
				}
			
				if( pTarget->AddCrtPropNum(PropID,nPerValue) == false){
					return enPacketRetCode_ErrActorProp;
				}

				Value += nPerValue;			
			}

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTarget->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(10070)<<  g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << Value;
			//os << "使用成功," << pTarget->GetName() << "增加" << pGoodsUseCnfg->m_UseDesc << Value;
		}
		break;
	case enCrtProp_ActorNimbus:
		{
			//灵气上限判断
			const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTarget->GetCrtProp(enCrtProp_ActorLayer));
			if( 0 == pLayerCnfg){
				TRACE("<error> %s : %d 行, 境界配置表获取错误!!,境界=%d",__FUNCTION__, __LINE__, pTarget->GetCrtProp(enCrtProp_ActorLayer));
				return enPacketRetCode_Error;
			}

			if( pTarget->GetCrtProp(enCrtProp_ActorNimbus) >= pLayerCnfg->m_NimbusUp){
				return enPacketRetCode_ErrNimbusUp;
			}

			for( ; UseNum < nHaveNum; ++UseNum)
			{
				INT32 Nimbus = pTarget->GetCrtProp(enCrtProp_ActorNimbus);

				if( Nimbus >= pLayerCnfg->m_NimbusUp){
					break;
				}

				nPerValue = (Nimbus + nPerValue) > pLayerCnfg->m_NimbusUp ? (pLayerCnfg->m_NimbusUp - Nimbus) : nPerValue;

				if(pTarget->AddCrtPropNum(PropID,nPerValue)==false){
					return enPacketRetCode_ErrActorProp;
				}

				Value += nPerValue;	
			}

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTarget->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(10070) << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << Value;
			//os << "使用成功," << pTarget->GetName() << "增加" << pGoodsUseCnfg->m_UseDesc << Value;
		}
		break;
	case enCrtProp_ActorGodSwordNimbus:
		{
			//仙剑灵气上限验证
			INT32 nMaxGodSwordNimbus = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;

			if( pTarget->GetCrtProp(enCrtProp_ActorGodSwordNimbus) >= nMaxGodSwordNimbus){
				return enPacketRetCode_ErrGodSwordNimbusUp;
			}

			for( ; UseNum < nHaveNum; ++UseNum)
			{
				INT32 GodSwordNimbus = pTarget->GetCrtProp(enCrtProp_ActorGodSwordNimbus);

				if( GodSwordNimbus >= nMaxGodSwordNimbus){
					break;
				}

				nPerValue = (GodSwordNimbus + nPerValue) > nMaxGodSwordNimbus ? (nMaxGodSwordNimbus - GodSwordNimbus) : nPerValue;

				if(pTarget->AddCrtPropNum(PropID,nPerValue)==false){
					return enPacketRetCode_ErrActorProp;
				}

				Value += nPerValue;	
			}
						
			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10071) << Value;
			//os << "获得仙剑灵气" << Value;
		}
		break;
	default:
		{
			UseNum = nHaveNum;
			Value = nPerValue * UseNum;

			if(pTarget->AddCrtPropNum(PropID,Value)==false){
				return enPacketRetCode_ErrActorProp;	
			}

			// fly add	20121106
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTarget->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(10070) << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID)<< Value;
			//os << "使用成功," << pTarget->GetName() << "增加" << pGoodsUseCnfg->m_UseDesc << Value;
		}
		break;
	}

	strUserDesc = os.str();

	return enPacketRetCode_OK;
}

//增加好友度
INT32 API_AddFriendRelation(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=好友关系度;

	if(pGoodsUseCnfg->m_vectParam.size()<1)
	{
		return enPacketRetCode_ErrParam;
	}
	
	//好友关系度
	INT32  Value = pGoodsUseCnfg->m_vectParam[0] * UseNum;
	
	IFriendPart * pFriendPart = pActor->GetFriendPart();

	char szBuffer[1024] = {0};

	// fly add	20121106

	sprintf_s(szBuffer,sizeof(szBuffer),g_pGameServer->GetGameWorld()->GetLanguageStr(10049),pActor->GetName(),pGoods->GetName(),Value);
	//sprintf_s(szBuffer,sizeof(szBuffer),"玩家[%s]使用物品[%s]增加好友度%d",pActor->GetName(),pGoods->GetName(),Value);

	pFriendPart->AddRelationNum(uidTarget,Value,szBuffer,szBuffer);	

	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//增加物品属性
INT32 API_AddGoodsProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=物品属性，m_vectParam[1]=属性值;

	if(pGoodsUseCnfg->m_vectParam.size()<2)
	{
		return enPacketRetCode_ErrParam;
	}

	//属性类型
	enGoodsProp PropID = (enGoodsProp)pGoodsUseCnfg->m_vectParam[0];

	if(enGoodsProp_Max <= PropID)
	{
		return enPacketRetCode_ErrParam;
	}

	//属性值
	INT32  Value = pGoodsUseCnfg->m_vectParam[1] * UseNum;
		
	if(pGoods->AddPropNum(pActor,PropID,Value)==false)
	{
		return enPacketRetCode_ErrGoodsProp;
	}

	// fly add	20121106
	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//增加帮派属性
INT32 API_AddGangProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//SGoodsUseCnfg::m_vectParam 参数 m_vectParam[0]=增加的帮贡值;

	if(pGoodsUseCnfg->m_vectParam.size() < 1)
	{
		return enPacketRetCode_ErrParam;
	}

	//属性类型
	

	//属性值
	INT32  Value = pGoodsUseCnfg->m_vectParam[0] * UseNum;

	ISyndicateMember * pSyndicateMember = g_pGameServer->GetSyndicateMgr()->GetSyndicateMember(uidTarget);
	if(pSyndicateMember == 0)
	{
		return enPacketRetCode_NotSynMember;
	}

	pSyndicateMember->AddContribution(Value);

	// fly add	20121106
	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//给玩家加状态
INT32 API_AddStatus(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	if(pGoodsUseCnfg->m_vectParam.size() < 1)
	{
		return enPacketRetCode_ErrParam;
	}

	//状态ID
	TStatusID lID = pGoodsUseCnfg->m_vectParam[0];

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidTarget);
	if( 0 == pTargetActor){
		return enPacketRetCode_Error;
	}

	IStatusPart * pStatusPart = (IStatusPart *)pTargetActor->GetPart(enThingPart_Crt_Status);
	if( 0 == pStatusPart){
		return enPacketRetCode_Error;
	}

	SAddStatus AddStatus;

	AddStatus.m_Index = Index;
	AddStatus.m_UseGoodsNum = UseNum;

	pStatusPart->AddStatus_UseGoods(lID, pTargetActor->GetUID(), AddStatus); 

	if( AddStatus.m_PacketRetCode == enPacketRetCode_FullStatusGroup){

		OBuffer1k ob;

		if( AddStatus.m_bReplace){
			//询问是否替换
			SC_ReplaceStatus_Rsp  ReplaceStatus_Rsp;

			ReplaceStatus_Rsp.m_Index     = AddStatus.m_Index;
			ReplaceStatus_Rsp.m_uidTarget = uidTarget;
			ReplaceStatus_Rsp.m_uidUseGoods = pGoods->GetUID();
			ReplaceStatus_Rsp.m_UseNum	  = UseNum;
	
			strncpy(ReplaceStatus_Rsp.m_szRepalceTiShi, AddStatus.m_strUserDesc.c_str(), sizeof(ReplaceStatus_Rsp.m_szRepalceTiShi));

			ob << Packet_Header(enPacketCmd_SC_TiShiReplaceOK,sizeof(ReplaceStatus_Rsp)) << ReplaceStatus_Rsp;
		}else{
			//最多存在2种效果，请取消后其中一种再使用
			return enPacketRetCode_ErrMaxEffect;
			//只给提示
			//SC_TiShi_Rsp TiShi_Rsp;
			//strncpy(TiShi_Rsp.m_szRepalceTiShi,  AddStatus.m_strUserDesc.c_str(), sizeof(TiShi_Rsp.m_szRepalceTiShi));

			//ob << Packet_Header(enPacketCmd_SC_TiShi,sizeof(TiShi_Rsp)) << TiShi_Rsp;
		}

		pActor->SendData(ob.TakeOsb());
	}

	// fly add	20121106
	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return AddStatus.m_PacketRetCode;
}

//刷新任务
INT32 API_FlushTask(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	return enPacketRetCode_OK;
}

//更改招募角色名称
INT32 API_ChangeEmployeeName(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	if( pActor->GetUID() == uidTarget){
		return enPacketRetCode_ErrChangeUserName;
	}

	SC_ActorChangeName ActorChangeName;
	ActorChangeName.m_uidTarget = uidTarget;
	ActorChangeName.m_uidUseGoods = pGoods->GetUID();
	ActorChangeName.m_UseNum	= UseNum;

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_ActorChangeName,sizeof(ActorChangeName)) << ActorChangeName;
	pActor->SendData(ob.TakeOsb());

	return enPacketRetCode_ActorChangeName;
}

//全服务公告
INT32 API_BroadcastToGameWorld(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	SC_ChangYinFu ChangYinFu;
	ChangYinFu.m_uidTarget = uidTarget;
	ChangYinFu.m_uidUseGoods = pGoods->GetUID();
	ChangYinFu.m_UseNum	   = UseNum;

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_ChangYinFu,sizeof(ChangYinFu)) << ChangYinFu;
	pActor->SendData(ob.TakeOsb());

	return enPacketRetCode_ChangYinFu;
}

//打开随机礼包
INT32 API_OpenRandomGiftPacket(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//参数:m_vectParam[0]:抽取的物品个数 m_vectParam[1]:物品ID m_vectParam[2]:数量 m_vectParam[3]:几率 .......
	if(pGoodsUseCnfg->m_vectParam.size() < 4){
		return enPacketRetCode_ErrParam;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enPacketRetCode_Error;
	}

	INT32 nTotalRandom = 0;

	//要抽取的物品数量
	INT32 nCountGoods = pGoodsUseCnfg->m_vectParam[0];

	for( int i = 1; i + 2 < pGoodsUseCnfg->m_vectParam.size(); i += 3)
	{
		nTotalRandom += pGoodsUseCnfg->m_vectParam[i + 2];
	}

	//保存得到的物品
	std::vector<UINT16> vectGoods;	//vectGoods[0] = goodsID,vectGoods[1] = num,....

	for( int i = 0; i < nCountGoods; ++i)
	{
		INT32 nRandom = RandomService::GetRandom() % nTotalRandom;

		INT32 nTemp = 0;

		for( int k = 1; k + 2 < pGoodsUseCnfg->m_vectParam.size(); k += 3)
		{
			//先检测是否已经在得到的物品列表里面
			bool bExist = false;
			for( int n = 0; n + 1 < vectGoods.size(); n += 2)
			{
				if( pGoodsUseCnfg->m_vectParam[k] == vectGoods[n]){
					bExist = true;
					continue;
				}
			}

			if( bExist){
				//已经在得到的物品列表里面
				continue;
			}

			nTemp += pGoodsUseCnfg->m_vectParam[k + 2];

			if( nTemp > nRandom){
				vectGoods.push_back(pGoodsUseCnfg->m_vectParam[k]);
				vectGoods.push_back(pGoodsUseCnfg->m_vectParam[k + 1]);

				nTotalRandom -= pGoodsUseCnfg->m_vectParam[k + 2];
				break;
			}
		}
	}

	//检测背包是否足够容量
	if( !pPacketPart->CanAddGoods(vectGoods)){
		return enPacketRetCode_NoSpace;
	}

	std::ostringstream os;

	// fly add	20121106

	os << g_pGameServer->GetGameWorld()->GetLanguageStr(10072);
	//os << "使用成功,获得了";	

	for( int i = 0; i + 1 < vectGoods.size(); i += 2)
	{
		pPacketPart->AddGoods(vectGoods[i], vectGoods[ i + 1], true);
		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Other,vectGoods[i],UID(),vectGoods[ i + 1],"打开随机礼包获得物品");

		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(vectGoods[i]);
		if( 0 == pGoodsCnfg){
			continue;
		}

		// fly add	20121106
		if( 0 == i){


			os << (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID);
			//os << (char*)pGoodsCnfg->m_szName;
		}else{

			os << "," << (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID);
			//os << "," << (char*)pGoodsCnfg->m_szName;
		}
	}

	strUserDesc = os.str();

	return enPacketRetCode_OK;
}


//物品合成
INT32 API_GoodsCompose(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	//参数，m_vectParam[0] = goodsID
	if(pGoodsUseCnfg->m_vectParam.size() < 1)
	{
		return enPacketRetCode_ErrParam;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return enPacketRetCode_Error;
	}

/*	const SGoodsComposeCnfg * pGoodsComposeCnfg = g_pGameServer->GetConfigServer()->GetGoodsComposeCnfg(pGoodsUseCnfg->m_vectParam[0]);
	if( 0 == pGoodsComposeCnfg){
		TRACE("<error> %s : %d 行，物品使用配置表找不到！！　物品ID　＝　%d", __FUNCTION__,__LINE__, pGoodsUseCnfg->m_vectParam[0]);
		return enPacketRetCode_Error;
	}

	if( pActor->GetCrtProp(enCrtProp_ActorMoney) < pGoodsComposeCnfg->m_Charge){
		return enPacketRetCode_NoMoney;
	}

	if( pGoodsComposeCnfg->m_gidMaterial1 != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pGoodsComposeCnfg->m_gidMaterial1, pGoodsComposeCnfg->m_Material1Num)){
		return enPacketRetCode_NoMaterial;
	}

	if( pGoodsComposeCnfg->m_gidMaterial2 != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pGoodsComposeCnfg->m_gidMaterial2, pGoodsComposeCnfg->m_Material2Num)){
		return enPacketRetCode_NoMaterial;
	}

	if( pGoodsComposeCnfg->m_gidMaterial3 != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pGoodsComposeCnfg->m_gidMaterial3, pGoodsComposeCnfg->m_Material3Num)){
		return enPacketRetCode_NoMaterial;
	}

	if( pGoodsComposeCnfg->m_gidMaterial4 != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pGoodsComposeCnfg->m_gidMaterial4, pGoodsComposeCnfg->m_Material4Num)){
		return enPacketRetCode_NoMaterial;
	}

	//扣除
	pActor->AddCrtPropNum(enCrtProp_ActorMoney, -pGoodsComposeCnfg->m_Charge);


	g_pGameServer->GetGameWorld()->Save_GodStoneLog(pActor->GetCrtProp(enCrtProp_ActorUserID), pGoodsComposeCnfg->m_Charge, pActor->GetCrtProp(enCrtProp_ActorMoney), "物品合成");

	pPacketPart->DestroyGoods(pGoodsComposeCnfg->m_gidMaterial1, pGoodsComposeCnfg->m_Material1Num);


	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_gidMaterial1,UID(),pGoodsComposeCnfg->m_Material1Num,"物品合成扣除材料");

	pPacketPart->DestroyGoods(pGoodsComposeCnfg->m_gidMaterial2, pGoodsComposeCnfg->m_Material2Num);

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_gidMaterial2,UID(),pGoodsComposeCnfg->m_Material2Num,"物品合成扣除材料");

	pPacketPart->DestroyGoods(pGoodsComposeCnfg->m_gidMaterial3, pGoodsComposeCnfg->m_Material3Num);

	
	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_gidMaterial3,UID(),pGoodsComposeCnfg->m_Material3Num,"物品合成扣除材料");

	pPacketPart->DestroyGoods(pGoodsComposeCnfg->m_gidMaterial4, pGoodsComposeCnfg->m_Material4Num);

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_gidMaterial4,UID(),pGoodsComposeCnfg->m_Material4Num,"物品合成扣除材料");

	//给玩家合成的物品
	pPacketPart->AddGoods(pGoodsComposeCnfg->m_GoodsID);

	g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_GoodsID,UID(),1,"合成物品");
*/
	// fly add	20121106
	strUserDesc = g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID);
	//strUserDesc = pGoodsUseCnfg->m_UseDesc;

	return enPacketRetCode_OK;
}

//需要单独做特殊处理的物品
INT32 API_SpecialTreatment(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index)
{
	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidTarget);
	if( 0 == pTargetActor){
		TRACE("<error> %s : %d 行 使用物品找不到目标玩家!!", __FUNCTION__, __LINE__);
		return enPacketRetCode_NoTarget;
	}

	//培无丹ID
	const TGoodsID PeiYuanDanID = 10059;

	//高级经验丹特殊处理
	const TGoodsID GaoJiExpID = 10135;

	//超级经验丹特殊处理
	const TGoodsID SuperGaoJiExpID = 10136;

	//千年灵根
	const TGoodsID QianNianLiGen = 10060;

	switch(pGoodsUseCnfg->m_GoodsID)
	{
	case PeiYuanDanID:
		{
			if( pGoodsUseCnfg->m_vectParam.size() < 1){
				return enPacketRetCode_ErrParam;
			}

			INT32 Aptitude = pTargetActor->GetCrtProp(enCrtProp_ActorAptitude);
			//最大资质判断
			if( Aptitude >= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxAptitude){
				return enPacketRetCode_ErrFullAptitude;
			}

			//得到提升资质配置
			const SUpAptitude * pUpAptitude = g_pGameServer->GetConfigServer()->GetUpAptitude(Aptitude);

			if ( 0 == pUpAptitude )
			{
				TRACE("<error> %s : %d Line 获取提升资质配置信息！！资质%d", __FUNCTION__, __LINE__, Aptitude);
				return enPacketRetCode_Err;
			}

			IPacketPart * pPacketPart = pActor->GetPacketPart();

			if ( 0 == pPacketPart )
				return enPacketRetCode_Err;

			if ( pUpAptitude->m_NeedGoods != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pUpAptitude->m_NeedGoods, pUpAptitude->m_NeedNum) )
			{
				const INT32 constAptitude = 2000;

				if ( Aptitude >= constAptitude )
				{
					return enPacketRetCode_ErrAptitude20;
				}

				return enPacketRetCode_ErrAptitude18;
			}

			pPacketPart->DestroyGoods(pUpAptitude->m_NeedGoods, pUpAptitude->m_NeedNum);


			const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

			for( int i = 0; (i + 2) < GameParam.m_PeiYuanDanRand.size(); i += 3)
			{
				if( Aptitude >= GameParam.m_PeiYuanDanRand[i] && Aptitude <= GameParam.m_PeiYuanDanRand[i + 1]){
					//使用培元丹的成功概率在这个区间获取

					INT32 nRandom = RandomService::GetRandom() % 1000;

					if( nRandom < GameParam.m_PeiYuanDanRand[i + 2]){
						//增加的资质数量
						INT32  Value = pGoodsUseCnfg->m_vectParam[0] * UseNum;

						if( (Aptitude + Value) > g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxAptitude){
							Value = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxAptitude - Aptitude;
						}
	
						pTargetActor->AddCrtPropNum(enCrtProp_ForeverAptitude, Value);

						std::ostringstream os;

						// fly add	20121106
						os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068)<< pTargetActor->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10074)<< (Value / 1000.0f);
						//os << "使用成功," << pTargetActor->GetName() << pGoodsUseCnfg->m_UseDesc << "提升" << (Value / 1000.0f);
						strUserDesc = os.str();
					}else{
						// fly add	20121106
						strUserDesc += g_pGameServer->GetGameWorld()->GetLanguageStr(10075);
					}
					break;
				}
			}
		}
		break;
	case GaoJiExpID:
		{
			if (pActor->GetUID() == pTargetActor->GetUID()){
				
				return enPacketRetCode_ErrNoMasterUse;
			}

			//等级上限判断
			const SActorLevelCnfg * pActorNextLevel = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(pTargetActor->GetCrtProp(enCrtProp_Level) + 1);

			if (0 == pActorNextLevel){
				//已达等级上限
				return enPacketRetCode_ErrLevelUp;			
			}

			///高级经验丹增加多少经验
			UINT32    GaoJiExpAddExpNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_GaoJiExpAddExpNum;

			INT32 nExp = pActor->GetCrtProp(enCrtProp_ActorExp) - pTargetActor->GetCrtProp(enCrtProp_ActorExp);

			for (int i = pTargetActor->GetCrtProp(enCrtProp_Level); i < pActor->GetCrtProp(enCrtProp_Level); ++i)
			{
				const SActorLevelCnfg * pLvCnfg = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(i + 1);
				
				if (0 == pLvCnfg){
					return enPacketRetCode_ErrActorProp;
				}

				nExp += pLvCnfg->m_NeedExp;
			}

			INT32 nHaveNum = UseNum;

			UseNum = 0;

			INT32 nMaxUserNum = nExp / GaoJiExpAddExpNum;

			if (0 == nMaxUserNum){
				
				return enPacketRetCode_ErrMasterLv;
			}

			INT32  Value = 0;										//实际加的资源数量

			for (int i = 0; i < nHaveNum && i < nMaxUserNum; ++i)
			{
				if (pTargetActor->AddCrtPropNum(enCrtProp_ActorExp,GaoJiExpAddExpNum) == false){

					return enPacketRetCode_ErrActorProp;
				}

				Value += GaoJiExpAddExpNum;

				++UseNum;
			}

			std::ostringstream os;

			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTargetActor->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(10070)<<  g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << Value;
			
			strUserDesc = os.str();
		}
		break;
	case SuperGaoJiExpID:
		{
			if (pActor->GetUID() == pTargetActor->GetUID()){
				
				return enPacketRetCode_ErrNoMasterUseSuper;
			}

			//等级上限判断
			const SActorLevelCnfg * pActorNextLevel = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(pTargetActor->GetCrtProp(enCrtProp_Level) + 1);

			if (0 == pActorNextLevel){
				//已达等级上限
				return enPacketRetCode_ErrLevelUp;			
			}

			///超级经验丹增加多少经验
			UINT32    GaoJiExpAddExpNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_SuperGaoJiExpAddExpNum;

			INT32 nExp = pActor->GetCrtProp(enCrtProp_ActorExp) - pTargetActor->GetCrtProp(enCrtProp_ActorExp);

			for (int i = pTargetActor->GetCrtProp(enCrtProp_Level); i < pActor->GetCrtProp(enCrtProp_Level); ++i)
			{
				const SActorLevelCnfg * pLvCnfg = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(i + 1);
				
				if (0 == pLvCnfg){
					return enPacketRetCode_ErrActorProp;
				}

				nExp += pLvCnfg->m_NeedExp;
			}

			INT32 nHaveNum = UseNum;

			UseNum = 0;

			INT32 nMaxUserNum = nExp / GaoJiExpAddExpNum;

			if (0 == nMaxUserNum){
				
				return enPacketRetCode_ErrMasterLv;
			}

			INT32  Value = 0;										//实际加的资源数量

			for (int i = 0; i < nHaveNum && i < nMaxUserNum; ++i)
			{
				if (pTargetActor->AddCrtPropNum(enCrtProp_ActorExp,GaoJiExpAddExpNum) == false){

					return enPacketRetCode_ErrActorProp;
				}

				Value += GaoJiExpAddExpNum;

				++UseNum;
			}

			std::ostringstream os;

			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068) << pTargetActor->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(10070)<<  g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << Value;
			
			strUserDesc = os.str();			
		}
		break;
	case QianNianLiGen:
		{
			if( pGoodsUseCnfg->m_vectParam.size() < 1){
				return enPacketRetCode_ErrParam;
			}

			const SGameConfigParam & GameCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam();

			INT32 Aptitude = pTargetActor->GetCrtProp(enCrtProp_ActorAptitude);

			//最大资质判断
			if( Aptitude >= GameCnfg.m_MaxAptitude)
			{
				return enPacketRetCode_ErrFullAptitude;
			}

			//得到提升资质配置
			const SUpAptitude * pUpAptitude = g_pGameServer->GetConfigServer()->GetUpAptitude(Aptitude);

			if ( 0 == pUpAptitude )
			{
				TRACE("<error> %s : %d Line 获取提升资质配置信息！！资质%d", __FUNCTION__, __LINE__, Aptitude);
				return enPacketRetCode_Err;
			}

			IPacketPart * pPacketPart = pActor->GetPacketPart();

			if ( 0 == pPacketPart )
				return enPacketRetCode_Err;

			if ( pUpAptitude->m_NeedGoods != INVALID_GOODS_ID && !pPacketPart->HaveGoods(pUpAptitude->m_NeedGoods, pUpAptitude->m_NeedNum) )
			{
				const INT32 constAptitude = 2000;

				if ( Aptitude >= constAptitude )
				{
					return enPacketRetCode_ErrAptitude20;
				}

				return enPacketRetCode_ErrAptitude18;
			}

			pPacketPart->DestroyGoods(pUpAptitude->m_NeedGoods, pUpAptitude->m_NeedNum);

			//增加的资质数量
			INT32  Value = pGoodsUseCnfg->m_vectParam[0];

			if ( Aptitude + Value >= GameCnfg.m_MaxAptitude )
			{
				Value = GameCnfg.m_MaxAptitude - Aptitude;
			}

			pTargetActor->AddCrtPropNum(enCrtProp_ForeverAptitude, Value);

			std::ostringstream os;
			os << g_pGameServer->GetGameWorld()->GetLanguageStr(10068)<< pTargetActor->GetName() << g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsUseCnfg->m_UseDescLangID) << g_pGameServer->GetGameWorld()->GetLanguageStr(10074)<< (Value / 1000.0f);
			strUserDesc = os.str();
		}
		break;
	default:
		break;
	}

	return enPacketRetCode_OK;
}
