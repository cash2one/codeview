/**
 * 视频解码类
 */
#include "CCVideoDecode.h"
#include "CCVideoTextureCache.h"


NS_CC_BEGIN

CCVideoPic::CCVideoPic()
{
    m_pPicture = NULL;
}

CCVideoPic::~CCVideoPic()
{  
	if (m_pPicture)
	{
		delete m_pPicture;
	}
}

bool CCVideoPic::init(const char *path, int frame,unsigned int width,  unsigned int height, unsigned char* data)
{
  
    //unsigned char* data = pic.data[m_videoStream];
    m_width = width;
    m_height = height;
    m_frame = frame;
    m_path = path;
    unsigned int length = m_width * m_height * 3;
    
    m_pPicture = new unsigned char[length];
    for(unsigned int i = 0; i < length; ++i)
    {
        m_pPicture[i] = data[i];
    }
    return true;
}

CCVideoDecode::CCVideoDecode()
{
	m_filepath = NULL;
	m_pFormatCtx = NULL;
	m_videoStream = -1;
    m_pSwsCtx = NULL;
	m_picture = NULL;
    m_pCodecCtx = NULL;
    m_frameCount = 0;
    m_playedCount = 0;
	m_pFrame = NULL;
	m_close = true;
	m_dispose = true;
	m_loop = false;
}


bool CCVideoDecode::copyFile2WriteablePath(const char *path)
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
bool CCVideoDecode::init(const char *path)
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
	std::string sPath = FileUtils::getInstance()->fullPathForFilename(path);
#endif
	CCLOG("path:%s,\nfull path:%s", path, sPath.c_str());
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
		CCLOG("file size is %ld", size);
	}

	//std::unique_lock<std::mutex> lck(avcodec_mutex);

	if ((err_code = avformat_open_input(&m_pFormatCtx, sPath.c_str(), NULL, NULL)) != 0) {
		CCLOG("avformat_open_input false");
		av_strerror(err_code, buf, 1024);
		CCLOG("Couldn't open file %s: %d(%s)", sPath.c_str(), err_code, buf);
		return false;
	}

	/* 2、获取流信息 */
	if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
		CCLOG("avformat_find_stream_info false");
		return false;
	}

	m_videoStream = -1;

	for (int i = 0; i<m_pFormatCtx->nb_streams; i++) {

		if (m_videoStream == -1 && m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_videoStream = i;
			break;
		}

	}

	//  没有视频流，无法播放  
	if (m_videoStream == -1) {
		CCLOGERROR("没有视频流，无法播放");
		return false;
	}

	m_pCodecCtx = m_pFormatCtx->streams[m_videoStream]->codec;
	// 获取基本信息
	if (m_pCodecCtx->width)
	{
		 m_width = m_pCodecCtx->width;
		 m_height = m_pCodecCtx->height;
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
		duration = m_pFormatCtx->duration + 5000;
		/*secs = duration / AV_TIME_BASE;
		us = duration % AV_TIME_BASE;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
		CCLOG("%02d:%02d:%02d.%02d", hours, mins, secs, (100 * us) / AV_TIME_BASE);*/
	}
	else {
		CCLOGERROR("duration is null");
		return false;
	}

	AVRational rational;

	if (m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->avg_frame_rate.num)
	{
		rational = m_pFormatCtx->streams[m_videoStream]->avg_frame_rate;
	}
	else if (m_pFormatCtx->streams[m_videoStream]->r_frame_rate.den && m_pFormatCtx->streams[m_videoStream]->r_frame_rate.num)
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

	if (pCodec == NULL) {
		CCLOGERROR("avcodec_find_decoder error");
		return false;
	}

	if (avcodec_open2(m_pCodecCtx, pCodec, NULL)) {
		CCLOGERROR("avcodec_open2 error");
		return false;
	}


	// Allocate video frame
	m_pFrame = avcodec_alloc_frame();

	// scale
	sws_freeContext(m_pSwsCtx);

	// 用于渲染的一帧图片数据。注意其中的data是一个指针数组，我们取视频流用于渲染(一般是第0个流)
	m_picture = new AVPicture;
	avpicture_alloc(m_picture, PIX_FMT_RGB24, m_width, m_height);

	// 用于缩放视频到实际需求大小
	static int sws_flags = SWS_FAST_BILINEAR;
	m_pSwsCtx = sws_getContext(m_pCodecCtx->width,
		m_pCodecCtx->height,
		m_pCodecCtx->pix_fmt,
		m_width,
		m_height,
		PIX_FMT_RGB24,
		sws_flags, NULL, NULL, NULL);

	if (!m_pSwsCtx)
	{
		CCLOGERROR("sws_getContext error");
		return false;
	}
    
	m_close = false;
	m_dispose = false;

    return true;
}

AVPicture * CCVideoDecode::getPicture()
{
	return m_picture;
}

int CCVideoDecode::getVideoStream()
{
	return m_videoStream;
}

unsigned int CCVideoDecode::getWidth()
{
    return m_width;
}

unsigned int CCVideoDecode::getHeight()
{
    return m_height;
}

double CCVideoDecode::getFrameRate(){
    return m_frameRate;
}

unsigned int CCVideoDecode::getFrames()
{
    return m_frames;
}

const char* CCVideoDecode::getFilePath()
{
    return m_filepath;
}

CCVideoDecode::~CCVideoDecode()
{
    CCLOGINFO("cocos2d: deallocing CCVideoDecode.");
    //if(m_pSwsCtx) sws_freeContext(m_pSwsCtx); 
    

    // Free the YUV frame 
    //if(m_pFrame) av_free(m_pFrame);  
	//close();
	dispose();
}

void CCVideoDecode::close()
{
	m_close = true;
}

bool CCVideoDecode::isClose()
{
	return m_close;
}

bool CCVideoDecode::isDispose()
{
	return m_dispose;
}


void CCVideoDecode::repeat()
{
    
    av_seek_frame(m_pFormatCtx, m_videoStream , 0, AVSEEK_FLAG_ANY);
}

void CCVideoDecode::setLoop(bool isLoop)
{
	m_loop = isLoop;
}


void CCVideoDecode::dispose()
{
	//std::unique_lock<std::mutex> lck(avcodec_mutex);

	if (m_pCodecCtx) avcodec_close(m_pCodecCtx);
	if (m_pFormatCtx) avformat_close_input(&m_pFormatCtx);
	if (m_pFrame) av_free(m_pFrame);
	if (m_picture){
		avpicture_free(m_picture);
		delete m_picture;
	}

	m_filepath = NULL;
	m_pFormatCtx = NULL;
	m_videoStream = -1;
	m_pSwsCtx = NULL;
	m_pCodecCtx = NULL;
	m_frameCount = 0;
	m_playedCount = 0;
	m_picture = NULL;
	m_pFrame = NULL;
	m_loop = false;

	m_dispose = true;
}

void CCVideoDecode::setPlayedFrame(int frame)
{
	m_playedCount = frame;
}

bool CCVideoDecode::isPassPlay()
{
	return m_frameCount - m_playedCount >= CCVideoTextureCache::getInstance()->getMaxPass()|| (!m_loop &&m_frameCount >= m_frames);//预先4帧
}


/**
 * 解码
 * @return [description]
 */
bool CCVideoDecode::decode()
{


	AVPacket packet;
	int frameFinished = 0;

	while (!frameFinished && av_read_frame(m_pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == m_videoStream) {
			// Decode video frame
			avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished, &packet);
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}
	sws_scale(m_pSwsCtx, m_pFrame->data, m_pFrame->linesize,
		0, m_pCodecCtx->height,
		m_picture->data, m_picture->linesize);

	

	m_frameCount++;

	if (m_frameCount>=m_playedCount){ // 
		CCVideoPic *pVideoPic = new CCVideoPic();
		pVideoPic->init(m_filepath, m_frameCount, m_width, m_height, m_picture->data[0]);
		CCVideoTextureCache::getInstance()->addPicData(pVideoPic);
	}

	if (frameFinished == 0) {
		if (this->m_loop)
		{
			repeat();
		}
	}
 
    return true;
}

NS_CC_END
