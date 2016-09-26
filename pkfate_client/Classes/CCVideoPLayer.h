#pragma once

#include "cocos2d.h"  
  
NS_CC_BEGIN  
  
class CCVideoPlayer : public Sprite
{
protected:
     CustomCommand _customCommand;
public:
    static CCVideoPlayer* create(const char* path,bool loop = false); 
    void onDraw(const Mat4 &transform, uint32_t flags);

    CCVideoPlayer();  
    virtual ~CCVideoPlayer();  
      
	bool init(const char* path, bool loop = false);
    void playVideo();  
    void stopVideo(void); 
    void seek(int frame);  
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    void update(float dt);  
	std::string getVideoPath();
      
    void registerPlayScriptHandler(int nHandler);
    void setVideoEndCallback(std::function<void()> func);       // 关闭视频回调
    unsigned int m_width;
    unsigned int m_height;
    void closeVideo();
private:
	int getFrame();
	bool isForceUpdate;
	void updateImage();
    void  repeat();
    void unregisterPlayScriptHandler(void);
    bool loop;
    unsigned int m_frames;   // 总帧数 
    double m_frameRate;             // 帧率      
	double m_elapsed;               // 用于帧率控制

    int m_playEndScriptHandler;
    unsigned int m_frame_count;               // 已经播放帧数
    std::string m_strFileName;
	int m_videoStream;
	int m_datalen;

    bool m_enableTouchEnd;  
    std::function<void()> m_videoEndCallback;  
};  
  
  
NS_CC_END

