#include "comm.h"
#include "cJSON.h"
#include "Settings.h"
#include "AudioEngine.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)  
#include <regex>
#else
#include <regex.h>  
#endif
using namespace std;
extern "C"
{
#include "sha256.h"
}
RECVMSG* RECVMSG::create(SOCKETDATA *socketData)
{
	RECVMSG *msg = nullptr;
	if (!socketData)
		return msg;
	//create msg
	msg = new RECVMSG();
	msg->type = socketData->type;
	msg->typesub = socketData->typesub;
	msg->code = socketData->code;
	msg->len = socketData->len;
	msg->data = socketData->content;
	return msg;
}
unsigned char HexChar2Char(const char* str)
{
	if (str == NULL)
		return 0;
	unsigned char val;
	if (str[0] >= '0'&&str[0] <= '9')
		val = str[0] - '0';
	else if (str[0] >= 'a'&&str[0] <= 'f')
		val = str[0] - 'a' + 10;
	else if (str[0] >= 'A'&&str[0] <= 'F')
		val = str[0] - 'A' + 10;
	else
		return 0;
	val = val << 4;
	if (str[1] >= '0'&&str[1] <= '9')
		val += str[1] - '0';
	else if (str[1] >= 'a'&&str[1] <= 'f')
		val += str[1] - 'a' + 10;
	else if (str[1] >= 'A'&&str[1] <= 'F')
		val += str[1] - 'A' + 10;
	else
		return 0;
	return val;
}

unsigned long long Str2Uuid(const char* str)
{
	if (str == NULL)
		return 0;
	int i;
	unsigned long long val;
	char *p = (char*)&val;
	for (i = 0; i<8; i++)
	{
		*(p + 7 - i) = HexChar2Char(str + i * 2); //内存中是倒序存储的
	}
	return val;
}

char* Char322Char64(unsigned char* char32)
{
	int i;
	static char char64[65];
	memset(char64, 0, 65);
	for (i = 0; i<32; i++)
		sprintf(char64 + i * 2, "%02x", char32[i]);
	return char64;
}
int SHA256(unsigned char *d, unsigned char *s, unsigned long n)
{
	sha256_context ctx;
	sha256_starts(&ctx);
	sha256_update(&ctx, s, n);
	sha256_finish(&ctx, d);
	return 0;
}
std::string Comm::GetResDir()
{
	std::string path = UserDefault::getInstance()->getStringForKey("ResDir");
	if (path.empty())
	{
		path = FileUtils::getInstance()->getWritablePath() + "PkfateRes/";
		UserDefault::getInstance()->setStringForKey("ResDir", path);
	}
	return path;
}
std::string Comm::GetShortStringFromInt64(long long  num)
{

	if (num < 0)
	{
		return StringUtils::format("-%s", GetGameTypeStringFromInt64(-num).c_str());
	}
	else
	{
		return GetGameTypeStringFromInt64(num);
	}

}

std::string Comm::GetFloatShortStringFromInt64(long long  num)
{
	if (num == 1000)
	{
		int test = 0;
	}

	if (num < 0)
	{
		return StringUtils::format("-%s", GetGameTypeStringFromInt64(-num).c_str());
	}
	else
	{
		return GetGameTypeStringFromInt64(num);
	}

}

std::string Comm::GetGameTypeStringFromInt64(long long  num)
{
	if (num < 1000000)   //小于 1000K
		return GetFormatThousandsSeparatorFromInt64(num);
	if (num < 1000000000)// 小于 1000M
		return StringUtils::format("%sK", GetFormatThousandsSeparatorFromInt64(num / 1000).c_str());
	if (num<1000000000000)//小于 1000B
		return StringUtils::format("%sM", GetFormatThousandsSeparatorFromInt64(num / 1000000).c_str());
	return StringUtils::format("%sB", GetFormatThousandsSeparatorFromInt64(num / 1000000000).c_str());
}

std::string Comm::GetStringFromInt64(long long  num)
{
	if (num < 1000)
		return StringUtils::format("%d", num);
	if (num < 1000000)
		return StringUtils::format("%dK", num / 1000);
	if (num < 1000000000)
		return StringUtils::format("%dM", num / 1000000);
	return StringUtils::format("%dB", num / 1000000000);

}





std::string Comm::GetFormatThousandsSeparatorFromInt64(long long  num)
{
	
	static char buf[16][16];
	static int  c = 0;
	long m, n = 0;
	char* p = &buf[c++ % 16][15];
	*p = '\0';
	do
	{
		m = num % 10; num = num / 10; *--p = '0' + m;
		if (num && !(++n % 3)) *--p = ',';
	} while (num);
	return p;
}

//从UTC时间字符串，转换为本地时间字符串
std::string Comm::GetLocalTimeStrngFromUTCString(std::string strUTC)
{
	tm tp = { 0 };
	if (sscanf(strUTC.c_str(), "%d-%d-%d %d:%d:%d", &tp.tm_year, &tp.tm_mon, &tp.tm_mday, &tp.tm_hour, &tp.tm_min, &tp.tm_sec) != 6)
		return "";
	tp.tm_year -= 1900;
	tp.tm_mon -= 1;
	time_t t;									// UTC秒数
	t = mktime(&tp) + Comm::TIMEZONEBIAS;		// tp中为UTC，经过mktime后，会将时区信息减掉，因些需要再加上时区信息
	tm *ltp = localtime(&t);
	return StringUtils::format("%04d-%02d-%02d %02d:%02d:%02d", ltp->tm_year + 1900, ltp->tm_mon + 1, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
}
//从本地时间字符串，转换为UTC时间字符串
std::string Comm::GetUTCStringFromLocalTimeStrng(std::string strLocalTime)
{
	time_t t;  // UTC秒数
	tm tp = { 0 };
	if (sscanf(strLocalTime.c_str(), "%d-%d-%d %d:%d:%d", &tp.tm_year, &tp.tm_mon, &tp.tm_mday, &tp.tm_hour, &tp.tm_min, &tp.tm_sec) != 6)
		return "";
	tp.tm_year -= 1900;
	tp.tm_mon -= 1;
	t = mktime(&tp);
	tm *ltp = gmtime(&t);
	return StringUtils::format("%04d-%02d-%02d %02d:%02d:%02d", ltp->tm_year + 1900, ltp->tm_mon + 1, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
}
const time_t Comm::TIMEZONEBIAS = Comm::InitTimeZoneBias();
time_t Comm::InitTimeZoneBias()
{
	time_t t = 86400;
	return 86400 - mktime(gmtime(&t));
}
bool Comm::RegexIsMatch(const char* str, const char* reg)
{
	bool ret = false;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)  
	regex pattern(reg);
	ret = regex_match(str, pattern);
#else
	regmatch_t pmatch[4];
	regex_t match_regex;

	if(0==regcomp(&match_regex,reg,REG_EXTENDED))
	{
		if (regexec(&match_regex, str, 1, pmatch, 0) == 0)
			ret=true;
		regfree(&match_regex);
	}
#endif
	return ret;
}

std::string Comm::DeleteCharcter(string source, string del)
{
	int begin = 0;
	begin = source.find(del, begin);
	while (begin != -1)  
	{
		source.replace(begin, 1, "");
		begin = source.find(del, begin);
	}

	return source;
}


