
#ifndef __BASELIB_FILECSV_H__
#define __BASELIB_FILECSV_H__

/*
CSV 即 Comma Separate Values ，是一种纯文本格式，用来存储数据。这种文件格式经常用来作为不同程序之间的数据交互的格式。最终文件可以用电子表格程序（如 Microsoft Excel ）打开，也可以用作其他程序的导入格式。 

CSV 文件格式 

·                       每条记录占一行 （但字段中有换行符的情况，一行也会变成多行） 
·                       以逗号为分隔符 
·                       逗号前后的空格会被忽略 
·                       字段中包含有逗号，该字段必须用双引号括起来 
·                       字段中包含有换行符，该字段必须用双引号括起来 
·                       字段前后包含有空格，该字段必须用双引号括起来 
·                       字段中的双引号用两个双引号表示 
·                       字段中如果有双引号，该字段必须用双引号括起来 
·                       第一条记录，可以是字段名 
*/

#include <string>
#include <vector>
#include "stdio.h"
#include "StringUtil.h"

/***************算法说明 ****************************************************************
根据CSV 文件格式我们知道：每列以逗号分隔，列格式有三种情况
1) <普通列> ,                                //需要把逗号前后的空格去掉
2) "<本列带有:逗号 空格 换行\n符中的一种>",  //需要把头尾的双引号去掉
3) "<本列带有需要转义的双引号"">",           //需要把头尾的双引号去掉,且把中间的两个双引号替换为一个双引号


*****************************************************************************************/

/****************************************************************************************
函数:ReadCsvFile 读取csv文件
参数:[in] szFileName 文件名
[out] table 把文件内容读到该表中
返回值: true 成功，false 失败
****************************************************************************************/
static bool ReadCsvFile(const char *szFileName ,std::vector<std::vector<std::string> > & table)
{
	if(szFileName==NULL || *szFileName==0)
	{
		return false;
	}

	FILE * pFile = ::fopen(szFileName,"r");
	if(pFile == NULL)
	{
		return false;
	}


#define ADD_TEXT(table,row,col,buf,len)                       \
{	                                                          \
	if(row>=table.size())                                     \
	{                                                         \
		table.resize(row+1);                                  \
	}                                                         \
	if(col >= table[row].size())                              \
	{                                                         \
		table[row].resize(col+1);                             \
	}                                                         \
	buf[len] = 0;                                             \
	table[row][col] += buf;                                   \
	len = 0;                                                  \
}

	const int buff_max = 1024;
	char context[buff_max]; //文件内容
	int file_len = 0;

	char buf[buff_max]={0}; //保存每个列的内容
	int len = 0;       //本列有效长度
	int DquoteNum = 0;  //双引号数量
	size_t row = 0;	    //行
	size_t col = 0;       //列		
	
	while( (file_len = ::fread(context,1,sizeof(context)-1,pFile))>0)
	{
		int ch = 0;  //读取一个字符
		for(int i=0;i<file_len && (ch=context[i]) !=NULL;i++)
		{			
			switch(ch)
			{
			case  ',': //逗号
				{
					if(DquoteNum%2==0) //为偶数，所有双引号匹配
					{							
						ADD_TEXT(table,row,col,buf,len);
						col++;
						DquoteNum = 0;
						continue;
					}				
				}
				break;
			case '\n':  //换行
				{
					if(DquoteNum%2==0)  //为偶数，所有双引号匹配
					{	
						ADD_TEXT(table,row,col,buf,len);						
						col = 0;
						DquoteNum = 0;
						row++;
						continue;
					}				
				}
				break;
			case ' ':  //空格
			case '\t': //tab键
				{			
					if(DquoteNum == 0 ) //本列不是以双引号开头，所以不能包含空格
					{
						continue;
					}
				}
				break;
			case  '"':  //双引号
				{		
					DquoteNum++;

					if(DquoteNum == 1) //开头的双引号忽略
					{
						len=0; //去掉第一个双引号前的字符
						continue;
					}
					else if(DquoteNum % 2 == 0) 
					{
						//当一列中带有特殊字符:逗号，空格，换行，双引号时，本列以双引号开头及双引号结尾,分为两种情况:
						//1) "本列不带有双引号,只带其他特殊字符" 
						//   ^                                 ^
						// DquoteNum=1 (不进该分支)       DquoteNum=2 忽略

						//2) "本列带有需要转义的双引号("")"
						//   ^                         ^^
						// DquoteNum=1 (不进该分支)    2,3 第二个忽略,第三个保留，

						continue;
					}
				}
				break;

			default:
				break;
			}

			if( len==sizeof(buf)-1) //缓存区已满
			{
               ADD_TEXT(table,row,col,buf,len);			
			}

			buf[len++] = ch;

		}		
	}

	//需要处理最后一列
	if(len>0)
	{
		ADD_TEXT(table,row,col,buf,len);
	}

	::fclose(pFile);

	return true;
}

/************************************算法说明*********************************************
扫描每一列，是否包含有特殊符号，
1) 如有，往文件中写入双引号，且把本列开头至第一个特殊字符写入文件，继续本列，遇到双引号则多写一个双引号，最后在列尾多写一个双引号
2) 如没有，把整列写入文件
3) 处理完每一行后，写一个'\n'到文件,继续处理下一行 
*****************************************************************************************/

/****************************************************************************************
函数:WriteCsvFile 写csv文件
参数:[in] szFileName 文件名
[out] table 把该表中内容写到文件
返回值: true 成功，false 失败
****************************************************************************************/
static bool WriteCsvFile(const char *szFileName ,const std::vector<std::vector<std::string> > & table)
{
	if(szFileName==NULL || *szFileName==0)
	{
		return false;
	}

	FILE * pFile = ::fopen(szFileName,"w");
	if(pFile == NULL)
	{
		return false;
	}

	//保存需要写到文件中去的内容，缓存满的时候才真正写文件，避免多次写文件，性能低下
	const int buff_max = 1024;

	char context[buff_max]; //文件内容
	int file_len = 0;

#define WRITE_CHAR(ch)                                                       \
{                                                                           \
	if(file_len>=sizeof(context)-1)                                          \
	{                                                                        \
	if(::fwrite(context,1,file_len,pFile) !=file_len ){ return false;}     \
	file_len = 0;                                                          \
	}                                                                        \
	context[file_len++] = ch;                                                \
}

#define WRITE_STR( str ,len )                                                              \
{                                                                                          \
	if( len >= sizeof(context)-1 || len + file_len >= sizeof(context)-1)                   \
	{                                                                                      \
	if(file_len > 0)                                                                 \
	{                                                                                \
	if(::fwrite(context,1,file_len,pFile) !=file_len ) { return false; }       \
	file_len = 0;                                                              \
	}                                                                                \
	if(len > 0)                                                                      \
	{                                                                                \
	if(::fwrite(str,1,len,pFile) != len ) { return false; }                    \
	}                                                                                \
	}                                                                                      \
	else if( len > 0)                                                                      \
	{                                                                                      \
	memcpy(context+file_len,str,len);                                                   \
	file_len += len;                                                                    \
	}	                                                                                   \
}  


	for(size_t row =0; row<table.size();row++)
	{
		const std::vector<std::string> & vectRow = table[row];
		for(size_t col=0; col < vectRow.size();col++)
		{
			//处理每一列
			if(col != 0)
			{
				WRITE_CHAR( ',');  //不是首列，先增加逗号分隔符
			}
			const char * ptr = vectRow[col].c_str();
			if (ptr != NULL )
			{
				const char * ptrSrc = ptr;
				char ch;
				bool bHaveSeparate = false; //是否有特殊字符
				for(;(ch = * ptr ) !=NULL; ptr++)
				{
					if(bHaveSeparate==false)
					{
						if((ch == '"' || ch==',' || ch=='\n' || ch == ' ' || ch== '\t'))
						{						
							WRITE_CHAR( '"');  //增加列开头的双引号		
							WRITE_STR(ptrSrc,(ptr-ptrSrc));
							bHaveSeparate = true;						
						}
						else
						{
							continue;  //目前为止，还不知道本列是否含有特殊字符
						}
					}

					WRITE_CHAR( ch );
					//本字符是双引号，需要增加一个
					if(ch == '"')
					{
						WRITE_CHAR( '"');
					}
				}

				if( bHaveSeparate ) //有殊字符
				{
					WRITE_CHAR('"'); //增加双引号
				}
				else
				{
					//写本列内容
					WRITE_STR(ptrSrc,(ptr-ptrSrc));				
				}
			}
		}
		//增加换行符
		WRITE_CHAR(  '\n' );		
	}

	if(file_len>0)
	{
		if(::fwrite(context,1,file_len,pFile) !=file_len )
		{
			//写文件出错了
			return false;
		}
		file_len = 0;
	}	

	::fclose(pFile);

	return true;
}


class FileCSV
{
public:
	bool Open(const char* szFileName)
	{
		if(ReadCsvFile(szFileName,m_Table)==false)
		{
			return false;
		}
		m_strFileName = szFileName;
		m_bUpdate = false;
		return true;
	}

	//仅是更新到内存
	bool Update(int row,int col, const char* szField)
	{
		if(szField == NULL)
		{
			return false;
		}
		if(row >=(int) m_Table.size())
		{
			m_Table.resize(row+1);
		}
		if(col >= (int)m_Table[row].size())
		{
			m_Table[row].resize(col+1);
		}

		m_Table[row][col] = szField;

		m_bUpdate = true;

		return true;
	}

	int GetRowNum(){ return m_Table.size();}
	int GetColNum()
	{
		if( m_Table.size())
		{
			return m_Table[0].size();
		}
		return 0;
	}

	const std::vector<std::string> *  GetRow(int row)
	{
		if(row<(int)m_Table.size())
		{
			return &m_Table[row];
		}

		return NULL;
	}

	template<typename type>
		bool GetRow(int row,type & value)
	{
		const std::vector<std::string> *pVect = GetRow(row);
		if(pVect==NULL)
		{
			return false;
		}

		return StringUtil::StrToNumber(*pVect,value);
	}

		template<typename type>
			bool GetTable(std::vector<type> & value,int begin=1)
	{
		for(int row=begin;row<GetRowNum();++row)
		{
		   const std::vector<std::string> *pVect = GetRow(row);
		   if(pVect==NULL)
		   {
			 return false;
		   }
           type v;
		  if( StringUtil::StrToNumber(*pVect,v)==false)
		  {
			  return false;
		  }
		  value.push_back(v);
		}

		return true;
	}

	const std::string * GetField(int row,int col)
	{
		const std::vector<std::string> * pVect = GetRow(row);
		if(pVect && col< (int)pVect->size())
		{
			return &(*pVect)[col];
		}
		return NULL;
	}

	template<typename type>
	bool GetField(int row,int col,type & value)
	{
		const std::vector<std::string> * pVect = GetRow(row);
		if(pVect && col<pVect->size())
		{
			StringUtil::StrToNumber((*pVect)[col],value);
			return true;
		}
		return false;
	}

	//更新一行
	template<typename type>
	bool UpdateRow(int row, const type & value)
	{
		std::vector<std::string> vectStr;
		if (StringUtil::NumberToStr(value,vectStr)==false)
		{
			return false;
		}
		for (int col=0; col<vectStr.size();col++)
		{
			if (Update(row,col,vectStr[col].c_str())==false)
			{
				return false;
			}
		}
		return true;
	}

	template<typename type>
		bool UpdateTable(const std::vector<type> & vectValue,int begin=1)
	{
		for (int row=0; row<vectValue.size();++row)
		{
			std::vector<std::string> vectStr;
			if (StringUtil::NumberToStr(vectValue[row],vectStr)==false)
			{
				return false;
			}
			for (int col=0; col<vectStr.size();col++)
			{
				if (Update(row+begin,col,vectStr[col].c_str())==false)
				{
					return false;
				}
			}
		}
		
		return true;
	}


	//马上写到硬盘
	bool Flush()
	{
		if(m_bUpdate)
		{
			return WriteCsvFile(m_strFileName.c_str(),m_Table);

		}

		return true;
	}

	~FileCSV()
	{
		Flush();
	}

private:

private:
	typedef std::vector<std::string> LINE;
	typedef std::vector<LINE>        TABLE;
	TABLE  m_Table;          //表
	std::string m_strFileName; //文件名
	bool m_bUpdate;  //是否有更新
};

#endif



