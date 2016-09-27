#ifndef __RELATIONSERVER_SENDWORLDVIEWGOODS_H__
#define __RELATIONSERVER_SENDWORLDVIEWGOODS_H__

#include "GameSrvProtocol.h"
#include "IActor.h"
#include "IVisitWorldThing.h"
#include "TalkCmd.h"
#include "IMagicPart.h"
#include "IMagic.h"

class SendWorldViewGoods : public IVisitWorldThing
{
public:
	SendWorldViewGoods(IActor * pActor, const UID & uidGoods)
	{
		if ( 0 == pActor){
			
			return;
		}

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(uidGoods);
		if ( 0 == pGoods){

			return;
		}

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if ( 0 == pChengJiuPart){

			return;
		}

		m_GoodsID = pGoods->GetGoodsID();

		std::string strTitleName = pChengJiuPart->GetTitle();

		m_Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		m_Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
		m_Rsp.m_SuperLink = enSuperLink_Goods;

		m_GoodsRsp.m_GoodsID = pGoods->GetGoodsID();
		m_GoodsRsp.m_uidGoods = uidGoods;
	}

	virtual void Visit(IThing * pThing)
	{
		if(pThing==0 || pThing->GetThingClass() != enThing_Class_Actor)
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		pActor->SendGoodsCnfg(m_GoodsID);
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SycWorldViewItem, sizeof(SC_WorldViewThing_Rsp) + sizeof(SynViewGoods)) << m_Rsp << m_GoodsRsp;

		pActor->SendData(ob.TakeOsb());	
	}

private:
	SC_WorldViewThing_Rsp	m_Rsp;
	SynViewGoods			m_GoodsRsp;
	TGoodsID				m_GoodsID;
};

class SendWorldViewMagic : public IVisitWorldThing
{
public:
	SendWorldViewMagic(IActor * pActor, TMagicID MagicID)
	{
		if ( 0 == pActor){
			return;
		}

		m_MagicID   = MagicID;

		IMagicPart *pMagicPart = pActor->GetMagicPart();

		if ( 0 == pMagicPart){
			return;
		}

		IMagic * pMagic = pMagicPart->GetMagic(MagicID);
		if ( 0 == pMagic){
			return;
		}

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if ( 0 == pChengJiuPart){

			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		m_Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		m_Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
		m_Rsp.m_SuperLink = enSuperLink_Magic;

		m_MagicRsp.m_MagicID = MagicID;
		m_MagicRsp.m_Level	= pMagic->GetLevel();
	}

	virtual void Visit(IThing * pThing)
	{
		if(pThing==0 || pThing->GetThingClass() != enThing_Class_Actor)
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		pActor->SendMagicLevelCnfg(m_MagicID);
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SycWorldViewItem, sizeof(SC_WorldViewThing_Rsp) + sizeof(SynViewMagic)) << m_Rsp << m_MagicRsp;

		pActor->SendData(ob.TakeOsb());	
	}
private:
	SC_WorldViewThing_Rsp  m_Rsp;
	SynViewMagic		   m_MagicRsp;
	TMagicID			   m_MagicID;
};

class SendWorldViewChengJiu : public IVisitWorldThing
{
public:
	SendWorldViewChengJiu(IActor * pActor, TChengJiuID ChengJiuID, UINT32 nFinishTime)
	{
		if ( 0 == pActor){
			return;
		}

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if ( 0 == pChengJiuPart){
			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		m_Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		m_Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
		m_Rsp.m_SuperLink = enSuperLink_ChengJiu;

		m_ChengJiuRsp.m_ChengJiuID = ChengJiuID;
		m_ChengJiuRsp.m_FinishTime = nFinishTime;
	}

	virtual void Visit(IThing * pThing)
	{
		if(pThing==0 || pThing->GetThingClass() != enThing_Class_Actor)
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SycWorldViewItem, sizeof(SC_WorldViewThing_Rsp) + sizeof(SynViewChengJiu)) << m_Rsp << m_ChengJiuRsp;

		pActor->SendData(ob.TakeOsb());	
	}

private:
	SC_WorldViewThing_Rsp m_Rsp;
	SynViewChengJiu		  m_ChengJiuRsp;
};

#endif
