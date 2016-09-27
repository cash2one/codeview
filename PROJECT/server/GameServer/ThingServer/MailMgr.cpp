#include "GameSrvProtocol.h"
#include "MailMgr.h"
#include "IMailPart.h"
#include "IActor.h"
#include "IBasicService.h"
#include <string>
#include "ThingServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IFriendPart.h"
#include "ICDTimerPart.h"
#include <vector>
#include "windows.h"
#include  <process.h>
#import "jmail.dll"
using namespace jmail;


//全局事件句柄，用于线程控制
HANDLE   g_hEvent;

std::vector<KFMail_Data>  g_vectKFMail;

MailMgr::MailMgr()
{
}

MailMgr::~MailMgr()
{
	CloseHandle(g_hEvent);
}

unsigned   __stdcall   SendMail(   void*   pArguments   )
{  
	const SServer_Info & ServerInfo = g_pGameServer->GetServerInfo();

	try
	{
		const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

		//   COM的初始化 
		::CoInitialize(NULL);

		jmail::IMessagePtr pMessage("JMail.Message");
		
		// 发件人邮箱
		pMessage->From = GameParam.m_KFMailAddress.c_str();

		pMessage->MailServerUserName = GameParam.m_KFMailAddress.c_str();

		pMessage->MailServerPassWord = GameParam.m_KFMailPassword.c_str();
		
		// 发件人姓名
		pMessage->FromName = "客服";
		
		// 优先级设置,1-5逐次降低, 3为中级
		pMessage->Priority = 3;
		
		// 编码方式设置, 默认是iso-8859-1
		pMessage->Charset = "GB2312";
		
		while(true)
		{
			WaitForSingleObject(g_hEvent, INFINITE);

			// 添加收件人
			pMessage->AddRecipient(ServerInfo.m_strServiceEmail.c_str(), "剑仙传奇", "");

			for( int i = 0; i < g_vectKFMail.size(); ++i)
			{
				// 主题
				pMessage->Subject = g_vectKFMail[i].szTheme;
				
				// 正文
				pMessage->Body = g_vectKFMail[i].szBody;
				
				// 开始发送
				//pMessage->Send(/*(LPCTSTR)L*/"smtp.qq.com",VARIANT_FALSE);
				pMessage->Send(GameParam.m_KFMailServer.c_str(),VARIANT_FALSE);

				TRACE("发送客服邮件成功：%s!", ServerInfo.m_strServiceEmail.c_str());
			}

			g_vectKFMail.clear();

			//重新设为未重置状态
			ResetEvent(g_hEvent);
		}

		pMessage.Release();

	}
	catch (_com_error e)
	{
		TRACE("发送客服邮件失败：%s", ServerInfo.m_strServiceEmail.c_str());
	}

	CoUninitialize();

    return   0;   
}  

bool MailMgr::Create()
{
	//创建人工事件对象
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE hThread;

	//创建发邮件线程
	hThread = (HANDLE)_beginthreadex(   NULL,   0,   &SendMail,   0,   0,   0);

	CloseHandle(hThread);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	char szAddress[1024] = "0";

	strncpy(szAddress, GameParam.m_KFMailAddress.c_str(), sizeof(szAddress));
	
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Mail,this);
}

void MailMgr::Close()
{
}

//收到MSG_ROOT消息
void MailMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef  void (MailMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enMailCmd_Max] = 
	{
		&MailMgr::ViewMailMail,
		&MailMgr::ViewMail,
		&MailMgr::WritePersonMail,
		&MailMgr::WriteSynMail,
		&MailMgr::AcceptSynInvite,
		&MailMgr::RefuseSynInvite,
		&MailMgr::AcceptMailItem,
		&MailMgr::DeleteMail,
		&MailMgr::CloseMailWindow,
		&MailMgr::ClickFriend,
		&MailMgr::SendCustomerServiceMail,
	};

	
	 if(nCmd >= enMailCmd_Max || 0 == s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}

//IDBProxyClientSink接口
void MailMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IMailPart * pMailPart = pActor->GetMailPart();
	if( 0 == pMailPart){
		return;
	}

	pMailPart->OnDBRet(userID, ReqCmd, nRetCode, RspOsb, ReqOsb, userdata);
}

//显示邮件主界面
void	MailMgr::ViewMailMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewMailMain_Req Req;

	ib >> Req;
	
	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据包长度错误！！", __FUNCTION__, __LINE__);
		return;
	}

	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 != pMailPart){
		pMailPart->ViewMailMain(Req.m_Begin, Req.m_End);
	}
}

//查看邮件
void	MailMgr::ViewMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewMailData_Req Req;

	ib >> Req;

	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 != pMailPart){
		pMailPart->ViewMail(Req.m_MailID);
	}
}

//写私人邮件
void	MailMgr::WritePersonMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_WritePersonMail_Req Req;

	ib >> Req;

	SWriteData WriteData;
	
	strncpy(WriteData.m_szContentText, Req.m_szContentText, sizeof(WriteData.m_szContentText));
	strncpy(WriteData.m_szDestUserName, Req.m_szDestUserName, sizeof(WriteData.m_szDestUserName));
	strncpy(WriteData.m_szThemeText, Req.m_szThemeText, sizeof(WriteData.m_szThemeText));

	WriteData.m_MailType = enMailType_Person;	//非帮派邮件

	pMailPart->WriteMail(WriteData);
}


//写帮派邮件
void	MailMgr::WriteSynMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}
	ISyndicate * pSyn = pSynMgr->GetSyndicate(pActor->GetUID());
	if( 0 == pSyn){
		return;
	}

	CS_WriteSynMail_Req Req;

	ib >> Req;

	SWriteData WriteData;

	// fly add	20121106

	std::string strSynName(g_pGameServer->GetGameWorld()->GetLanguageStr(10012));	//玩家的帮派名称，现在没有，先用这代替
	const std::string strContent = strSynName + g_pGameServer->GetGameWorld()->GetLanguageStr(10013);
	//std::string strSynName("帮派名称");	//玩家的帮派名称，现在没有，先用这代替
	//const std::string strContent = strSynName + "帮派邀请您加入";
	
	strncpy(WriteData.m_szContentText, strContent.c_str(), sizeof(WriteData.m_szContentText));
	strncpy(WriteData.m_szDestUserName, Req.m_szDestUserName, sizeof(WriteData.m_szDestUserName));
	strncpy(WriteData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10011), sizeof(WriteData.m_szThemeText));
	//strncpy(WriteData.m_szThemeText, "邀请加入", sizeof(WriteData.m_szThemeText));

	WriteData.m_MailType = enMailType_Syn;	//帮派邮件

	pMailPart->WriteMail(WriteData);
}

//接受帮派邀请
void	MailMgr::AcceptSynInvite(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_MailID_Req Req;

	ib >> Req;

	pMailPart->AcceptSynInvite(Req.m_MailID);
}

//拒绝帮派邀请
void	MailMgr::RefuseSynInvite(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_DeleteMail_Req Req;

	ib >> Req;

	pMailPart->RefuseSynInvite(Req.m_MailID);

}

//接受邮件物品
void	MailMgr::AcceptMailItem(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_MailID_Req Req;

	ib >> Req;

	pMailPart->AcceptMailItem(Req.m_MailID);
}

//删除邮件
void	MailMgr::DeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_MailID_Req Req;

	ib >> Req;

	pMailPart->DeleteMail(Req.m_MailID);
}

//确认删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
void	MailMgr::OKDeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_MailID_Req Req;

	ib >> Req;

	pMailPart->OKDeleteMail(Req.m_MailID);
}

//取消删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
void	MailMgr::CancelDeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	CS_MailID_Req Req;

	ib >> Req;

	pMailPart->CancelDeleteMail(Req.m_MailID);
}

//关闭邮件窗口
void	MailMgr::CloseMailWindow(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IMailPart * pMailPart = pActor->GetMailPart();

	if( 0 == pMailPart){
		return;
	}

	//清除数据
	pMailPart->ClearAllMailData();
	
}

//点击选择收信人按钮
void	MailMgr::ClickFriend(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFriendPart * pFriendPart = pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->ClickMailFriend();
}

//unsigned   __stdcall   SendMail(   void*   pArguments   )   
//{   
//	try
//	{
//		KFMail_Data * pKFData = (KFMail_Data *)pArguments;
//		if( 0 == pKFData){
//			return 0;
//		}
//
//		//   COM的初始化 
//		::CoInitialize(NULL);
//
//		jmail::IMessagePtr pMessage("JMail.Message");
//		
//		// 发件人邮箱
//		pMessage->From = /*(LPCTSTR)L*/"497007809@qq.com";
//
//		pMessage->MailServerUserName = "497007809@qq.com";
//
//		pMessage->MailServerPassWord = "497007809";
//		
//		// 发件人姓名
//		pMessage->FromName = "我的名字";
//		
//		// 添加收件人
//		pMessage->AddRecipient(/*(LPCTSTR)L*/"jiansen@newpolar.cn", "森", "");
//		
//		// 优先级设置,1-5逐次降低, 3为中级
//		pMessage->Priority = 3;
//		
//		// 编码方式设置, 默认是iso-8859-1
//		pMessage->Charset = "GB2312";
//		
//		// 主题
//		pMessage->Subject = pKFData->szTheme;
//		
//		// 正文
//		pMessage->Body = pKFData->szBody;
//		
//		// 开始发送
//		pMessage->Send(/*(LPCTSTR)L*/"smtp.qq.com",VARIANT_FALSE);
//
//		pMessage.Release();
//		
//		printf("发送成功!");
//
//		CoUninitialize();
//
//		delete pKFData;
//	}
//	catch (_com_error e)
//	{
//		printf("发送失败");
//	}
//
//	CoUninitialize();
//
//    return   0;   
//}  

//发送客服邮件
void	MailMgr::SendCustomerServiceMail(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICDTimerPart * pCDTmerPart = pActor->GetCDTimerPart();
	if( 0 == pCDTmerPart){
		return;
	}

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	SC_WriteKFMail_Rsp Rsp;

	if( !pCDTmerPart->IsCDTimeOK(GameParam.m_KFMailCDTimerID)){
		//发送失败，冷却时间未到
		Rsp.MailRetCode = enMailRetCode_ErrCDTimer;
	}else{
		CS_WriteKFMail_Req Req;
		ib >> Req;
		if( ib.Error()){
			TRACE("<error> %s : %d 行 客户端消息长度有误！！", __FUNCTION__, __LINE__);
			return;
		}

		KFMail_Data KFData;

		strncpy(KFData.szTheme, Req.m_szThemeText, sizeof(KFData.szTheme));

		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return;
		}

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pActor->GetUID());
		
		if( 0 == pSyndicate){
			// fly add	20121106
			sprintf_s(KFData.szBody, sizeof(KFData.szBody), g_pGameServer->GetGameWorld()->GetLanguageStr(10047), pActor->GetName(), pActor->GetCrtProp(enCrtProp_ActorUserID), pActor->GetCrtProp(enCrtProp_Level), Req.m_szContentText);
			//sprintf_s(KFData.szBody, sizeof(KFData.szBody), "玩家名称 %s(%d);玩家等级 %d;没帮派;  %s", pActor->GetName(), pActor->GetCrtProp(enCrtProp_ActorUserID), pActor->GetCrtProp(enCrtProp_Level), Req.m_szContentText);
		}else{
			sprintf_s(KFData.szBody, sizeof(KFData.szBody), g_pGameServer->GetGameWorld()->GetLanguageStr(10048), pActor->GetName(), pActor->GetCrtProp(enCrtProp_ActorUserID), pActor->GetCrtProp(enCrtProp_Level), pSyndicate->GetSynName(), Req.m_szContentText);
			//sprintf_s(KFData.szBody, sizeof(KFData.szBody), "玩家名称 %s(%d);玩家等级 %d;帮派名称 %s;  %s", pActor->GetName(), pActor->GetCrtProp(enCrtProp_ActorUserID), pActor->GetCrtProp(enCrtProp_Level), pSyndicate->GetSynName(), Req.m_szContentText);
		}

		g_vectKFMail.push_back(KFData);

		//触发事件对象
		SetEvent(g_hEvent);

		pCDTmerPart->RegistCDTime(GameParam.m_KFMailCDTimerID);
	}

	OBuffer1k ob;
	ob << MailHeader(enMailCmd_CustomerService, sizeof(SC_WriteKFMail_Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}
