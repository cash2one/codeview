/**
 * 视频纹理缓存
 * @author leoluo<luochong1987@gmail.com>
 */
#ifndef __CCVIDEOTEXTURE_CACHE_H__
#define __CCVIDEOTEXTURE_CACHE_H__
#include "stdio.h"
#include "signal.h"
#include <thread>
#include <mutex>
#include <string>
#include "CCVideoDecode.h"

NS_CC_BEGIN
class  CCVideoTextureCache : public Ref
{
    private:
		std::mutex  s_decodeMutex;

        std::thread s_decodeThread;
		std::condition_variable s_decodeCond;  //初始化条件变量

		CCVideoDecode* m_videoDecode;
		std::queue<CCVideoPic*>m_videoPics;

		static void* videoDecodeHandler(void *data);

	protected:
		
	public:
		int getMaxPass();
		std::condition_variable* getDecodeCond();
		std::mutex * getDecodeMutex();
		CCVideoDecode* getVideoDecode();
		
		CCVideoTextureCache();
		virtual ~CCVideoTextureCache();

		static CCVideoTextureCache * getInstance();
		static void purgeSharedTextureCache();
		
		//Texture2D* addImageWidthData(const char *filename, int frame, const void *data, CCTexture2DPixelFormat pixelFormat, unsigned int pixelsWide, unsigned int pixelsHigh, const CCSize& contentSize);
		CCVideoPic* getVideoPic(int frame);

		void removeVideoPics();

		void removeVideoPic(int frame);

	    /**
	     * 视频解码
	     */
		CCVideoDecode* addVideo(const char *path, bool loop = false);
	    void removeVideo();

	    void addPicData(CCVideoPic *pVideoPic);
	    void picToTexture(float dt);

};
NS_CC_END
#endif //__CCVIDEOTEXTURE_CACHE_H__

