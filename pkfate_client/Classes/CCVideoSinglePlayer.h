#ifndef __CCVIDEO_SINGLE_PLAYER_H__
#define __CCVIDEO_SINGLE_PLAYER_H__


#include "cocos2d.h" 

USING_NS_CC; 
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPicture;
struct SwsContext;


NS_CC_BEGIN  
  
class CCVideoSinglePlayer : public Sprite
{
protected:
     CustomCommand _customCommand;
public:
    static CCVideoSinglePlayer* create(const char* path,int width =0,int height =0);
   void onDraw(const Mat4 &transform, uint32_t flags);

    CCVideoSinglePlayer();  
    virtual ~CCVideoSinglePlayer();  
      
    bool init(const char* path,int width = 0, int height = 0);
    void playVideo(bool loop = false,float frameRate = 0);  
    void stopVideo(void); 
    void seek(int frame);  
    //virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    void update(float dt);
      
    void setVideoEndCallback(std::function<void()> func);       // 关闭视频回调
    unsigned int m_width;
    unsigned int m_height;
    void closeVideo();

    bool decodeInit(const char * path);

	void setFrameRate(float frameRate);
    unsigned char *getFrameData();      //获取下一帧图像数据
private:
    void  repeat();
    int idSound;
    bool loop;
    unsigned int m_frames;   // 总帧数 
    double m_frameRate;             // 帧率      

    unsigned int m_frame_count;               // 已经播放帧数
    std::string m_strFileName;
    double m_elapsed;               // 用于帧率控制
   
    std::function<void()> m_videoEndCallback;  

    bool copyFile2WriteablePath(const char *path);
    
    AVFormatContext *m_pFormatCtx;
    AVCodecContext *m_pCodecCtx;
    AVFrame *m_pFrame;
    AVPicture* m_picture;    
    int m_videoStream;                // 视频流
    int m_audioStream;                // 音频流
    SwsContext *m_pSwsCtx;
    bool GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx,int videoStream, AVFrame *pFrame);
};  
  
  
NS_CC_END

#endif //__CCVIDEO_LAYER_H__