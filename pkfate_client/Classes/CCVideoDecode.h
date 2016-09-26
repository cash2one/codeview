/**
 * 视频解码类
 * 目前只支持视频流、不包括音频
 */

#pragma once
#include "cocos2d.h"



extern "C" { 
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"  
    #include "libswscale/swscale.h"  
}

struct AVFormatContext;  
struct AVCodecContext;  
struct AVFrame;  
struct AVPicture;  
struct SwsContext; 


NS_CC_BEGIN  

class CCVideoPic 
{
	public:
		CCVideoPic();
		bool init(const char *path, int frame,unsigned int width,  unsigned int height, unsigned char* data);
		virtual ~CCVideoPic();
		const char *m_path;
		int m_frame;
		int m_width;
		int m_height;
		unsigned char* m_pPicture;
};

 
class  CCVideoDecode : public CCObject
{
	private:
		AVFormatContext *m_pFormatCtx;  
	    AVCodecContext *m_pCodecCtx;  
	    AVFrame *m_pFrame;
		AVPicture *m_picture;

		bool m_close;
		bool m_loop;
		bool m_dispose;

		void repeat();
        std::mutex avcodec_mutex; /*初始化avcodec锁*/
	    int m_videoStream;
	    SwsContext *m_pSwsCtx;
        int m_playedCount;//播放到第几帧
	    int m_frameCount;   //解码到第几帧
	    const char *m_filepath; //视频文件路径
	    double m_frameRate; //帧率
	    unsigned int m_frames;   // 总帧数
	    unsigned int m_width;  
    	unsigned int m_height;
        bool copyFile2WriteablePath(const char *path);
       
	public:

		int getVideoStream();
		AVPicture *getPicture();

		CCVideoDecode();
		virtual ~CCVideoDecode();
		bool decode();
		bool init(const char *path);
		unsigned int getWidth();
		unsigned int getHeight();
		double getFrameRate();
		unsigned int getFrames();
		const char* getFilePath();
    
		void setPlayedFrame(int frame);

		bool isPassPlay();

		
		void close();

		void setLoop(bool isLoop);

		
		bool isDispose();
		bool isClose();

		void dispose();
    
};

NS_CC_END
