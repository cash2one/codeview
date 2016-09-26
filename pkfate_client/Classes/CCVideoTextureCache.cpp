/**
 * 视频材质缓存类
 */

#include "CCVideoTextureCache.h"
#include "CCVideoDecode.h"
#include "cocos2d.h"
#include <sstream>
#include <queue>


USING_NS_CC;

using namespace std;

NS_CC_BEGIN

#define MAX_PRE_LOAD 5;

static CCVideoTextureCache *g_sharedTextureCache = NULL;


CCVideoTextureCache * CCVideoTextureCache::getInstance()
{
    if (!g_sharedTextureCache)
    {
        g_sharedTextureCache = new CCVideoTextureCache();
    }
    return g_sharedTextureCache;
}



void CCVideoTextureCache::purgeSharedTextureCache()
{
    CC_SAFE_RELEASE_NULL(g_sharedTextureCache);
}



CCVideoDecode* CCVideoTextureCache::getVideoDecode()
{
	return m_videoDecode;
}


std::condition_variable* CCVideoTextureCache::getDecodeCond()
{
	return &s_decodeCond;
}

std::mutex * CCVideoTextureCache::getDecodeMutex()
{
	return &s_decodeMutex;
}



void* CCVideoTextureCache::videoDecodeHandler(void *data)
{
	CCVideoDecode * videoDecoder = CCVideoTextureCache::getInstance()->getVideoDecode();
	std::mutex * mutexRq = CCVideoTextureCache::getInstance()->getDecodeMutex();
	std::condition_variable* conditionRq = CCVideoTextureCache::getInstance()->getDecodeCond();
	

	while (!videoDecoder->isClose()){
		std::unique_lock<std::mutex> lck(*mutexRq);

		if (videoDecoder->isPassPlay()){
			conditionRq->wait(lck); // 等待
		}
		else{
			videoDecoder->decode();
		}

	}

	videoDecoder->dispose();
	return 0;
}

int CCVideoTextureCache::getMaxPass()
{
	return MAX_PRE_LOAD;
}


//同时间只支持一个路径解析播放
CCVideoDecode* CCVideoTextureCache::addVideo(const char *path, bool loop)
{
	if (m_videoDecode == nullptr)
	{
		m_videoDecode = new CCVideoDecode();
	}
	if (!m_videoDecode->isDispose()) //必须释放才能init 新路径,removeVideo 中同步锁
	{
		removeVideo();
	}

	if (m_videoDecode->init(path))
	{
		m_videoDecode->setLoop(loop);
		//开启线程进行解码
		std::thread s_decodeThread(videoDecodeHandler, this);
		s_decodeThread.detach();
	}
	else
	{
		CCLOGERROR("CCVideoDecode init error in CCVideoTextureCache");
		return NULL;
	}

	return m_videoDecode;
}



void CCVideoTextureCache::addPicData(CCVideoPic *pVideoPic)
{
	
	if (pVideoPic != nullptr){
		
		m_videoPics.push(pVideoPic);
	}
}


CCVideoPic* CCVideoTextureCache::getVideoPic(int frame)
{
	CCVideoDecode * videoDecoder = CCVideoTextureCache::getInstance()->getVideoDecode();
	std::condition_variable* conditionRq = CCVideoTextureCache::getInstance()->getDecodeCond();
	videoDecoder->setPlayedFrame(frame);
	CCVideoPic* videoPic = nullptr;
	std::mutex * mutexRq = CCVideoTextureCache::getInstance()->getDecodeMutex();
	std::unique_lock<std::mutex> lck(*mutexRq);

	while (m_videoPics.size() > 0){  //将在帧之前的videoPic删除。防止跳帧导致泄漏
		videoPic = m_videoPics.front();
		if (videoPic == nullptr){
			m_videoPics.pop();
			continue;
		}
		if (videoPic->m_frame < frame){ //  frame递增  在之前的都清除
			m_videoPics.pop();
			delete videoPic;
		}
		else{
		    break;
		}
	}

	conditionRq->notify_all();
	if (videoPic&&videoPic->m_frame == frame)
	{
		return videoPic;
	}

	return nullptr;
	
}


    	
void CCVideoTextureCache::removeVideo()
{
	CCVideoDecode * videoDecoder = CCVideoTextureCache::getInstance()->getVideoDecode();
	std::mutex * mutexRq = CCVideoTextureCache::getInstance()->getDecodeMutex();
	std::unique_lock<std::mutex> lck(*mutexRq);

	if (videoDecoder != nullptr && !videoDecoder->isClose()){
		
		removeVideoPics();

		videoDecoder->close();
		std::condition_variable* conditionRq = CCVideoTextureCache::getInstance()->getDecodeCond();
		while (!m_videoDecode->isDispose()){// wait dipose;
			conditionRq->notify_all();
			while (conditionRq->wait_for(lck, std::chrono::milliseconds(30)) == std::cv_status::timeout) {//等待
				break;
			}
		}

	}
}


void CCVideoTextureCache::removeVideoPics()
{
	CCVideoPic* videoPic = nullptr;
	while (m_videoPics.size() > 0){
		videoPic = m_videoPics.front();
		m_videoPics.pop();
		delete videoPic;
	}
}



void CCVideoTextureCache::removeVideoPic(int frame)
{
	CCVideoPic* videoPic = nullptr;
	while (m_videoPics.size() > 0){
		videoPic = m_videoPics.front();
		if (videoPic->m_frame <= frame){
			m_videoPics.pop();
			delete videoPic;
		}
		else{
			break;
		}
	}

}

CCVideoTextureCache::~CCVideoTextureCache()
{
	CCLOGINFO("cocos2d: deallocing CCVideoTextureCache.");

	if (m_videoDecode != nullptr){
		m_videoDecode->close();
		CC_SAFE_RELEASE(m_videoDecode);
	}
	removeVideoPics();

}

CCVideoTextureCache::CCVideoTextureCache() :m_videoDecode(nullptr)
{
	
}

NS_CC_END


