
#include "IActor.h"
#include "RelationServer.h"
#include "ISyndicateMgr.h"
#include "SyndicateMgr.h"

IGameServer * g_pGameServer = 0;


IRelationServer * CreateRelationServer(IGameServer *pServerGlobal)
{
	g_pGameServer = pServerGlobal;

	RelationServer * pRelationServer = new RelationServer();
	if(pRelationServer->Create()==false)
	{
		delete pRelationServer;
		pRelationServer = 0;
	}

	return pRelationServer;
}


RelationServer::RelationServer()
{
	m_pSyndicateMgr = 0;
}

RelationServer::~RelationServer()
{
}

bool RelationServer::Create()
{



	if( false == m_Trade.Create()){
		return false;
	}

	m_pSyndicateMgr = new SyndicateMgr();
	if( false == m_pSyndicateMgr->Create()){
		delete m_pSyndicateMgr;
		return false;
	}

	if( false == m_Talk.Create()){
		return false;
	}

	if( false == m_SystemMsg.Create()){
		return false;
	}

	//if( false == m_SyndicateShop.Create((SyndicateMgr *)m_pSyndicateMgr)){
	//	return false;
	//}

	//if( false == m_SynMagic.Create((SyndicateMgr *)m_pSyndicateMgr)){
	//	return false;
	//}

	if( false == m_Rank.Create()){
		return false;
	}

	return true;
}

ISyndicateMgr * RelationServer::GetSyndicateMgr()
{
	return m_pSyndicateMgr;
}

//释放
void RelationServer::Release(void)
{
	delete this;
}

void RelationServer::Close()
{
	if(m_pSyndicateMgr)
	{
		m_pSyndicateMgr->Close();
		delete m_pSyndicateMgr;
		m_pSyndicateMgr = 0;
	}

	m_Talk.Close();
	m_Trade.Close();
}

//帮派招募成员
void RelationServer::ZhaoMuSynMember(IActor * pActor)
{
	m_Talk.ZhaoMuSynMember(pActor);
}


		//注册GM命令处理者
bool RelationServer::RegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler)
{
	if( 0 == Handler){
		return false;
	}

	m_mapGMCmdHandle[(UINT8)Cmd] = Handler;

	return true;
}

		//取消注册GM命令处理者
bool RelationServer::UnRegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler)
{
	if( 0 == Handler){
		return false;
	}
	
	MAP_GMCMDHANDLE::iterator iter = m_mapGMCmdHandle.find((UINT8)Cmd);
	if( iter == m_mapGMCmdHandle.end()){
		return false;
	}

	m_mapGMCmdHandle.erase(iter);

	return true;
}

//分发GM命令
void RelationServer::DispatchGMMessage(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam)
{
	MAP_GMCMDHANDLE::iterator iter = m_mapGMCmdHandle.find(Cmd);
	if( iter == m_mapGMCmdHandle.end()){
		return;
	}

	IGMCmdHandler * pGMCmdHandle = iter->second;
	if( 0 == pGMCmdHandle){
		return;
	}

	pGMCmdHandle->OnHandleGMCmd(pActor, Cmd, vectParam);
}

ITalk * RelationServer::GetTalk()
{
	return &m_Talk;
}

ISystemMsg * RelationServer::GetSystemMsg()
{
	return &m_SystemMsg;
}

//右上角的提示悬浮框
void  RelationServer::SendTipBox(IActor * pActor, const char * pszTip)
{
	m_Talk.SendTipBox(pActor, pszTip);
}

//刷新帮战排行
void  RelationServer::WarEndFlushRank()
{
	m_Rank.FlushSynWarRank();
}
