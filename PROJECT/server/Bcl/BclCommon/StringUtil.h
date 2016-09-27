
#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__

#include <string>
#include <vector>
#include <sstream>
#include "stdio.h"
#include "stdlib.h"
#include <locale.h>

#include "FieldDef.h"

struct StringUtil
{
	struct   eqstr 
    { 
           bool   operator()(const   char*   s1,   const   char*   s2)   const 
           { 
                 return   strcmp(s1,   s2)   ==   0; 
           } 
     };

	//字符串IP转化为整数
	static unsigned int StrToIntIp4(const char * szIp)
	{
		unsigned int ip = 0;

		

		const char * ptr1 = szIp;
		const char * ptr2 = 0;

		while((ptr2=strchr(ptr1,'.')) != 0)
		{
			char szTemp[20]={0};
			memmove(szTemp,ptr1,ptr2-ptr1);
			ip += atoi(szTemp);
			ip <<=  8;

			ptr1 = ptr2+1;
		}

		if(ptr1 && *ptr1)
		{
			ip += atoi(ptr1);
		}

		return ip;
	}

	//整数IP转化为字符串

	static std::string IpToStr( unsigned int Ip)
	{
		char szIp[30]={0}; 
		const unsigned char * ptr = (const unsigned char*)&Ip;
		sprintf(szIp,"%d.%d.%d.%d",ptr[3],ptr[2],ptr[1],ptr[0]);

		return szIp;
	}

		//Unicode,mbcs转换
	static std::wstring	MBCSToWString(const std::string & str)
	{
		setlocale(LC_ALL, "chs");

		int unsigned size = mbstowcs(NULL, str.c_str(), str.length())+1;

	    wchar_t * wbuf = (wchar_t*)malloc(size*sizeof(wchar_t));

	   int unsigned r = mbstowcs(wbuf, str.c_str(), str.length());

	   std::wstring wstr;

	   if(r != -1)
	   {
		   wbuf[r]=0;
		   wstr = wbuf;
	   }

	   free(wbuf);

	    return wstr;
	}



	struct TStrToNumber
	{	
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value,const std::vector<std::string> & vectStr) const
		{			
			if(vectStr.size() <index || index<1)
			{
				return false;
			}

			return StringUtil::StrToNumber(vectStr[index-1],value.GetValue(INT2TYPE(index)));
		}
	};

	template<typename type> static bool StrToNumber(const std::string & str, type & value)
	{
		return StrToNumber( str, value,typename TTypeCategory<Type2Int<type>::value>::TCategory ());
	}


	template<typename type> static bool StrToNumber(const std::string & str, type & value,TypeCategoryUnknow,const char* separator = ";")
	{
		std::vector<std::string>  vectStr;

		int separator_len = strlen(separator);

		std::string  strTemp = str.c_str();
		char * ptr1 = (char*)strTemp.c_str();
		char *ptr2 = NULL;
		while ((ptr2 = strstr(ptr1,separator)) != NULL)
		{
			*ptr2 = 0;
			vectStr.push_back(ptr1);
			ptr1 = ptr2 + separator_len;
		}

		if (*ptr1 != 0)
		{
			vectStr.push_back(ptr1);
		}

		return StrToNumber(vectStr,value);
	}

	template<typename type> static bool StrToNumber(const std::string & str, type & value,TypeCategoryBase)
	{
		std::istringstream ist(str);
		ist >> value;
		return true;
	}

	static bool StrToNumber(const std::string & str, unsigned char & value,TypeCategoryBase)
	{
		value = atoi(str.c_str());

		return true;
	}


	static bool StrToNumber(const std::string & str, std::string & value)
	{
		value = str;
		return true;
	}

	template<typename type> static bool StrToNumber(const std::vector<std::string> & vectStr,type & value)
	{
		return ForEach(value,StringUtil::TStrToNumber(),vectStr);
	}

	template<typename type> static bool StrToNumber(const std::string & str,std::vector<type> & value,int separator = ',')
	{
		std::string  strTemp = str.c_str();
		char * ptr1 = (char*)strTemp.c_str();
		char *ptr2 = NULL;
		while ((ptr2 = strchr(ptr1,separator)) != NULL)
		{
			*ptr2 = 0;
			type v;
			StrToNumber(ptr1,v);
			value.push_back(v);
			ptr1 = ptr2 + 1;
		}

		if (*ptr1 != 0)
		{
			type v;
			StrToNumber(ptr1,v);
			value.push_back(v);
		}

		return true;
	}


	template<typename type,int size> static bool StrToNumber(const std::string & str,TArray<type,size> & value,TypeCategoryArray,int separator = ',')
	{
		std::string  strTemp = str.c_str();
		char * ptr1 = (char*)strTemp.c_str();
		char *ptr2 = NULL;
		int i=0;
		while (i<size && (ptr2 = strchr(ptr1,separator)) != NULL)
		{
			*ptr2 = 0;
			type v;
			StrToNumber(ptr1,v);
			value[i++] = v;
			ptr1 = ptr2 + 1;
		}

		if (i<size && *ptr1 != 0)
		{
			type v;
			StrToNumber(ptr1,v);
			value[i] = v;
		}

		return true;
	}

	template<int size> static bool StrToNumber(const std::string & str,TArray<char,size> & value,TypeCategoryArray,int separator = ',')
	{
		strncpy((char*)value,str.c_str(),size);
		value[size-1] = 0;
		return true;
	}

	//二进制
	template<int size> static bool StrToNumber(const std::string & str,TBinArray<size> & value,TypeCategoryBinArray,int separator = ',')
	{
		memset((unsigned char*)value,0,size);
		int len = str.length();
		const unsigned char* ptr = (const unsigned char*)str.c_str();
		for(int i=0;i<len/2 && i<size; i++)
		{
			unsigned char ch = ptr[i*2];
			if(ch>='A')
			{
				ch -= 'A';
			}
			else if(ch >= 'a')
			{
				ch -= 'a';
			}

			value[i] = (ch<<4);

			ch = ptr[i*2+1];

			if(ch>='A')
			{
				ch -= 'A';
			}
			else if(ch >= 'a')
			{
				ch -= 'a';
			}

			value[i] |= (ch & 0x0f);
		}

		return true;
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//数字转化为字符串

	struct TNumberToStr
	{
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value, std::vector<std::string> & vectStr) const
		{	
			std::string str ;
			StringUtil::NumberToStr(value.GetValue(INT2TYPE(index)),str);
			vectStr.push_back( str);
			return true;
		}
	};

	static bool NumberToStr( const std::string & value,std::string & str)
	{
		str = value;
		return true;
	}


	template<typename type> static bool  NumberToStr(const type & value,std::string & str)
	{
		return NumberToStr(value,str,TTypeCategory<Type2Int<type>::value>::TCategory  ());
	}

	template<typename type> static bool  NumberToStr(const type & value,std::string & str,TypeCategoryUnknow,const char * separator = ";")
	{
		std::vector<std::string>  vectStr;
		if(NumberToStr(value,vectStr)==false)
		{
			return false;
		}

		std::ostringstream ost;

		for(int i=0; i< vectStr.size();++i)
		{
			ost << vectStr[i] << separator;
		}

		str = ost.str();

		return true;
	}

	template<typename type> static bool  NumberToStr(const type & value,std::string & str,TypeCategoryBase)
	{
		std::ostringstream ost;
		ost << value;
		str = ost.str();
		return true;
	}

	 static bool  NumberToStr(const unsigned char & value,std::string & str,TypeCategoryBase)
	{
		char szTemp[30] = {0};
		
		sprintf(szTemp,"%d",value);
		str = szTemp;
		return true;
	}



	template<typename type> static bool NumberToStr( type & value, std::vector<std::string> & vectStr)
	{
		return ForEach(value,TNumberToStr(),vectStr);
	}


	template<typename type> static bool NumberToStr(const std::vector<type> & value,std::string &str,int separator = ',')
	{
		char szTemp[30]={0};
		sprintf(szTemp,"%c",separator);

		std::ostringstream ost;
		for(int i=0; i<value.size();++i)
		{			
			std::string str;
			NumberToStr(value[i],str);
			ost << str << szTemp;
		}

		str = ost.str();
		return true;
	}

	template<typename type,int size> static bool NumberToStr(const TArray<type,size> & value,std::string &str,TypeCategoryArray,int separator = ',')
	{
		char szTemp[30]={0};
		sprintf(szTemp,"%c",separator);

		std::ostringstream ost;
		for( int  i=0; i<size;++i)
		{			
			std::string str;
			NumberToStr(value[i],str);
			ost << str << szTemp;
		}

		str += ost.str();
		return true;
	}

	template<int size > static bool NumberToStr(const TArray<char,size> & value,std::string &str,TypeCategoryArray,int separator = ',')
	{
		str += (char*)value;

		return true;
	}

	template<int size > static bool NumberToStr(const TBinArray<size> & value,std::string &str,TypeCategoryBinArray,int separator = ',')
	{
		char szTemp[30]={0};

		std::ostringstream ost;
		for( int  i=0; i<size;++i)
		{	
			sprintf(szTemp,"%02x",value[i]);	
			ost << szTemp;
		}

		str += ost.str();
		return true;

		return true;
	}


};


#endif

