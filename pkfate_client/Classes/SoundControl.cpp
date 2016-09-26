#include "comm.h"
#include "SoundControl.h"
#include "AudioEngine.h"

using namespace std;
using namespace experimental;

#define BGM_LOGIN_URL 
#define BGM_HALL_URL 
#define BGM_POKER_URL 
#define BGM_BACCARAT_URL 

#define EFFECT_BUTTON_URL "sound/button.mp3"
#define EFFECT_CHECK_URL "sound/check.mp3"
#define EFFECT_FOLD_URL "sound/fold.mp3"
#define EFFECT_SELL_URL "sound/mall.mp3"
#define EFFECT_CONFIRM_URL "sound/ok.mp3"
#define EFFECT_CLOSE_URL "sound/close.mp3"
#define EFFECT_RETURN_URL "sound/return.mp3"
#define EFFECT_CALL_URL "sound/call.mp3"
#define EFFECT_THINK_URL "sound/think.mp3"
#define EFFECT_BET_URL "sound/bet.mp3"
#define EFFECT_COLLECT_URL "sound/collect.mp3"
#define EFFECT_WIN_URL "sound/youwin.mp3"
#define EFFECT_BACCARATE_SELECT_URL "sound/select.mp3"
#define EFFECT_BACCARATE_BET_URL "sound/bet.mp3"
#define EFFECT_REWARD_URL "sound/reward.mp3"
#define EFFECT_ALLIN_URL "sound/all_in.mp3"
#define EFFECT_RAISE_URL "sound/raise.mp3"

const string SoundControl::BGM_URLS[4] = { "sound/bgm_login.mp3" ,"sound/bgm_login.mp3","sound/bgm_Poker.mp3","sound/bgm_Baccarat.mp3" };


int SoundControl::idBackgroundMusic = experimental::AudioEngine::INVALID_AUDIO_ID;
string SoundControl::BACKGROUNDMUSIC_PATH = "";
void SoundControl::PlayMusic(BGM bgm)
{
	if (GetMusicMute())
		return;
	
	string url = BGM_URLS[bgm];
	
	if (url.empty()||url == BACKGROUNDMUSIC_PATH) return;

	AudioEngine::stop(idBackgroundMusic);		//停止旧的背景音乐
	BACKGROUNDMUSIC_PATH = url;
	idBackgroundMusic = AudioEngine::play2d(BACKGROUNDMUSIC_PATH, true, GetMusicVolume());
}

void SoundControl::PlayPassMusic()
{
	if (GetMusicMute())
		return;

	if (BACKGROUNDMUSIC_PATH.empty()){
		PlayMusic(BGM::BGM_LOGIN);
	}
	else{
		AudioEngine::stop(idBackgroundMusic);		//停止旧的背景音乐
		idBackgroundMusic = AudioEngine::play2d(BACKGROUNDMUSIC_PATH, true, GetMusicVolume());
	}
}


void SoundControl::StopMusic()
{
	AudioEngine::stop(idBackgroundMusic);		//停止背景音乐
	idBackgroundMusic = experimental::AudioEngine::INVALID_AUDIO_ID;
}
int SoundControl::PlayEffect(std::string filePath)
{
	if (GetSoundMute() || filePath.empty())
		return AudioEngine::INVALID_AUDIO_ID;
	return AudioEngine::play2d(filePath, false, GetSoundVolume());
}
bool SoundControl::GetMusicMute()
{
	return UserDefault::getInstance()->getBoolForKey("MusicMute", false);
}
float SoundControl::GetMusicVolume()
{
	return UserDefault::getInstance()->getFloatForKey("MusicVolume", 0.5f);
}
bool SoundControl::GetSoundMute()
{
	return UserDefault::getInstance()->getBoolForKey("SoundMute", false);
}
float SoundControl::GetSoundVolume()
{
	return UserDefault::getInstance()->getFloatForKey("SoundVolume", 1.0f);
}
void SoundControl::SetMusicMute(bool mute)
{
	UserDefault::getInstance()->setBoolForKey("MusicMute", mute);
	if (mute)
	{
		if(idBackgroundMusic!=experimental::AudioEngine::INVALID_AUDIO_ID)
			AudioEngine::pause(idBackgroundMusic);
	}
	else
	{
		if(idBackgroundMusic!=experimental::AudioEngine::INVALID_AUDIO_ID)
			AudioEngine::resume(idBackgroundMusic);
		else
			idBackgroundMusic = AudioEngine::play2d(BACKGROUNDMUSIC_PATH, true, GetMusicVolume());
	}
}
void SoundControl::SetMusicVolume(float vol)
{
	UserDefault::getInstance()->setFloatForKey("MusicVolume", vol);
	AudioEngine::setVolume(idBackgroundMusic, vol);
}
void SoundControl::SetSoundMute(bool mute)
{
	UserDefault::getInstance()->setBoolForKey("SoundMute", mute);
}
void SoundControl::SetSoundVolume(float vol)
{
	UserDefault::getInstance()->setFloatForKey("SoundVolume", vol);
}
void SoundControl::PauseMusic()
{
	if (!GetMusicMute())
		AudioEngine::pause(idBackgroundMusic);
}
void SoundControl::ResumeMusic()
{
	if (!GetMusicMute())
		AudioEngine::resume(idBackgroundMusic);

}

/**
 * 播放点数，因为点数音频是由两个MP3文件组合起来顺序播放，用到了回调，所以才将两个放在同一个地方
 * @param playerPoint int 闲家点数
 * @param bankerPoint int 庄家点数
 */
void SoundControl::PlayPoint(int playerPoint,int bankerPoint)
{
	vector<string> effects;
	effects.push_back(StringUtils::format("sound/result/%s.mp3", "player"));		//闲家
	effects.push_back(StringUtils::format("sound/result/%d.mp3", playerPoint));		//闲家点数
	effects.push_back(StringUtils::format("sound/result/%s.mp3", "banker"));		//庄家
	effects.push_back(StringUtils::format("sound/result/%d.mp3", bankerPoint));		//庄家点数
	effects.push_back(StringUtils::format("sound/result/%s.mp3",
		(playerPoint > bankerPoint ? "player_win" : (playerPoint < bankerPoint ? "banker_win" : "tie"))));			//赢家
	PlayEffects(effects);
}

int SoundControl::PlayGameEffect(EFFECTSOUND effect)
{
	string effectUrl = "";
	switch (effect)
	{
	case EFFECT_BUTTON:
		effectUrl = EFFECT_BUTTON_URL;
		break;
	case EFFECT_SELL:
		effectUrl = EFFECT_SELL_URL;
		break;
	case EFFECT_CONFIRM:
		effectUrl = EFFECT_CONFIRM_URL;
		break;
	case EFFECT_CLOSE:
		effectUrl = EFFECT_CLOSE_URL;
		break;
	case EFFECT_RETURN:
		effectUrl = EFFECT_RETURN_URL;
		break;
	case EFFECT_CHECK:
		effectUrl = EFFECT_CHECK_URL;
		break;
	case EFFECT_CALL:
		effectUrl = EFFECT_CALL_URL;
		break;
	case EFFECT_FOLD:
		effectUrl = EFFECT_FOLD_URL;
		break;
	case EFFECT_THINK:
		effectUrl = EFFECT_THINK_URL;
		break;
	case EFFECT_REWARD:
		effectUrl = EFFECT_REWARD_URL;
		break;
	case EFFECT_BET:
		effectUrl = EFFECT_BET_URL;
		break;
	case EFFECT_COLLECT:
		effectUrl = EFFECT_COLLECT_URL;
		break;
	case EFFECT_WIN:
		effectUrl = EFFECT_WIN_URL;
		break;
	case EFFECT_BACCARATE_SELECT:
		effectUrl = EFFECT_BACCARATE_SELECT_URL;
		break;
	case EFFECT_ALLIN:
		effectUrl = EFFECT_ALLIN_URL;
		break;
	case EFFECT_RAISE:
		effectUrl = EFFECT_RAISE_URL;
		break;
	default:
		break;
	}
	return SoundControl::PlayEffect(effectUrl);
}
/**
 * 播放发牌音效
 */
int SoundControl::PlayPokerEffect()
{
    #define SOUND_POKER_EFFECT_MP3_PATH "sound/send_poker.mp3"
    return SoundControl::PlayEffect(SOUND_POKER_EFFECT_MP3_PATH);
}
// 批量播放音效,按数组顺序轮播
// filePaths - 音效路径数组
void SoundControl::PlayEffects(vector<string> filePaths){
	auto pathIt = filePaths.begin();
	if (pathIt == filePaths.end())
		return;
	int audioId = PlayEffect(*pathIt);
	filePaths.erase(pathIt);
	if (audioId != AudioEngine::INVALID_AUDIO_ID){
		AudioEngine::setFinishCallback(audioId, [=](int id, const std::string& filePath){
			PlayEffects(filePaths);
		});
	}
}