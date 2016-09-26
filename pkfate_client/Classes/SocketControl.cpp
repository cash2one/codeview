#include "cocos2d.h"
#include "SocketControl.h"
#include "SceneStart.h"
#include "PKNotificationCenter.h"
#include "Settings.h"
#include "cmd.h"
#include "DZControl.h"

static SocketControl *m_pInstance = nullptr;
SocketControl::SocketControl()
	:mSocket(nullptr)
{
}

SocketControl::~SocketControl()
{
	Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
    if (mSocket){
		delete mSocket;
        mSocket=nullptr;
    }
}
SocketControl* SocketControl::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new SocketControl();
		if (m_pInstance)
		{
			if (!m_pInstance->init())
			{
				delete m_pInstance;
				m_pInstance = nullptr;
			}
		}
	}
	return m_pInstance;
}
bool SocketControl::init()
{
	mSocket = new SocketClient();
	mSocket->onRecv = CC_CALLBACK_1(SocketControl::OnMessage, this);
	mSocket->onDisconnect = CC_CALLBACK_0(SocketControl::OnDisconnected, this);

	respQueue = new MessageQueue();
	isMessageEmpty = true;
	Director::getInstance()->getScheduler()->scheduleSelector(schedule_selector(SocketControl::dispatchResponseCallbacks), this, 0.03, false);
	return true;
}

void SocketControl::dispatchResponseCallbacks(float delta)
{
	if (isMessageEmpty) return;
	RECVMSG *msg = getAndRemoveMessageFromReceiveQueue();
	if (msg != nullptr){

		if (msg->type == MSGTYPE_SNG_DZPK)//如果回调类型是 sng赛事德州 则切换
		{
			if (DZControl::GetInstance()->isSNG){
				msg->type = MSGTYPE_DZPKGAMEMO;
			}
		}
		
		PKNotificationCenter::getInstance()->postNotification(GetMsgTypeString(msg->type, msg->typesub), (Ref*)msg);
		msg->release();
	}
}


void SocketControl::OnMessage(SOCKETDATA *data)
{
	RECVMSG *msg = RECVMSG::create(data);
	if (msg) {
		
		/*PKNotificationCenter::getInstance()->postNotification(GetMsgTypeString(data->type, data->typesub), (Ref*)msg);
		msg->release();*/

		addMessageToReceiveQueue(msg);
	}
}


void SocketControl::addMessageToReceiveQueue(RECVMSG* m){
	std::unique_lock<std::mutex> lck(socket_resp_mutex);
	respQueue->push(m);
	isMessageEmpty = false;
}


RECVMSG* SocketControl::getAndRemoveMessageFromReceiveQueue(){

	std::unique_lock<std::mutex> lck(socket_resp_mutex);
	RECVMSG* m = respQueue->pop();
	if (!m){
		isMessageEmpty = true;
	}
	return m;
}


void SocketControl::close()
{
	//此处只关闭连接，系统会自动调用 onDisconnected
	mSocket->close();
}


void SocketControl::OnDisconnected()
{
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto scene = SceneStart::create();
		// run
		if (scene)
			Director::getInstance()->replaceScene(scene);
	});
}
bool SocketControl::CheckConnect()
{
	return mSocket->IsConnected() ? true : mSocket->connectServer(SERVERURL, SERVERPORT);
}
bool SocketControl::SendMsg(unsigned short type, unsigned short typesub, std::string data)
{
	if (!CheckConnect())
	{
		CCLOG("[SocketControl.SendMsg]: connect error. pls check network.");
		return false;
	}
	
	if (type == MSGTYPE_DZPKGAMEMO )//如果回调类型是 sng赛事德州 则切换
	{
		if (DZControl::GetInstance()->isSNG){
			type = MSGTYPE_SNG_DZPK;
		}
	}

	bool result = mSocket->sendMessage(type, typesub, data.c_str());
	if (!result)
		CCLOG("[SocketControl.SendMsg]:send message error");
	return result;
}