
#ifndef __BCL_ICRYPT_SERVICE_H__
#define __BCL_ICRYPT_SERVICE_H__

#include "BclHeader.h"
//加密相关服务接口
#include <string>
#include "stdio.h"

struct TMD5
{
    UINT8 data[16];
    TMD5()
    {
        memset(data,0,sizeof(data));
    }
    bool    operator == (const TMD5 & md5)
    {
        return 0==memcmp(data, &md5, sizeof(md5));
    }
    bool    operator !=(const TMD5 & md5)
    {
        return memcmp(data, &md5, sizeof(md5));
    }
    bool    IsValid()
    {
        return  *this!=TMD5();
    }
    void    FromData(const UINT8  data[16])
    {
        memcpy(this->data, data, sizeof(this->data));
    }
    void    ToData(UINT8 data[16])
    {
        memcpy(data, this->data, 16);
    }

	void FromHexStr(const INT8 strHex[32])
	{

		for (int i=0;i<32 && strHex[i]!=0;i++)
		{
			char ch ;
			if (strHex[i]>= 'a')
			{
				ch = strHex[i]-'a'+10;
			}
			else if (strHex[i]>='A')
			{
				ch = strHex[i]-'A'+10;
			}
			else
			{
				ch = strHex[i]-'0';
			}

			if (i%2==0)
			{
				this->data[i/2] = ch<<4;
			}
			else
			{
				this->data[i/2] |= ch;
			}
		}
	}

	//bUpper=true转化为大写字母
	std::string toString(bool bUpper=true)
	{		
		char szBuff[33]={0};
		for (int i=0;i<sizeof(data);i++)
		{
			if(bUpper)
			{
				sprintf_s(szBuff+i*2,sizeof(szBuff)-i*2,"%02X",data[i]);
			}
			else
		    {
			    sprintf_s(szBuff+i*2,sizeof(szBuff)-i*2,"%02x",data[i]);
		    }
		}
		
		
		return szBuff;
	}
};
struct TSHA1 
{
    UINT8 data[20];
    TSHA1()
    {
        memset(data,0,sizeof(data));
    }
    bool    operator == (const TSHA1 & sha)
    {
        return 0==memcmp(this, &sha, sizeof(sha));
    }
    bool    operator !=(const TSHA1 & sha)
    {
        return memcmp(this, &sha, sizeof(sha));
    }
    bool    IsValid()
    {
        return *this!=TSHA1();
    }
    void    FromData(const UINT8 data[20])
    {
        memcpy(this->data, data, sizeof(this->data));
    }
    void    ToData(UINT8 data[20])
    {
        memcpy(data, this->data, 20);
    }
};


struct TSHA256
{
    UINT8 data[32];
    TSHA256()
    {
        memset(data,0,sizeof(data));
    }
    bool    operator == (const TSHA256 & sha256)
    {
        return 0==memcmp(data, &sha256, sizeof(sha256));
    }
    bool    operator !=(const TSHA256 & sha256)
    {
        return memcmp(data, &sha256, sizeof(sha256));
    }
    bool    IsValid()
    {
        return  *this!=TSHA256();
    }
    void    FromData(const UINT8  data[32])
    {
        memcpy(this->data, data, sizeof(this->data));
    }
    void    ToData(UINT8 data[32])
    {
        memcpy(data, this->data, sizeof(this->data));
    }

	void FromHexStr(const INT8 strHex[64])
	{

		for (int i=0;i<64 && strHex[i]!=0;i++)
		{
			char ch ;
			if (strHex[i]>= 'a')
			{
				ch = strHex[i]-'a'+10;
			}
			else if (strHex[i]>='A')
			{
				ch = strHex[i]-'A'+10;
			}
			else
			{
				ch = strHex[i]-'0';
			}

			if (i%2==0)
			{
				this->data[i/2] = ch<<4;
			}
			else
			{
				this->data[i/2] |= ch;
			}
		}
	}

	std::string toString()
	{		
		char szBuff[65]={0};
		for (int i=0;i<sizeof(data);i++)
		{
			sprintf_s(szBuff+i*2,sizeof(szBuff)-i*2,"%02X",data[i]);
		}
		
		return szBuff;
	}
};


struct  ICryptService
{
    virtual TMD5    CalculateMD5(const UINT8 * data, int len)=0;
	virtual TMD5    CalculateMD5FromFile(const char * szFileName)=0;
    virtual TSHA1   CalculateSHA1(const UINT8 * data, int len)=0;

	virtual TSHA256  CalculateSHA256(const UINT8 * data, int len)=0;

	virtual	void	Encrpyt(UINT8 * data, INT32 len, const std::string & key)=0;
	virtual	void	Decrpyt(UINT8 * data, INT32 len, const std::string & key)=0;
};





#endif
