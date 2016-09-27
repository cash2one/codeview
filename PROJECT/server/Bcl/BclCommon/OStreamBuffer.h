
#ifndef __BCL_OSTREAM_BUFFER_H__
#define __BCL_OSTREAM_BUFFER_H__

#include "BclHeader.h"

//向前声明
 class OStreamBuffer;

//可以把OStreamBuffer信息打包为该结构，该结构可用于容器中，
class OStreamBufferInfo
{  
	friend class OStreamBuffer;
	char*   m_pData; //内存
	size_t  m_Size;  //有效长度
	size_t  m_Capacity; //容量
public:
	OStreamBufferInfo()
	{
		m_pData = 0;
		m_Size = 0;
		m_Capacity = 0;
	}
};

//输入流buffer，赋值和拷贝时，内存所有权会转移,析构函数会release函数
class BCL_API OStreamBuffer
{
public:
    OStreamBuffer();

	~OStreamBuffer();

	//预分配内存
	explicit OStreamBuffer(size_t size);

	//扩展内存
	bool ResetCapacity(size_t size);

	//获得有效数据
	size_t Size() const;

	//获得容量
	size_t Capacity() const;

	//获得从pos位置开始的内存
	const char * Buffer(size_t pos = 0) const; 

	//获取有效数据结束位置的buffer,相当于调用Buffer(Size())
	const char * End() const;

	//复位
	void Reset();

	//剩余空间
	size_t Remain() const ;

	//写入二进制数据
	bool Write(const void * pData,size_t len);

	//拷贝构造函数，注意:内存所有权会转移
	OStreamBuffer( OStreamBuffer & Other);

	//拷贝赋值函数，注意:内存所有权会转移
	OStreamBuffer & operator =( OStreamBuffer & Other);

	//转化为OStreamBufferInfo结构，
	//注意，内存所有权会转移
	//后面需要用该OStreamBufferInfo，构造新的OStreamBuffer对象，否则会内存泄漏
	OStreamBufferInfo TakeOStreamBufferInfo();

	OStreamBuffer(OStreamBufferInfo & obcInfo);

		//有效数据长度增加len,如果内存不足，会自动扩展
	bool Skip(size_t len);

private:
	//初始化
	void __Init() ;

	void __Release();

private:
	char*   m_pData; //内存
	size_t  m_Size;  //有效长度
	size_t  m_Capacity; //容量

};











#endif

