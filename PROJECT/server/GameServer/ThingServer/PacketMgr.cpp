
#include "PacketMgr.h"
#include "IActor.h"
#include "GameSrvProtocol.h"
#include "IPacketPart.h"
#include "IBasicService.h"
#include "ThingServer.h"
#include "ICDTimerPart.h"
#include "IKeywordFilter.h"
#include "IGoodsServer.h"

PacketMgr::PacketMgr()
{
}

PacketMgr::~PacketMgr()
{
}

bool PacketMgr::Create()
{
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Packet,this);
}
void PacketMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Packet,this);
}

//收到MSG_ROOT消息
void PacketMgr::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	 typedef  void (PacketMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enPacketCmd_Max]=
	 {
		 NULL,
		& PacketMgr::AddGoods,
		& PacketMgr::RemoveGoods,	
		& PacketMgr::ExtendCapacity,
		& PacketMgr::UseGoods,
		& PacketMgr::DiscardGoods,
		& PacketMgr::Equip,
		& PacketMgr::Compose,
		& PacketMgr::UseActorChangeName,
		NULL,
		NULL,
		& PacketMgr::UseChangYinFu,
		& PacketMgr::SellGoods,
		& PacketMgr::ConfirmationUseGoods,
		& PacketMgr::ClickAptitude,
	 };

	 if(nCmd>=enPacketCmd_Max || 0==s_funcProc[nCmd])
	 {
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}

//增加物品
void PacketMgr::AddGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if(pPacketPart==0)
	{
		return ;
	}

	SC_PacketAddGoods Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->AddGoods(Req.m_uidGoods);	
}


//移除移品
void PacketMgr::RemoveGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if(pPacketPart==0)
	{
		return ;
	}

	SC_PacketRemoveGoods Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->RemoveGoods(Req.m_uidGoods);
}



//扩充容量
void PacketMgr::ExtendCapacity(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	
	if(pPacketPart==0)
	{
		return ;
	}

	pPacketPart->ExtendCapacity();	
}

//使用物品
void PacketMgr::UseGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();

	if(pPacketPart==0)
	{
		return ;
	}

	CS_PacketUseGoods_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->UseGoods(Req.m_uidUseGoods,Req.m_uidTarget, Req.m_UseNum, false);	
}

//丢弃物品
void PacketMgr::DiscardGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if(pPacketPart==0)
	{
		return ;
	}

	CS_PacketDiscardGoods_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->DiscardGoods(Req.m_uidGoods);	
}

//装备
void PacketMgr::Equip(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if(pPacketPart==0)
	{
		return ;
	}

	CS_PacketEquip_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->Equip(Req.m_uidGoods,Req.m_uidTarget,Req.m_Pos);	
}

//合成
void PacketMgr::Compose(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();

	if(pPacketPart==0)
	{
		return ;
	}

	CS_PacketCompose_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->Compose(Req.m_gidOutput,Req.m_OutputNum);	
}

//角色更名卡确认使用
void PacketMgr::UseActorChangeName(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ActorChangeName Req;
	ib >> Req;

	if(ib.Error()){
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pEmployee = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidTarget);
	if( 0 == pEmployee){
		return;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(Req.m_uidUseGoods);
	if( 0 == pGoods){
		return;
	}

	const SGoodsUseCnfg * pGoodsUseCnfg=g_pGameServer->GetConfigServer()->GetGoodsUseCnfg(pGoods->GetGoodsID());
	if( 0 == pGoodsUseCnfg){
		return;
	}

	ICDTimerPart * pCDTimerPart = pEmployee->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	const std::vector<SNoChangeName> & vectNoChangeName = g_pGameServer->GetConfigServer()->GetAllNotChangeNameCnfg();

	SC_ActorChangeNameRet Rsp;
	Rsp.m_Result = enPacketRetCode_NoActor;

	if( pActor->GetUID() == Req.m_uidTarget){
		//不能对主角使用
		Rsp.m_Result = enPacketRetCode_ErrChangeUserName;
	}else if( !pCDTimerPart->IsCDTimeOK(pGoodsUseCnfg->m_CDTimerID, pGoodsUseCnfg->m_CDTimeRange)){
		//CD时间未到
		Rsp.m_Result = enPacketRetCode_ErrCDTime;
	}else if( g_pGameServer->GetKeywordFilter()->IsValidName(Req.m_szChangeName) == false){
		//不能使用非法字符
		Rsp.m_Result = enPacketRetCode_ErrKeyword;	
	}else{
		Rsp.m_Result = enPacketRetCode_OK_ChangeName;



		//检测是否是不可以更改名字的角色或不可更改成这名字，用名字判断
		for( int i = 0; i < vectNoChangeName.size(); ++i)
		{
		//fly add	20121106
			const std::string & strName = g_pGameServer->GetGameWorld()->GetLanguageStr(vectNoChangeName[i].m_NoChangeNameLangID);
			if( 0 == strcmp(pEmployee->GetName(), strName.c_str())){
				Rsp.m_Result = enPacketRetCode_NoChangeCardName;
				break;
			}

			if( 0 == strcmp(Req.m_szChangeName, strName.c_str())){
				Rsp.m_Result = enPacketRetCode_NoChangeThisName;
				break;
			}
			
			//const std::string & strName = vectNoChangeName[i].m_NoChangeName;

			//if( 0 == strcmp(pEmployee->GetName(), strName.c_str())){
			//	Rsp.m_Result = enPacketRetCode_NoChangeCardName;
			//	break;
			//}

			//if( 0 == strcmp(Req.m_szChangeName, strName.c_str())){
			//	Rsp.m_Result = enPacketRetCode_NoChangeThisName;
			//	break;
			//}
		}

		if( Rsp.m_Result == enPacketRetCode_OK_ChangeName){

			IPacketPart * pPacketPart = pActor->GetPacketPart();
			if( 0 == pPacketPart){
				return;
			}

			pEmployee->SetName(Req.m_szChangeName);
			
			//删除物品
			pPacketPart->DestroyGoods(pGoods->GetGoodsID(), 1);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Use,pGoodsUseCnfg->m_GoodsID,UID(),1,"使用角色更名卡");

			pCDTimerPart->RegistCDTime(pGoodsUseCnfg->m_CDTimerID, pGoodsUseCnfg->m_CDTimeRange);	
		}
	}

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_ActorChangeName, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//使用传音符
void PacketMgr::UseChangYinFu(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ChangYinFu ChangYinFu;
	ib >> ChangYinFu;

	if(ib.Error()){
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	IGoods * pGoods = pPacketPart->GetGoods(ChangYinFu.m_uidUseGoods);
	if( 0 == pGoods){
		return;
	}

	const SGoodsUseCnfg * pGoodsUseCnfg=g_pGameServer->GetConfigServer()->GetGoodsUseCnfg(pGoods->GetGoodsID());
	if( 0 == pGoodsUseCnfg){
		return;
	}

	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	SC_ChangYinFuRet ChangYinFuRet;
	ChangYinFuRet.m_Result = enPacketRetCode_OK;

	if( !pCDTimerPart->IsCDTimeOK(pGoodsUseCnfg->m_CDTimerID, pGoodsUseCnfg->m_CDTimeRange)){
		//CD时间未到
		ChangYinFuRet.m_Result = enPacketRetCode_ErrCDTime;
	}else{
		//关键字过滤	
		IKeywordFilter * pKeywordFilter = g_pGameServer->GetKeywordFilter();
		if( 0 != pKeywordFilter){
			pKeywordFilter->Filter(ChangYinFu.m_szContext);
		}

		g_pGameServer->GetGameWorld()->WorldNotice(pActor, ChangYinFu.m_szContext);

		pPacketPart->DestroyGoods(ChangYinFu.m_uidUseGoods, 1);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Use,pGoodsUseCnfg->m_GoodsID,UID(),1,"使用传音符");

		pCDTimerPart->RegistCDTime(pGoodsUseCnfg->m_CDTimerID, pGoodsUseCnfg->m_CDTimeRange);
	}

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_ChangYinFu, sizeof(ChangYinFuRet)) << ChangYinFuRet;
	pActor->SendData(ob.TakeOsb());

}

//物品出售
void PacketMgr::SellGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_SellPacketGoods_Req SellGoods;
	
	ib >> SellGoods;

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	pPacketPart->SellGoods(SellGoods.m_GoodsUID, SellGoods.m_SellGoodsNum);
}

//确认使用物品
void PacketMgr::ConfirmationUseGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();

	if(pPacketPart==0)
	{
		return ;
	}

	CS_ConfirmationUseGoods_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pPacketPart->UseGoods(Req.m_uidUseGoods,Req.m_uidTarget, Req.m_UseNum, Req.m_Index);	
}

//点击人物面板的资质按键
void PacketMgr::ClickAptitude(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ClickAptitude Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	const SGoodsUseCnfg * pGoodsUseCnfg = g_pGameServer->GetConfigServer()->GetGoodsUseCnfg(Req.m_GoodsID);

	if ( 0 == pGoodsUseCnfg )
	{
		TRACE("<error> %s : %d Line 获取物品使用配置失败！！物品ID = %d", __FUNCTION__, __LINE__, Req.m_GoodsID);
		return;
	}

	if ( pGoodsUseCnfg->m_vectParam.size() < 1 )
	{
		TRACE("<error> %s : %d Line 物品使用参数个数错误！！个数%d", __FUNCTION__, __LINE__, pGoodsUseCnfg->m_vectParam.size());
		return;
	}

	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_TargetUID);

	if ( 0 == pTargetActor )
	{
		return;
	}

	INT32 Aptitude = pTargetActor->GetCrtProp(enCrtProp_ActorAptitude);

	const SUpAptitude * pUpAptitude = g_pGameServer->GetConfigServer()->GetUpAptitude(Aptitude);

	if ( 0 == pUpAptitude )
	{
		TRACE("<error> %s : %d Line 获取资质提升配置信息失败！！当前资质%d", __FUNCTION__, __LINE__, Aptitude);
		return;
	}

	SC_ClickAptitude Rsp;

	Rsp.m_GoodsID = pUpAptitude->m_NeedGoods;

	if ( Rsp.m_GoodsID != INVALID_GOODS_ID )
	{
		pActor->SendGoodsCnfg(Rsp.m_GoodsID);
	}

	Rsp.m_GoodsNum = pUpAptitude->m_NeedNum;
	Rsp.m_DescAptitude = pUpAptitude->m_MinAptitudePoint;

	++Rsp.m_DescAptitude;

	//if ( Rsp.m_DescAptitude > 1 )
	//{
	//	//比如描述时1801要变成1800
	//	--Rsp.m_DescAptitude;
	//}

	//提升后资质
	float NextAptitude = (pTargetActor->GetCrtProp(enCrtProp_ActorAptitude) + pGoodsUseCnfg->m_vectParam[0]) / 1000.0f;

	//当前等级
	UINT8 Level = pTargetActor->GetCrtProp(enCrtProp_Level);

	const SActorLayerCnfg * pActorLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pTargetActor->GetCrtProp(enCrtProp_ActorLayer));

	if ( 0 == pActorLayerCnfg){
	
		TRACE("<error> %s : %d Line 获取玩家境界配置信息出错！！境界等级:%d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_ActorLayer));
		return;
	}

	//增加的灵力
	INT32 AddSpirit = pTargetActor->RecalculateSpiri(NextAptitude, Level, pTargetActor->GetCrtProp(enCrtProp_ForeverSpirit), pActorLayerCnfg->m_AddSpirit) - pTargetActor->GetCrtProp(enCrtProp_ActorSpiritBasic);
	Rsp.m_NextSpirit = pTargetActor->GetCrtProp(enCrtProp_ActorSpiritBasic) + AddSpirit;

	//增加的护盾
	INT32 AddShield = pTargetActor->RecalculateShield(NextAptitude, Level ,pTargetActor->GetCrtProp(enCrtProp_ForeverShield) , pActorLayerCnfg->m_AddShield) - pTargetActor->GetCrtProp(enCrtProp_ActorShieldBasic);
	Rsp.m_NextShield = pTargetActor->GetCrtProp(enCrtProp_ActorShieldBasic) + AddShield;

	//增加的身法
	INT32 AddAvoid =  pTargetActor->RecalculateAvoid(NextAptitude,Level , pTargetActor->GetCrtProp(enCrtProp_ForeverAvoid) , pActorLayerCnfg->m_AddAvoid) - pTargetActor->GetCrtProp(enCrtProp_ActorAvoidBasic);
	Rsp.m_NextAvoid = pTargetActor->GetCrtProp(enCrtProp_ActorAvoidBasic) + AddAvoid;

	//增加的气血上限
	INT32 AddBloodUp = pTargetActor->RecalculateBoold(NextAptitude,Level , pTargetActor->GetCrtProp(enCrtProp_ForeverBloodUp) , pActorLayerCnfg->m_AddBlood) - pTargetActor->GetCrtProp(enCrtProp_ActorBloodUpBasic);
	Rsp.m_NextBloodUp = pTargetActor->GetCrtProp(enCrtProp_ActorBloodUpBasic) + AddBloodUp;

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_ClickAptitude, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}
