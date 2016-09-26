/**
 * 视频播放层
 * 依赖ffmpeg库
 * 
 * 
 */
  
#include "CCVideoPlayer.h"  
#include "CCVideoTextureCache.h" 
#include "CCVideoDecode.h"

extern "C" { 
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
}

static  const GLchar *transparentshader =
"                                                       \n\
#ifdef GL_ES                                            \n\
precision lowp float;                                   \n\
#endif                                                  \n\
varying vec4 v_fragmentColor;                           \n\
varying vec2 v_texCoord;                                \n\
uniform sampler2D u_texture;                            \n\
void main()                                             \n\
{                                                       \n\
    float ratio=0.0;                                    \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);   \n\
    ratio = texColor[0] > texColor[1]?(texColor[0] > texColor[2] ? texColor[0] : texColor[2]) :(texColor[1] > texColor[2]? texColor[1] : texColor[2]);                                      \n\
    if (ratio != 0.0)                                          \n\
    {   if(ratio < 0.02){                                                      \n\
texColor[0] = texColor[0] *  ratio;                    \n\
texColor[1] = texColor[1] *  ratio;                    \n\
texColor[2] = texColor[2] *  ratio;                    \n\
texColor[3] = ratio;                                   \n\                               \n\
        }                                   \n\
    }                                                          \n\
    else                                                       \n\
    {                                                          \n\
        texColor[3] = 0.0;                                     \n\
    }                                                          \n\
    gl_FragColor = v_fragmentColor*texColor;                   \n\
}";

NS_CC_BEGIN  
CCVideoPlayer* CCVideoPlayer::create(const char* path,bool loop)  
{  
    CCVideoPlayer* video = new CCVideoPlayer();  
	if (video && video->init(path, loop)) {
         video->autorelease();
         return video;
    }
    CC_SAFE_DELETE(video);
    return NULL;  
}  
  
CCVideoPlayer::CCVideoPlayer()  
{  
	m_videoEndCallback = nullptr;
    m_frameRate = 1.0 / 24;
    m_frame_count = 0;  
    m_enableTouchEnd = false;  
    m_width = 100;
    m_height = 100;
    m_playEndScriptHandler = 0;
	m_elapsed = 0;
	isForceUpdate = false;
	m_datalen = 0;
    this->loop = false;
}  
  
CCVideoPlayer::~CCVideoPlayer()  
{
	closeVideo();
	m_videoEndCallback = nullptr;
	unregisterPlayScriptHandler();
}


  
bool CCVideoPlayer::init(const char* path,bool loop)  
{  
    m_strFileName = path;
	this->loop = loop;
    
    CCVideoDecode *pVideoDecode = CCVideoTextureCache::getInstance()->addVideo(path,loop);
    if(!pVideoDecode)
    {
        CCLOGERROR("videoDecode get error in %s", "CCVideoPlayer");
        return false;
    }

    m_width = pVideoDecode->getWidth();
    m_height = pVideoDecode->getHeight();
    m_frames = pVideoDecode->getFrames();   // 总帧数 
    m_frameRate = pVideoDecode->getFrameRate();             // 帧率

    
	AVPicture *picture = pVideoDecode->getPicture();

	m_videoStream = pVideoDecode->getVideoStream();
	m_datalen = picture->linesize[m_videoStream] * m_height;
	// 渲染的纹理
	Texture2D *texture = new Texture2D();
	texture->initWithData(picture->data[m_videoStream], picture->linesize[m_videoStream] * m_height, kCCTexture2DPixelFormat_RGB565, m_width, m_height, Size(m_width, m_height));
	initWithTexture(texture);

	// CCLayerColor* back = CCLayerColor::create(ccc4(0, 0, 0, 160));
	// this->addChild(back);//给clip加一个颜色层，clip不受影响，其他区域有层级遮挡
	// back->setContentSize(Size(m_width, m_height));


	this->setContentSize(Size(m_width, m_height));


	auto glprogram = GLProgram::createWithByteArrays(ccPositionTextureColor_vert, transparentshader);
	auto glprogramstate = GLProgramState::getOrCreateWithGLProgram(glprogram);
	setGLProgramState(glprogramstate);
	CHECK_GL_ERROR_DEBUG();
    

    return true;
}


void CCVideoPlayer::repeat()
{
    
}

std::string  CCVideoPlayer::getVideoPath()
{
	return m_strFileName;
}

void CCVideoPlayer::playVideo()
{
	this->scheduleUpdate();
}  
  
void CCVideoPlayer::stopVideo(void)  
{  
	
	this->unscheduleUpdate();
}  
  
void CCVideoPlayer::closeVideo()
{
    stopVideo();
    CCVideoTextureCache::getInstance()->removeVideo();
}
  
void CCVideoPlayer::seek(int frame)  
{  
	/*if (frame - m_frame_count > 1){
		CCLOG("jump frame %d m_elapsed:%f", frame - m_frame_count, m_elapsed);
	}*/
    m_frame_count = frame;
	
}  

int CCVideoPlayer::getFrame()
{
	int frame = m_elapsed / m_frameRate;
	if (!this->loop &&frame > m_frames){
		frame = m_frames;
	}
	return frame;
}
  
void CCVideoPlayer::update(float dt)  
{ 
	
	m_elapsed += dt;
	if (m_elapsed < m_frameRate) {
	       return;
	}
	else{
		int frame = getFrame();
		if (m_frame_count%m_frames == frame%m_frames){
			m_frame_count = frame;
			return; //不做操作
		}
		else{
			seek(frame);
		}
	}

	
	isForceUpdate = true;
    
    
    if(m_frame_count >= m_frames)
    {
        if(this->loop){
           // this->repeat();
        }else{
            closeVideo();
            
            if (m_videoEndCallback) {
                m_videoEndCallback();
				m_videoEndCallback = nullptr;
            }
			this->removeFromParent();//非单列直接删除
        }
        
    }

}



void CCVideoPlayer::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder+100);
    _customCommand.func = CC_CALLBACK_0(CCVideoPlayer::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}


void CCVideoPlayer::updateImage()
{
	if (isForceUpdate){
		CCVideoPic *pVideoPic = CCVideoTextureCache::getInstance()->getVideoPic(m_frame_count);
		if (pVideoPic != nullptr){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pVideoPic->m_pPicture);
			isForceUpdate = false;
			//CCVideoTextureCache::getInstance()->removeVideoPic(pVideoPic->m_frame);
		}
	}
}

void CCVideoPlayer::onDraw(const Mat4 &transform, uint32_t flags)
{
	auto glProgramState = getGLProgramState();
	glProgramState->apply(transform);
	GL::blendFunc(_blendFunc.src, _blendFunc.dst);


	if (_texture != NULL)
	{
		GL::bindTexture2D(_texture->getName());
		updateImage();
	}
	else
	{
		GL::bindTexture2D(0);
	}


	//GL::bindTexture2D( _texture->getName() );
	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

#define kQuadSize sizeof(_quad.bl)
	size_t offset = (size_t)&_quad;

	// vertex
	int diff = offsetof(V3F_C4B_T2F, vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

	// texCoods
	diff = offsetof(V3F_C4B_T2F, texCoords);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

	// color
	diff = offsetof(V3F_C4B_T2F, colors);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CHECK_GL_ERROR_DEBUG();
	CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}


void CCVideoPlayer::registerPlayScriptHandler(int nHandler)
{
    unregisterPlayScriptHandler();
    m_playEndScriptHandler = nHandler;
    LUALOG("[LUA] Add CCVideoPlayer event handler: %d", m_playEndScriptHandler);
}

void CCVideoPlayer::unregisterPlayScriptHandler(void)
{
    if (m_playEndScriptHandler)
    {
        //CCScriptEngineManager::sharedManager()->getScriptEngine()->removeScriptHandler(m_playEndScriptHandler);
        LUALOG("[LUA] Remove CCVideoPlayer event handler: %d", m_playEndScriptHandler);
        m_playEndScriptHandler = 0;
    }
}


  
void CCVideoPlayer::setVideoEndCallback(std::function<void(void)> func)  
{  
    m_videoEndCallback = func;  
}  
  
  
NS_CC_END  