
#ifndef __BASELIB_FILECONFIG_H__
#define __BASELIB_FILECONFIG_H__

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include "stdio.h"
#include "StringUtil.h"

#ifdef WIN32
#include "windows.h"
#endif


#define SECTION "section"

class FileConfig
{
public:
	FileConfig()
	{
	}
	~FileConfig()
	{
	}
	bool Open(const char *szFileName,const char * szSection=0)
	{
		if(szFileName==NULL || *szFileName==0)
		{
		   return false;
	    }

		m_strFileName = szFileName;
		if(szSection)
		{
			m_strSection = szSection;
		}
		else
		{
			m_strSection = SECTION;
		}

	    FILE * pFile = ::fopen(szFileName,"r+");
	    if(pFile == NULL)
	    {
		  return false;
	    }

		char Buff[1024];
		 int len = 0;

		std::string strLine;

		while((::fgets(Buff,sizeof(Buff),pFile))!=NULL)
		{
			len = strlen(Buff);
			if(Buff[len-1] != '\n')
			{
				 strLine += Buff;
			}
			else
			{
				 Buff[len-1] =  0;
				 strLine += Buff;				
				Add_Line(strLine);
			}
			
		}

		if (strLine.length())
		{
			Add_Line(strLine);		
		}

		::fclose(pFile);
		return true;
	}

	struct TStrToNumber
	{	
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value,FileConfig * pFileConfig) const
		{				
			return pFileConfig->Read(value.GetValue(INT2TYPE(index)),value.GetName(INT2TYPE(index)));
		}
	};


	//读取
	template<class type>
    bool Read(type & value,const char* key=NULL)
	{
		return Read(value,key,typename TTypeCategory<Type2Int<type>::value>::TCategory ());
	}

	template<class type>
	bool Read(std::vector<type> & value,const char* key=NULL)
	{
		return __Read(value,key,typename TTypeCategory<Type2Int<type>::value>::TCategory ());
		
	}

	template<class type>
	bool __Read(std::vector<type> & value,const char* key,TypeCategoryKnow)
	{
		if(key==NULL) return false;

		int i= 1;
		while (1)
		{
			std::ostringstream ost;
			ost << key << "_" << i++;
			type v;
			if (Read(v,ost.str().c_str())==false)
			{
				break;
			}
			value.push_back(v);
		}
		return true;
		
	}

	template<class type>
	bool __Read(std::vector<type> & value,const char* key,TypeCategoryUnknow)
	{
		if(key==NULL) return false;

		int i= 1;
		while (1)
		{
			std::ostringstream ost;
			ost << key << "_" << i++;
			type v;
			if (Read(v,ost.str().c_str())==false)
			{
				break;
			}
			value.push_back(v);
		}
		return true;
		
	}

	template<class type>
	bool __Read(std::vector<type> & value,const char* key,TypeCategoryBase)
	{
		if(key==NULL) return false;
		
		return Read(value,key,TypeCategoryKnow());
		
	}
	
	//区分是不是结构体
	template<class type>
    bool Read(type & value,const char* key,TypeCategoryUnknow)
	{
		if(key != NULL)
		{
			return Read(value,key,TypeCategoryKnow());
		}
		return ForEach(value,TStrToNumber(),this);
	}

	template<class type>
    bool Read(type & value,const char* key,TypeCategoryKnow)
	{

		std::string str;
		if(GetStrFromKey(key,str)==false)
		{
			return false;
		}
		
		return StringUtil::StrToNumber(str,value);
	}
	

	template<class type>
	bool Read(std::vector<std::vector<type> > & value,const char* key)
	{
		int i= 1;
		while (1)
		{
			std::ostringstream ost;
			ost << key << "_" << i++;
			std::vector<type> vect;
			if (Read(vect,ost.str().c_str())==false)
			{
				break;
			}
			value.push_back(vect);
		}
		return true;
	}

	//template<>
	bool Read(std::vector<std::string > & value,const char* key)
	{
		int i= 1;
		while (1)
		{
			std::ostringstream ost;
			ost << key << "_" << i++;
			std::string str;
			if (Read(str,ost.str().c_str())==false)
			{
				break;
			}
			value.push_back(str);
		}
		return true;
	}

	template<int size>
	bool Read(TArray<std::string,size> & value,const char* key,TypeCategoryKnow)
	{
		for(int i=0;i<size;++i)
		{
			std::ostringstream ost;
			ost << key << "_" << i+1;
			std::string str;
			if (Read(str,ost.str().c_str())==false)
			{
				break;
			}
			value[i]=str;
		}
		return true;
	}	
	
	
	//写文件

	struct TNumberToStr
	{	
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value,FileConfig * pFileConfig) const
		{				
			return pFileConfig->Write(value.GetValue(INT2TYPE(index)),value.GetName(INT2TYPE(index)));
		}
	};


	template<class type>
		bool Write(type & value,const char* key=NULL)
	{
		return Write(value,key,typename TTypeCategory<Type2Int<type>::value>::TCategory ());
	}

	template<class type>
	bool Write(std::vector<type> & value,const char* key=NULL)
	{		
		return __Write(value,key,typename TTypeCategory<Type2Int<type>::value>::TCategory ());
	}

	template<class type>
	bool __Write(std::vector<type> & value,const char* key,TypeCategoryBase)
	{		
		return Write(value,key,TypeCategoryKnow());
	}

	template<class type>
	bool __Write(std::vector<type> & value,const char* key,TypeCategoryKnow)
	{
		if(key==NULL) return false;

		for(int i=0; i<value.size();i++)
			{
				std::ostringstream ost;
				ost << key << "_" << i+1;			
				if (Write(value[i],ost.str().c_str())==false)
				{
					break;
				}
			}
		return true;
	}

	template<class type>
	bool __Write(std::vector<type> & value,const char* key,TypeCategoryUnknow)
	{
		if(key==NULL) return false;

		for(int i=0; i<value.size();i++)
			{
				std::ostringstream ost;
				ost << key << "_" << i+1;			
				if (Write(value[i],ost.str().c_str())==false)
				{
					break;
				}
			}
		return true;
	}

		template<class type>
		bool Write(type & value,const char* key,TypeCategoryUnknow)
	{
		if(key != NULL)
		{
			return Write(value,key,TypeCategoryKnow());
		}

		return ForEach(value,TNumberToStr(),this);
	}

	template<class type>
		bool Write(type & value,const char* key,TypeCategoryKnow)
	{
		std::string str ;
		if(StringUtil::NumberToStr(value,str)==false)
		{
			return false;
		}
		return SetStrToKey(key,str);
	}

	template<class type>
	bool Write(type & value,const char* key,TypeCategoryString)
	{
		std::string str ;
		if(StringUtil::NumberToStr(value,str)==false)
		{
			return false;
		}
		return SetStrToKey(key,str);
	}

	bool Write( std::vector<std::string> & value,const char* key )
	{
			for(int i=0; i<value.size();i++)
			{
				std::ostringstream ost;
				ost << key << "_" << i+1;			
				if (Write(value[i],ost.str().c_str())==false)
				{
					break;
				}
			}
			return true;
	}

	template<int size>
	bool Write( TArray<std::string,size> & value,const char* key,TypeCategoryKnow )
	{
			for(int i=0; i<size;i++)
			{
				std::ostringstream ost;
				ost << key << "_" << i+1;			
				if (Write(value[i],ost.str().c_str())==false)
				{
					break;
				}
			}
			return true;
	}

		template<class type>
		bool Write(std::vector<std::vector<type> > & value,const char* key)
		{
			for(int i=0; i<value.size();i++)
			{
				std::ostringstream ost;
				ost << key << "_" << i+1;			
				if (Write(value[i],ost.str().c_str())==false)
				{
					break;
				}
			}
			return true;
		}


		
private:

	void Add_Line(std::string & strLine)
	{

		std::string strKey;
		std::string strValue;

		//找'//'	
		int pos = strLine.find("///");
		if(pos != std::string::npos)
		{
			strLine = strLine.substr(0,pos);
		}

		//找'='
		pos = strLine.find('=');
		if(pos != std::string::npos)
		{
			strKey = strLine.substr(0,pos);
			strValue = strLine.substr(pos+1);
			strLine.clear();
			if (strKey.empty()|| strValue.empty())
			{
				return;
			}

			//处理左空格	
			if ((pos = strKey.find_first_not_of(" \t")) == std::string::npos)
			{
				return;
			}
			
			if (pos != 0)
			{
				strKey = strKey.substr(pos,std::string::npos);
			}
			

			//处理右空格
			if ((pos=strKey.find_last_not_of(" \t"))!=std::string::npos)
			{
				strKey = strKey.substr(0, strKey.find_last_not_of(" \t")+1);
			}
		

			//处理左空格
			if ((pos = strValue.find_first_not_of(" \t")) == std::string::npos)
			{
				return;
			}
			if (pos != 0)
			{
				strValue = strValue.substr(pos,std::string::npos);
			}
			//处理右空格			
			if ((pos=strValue.find_last_not_of(" \t"))!=std::string::npos)
			{
				strValue = strValue.substr(0, strValue.find_last_not_of(" \t")+1);
			}

			m_Table[strKey] = strValue;
		}
		else
		{
			strLine.clear();
		}
	}

	bool GetStrFromKey(const char* key,std::string & str)
	{
		if(key==NULL) return false;

		TMAP_TABLE::iterator it = m_Table.find(key);
		if (it == m_Table.end())
		{
			return false;
		}

		str = (*it).second;

		return true;
	}

	bool SetStrToKey(const char* key,std::string & str)
	{
		if(key==NULL)
		{
			return false;
		}
#ifdef WIN32
		return ::WritePrivateProfileString(m_strSection.c_str(), key, str.c_str(), m_strFileName.c_str());
#else
		 return false;  //暂时不支持非windows系统
#endif
	}

private:
	typedef std::map<std::string,std::string> TMAP_TABLE;
	TMAP_TABLE m_Table;
	std::string m_strFileName;
	bool m_bUpdate;  //是否有更新

	std::string m_strSection;
};

#endif


