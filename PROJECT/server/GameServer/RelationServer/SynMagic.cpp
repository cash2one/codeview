
#include "IActor.h"
#include "SynMagic.h"
#include "RelationServer.h"
#include "IConfigServer.h"
#include "SyndicateMgr.h"
#include "ISyndicateMember.h"
#include "ISyndicate.h"
#include "ISynMagicPart.h"
#include "DMsgSubAction.h"
#include "ISynMagicPart.h"

SynMagic::SynMagic()
{
	m_pSyndicateMgr = 0;
}

SynMagic::~SynMagic()
{
}

bool	SynMagic::Create(SyndicateMgr * pSyndicateMgr)
{
	m_pSyndicateMgr = pSyndicateMgr;

	const std::vector<SSynMagicCnfg> vectSynMagic = g_pGameServer->GetConfigServer()->GetAllSynMagicCnfg();
	
	for( int i = 0; i < vectSynMagic.size(); ++i)
	{
		const SSynMagicCnfg & SynMagic = vectSynMagic[i];

		SynMagicData  MagicData;
		MagicData.m_NeedContribution	= SynMagic.m_NeedContribution;
		MagicData.m_NeedStone			= SynMagic.m_NeedStone;
		MagicData.m_NeedSynLevel		= SynMagic.m_NeedSynLevel;

		// fly add
		TLanguageID SynMagicDescLangID;
		SynMagicDescLangID = SynMagic.m_SynMagicDescLangID;
		const SLanguageTypeCnfg * pSynMagicLangConfig = g_pGameServer->GetConfigServer()->GetLanguageTypeCnfg(SynMagicDescLangID);
		if( 0 == pSynMagicLangConfig){
			//TRACE("<error> %s ; %d 行 获取语言类型配置数据出错!!语言ID = %d", __FUNCTION__, __LINE__, SynMagicDescLangID);
			return false;
		}
		MagicData.m_strSynMagicDes		= pSynMagicLangConfig->m_strEnglish;
		//MagicData.m_strSynMagicDes		= SynMagic.m_strSynMagicDes;
		MagicData.m_SynMagicID			= SynMagic.m_SynMagicID;
		MagicData.m_SynMagicLevel		= SynMagic.m_SynMagicLevel;

		// fly add
		TLanguageID SynMagicNameLangID;
		SynMagicNameLangID = SynMagic.m_SynMagicNameLangID;
		const SLanguageTypeCnfg * pLanguageTypeConfig = g_pGameServer->GetConfigServer()->GetLanguageTypeCnfg(SynMagicNameLangID);
		if( 0 == pLanguageTypeConfig){
			//TRACE("<error> %s ; %d 行 获取语言类型配置数据出错!!语言ID = %d", __FUNCTION__, __LINE__, SynMagicNameLangID);
			return false;
		}

		strncpy(MagicData.m_szSynMagicName, pLanguageTypeConfig->m_strEnglish.c_str(), sizeof(MagicData.m_szSynMagicName));		
		//strncpy(MagicData.m_szSynMagicName, SynMagic.m_szSynMagicName, sizeof(MagicData.m_szSynMagicName));
		MagicData.m_vectEffect		= SynMagic.m_vectEffect;
		
		std::hash_map<TSynMagicID, MAPSYNMAGIC>::iterator iter = m_mapSynMagicData.find(MagicData.m_SynMagicID);
		if( iter == m_mapSynMagicData.end()){
			MAPSYNMAGIC mapMagic;
			mapMagic[MagicData.m_SynMagicLevel] = MagicData;
			m_mapSynMagicData[MagicData.m_SynMagicID] = mapMagic;
		}else{
			MAPSYNMAGIC & mapMagic = iter->second;
			mapMagic[MagicData.m_SynMagicLevel] = MagicData;
		}
	}
	return true;
}

//查看帮派技能
void	SynMagic::ViewSynMagic(IActor * pActor)
{
	ISyndicateMember * pSyndicateMember = m_pSyndicateMgr->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSyndicateMember){
		return;
	}

	ISyndicate * pSyndicate = m_pSyndicateMgr->GetSyndicate(pSyndicateMember->GetSynID());
	if( 0 == pSyndicate){
		return;
	}

	ISynMagicPart * pSynMagicPart = pActor->GetSynMagicPart();
	if( 0 == pSynMagicPart){
		return;
	}

	SC_LearnSynMagicNum_Rsp Rsp;

	Rsp.m_SynMagicNum	= m_mapSynMagicData.size();

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_ViewSynMagic, sizeof(Rsp) + Rsp.m_SynMagicNum * sizeof(SC_LearnSynMagicData_Rsp)) << Rsp;

	std::hash_map<TSynMagicID, MAPSYNMAGIC>::iterator iter = m_mapSynMagicData.begin();

	for( ; iter != m_mapSynMagicData.end(); ++iter)
	{
		SC_LearnSynMagicData_Rsp RspData;
		RspData.m_SynMagicID = iter->first;
		RspData.m_Level		 = pSynMagicPart->GetSynMagicLevel(iter->first);

		ob << RspData;
		

		//发送配置信息
		pActor->SendSynMagicCnfg(iter->first);
	}

	pActor->SendData(ob.TakeOsb());
}

//学习帮派技能
void	SynMagic::LearnSynMagic(IActor * pActor, TSynMagicID SynMagicID)
{
	ISyndicateMember * pSyndicateMember = m_pSyndicateMgr->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSyndicateMember){
		return;
	}

	ISyndicate * pSyndicate = m_pSyndicateMgr->GetSyndicate(pSyndicateMember->GetSynID());
	if( 0 == pSyndicate){
		return;
	}

	ISynMagicPart * pSynMagicPart = pActor->GetSynMagicPart();
	if( 0 == pSynMagicPart){
		return;
	}

	SC_LearnSynMagic_Rsp Rsp;
	Rsp.m_SynRetCode = enSynRetCode_OK;

	SynMagicData * pSynMagicData = this->GetSynMagicData(SynMagicID, pSynMagicPart->GetSynMagicLevel(SynMagicID) + 1);
	if( 0 == pSynMagicData){
		//已经达到最高级
		Rsp.m_SynRetCode = enSynRetCode_ErrMaxMagicLevel;

	}else if( pActor->GetCrtProp(enCrtProp_ActorStone) < pSynMagicData->m_NeedContribution){
		//灵石不够
		Rsp.m_SynRetCode = enSynRetCode_NotStone;

	}else if( pSyndicateMember->GetContribution() < pSynMagicData->m_NeedContribution){
		//帮派贡献值是否够
		Rsp.m_SynRetCode = enSynRetCode_NotContribution;

	}else if( pSyndicate->GetSynLevel() < pSynMagicData->m_NeedSynLevel){
		//帮派等级是否够
		Rsp.m_SynRetCode = enSynRetCode_ErrorSynLevel;

	}else{
		pActor->AddCrtPropNum(enCrtProp_ActorStone, -pSynMagicData->m_NeedStone);

		pSyndicateMember->AddContribution(-pSynMagicData->m_NeedContribution);

		Rsp.m_SynRetCode = enSynRetCode_OK;

		//条件都满足则学习
		pSynMagicPart->LearnSynMagicOK(*pSynMagicData);

		//发布事件
		SS_LearnSynMagic LearnSynMagic;
		LearnSynMagic.m_MagicID = SynMagicID;
		LearnSynMagic.m_Level   = pSynMagicPart->GetSynMagicLevel(SynMagicID);

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_LearnSynMagic);
		pActor->OnEvent(msgID,&LearnSynMagic,sizeof(LearnSynMagic));

		//使用帮贡事件
		SS_UseSynContribution UseSynContribution;
		UseSynContribution.m_UseNum = pSynMagicData->m_NeedContribution;

		msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UseSynContribution);
		pActor->OnEvent(msgID,&UseSynContribution,sizeof(UseSynContribution));
	}

	Rsp.m_Contribution = pSyndicateMember->GetContribution();

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_LearnSynMagic, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	if( Rsp.m_SynRetCode == enSynRetCode_OK){
		this->ViewSynMagic(pActor);
	}
}

//得到帮派技能数据
SynMagicData * SynMagic::GetSynMagicData(TSynMagicID SynMagicID, UINT8 SynMagicLevel)
{
	std::hash_map<TSynMagicID, MAPSYNMAGIC>	::iterator iter = m_mapSynMagicData.find(SynMagicID);
	if( iter == m_mapSynMagicData.end()){
		return 0;
	}

	MAPSYNMAGIC & mapMagic = iter->second;

	MAPSYNMAGIC::iterator it = mapMagic.find(SynMagicLevel);
	if( it == mapMagic.end()){
		return 0;
	}

	return &(it->second);
}
