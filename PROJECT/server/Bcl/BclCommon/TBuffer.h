
#ifndef __BCL_TBUFFER_H__
#define __BCL_TBUFFER_H__

#include <memory>
#include "OStreamBuffer.h"
#include <string>
//输出buffer
template<int static_size>
class TOBuffer
{	
	template<int size>  friend  class TOBuffer;
public:
	TOBuffer() { __Init();}

	//预分配容量
	explicit TOBuffer(size_t len)
	{
		__Init();
		ResetCapacity(len);
	}

	//预分配容量
	TOBuffer(OStreamBuffer & Osb)
	{
		__Init();
		m_Osb = Osb;
	}

	TOBuffer(OStreamBufferInfo & OsbInfo)
	{
		__Init();
		m_Osb = OStreamBuffer(OsbInfo);
	}

	


	~TOBuffer()
	{
		__Init();
	}

	//复位
	void Reset()
	{
		m_Size = 0;
		m_bError = false;
		m_Osb.Reset();
	}

	//返回从pos开始的内存
	const char * Buffer(size_t pos=0) const
	{
		if(__IsOsb())
		{
			return m_Osb.Buffer(pos);
		}

		if(pos>static_size)
		{
			return 0;
		}

		return m_StaitcData + pos;
	}


	//获得数据大小
	size_t Size() const 
	{	
		if(__IsOsb())
		{
			return m_Osb.Size();
		}
		return m_Size; 
	}

	//当前容量
	size_t Capacity()
	{
		if(__IsOsb())
		{
			return m_Osb.Capacity();
		}
		return static_size; 
	}

	//余下空间
	size_t Remain() const 
	{
		if(__IsOsb())
		{
			return m_Osb.Remain();
		}

		return static_size - m_Size;
	}

	//有效数据长度增加len,如果内存不足，会自动扩展
	bool Skip(size_t len) 
	{
		size_t size = len + Size();
		if(size > Capacity())
		{
			if(ResetCapacity(size)==false)
			{
				return false;
			}
		}

		if(__IsOsb())
		{
			return m_Osb.Skip(len);
		}
		else 
		{
			m_Size = size;			
		}
		

		return true;
	}

	const char * CurrentBuffer() const
	{
		return Buffer(Size());
	}


	template <typename type> 
	TOBuffer<static_size> & operator << (type & Value)
	{		
		return Push(&Value,sizeof(Value));
	}

	template <int size> 
	TOBuffer<static_size> & operator << (TOBuffer<size> & Value)
	{		
		return Push(Value.Buffer(),Value.Size());
	}


	//template <> 
	TOBuffer<static_size> & operator << (const std::string  & Value)
	{		
		return Push(Value.c_str(),Value.length()+1);
	}

	//template <> 
	TOBuffer<static_size> & operator << ( std::string  & Value)
	{		
		return Push(Value.c_str(),Value.length()+1);
	}

	//push二进制内存
	TOBuffer<static_size> & Push(const void* pData,int len)
	{
		if(m_bError)
		{
			return *this;
		}
		size_t size = len + Size();
		if(Capacity()<size)
		{
			if(false == ResetCapacity(size))
			{
				m_bError = true;
				return *this;
			}
		}
		if(len >0 && 0 != pData)
		{
			if(__IsOsb())
			{
				if(false==m_Osb.Write(pData,len))
				{
					m_bError = true;
				}
			}
			else
			{
			   memmove(m_StaitcData+m_Size,pData,len);
			   m_Size += len;
			}
		}

		return *this;
	}

	//在指定位置插入
	bool Insert(int pos,const void* pData,int len)
	{
		if(m_bError || pData==0 || len<1|| pos+len > Size())
		{
			return false; 
		}

		char * ptr = (char*)Buffer(pos);
		if(ptr==0)
		{
			return false;
		}
		memmove(ptr,pData,len);

		return true;

	}


	//注意：内存所有权会转移
	OStreamBuffer TakeOsb()
	{
		if(__IsOsb()) 
		{
			return m_Osb;
		}

		OStreamBuffer Osb(m_Size);
		Osb.Write(m_StaitcData,m_Size);

		__Init();

		return Osb;
	}


		//扩展内存
	bool ResetCapacity(size_t len)
	{
		if(__IsOsb())
		{
			if(static_size>=len) //再次使用数组
			{
				m_Size = m_Osb.Size();
				if(m_Size)
				{
					memmove(m_StaitcData,m_Osb.Buffer(),m_Size);
				}
				m_Osb = OStreamBuffer(); 
				return true;
			}

			return m_Osb.ResetCapacity(len);
		}	
		else
		{
			//转为使用osb
			if(m_Osb.ResetCapacity(len)==false) return false;

			if(m_Size>0)
			{
				if(false== m_Osb.Write(m_StaitcData,m_Size))
				{
					return false;
				}

				m_Size = 0;
			}
		}

		return true;
	}

		//是否产生了错误
	bool Error() const {return m_bError;}




	TOBuffer(TOBuffer<static_size>& Other);
	TOBuffer<static_size> & operator =(TOBuffer<static_size>&);


private:
	//初始化
	void __Init()
	{
		m_Size = 0;
		m_bError = false;
	}

	//是否使用OStreamBuffer
	bool __IsOsb() const { return (m_Osb.Buffer()!=0);}	


private:
	OStreamBuffer  m_Osb;
	char           m_StaitcData[static_size]; //初始数组大小
	size_t         m_Size  ;   //有效数据长度
	bool           m_bError ;   //是否产生了错误
};

typedef TOBuffer<1> OBuffer;
typedef TOBuffer<16> OBuffer16;
typedef TOBuffer<32> OBuffer32;
typedef TOBuffer<64> OBuffer64;
typedef TOBuffer<128> OBuffer128;
typedef TOBuffer<128> OBuffer256;
typedef TOBuffer<128> OBuffer512;
typedef TOBuffer<1024> OBuffer1k;
typedef TOBuffer<1024*2> OBuffer2k;
typedef TOBuffer<1024*3> OBuffer3k;
typedef TOBuffer<1024*4> OBuffer4k;
typedef TOBuffer<1024*5> OBuffer5k;
typedef TOBuffer<1024*6> OBuffer6k;
typedef TOBuffer<1024*7> OBuffer7k;
typedef TOBuffer<1024*8> OBuffer8k;
typedef TOBuffer<1024*9> OBuffer9k;
typedef TOBuffer<1024*16> OBuffer16k;




//输入Buffer
class IBuffer
{
public:
	IBuffer(const char* pData,int len)
	{
		m_pData = pData;
		m_Capacity = len;
		m_Size = 0;	
		m_bError = false;
	}

	~IBuffer()
	{
		m_pData = 0;
		m_Size = 0;
		m_Capacity = 0;	
		m_bError = false;
	}

	//总长度
	int Capacity(){return m_Capacity;}

	//余下未读数据
	int Remain() { return m_Capacity - m_Size; }

	//已读数据
	int Size() {return m_Size;}

	//返回从len字节开始的内存
	const char * Buffer( int len =0)
	{
		if(len>m_Capacity) return NULL;
		return m_pData+len;
	}

	//返回未读buffer
	const char * CurrentBuffer()
	{
		return m_pData+m_Size;
	}


	//是否产生了错误
	bool Error(){return m_bError;}

	//读取
	template<typename type> 
	IBuffer & operator >> (type & value)
	{
		if((size_t)m_Size+sizeof(value)>(size_t)m_Capacity)
		{
			m_bError = true;
		}
		else
		{
			value = *(type*)(m_pData+m_Size);
			m_Size += sizeof(value);
		}

		return *this;
	}

	//读取
	//template<> 
	IBuffer & operator >> (std::string & value)
	{
		//查找'\0'
		bool bFinded = false;
		int len = 0;
		for(int i=m_Size; i<m_Capacity; ++i)
		{
			len++;
			if(m_pData[i]==0)
			{
				bFinded = true;
				break;
			}
		}

		if(bFinded)
		{
			value = (char*)(m_pData+m_Size);
			m_Size += len;
		}
		else
		{
			m_bError = true;
		}

		return *this;
	}

	//读取二进制内存
	IBuffer & Pop(void* pData,int len)
	{
		if(m_Size+len>m_Capacity)
		{
			m_bError = true;
		}
		else
		{
			memcpy(pData,m_pData+m_Size,len);
			m_Size += len;
		}

		return *this;
	}


private:
	const char * m_pData;  //有效数据
	int    m_Size;   //已读取的数据
	int    m_Capacity; //buffer容量	
	bool   m_bError;   //是否产生了错误
};


#endif
