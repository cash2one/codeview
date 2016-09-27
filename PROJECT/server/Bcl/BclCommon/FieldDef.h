

#ifndef __BCL_FIELD_DEF_H__
#define __BCL_FIELD_DEF_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //定义数组类型
#include <vector>
#include <string>

//数组元素数量
#define  ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

//内存大小
#define MEM_SIZE(array) (ARRAY_SIZE(array)*sizeof(array[0]))


template <typename type> type SizeOf(type  value);

template <typename type> void SizeOf(type * p);

//不能用于指针和数组的sizeof
#define SIZE_OF(type) (sizeof(SizeOf(type)))


template<size_t size> struct TObjectSize
{
	char data[size];
};
//内存置零

template <typename type> type ValueType(type *);

template <typename type> type ValueType(type );

template<typename type>
TObjectSize<sizeof(ValueType((type)0))> ObjectSize(type  ptr);

template<typename type,size_t size>
TObjectSize<sizeof(type)*size> ObjectSize(type (&) [size]);

#define  MEM_ZERO(ptr) if(ptr) memset(ptr,0,sizeof(ObjectSize(ptr)))

//指针的大小
#define PTR_SIZE   sizeof(void*)


//变长二进制
struct VarBinary
{
	int m_size;

	VarBinary()
	{
        m_size = 0;
	}

	int size()const {return m_size;}

	void * buffer()
	{
		   return (void*)(this+1);
	 }	

	 operator void *()const
	  {
		    return (void*)(this+1);
	  }	

	  operator void *()
	  {
		    return (void*)(this+1);
	  }	
};


 template <class type,int len>
struct TArray
{	
	TArray()
	{
		memset(this,0,sizeof(*this));
	}

	 type m_data[len];

	 int size()const {return len;}

	 template<typename IndexType>
	  type & operator [] (IndexType index) 
	 {
	      return m_data[index];
	 }

	 template<typename IndexType>
	 const type & operator [] (IndexType index) const
	 {
	      return m_data[index];
	 }

     operator type *()const
	  {
		   return (type*)this;
	  }	

	  operator const type *()const
	  {
		  return (type*)this;
	  }
	
};

//位数组
template<int param_size> struct TBitArray
{
	unsigned char m_data[(param_size+7)/8];

	TBitArray()
	{
		MEM_ZERO(this);
	}

	TBitArray(const void * buff,int len)
	{
		MEM_ZERO(this);

		if(buff && len > 0)
		{
			int k = ARRAY_SIZE(m_data) > len ? len : ARRAY_SIZE(m_data);
			memcpy(m_data,buff,k);
		}
		
	}

	//内存字节数
	size_t size()
	{
		return ARRAY_SIZE(m_data);
	}

	void zero()
	{
		MEM_ZERO(this);
	}

	void set(const void * buff,int len,int start=0)
	{
		int k = (ARRAY_SIZE(m_data)-start) > len ? len : (ARRAY_SIZE(m_data)-start);

		if(buff && len > 0 && k>0)
		{			
			memcpy(m_data+start,buff,k);
		}
	}

	bool get (int index)
	{
		if(index>=ARRAY_SIZE(m_data)*8)
		{
			return false;
		}

		static unsigned char s_get_mask[] =
		{
			0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
		};

		return m_data[index/8] & s_get_mask[index%8];
	}

	void set(int index ,bool value)
	{
		if(index>=ARRAY_SIZE(m_data)*8)
		{
			return ;
		}

		//设置
		static unsigned char s_set_mask[] =
		{
			0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
		};

		//清除
		static unsigned char s_clr_mask[] =
		{
			0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe
		};

		if(value)
		{
			 m_data[index/8] |= s_set_mask[index%8];
		}
		else
		{
			 m_data[index/8] &= s_clr_mask[index%8];
		}

	}
};

//二进制
template <int size> struct TBinArray : public TArray<unsigned char,size>{};

//整型数组
template <int size> struct TInt8Array : public TArray<char,size> { TInt8Array(){memset(this,0,sizeof(*this));}};
template <int size> struct TUint8Array : public TArray<unsigned char,size> {};

template <int size> struct TInt16Array : public TArray<short,size> {};
template <int size> struct TUint16Array : public TArray<unsigned short,size> {};

template <int size> struct TInt32Array : public TArray<int,size> {};
template <int size> struct TUint32Array : public TArray<unsigned int,size> {};

template <int size> struct TLongArray : public TArray<long,size> {};
template <int size> struct TUlongArray : public TArray<unsigned long,size> {};

template <int size> struct TFloatArray : public TArray<float,size> {};
template <int size> struct TDoubleArray : public TArray<double,size> {};

template <int size> struct TInt64Array : public TArray<long long,size> {};
template <int size> struct TUint64Array : public TArray<unsigned long long,size> {};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ARRAY(type,size) TArray<type,size>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//整数转化为类型，方便函数重载
template <size_t size> struct Int2Type
{
	enum{value = size};
};

//定义一个无名对象
#define INT2TYPE(size) Int2Type<(size)>()


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//定义求结构体字段数的函数
/*#define FIELD_SIZE(index) static Int2Type<index> GetFieldSize(Int2Type<index>); 


//求结构体大小
template<int num>
struct TObjSizeTest 
{	
	typedef struct{char a[2];} TCharTwo;
	static char test(...);
	static TCharTwo test(Int2Type<num>);
};


template<typename type,int num=1
,bool  = (sizeof(TObjSizeTest<num>::test(type::GetFieldSize(INT2TYPE(num))))==sizeof(TObjSizeTest <num>::TCharTwo)) >
struct TObjSize 
{	
	enum {size = TObjSize<type,num+1>::size};
};

template<typename type,int num>
struct TObjSize<type,num,false>
{
	enum {size = num-1};
};
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
//以该宏开始，后面紧跟各个字段的定义，字段间不能有空行，且每个字段独占一行
#define FIELD_BEGIN() enum{ BEGIN_LINE = __LINE__};  static Int2Type<0> GetFieldSize(...); static enParamType GetIntOutType(...) { return enParamType_Max;}

//字义一个字段

#define FIELD(data_type,var_name) 	\
	data_type var_name;            \
	data_type & GetValue(Int2Type<__LINE__ - BEGIN_LINE>) { return var_name;} \
	const data_type & GetValue(Int2Type<__LINE__ - BEGIN_LINE>) const { return var_name;} \
	const char * GetName(Int2Type<__LINE__ - BEGIN_LINE>) const {return #var_name;} \
	FIELD_SIZE(__LINE__ - BEGIN_LINE)*/

//以该宏开始，后面紧跟各个字段的定义，字段间不能有空行，且每个字段独占一行
#define FIELD_BEGIN() enum{ BEGIN_LINE = __COUNTER__};    

#define FIELD_END()   enum{ size = __COUNTER__ - BEGIN_LINE -1 };

#define FIELD_INDEX(data_type,var_name,index) 	\
	data_type var_name;            \
	const char * GetName(Int2Type<index>) const {return #var_name;}     \
	data_type & GetValue(Int2Type<index>) { return var_name;}           \
	const data_type & GetValue(Int2Type<index>) const { return var_name;} 
	


#define FIELD(data_type,var_name) 	FIELD_INDEX(data_type,var_name,__COUNTER__ - BEGIN_LINE)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//数据库绑定参数输入输出类型
enum enParamType
{
	enParamType_Int = 1,
	enParamType_IntOut = 2,
	enParamType_Out = 4,
	enParamType_Max,
};

//空输出参数
struct OutParamNull
{
	FIELD_BEGIN();	
	OutParamNull GetValue(...) {return OutParamNull();}
	FIELD_END();
};

//空结果集
struct ResultSetNull
{
	FIELD_BEGIN();
	FIELD_END();
};


//定义结果集
#define TRESULT_SET(Set1,Set2) typedef Set1 TResultSet; typedef Set2 TResultSet2;


//绑定参数开始

//以该宏开始，后面紧跟各个字段的定义，字段间不能有空行，且每个字段独占一行,proc_name为存储过程名
#define BIND_PROC_BEGIN( proc_name) FIELD_BEGIN() \
	static const char* GetProcName(){ return proc_name; } \
	bool IsStoredProc(){return true;}                      \
	OutParamNull GetValue(...) {return OutParamNull();}     \
	static enParamType GetIntOutType(...) { return enParamType_Max;}



//以该宏开始，后面紧跟各个字段的定义，字段间不能有空行，且每个字段独占一行,sql_stmt为SQL语句
#define BIND_SQL_BEGIN( sql_stmt) FIELD_BEGIN() \
	static const char* GetProcName(){ return sql_stmt; } \
	bool IsStoredProc(){return false;}                    \
   OutParamNull GetValue(...) {return OutParamNull();}     \
   static enParamType GetIntOutType(...) { return enParamType_Max;}


//数据库绑定参数定义的宏
#define BIND_PARAM(data_type,var_name,IntOutType) FIELD(data_type,var_name) static enParamType GetIntOutType(Int2Type<__COUNTER__ - BEGIN_LINE>) { return IntOutType;}

//绑定输入参数
#define BIND_PARAM_IN(data_type,var_name) BIND_PARAM(data_type,var_name,enParamType_Int)

//绑定输出参数
#define BIND_PARAM_OUT(data_type,var_name) BIND_PARAM(data_type,var_name,enParamType_Out)

//绑定输入输出参数
#define BIND_PARAM_INOUT(data_type,var_name) BIND_PARAM(data_type,var_name,enParamType_IntOut)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//遍历结构体
template<int begin=1, bool bEnd=false>
struct TForEach
{
	template<typename ValueType,typename TVistorType>
	static bool Call( ValueType & value,const TVistorType & visitor)
	{		
		if(visitor.operator () (INT2TYPE(begin),value) == false)
		{
			return false;
		}

		return TForEach<begin+1,(begin+1 > ValueType::size)>::Call(value,visitor);
	}


	template<typename ValueType,typename TVistorType,typename ActType>
	static bool Call( ValueType & value,const TVistorType & visitor, ActType & act )
	{		
		if(visitor.operator () (INT2TYPE(begin),value,act) == false)
		{
			return false;
		}

		return TForEach<begin+1,(begin+1 > ValueType::size)>::Call(value,visitor,act);
	}

	template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2>
	static bool Call( ValueType & value,const TVistorType & visitor, ActType1 & act1 ,ActType2 & act2  )
	{		
		if(visitor.operator () (INT2TYPE(begin),value,act1,act2) == false)
		{
			return false;
		}

		return TForEach<begin+1,(begin+1 > ValueType::size)>::Call(value,visitor,act1,act2);
	}

	template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2,typename ActType3>
	static bool Call( ValueType & value,const TVistorType & visitor , ActType1 & act1 
		,ActType2 & act2 ,ActType3 & act3  )
	{		
		if(visitor.operator () (INT2TYPE(begin),value,act1,act2,act3) == false)
		{
			return false;
		}

		return TForEach<begin+1,(begin+1 > ValueType::size)>::Call(value,visitor,act1,act2,act3);
	}
};

template<int begin>
struct TForEach<begin,true>
{
	template<typename ValueType,typename TVistorType>
	static bool Call( ValueType & value, const TVistorType & visitor)
	{
		return true;
	}


	template<typename ValueType,typename TVistorType,typename ActType>
	static bool Call( ValueType & value, const TVistorType & visitor,ActType & act)
	{
		return true;
	}

	template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2>
	static bool Call( ValueType & value, const TVistorType & visitor,ActType1 & act1 ,ActType2 & act2 )
	{
		return true;
	}

	template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2,typename ActType3>
	static bool Call( ValueType & value, const TVistorType & visitor,ActType1 & act1 ,
		ActType2 & act2,ActType3 & act3)
	{
		return true;
	}
};

//定义辅助函数
template<typename ValueType,typename TVistorType>
bool ForEach(ValueType & value,const TVistorType & vistor)
{
	return TForEach<>::Call(value,vistor);
}

template<typename ValueType,typename TVistorType,typename ActType>
bool ForEach(ValueType & value,const TVistorType & vistor, ActType & act)
{
	return TForEach<>::Call(value,vistor,act);
}

template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2>
bool ForEach(ValueType & value,const TVistorType & vistor, ActType1 & act1, ActType2 & act2 )
{
	return TForEach<>::Call(value,vistor,act1,act2);
}

template<typename ValueType,typename TVistorType,typename ActType1,typename ActType2,typename ActType3>
bool ForEach(ValueType & value,const TVistorType & vistor, ActType1 & act1
			 , ActType2 & act2 
			 , ActType3 & act3)
{
	return TForEach<>::Call(value,vistor,act1,act2,act3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//为各种类型定义一个值
enum enTypeValue
{
	enTypeValue_INT8 = 0,
	enTypeValue_UINT8 ,
	enTypeValue_INT16 ,
	enTypeValue_UINT16 ,
	enTypeValue_INT32 ,
	enTypeValue_UINT32 ,
	enTypeValue_INT64 ,
	enTypeValue_UINT64 ,
	enTypeValue_LONG ,
	enTypeValue_ULONG ,
	enTypeValue_FLOAT ,
	enTypeValue_DOUBLE ,
	enTypeValue_VECTOR,
	enTypeValue_STRING,
	enTypeValue_ARRAY,
	enTypeValue_BINARY,
	enTypeValue_BOOL,
	enTypeValue_VARBINARY,
	enTypeValue_MAX,
};

//判断类型是为是基本类型
template<typename type> struct Type2Int { enum {value = enTypeValue_MAX}; };

//特化
template<> struct Type2Int<bool> { enum {value = enTypeValue_BOOL}; };
template<> struct Type2Int<char> { enum {value = enTypeValue_INT8}; };
template<> struct Type2Int<unsigned char> { enum {value = enTypeValue_UINT8}; };
template<> struct Type2Int<short> { enum {value = enTypeValue_INT16}; };
template<> struct Type2Int<unsigned short> { enum {value = enTypeValue_UINT16}; };
template<> struct Type2Int<int> { enum {value = enTypeValue_INT32}; };
template<> struct Type2Int<unsigned int> { enum {value = enTypeValue_UINT32}; };
template<> struct Type2Int<long> { enum {value = enTypeValue_LONG}; };
template<> struct Type2Int<unsigned long> { enum {value = enTypeValue_ULONG}; };
template<> struct Type2Int<float> { enum {value = enTypeValue_FLOAT}; };
template<> struct Type2Int<double> { enum {value = enTypeValue_DOUBLE}; };
template<typename type> struct Type2Int<std::vector<type> > { enum {value = enTypeValue_VECTOR}; };
template<> struct Type2Int<std::string> { enum {value = enTypeValue_STRING}; };
template<typename type,int size> struct Type2Int< TArray<type,size> > { enum {value = enTypeValue_ARRAY}; }; 
template<int size> struct Type2Int<TBinArray<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TInt8Array<size> >  { enum {value = enTypeValue_ARRAY}; };
template<int size> struct Type2Int<TUint8Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TInt16Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TUint16Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TInt32Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TUint32Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TInt64Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TUint64Array<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TLongArray<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TUlongArray<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TFloatArray<size> >  { enum {value = enTypeValue_BINARY}; };
template<int size> struct Type2Int<TDoubleArray<size> >  { enum {value = enTypeValue_BINARY}; };
template<> struct Type2Int<VarBinary >  { enum {value = enTypeValue_VARBINARY}; };
template<> struct Type2Int<void* >  { enum {value = enTypeValue_BINARY}; };
 template<> struct Type2Int<long long> { enum {value = enTypeValue_INT64}; };
 template<> struct Type2Int<unsigned long long> { enum {value = enTypeValue_UINT64}; };

 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //定义两个布尔值
 struct TBoolTrue{};
 struct TBoolFalse{};

 template<bool value = true> struct TBOOL
 {
    typedef TBoolTrue Type;
 };

 template<> struct TBOOL<false>
 {
    typedef TBoolFalse Type;
 };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //类型分类
 struct TypeCategoryKnow{};  //已知的类型
 struct TypeCategoryBase : public TypeCategoryKnow {};  //基本类型
 struct TypeCategoryArray : public TypeCategoryKnow{}; //数组
 struct TypeCategoryString : public TypeCategoryKnow{};
 struct TypeCategoryVector : public TypeCategoryKnow{};
 struct TypeCategoryBinArray : public TypeCategoryKnow{};
 struct TypeCategoryVarBinary : public TypeCategoryKnow{};
 struct TypeCategoryUnknow{};

 template<int size> struct TTypeCategory { typedef TypeCategoryUnknow TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_BOOL> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_INT8> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_UINT8> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_INT16> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_UINT16> { typedef TypeCategoryBase TCategory;}; 
  template<> struct TTypeCategory<enTypeValue_INT32> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_UINT32> { typedef TypeCategoryBase TCategory;}; 
  template<> struct TTypeCategory<enTypeValue_LONG> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_ULONG> { typedef TypeCategoryBase TCategory;}; 
  template<> struct TTypeCategory<enTypeValue_INT64> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_UINT64> { typedef TypeCategoryBase TCategory;}; 
  template<> struct TTypeCategory<enTypeValue_FLOAT> { typedef TypeCategoryBase TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_DOUBLE> { typedef TypeCategoryBase TCategory;}; 

 template<> struct TTypeCategory<enTypeValue_VECTOR> { typedef TypeCategoryVector TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_STRING> { typedef TypeCategoryString TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_ARRAY> { typedef TypeCategoryArray TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_BINARY> { typedef TypeCategoryBinArray TCategory;}; 
 template<> struct TTypeCategory<enTypeValue_VARBINARY> { typedef TypeCategoryVarBinary TCategory;}; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//使用举例，Trace 结构体

struct TTRaceVistor
{
	template<size_t index,typename type>
	bool operator ()(Int2Type<index>,type & value,const char * szBeginText="{",const char* szEndText="}\n") const
	{
		if(index==1)
		{
			if(szBeginText) { std::cout << szBeginText;}
		}

		std::cout << value.GetName(INT2TYPE(index)) << "= " ;
		
		Trace(std::cout,value.GetValue(INT2TYPE(index)));

		std::cout << "  ";

		if(index==type::size) //结束
		{
			if(szEndText)
			{
				std::cout << szEndText;
			}
		}

		return true;
	}

private:

	template<typename type> void Trace(std::ostream & os,type & value) const
	{
		Trace(os,value,TTypeCategory<Type2Int<type>::value>::TCategory ());
	}
	//type为结构体
	template<typename type> void Trace(std::ostream & os,type & value,TypeCategoryUnknow) const
	{
		ForEach(value,TTRaceVistor(), "{","}");
	}

	//基本类型
	template<typename type> void Trace(std::ostream & os,type & value,TypeCategoryBase) const
	{
		os << value;
	}

	//向量特化
	template<typename type> void Trace(std::ostream & os, std::vector<type> & value,TypeCategoryVector) const
	{
		os << "<";
		for(int i=0; i<value.size();++i)
		{
			Trace(os ,value[i]);
			os << ",";
		}

		os << ">";
	}

	//数组特化
	template<typename type,int size> void Trace(std::ostream & os, TArray<type,size> & value,TypeCategoryArray) const
	{
		os << "<";
		for( unsigned  int i=0; i<size;++i)
		{
			Trace(os ,value[i]);
			os << ",";
		}

		os << ">";
	}

	//字符串
	template<int size> void Trace(std::ostream & os, TArray<char,size> & value,TypeCategoryArray) const
	{
		
		for(int i=0; i<size;++i)
		{			
			os << value[i];
		}
		
	}

	//字符串
	 void Trace(std::ostream & os, std::string & value,TypeCategoryString) const
	{
		os << value;			
	}

	//二进制
	template<int size> void Trace(std::ostream & os, TBinArray<size> & value,TypeCategoryBinArray) const
	{
		os << "<";
		char szTemp[20]={0};
		for(int i=0; i<size;++i)
		{
			sprintf(szTemp,"%02x ",value[i]);
			os << szTemp;			
		}
		os << ">";
	}
};


#define TRACE_OBJ(value) { char szTemp[300] = {0}; sprintf(szTemp,"%s(%d):%s\n%s={",__FILE__,__LINE__,__FUNCTION__, #value); ForEach(value,TTRaceVistor(), szTemp,"}\n"); }

/*
struct TestObj
{
	FIELD_BEGIN();
	FIELD(int ,m_i);
	FIELD(char,m_c);
};

TestObj obj;

obj.m_i = 1;
obj.m_c = 'A';

TRACE_OBJ(obj);


*/

#endif


