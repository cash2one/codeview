#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;

enum BGM{
	BGM_LOGIN,
	BGM_HALL,
	BGM_POKER,
	BGM_BACCARAT,
};

enum EFFECTSOUND{

	EFFECT_BUTTON,
	EFFECT_SELL,
	EFFECT_RAISE,
	EFFECT_CONFIRM,
	EFFECT_CLOSE,
	EFFECT_RETURN,
	EFFECT_CHECK,
	EFFECT_CALL,
	EFFECT_FOLD,
	EFFECT_THINK,
	EFFECT_REWARD,
	EFFECT_BET,
	EFFECT_COLLECT,
	EFFECT_WIN,
	EFFECT_BACCARATE_SELECT,
	EFFECT_ALLIN,
};

class SoundControl
{
private:
	
public:
	static const string BGM_URLS[4];
	static string BACKGROUNDMUSIC_PATH;
	static int idBackgroundMusic;
	static void PlayMusic(BGM bgm);
	static void PlayPassMusic();
	static void StopMusic();
	static void PauseMusic();
	static void ResumeMusic();
	static int PlayEffect(std::string filePath);
	static int PlayGameEffect(EFFECTSOUND effect);
	// music,sound func
	static void SetMusicMute(bool mute);
	static void SetMusicVolume(float vol);
	static void SetSoundMute(bool mute);
	static void SetSoundVolume(float vol);
	static bool GetMusicMute();
	static float GetMusicVolume();
	static bool GetSoundMute();
	static float GetSoundVolume();
    static void PlayPoint(int playerPoint,int bankerPoint);
    static int PlayPokerEffect();
	static void PlayEffects(vector<string> filePaths);
};