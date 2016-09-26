/**
 * 视频播放层
 * 依赖ffmpeg库
 *
 * @author leoluo<luochong1987@gmail.com>
 *
 */

#include "CCVideoSinglePlayer.h"
#include "AudioEngine.h"
//using namespace experimental;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <thread>
#include <unistd.h>
#define MAXLEN_FRAMEQUEUE 10           //帧队列最大容量
typedef struct structPictureData {
    int pos;                //帧索引
    unsigned char *data;    //图像数据
} PictureData;
std::queue<PictureData> _queueFrames;
bool _isDecodeThreadExit = false;
bool _isDecodeFinished = false;
int _posFrame = 0;
std::mutex _mutexFrames;
static pthread_t _decodeThread;
static void *videoDecode(void *data)
{
    CCVideoSinglePlayer *p = (CCVideoSinglePlayer *) data;
    if(p)
    {
        unsigned char *data = nullptr;
        _isDecodeThreadExit = false;
        while(!_isDecodeThreadExit)
        {
            //创建纹理数组
            if(!_isDecodeFinished&&_queueFrames.size()<MAXLEN_FRAMEQUEUE)
            {
                //解析1帧
                data=p->getFrameData();
                if(data == nullptr)
                    _isDecodeFinished = true;
                else
                {
                    //解析成功
                    _mutexFrames.lock();
                    PictureData picData;
                    picData.data=data;
                    picData.pos=++_posFrame;
                    _queueFrames.push(picData);
                    _mutexFrames.unlock();
                    CCLOG("set frame:%d",_posFrame);
                }
            }
            usleep(10);
        }
    }
    return 0;
}
NS_CC_BEGIN
CCVideoSinglePlayer* CCVideoSinglePlayer::create(const char* path,int width ,int height)
{
    CCVideoSinglePlayer* video = new CCVideoSinglePlayer();
    if (video && video->init(path,width,height)) {
        video->autorelease();
        return video;
    }
    CC_SAFE_DELETE(video);
    return NULL;
}

CCVideoSinglePlayer::CCVideoSinglePlayer()
{
    m_frameRate = 1.0 / 24;
    m_frame_count = 0;
    
    m_width = 100;
    m_height = 100;
    m_pSwsCtx = nullptr;
    m_pFormatCtx = nullptr;
    m_pCodecCtx = nullptr;
    m_pFrame = nullptr;
    m_videoStream = -1;
    
}


CCVideoSinglePlayer::~CCVideoSinglePlayer()
{
    this->loop = false;
    if(m_pSwsCtx) sws_freeContext(m_pSwsCtx);
    avpicture_free(m_picture);
    delete m_picture;
    av_free(m_pFrame);
    if(m_pCodecCtx) avcodec_close(m_pCodecCtx);
    if(m_pFormatCtx) avformat_close_input(&m_pFormatCtx);
    m_videoEndCallback = nullptr;
    _isDecodeThreadExit = true;         //退出解码线程
}

bool CCVideoSinglePlayer::copyFile2WriteablePath(const char *path)
{
    // -------- 将文件写入可读写目录
    std::string fullPath=FileUtils::getInstance()->fullPathForFilename(path);
    ssize_t size = 0;
    unsigned char *data = FileUtils::getInstance()->getFileData(fullPath, "rb", &size);
    if (data == nullptr || size <= 0)
    {
        CCLOG("get file data error.");
        return false;
    }
    
    std::string pathreal=FileUtils::getInstance()->getWritablePath()+path;
    if (!FileUtils::getInstance()->createDirectory(pathreal.substr(0, pathreal.find_last_of('/'))))
    {
        CCLOG("%s:%s", "create resources dir error.", pathreal.c_str());
        return false;
    }
    FILE *fp;
    fp = fopen(pathreal.c_str(), "wb");
    if (fp == nullptr)
    {
        CCLOG("%s:%s", "create resources file error.", pathreal.c_str());
        return false;
    }
    size_t wsize = fwrite(data, 1, size, fp);
    free(data);
    fclose(fp);
    if (wsize != size)
    {
        CCLOG("%s: %ld != %ld", "write file error.", size,wsize);
        return false;
    }
    return true;
}


bool CCVideoSinglePlayer::decodeInit(const char* path)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    // android 下资源目录无法写入，所以无法使用ffmpeg库，必须将文件copy到可写目录
    std::string sPath=FileUtils::getInstance()->getWritablePath()+path;
    if (!FileUtils::getInstance()->isFileExist(sPath)) {
        // 文件不存在，为首次进入，直接复制一次
        CCLOG("首次进入，复制文件!");
        if(!copyFile2WriteablePath(path))
            return false;
    }
    else
        CCLOG("再次进入，不用复制文件!");
#else
    // 其他平台，直接使用资源目录
    std::string sPath=FileUtils::getInstance()->fullPathForFilename(m_strFileName);
#endif
    CCLOG("path:%s,\nfull path:%s",m_strFileName.c_str(),sPath.c_str());
    // Register all formats and codecs
    av_register_all();
    
    /* 1、构建avformat */
    int err_code;
    char buf[1024];
    
    ssize_t size = 0;
    unsigned char *data = FileUtils::getInstance()->getFileData(sPath, "rb", &size);
    if (data == nullptr || size <= 0)
    {
        CCLOG("get file data error.");
        return false;
    }
    else
    {
        CCLOG("file size is %ld",size);
    }
    if((err_code=avformat_open_input(&m_pFormatCtx, sPath.c_str(), NULL, NULL)) != 0) {
        CCLOG("avformat_open_input false");
        av_strerror(err_code, buf, 1024);
        CCLOG("Couldn't open file %s: %d(%s)", sPath.c_str(), err_code, buf);
        return false;
    }
    
    /* 2、获取流信息 */
    if(avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
        CCLOG("avformat_find_stream_info false");
        return false;
    }
    
    m_videoStream = -1;
    
    for(int i=0; i<m_pFormatCtx->nb_streams; i++) {
        
        if(m_videoStream == -1 && m_pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            m_videoStream = i;
            break;
        }
        
    }
    
    //  没有视频流，无法播放
    if(m_videoStream == -1) {
        CCLOGERROR("没有视频流，无法播放");
        return false;
    }
    
    m_pCodecCtx = m_pFormatCtx->streams[m_videoStream]->codec;
    // 获取基本信息
    if (m_pCodecCtx->width)
    {
        // m_width = m_pCodecCtx->width;
        // m_height = m_pCodecCtx->height;
    }
    else
    {
        CCLOGERROR("获取视频尺寸失败");
        return false;
    }
    
    /*
     Duration: 00:00:07.32, start: 0.000000, bitrate: 579 kb/s
     Stream #0:0: Video: vp6a, yuva420p, 224x240,
     31 fps, 31 tbr, 1k tbn, 1k tbc
     */
    
    //算时间
    int64_t duration = 0;
    if (m_pFormatCtx->duration != AV_NOPTS_VALUE) {
        /*int hours, mins, secs, us;*/
        duration = m_pFormatCtx->duration;
        //duration = m_pFormatCtx->duration + 5000;
        /*secs = duration / AV_TIME_BASE;
         us = duration % AV_TIME_BASE;
         mins = secs / 60;
         secs %= 60;
         hours = mins / 60;
         mins %= 60;
         CCLOG("%02d:%02d:%02d.%02d", hours, mins, secs, (100 * us) / AV_TIME_BASE);*/
    } else {
        CCLOGERROR("duration is null");
        return false;
    }
    
    AVRational rational;
    
    if(m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.num)
    {
        rational = m_pFormatCtx->streams[m_videoStream]->avg_frame_rate;
    }
    else if(m_pFormatCtx->streams[m_videoStream]->r_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->r_frame_rate.num)
    {
        rational = m_pFormatCtx->streams[m_videoStream]->r_frame_rate;
    }
    else
    {
        CCLOGERROR("fps 获取失败");
        return false;
    }
    
    double fps = av_q2d(rational);
    m_frameRate = 1.0 / fps;
    m_frames = (int)((fps * duration) / AV_TIME_BASE);
    CCLOG("m_frameRate = %f , frames = %d", m_frameRate, m_frames);
    
    
    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
    
    if(pCodec == NULL) {
        CCLOGERROR("avcodec_find_decoder error");
        return false;
    }
    
    if(avcodec_open2(m_pCodecCtx, pCodec, NULL)) {
        CCLOGERROR("avcodec_open2 error");
        return false;
    }
    
    return true;
}


void CCVideoSinglePlayer::setFrameRate(float frameRate)
{
    if (frameRate == 0) return;
    m_frameRate = frameRate;
}
void thread_task() {
    CCLOG("OK");
}
bool CCVideoSinglePlayer::init(const char* path,int width, int height)
{
    m_strFileName = path;
    
    if(!decodeInit(path))
    {
        CCLOG("decode init error");
        return false;
    }
    
    m_width = width <= 0?m_pCodecCtx->width:width;
    m_height = height <= 0?m_pCodecCtx->height:height;
    
    
    
    // Allocate video frame
    m_pFrame=avcodec_alloc_frame();
    
    // scale
    sws_freeContext(m_pSwsCtx);
    
    // 用于渲染的一帧图片数据。注意其中的data是一个指针数组，我们取视频流用于渲染(一般是第0个流)
    m_picture = new AVPicture;
    avpicture_alloc(m_picture, PIX_FMT_RGBA, m_width, m_height);
    
    // 用于缩放视频到实际需求大小
    static int sws_flags =  SWS_FAST_BILINEAR;
    m_pSwsCtx = sws_getContext(m_pCodecCtx->width,
                               m_pCodecCtx->height,
                               m_pCodecCtx->pix_fmt,
                               m_width,
                               m_height,
                               PIX_FMT_RGBA,
                               sws_flags, NULL, NULL, NULL);
    
    // 渲染的纹理
    Texture2D *texture = new Texture2D();
    texture->initWithData(m_picture->data[m_videoStream], m_picture->linesize[m_videoStream]*m_height, kCCTexture2DPixelFormat_RGBA8888, m_width, m_height, Size(m_width, m_height));
    initWithTexture(texture);
    
    // CCLayerColor* back = CCLayerColor::create(ccc4(0, 0, 0, 160));
    // this->addChild(back);//给clip加一个颜色层，clip不受影响，其他区域有层级遮挡
    // back->setContentSize(Size(m_width, m_height));
    
    
    this->setContentSize(Size(m_width, m_height));
    
    
    //开启线程进行解码
    pthread_create(&_decodeThread, NULL, videoDecode, this);
    return true;
}
unsigned char * CCVideoSinglePlayer::getFrameData()
{
    AVPacket packet;
    int frameFinished = 0;
    int sizeData=m_width*m_height*4;
    unsigned char *data=nullptr;
    while(!frameFinished)
    {
        if(av_read_frame(m_pFormatCtx, &packet)<0)
        {
            //出错时，认为已经解析完成所有帧
            //m_frames=_queueFrames.size();  //修正总帧数
            //CCLOG("frames:%d",m_frames);
            return nullptr;
        }
        if((packet.stream_index== m_videoStream)
           &&(avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished, &packet)>0)
           &&(sws_scale (m_pSwsCtx, m_pFrame->data, m_pFrame->linesize,0, m_pCodecCtx->height,m_picture->data, m_picture->linesize)>0))
        {
            data=new unsigned char[sizeData];
            memcpy(data, m_picture->data[0], sizeData);
            //去除绿幕
            unsigned char r,g,b;
            for(unsigned int i = 0; i < sizeData; i+=4)
            {
                r=data[i];
                g=data[i+1];
                b=data[i+2];
                //if(g>(r+20)&&g>(b+20))
                if(r<6&&g<6&&b<6)
                    data[i+3]=0;  //MAX(0, (100-MIN(g-r, g-b)));
            }
            frameFinished = 1;
        }
        av_free_packet(&packet);
    }
    return data;
}

void CCVideoSinglePlayer::repeat()
{
    m_elapsed = 0;
    _mutexFrames.lock();
    avformat_seek_file(m_pFormatCtx, m_videoStream, 0, 0, 0, AVSEEK_FLAG_FRAME);
    _posFrame=0;
    //清除旧数据
    while(!_queueFrames.empty())
    {
        delete (_queueFrames.front()).data;
        _queueFrames.pop();
    }
    _isDecodeFinished = false;
    _isDecodeThreadExit = false;
    _mutexFrames.unlock();
}

void CCVideoSinglePlayer::playVideo(bool loop,float frameRate)
{
    this->loop = loop;
    if(frameRate != 0)
        m_frameRate = frameRate;
    repeat();
    this->scheduleUpdate();
    
}

void CCVideoSinglePlayer::stopVideo(void)
{
    //this->unscheduleAllSelectors();
    // if (this->idSound != -1){
    //         AudioEngine::stop(this->idSound);
    // }
    this->unscheduleUpdate();
    this->unscheduleAllCallbacks();
    this->stopAllActions();
}

void CCVideoSinglePlayer::closeVideo()
{
    stopVideo();
}

void CCVideoSinglePlayer::seek(int frame)
{
    // m_frame_count = frame;
    // update(0);
    
    
    // AVRational timeBase = pFormatCtx->streams[videoStream]->time_base;
    // int64_t targetFrame = (int64_t)((double)timeBase.den / timeBase.num * sec);
    
    
    m_elapsed=frame*m_frameRate;
    m_frame_count = frame;
    //int64_t targetFrame = frame;
    //avformat_seek_file(m_pFormatCtx, m_videoStream, targetFrame, targetFrame, targetFrame, AVSEEK_FLAG_FRAME);
    //avcodec_flush_buffers(m_pCodecCtx);
    
}

void CCVideoSinglePlayer::update(float dt)
{
    m_elapsed += dt;
    int pos=m_elapsed/m_frameRate;
    if(pos>=m_frames)
        pos=m_frames-1;
    CCLOG("time:%f\tpos:%d",m_elapsed,pos);
    
    
    //av_seek_frame(m_pFormatCtx, -1, m_elapsed*AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    //avformat_seek_file(m_pFormatCtx, -1, pos, pos, pos, AVSEEK_FLAG_FRAME);
    //取1帧
    int posFrame;
    int sizeFrame;
    unsigned char * data;
    while(true)
    {
        data=nullptr;
        _mutexFrames.lock();
        if(!_queueFrames.empty())
        {
            posFrame = _queueFrames.front().pos;
            data = _queueFrames.front().data;
            _queueFrames.pop();
        }
        sizeFrame = _queueFrames.size();
        _mutexFrames.unlock();
        if(data == nullptr)
            break;     //没取到数据，直接返回
        CCLOG("get frame:%d",posFrame);
        if((pos > posFrame)&&(sizeFrame > 0))
        {
            delete data;
            continue;   //取到旧数据，而且还有数据时，继续取
        }
        else
        {
            //显示数据
            getTexture()->updateWithData(data, 0, 0, m_width, m_height);
            delete data;
            break;
        }
    }
    if (pos==(m_frames-1)) {
        if(!this->loop)
        {
            this->closeVideo();
            if (m_videoEndCallback) {
                m_videoEndCallback();
            }
        }
        else
        {
            repeat();
            update(0);
        }
    }
}
void CCVideoSinglePlayer::setVideoEndCallback(std::function<void(void)> func)
{  
    m_videoEndCallback = func;  
}  


NS_CC_END