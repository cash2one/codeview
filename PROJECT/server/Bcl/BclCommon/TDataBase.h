
#ifndef __BCL_TDATABASE_H__
#define __BCL_TDATABASE_H__

#include "IDataBase.h"
#include <crtdbg.h>




class TDataBase
{
public:
	TDataBase(IDataBase *pDataBase)
	{
		m_pDataBase = pDataBase;
		m_StmtHandle = NULL;
	}

	~TDataBase()
	{
		FreeStmtHandle();
		m_pDataBase = NULL;
	}

	void SetDataBase(IDataBase *pDataBase)
	{
		FreeStmtHandle();
		m_pDataBase = pDataBase;	   
	}

	IDataBase * SetDataBase()
	{
		return m_pDataBase;	  
	}

	//准备sql语句
	bool Prepare(const char* szSql,int len=0)
	{
		if(m_pDataBase==NULL || szSql==NULL)
		{
			return false;
		}
		if(m_StmtHandle != NULL)
		{
			if(FreeStmtHandle()==false)
			{
				return false;
			}
		}

		m_StmtHandle = m_pDataBase->Prepare(szSql,len);

		return (m_StmtHandle != NULL);
	}

	//绑定参数
	template<typename ParamType>
	bool BindParam(int index,ParamType & Param,enParamType IntOutType=enParamType_Int,int  len = sizeof(ParamType))
	{				
		return BindParam_i(index,Param,IntOutType,len,TTypeCategory<Type2Int<ParamType>::value>::TCategory ());
	}

	bool BindParam(int index,VarBinary & Param,enParamType IntOutType=enParamType_Int,int  len = 0)
	{
		len = Param.size();
		void * pBuff = Param.buffer();
		return BindParam_i(index,pBuff,IntOutType,len,TypeCategoryBinArray());
	}

	//空参数，返回成功
	bool BindParam(int index,OutParamNull & Param,enParamType IntOutType=enParamType_Int,int len = sizeof(OutParamNull))
	{				
		return true;
	}


	template<typename ParamType>
	bool BindParamN(int index,ParamType & Param,enParamType IntOutType=enParamType_Int)
	{
		if(BindParam(index,Param,IntOutType)==false) { return false; }

		return true;
	}

	template<typename ParamType1,typename ParamType2>
	bool BindParamN(int index,ParamType1 & Param1,ParamType2 & Param2,enParamType IntOutType=enParamType_Int)
	{
		if(BindParam(index++,Param1,IntOutType)==false) { return false; }

		if(BindParam(index++,Param2,IntOutType)==false) { return false; }

		return true;
	}

	template<typename ParamType1,typename ParamType2,typename ParamType3>
	bool BindParamN(int index,ParamType1 & Param1,ParamType2 & Param2,
		ParamType3 & Param3,enParamType IntOutType=enParamType_Int)
	{
		if(BindParam(index++,Param1,IntOutType)==false)	 { return false;}

		if(BindParam(index++,Param2,IntOutType)==false) { return false; }

		if(BindParam(index++,Param3,IntOutType)==false)	 { return false; }

		return true;
	}
	template<typename ParamType1,typename ParamType2,typename ParamType3,typename ParamType4>
	bool BindParamN(int index,ParamType1 & Param1,ParamType2 & Param2,
		ParamType3 & Param3,ParamType4 & Param4,enParamType IntOutType=enParamType_Int)
	{
		if(BindParam(index++,Param1,IntOutType)==false)	 { return false;}

		if(BindParam(index++,Param2,IntOutType)==false) { return false; }

		if(BindParam(index++,Param3,IntOutType)==false)	 { return false; }

		if(BindParam(index++,Param4,IntOutType)==false){ return false; }

		return true;
	}

	template<typename ParamType1,typename ParamType2,typename ParamType3,typename ParamType4,typename ParamType5>
	bool BindParamN(int index,ParamType1 & Param1,ParamType2 & Param2,ParamType3 & Param3,
		ParamType4 & Param4,ParamType5 & Param5,enParamType IntOutType=enParamType_Int)
	{
		if(BindParam(index++,Param1,IntOutType)==false)	 { return false;}

		if(BindParam(index++,Param2,IntOutType)==false) { return false; }

		if(BindParam(index++,Param3,IntOutType)==false)	 { return false; }

		if(BindParam(index++,Param4,IntOutType)==false){ return false; }

		if(BindParam(index++,Param5,IntOutType)==false) { return false; }

		return true;
	}


	//执行
	bool Execute()
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}
		return m_pDataBase->Execute(m_StmtHandle);
	}

	//绑定列
	template<typename BindType>
	bool BindCol(int index,BindType & BindValue,int  len = sizeof(BindType))
	{
		return BindCol_i(index,BindValue,len,TTypeCategory<Type2Int<BindType>::value>::TCategory ());
	}

	//空结果集
	bool BindCol(int index,ResultSetNull & BindValue,int  len = sizeof(ResultSetNull))
	{
		return true;
	}

	template<typename BindType>
	bool BindCol(int index,BindType & BindValue,int & outLen,int len = sizeof(BindType))
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}

		outLen = len;

		return m_pDataBase->BindCol(m_StmtHandle,index,(enTypeValue)Type2Int<BindType>::value,&BindValue,outLen);
	}

	template<typename BindType>
	bool BindColN(BindType & BindValue)
	{
		if(BindCol(1,BindValue)==false) return false;
		return true;
	}

	template<typename BindType1,typename BindType2>
	bool BindColN(BindType1 & BindValue,BindType2 & BindValue2)
	{
		if(BindCol(1,BindValue)==false) return false;
		if(BindCol(2,BindValue2)==false) return false;
		return true;
	}

	template<typename BindType1,typename BindType2,typename BindType3>
	bool BindColN(BindType1 & BindValue,BindType2 & BindValue2,BindType3 & BindValue3)
	{
		if(BindCol(1,BindValue)==false) return false;
		if(BindCol(2,BindValue2)==false) return false;
		if(BindCol(3,BindValue3)==false) return false;
		return true;
	}


	template<typename BindType1,typename BindType2,typename BindType3,typename BindType4>
	bool BindColN(BindType1 & BindValue,BindType2 & BindValue2,BindType3 & BindValue3,BindType4 & BindValue4)
	{
		if(BindCol(1,BindValue)==false) return false;
		if(BindCol(2,BindValue2)==false) return false;
		if(BindCol(3,BindValue3)==false) return false;
		if(BindCol(4,BindValue4)==false) return false;
		return true;
	}

	template<typename BindType1,typename BindType2,typename BindType3,typename BindType4,typename BindType5>
	bool BindColN(BindType1 & BindValue,BindType2 & BindValue2,BindType3 & BindValue3,BindType4 & BindValue4,BindType5 & BindValue5)
	{
		if(BindCol(1,BindValue)==false) return false;
		if(BindCol(2,BindValue2)==false) return false;
		if(BindCol(3,BindValue3)==false) return false;
		if(BindCol(4,BindValue4)==false) return false;
		if(BindCol(5,BindValue5)==false) return false;
		return true;
	}


	//获取结果集
	bool Fetch()
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}
		return m_pDataBase->Fetch(m_StmtHandle);
	}

	//获取影响行数
	int GetRowNum()
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return -1;
		}
		return m_pDataBase->GetRowNum(m_StmtHandle);
	}


	bool MoreResults(){ return m_pDataBase->MoreResults(m_StmtHandle);}

	//获取错误信息
	int GetErrorInfo(std::string & strError)
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return -1;
		}
		return m_pDataBase->GetErrorInfo(m_StmtHandle,strError);
	}

	bool Execute(const char* szSql)
	{
		if(szSql==NULL)
		{
			return false;
		}

		if(m_StmtHandle != NULL)
		{
			if(FreeStmtHandle()==false)
			{
				return false;
			}
		}

		m_StmtHandle = m_pDataBase->ExecuteDirect(szSql);

		return (m_StmtHandle != NULL);
	}

	template<typename ParamType>
	bool Execute(const char* szSql,ParamType & Param,enParamType IntOutType=enParamType_Int)
	{
		if(Prepare(szSql)==false)
		{
			return false;
		}
		if(BindParamN(1,Param,IntOutType)==false)
		{
			return false;
		}

		return Execute();
	}

	template<typename ParamType,typename ParamType2>
	bool Execute(const char* szSql,ParamType & Param,ParamType2 & Param2,enParamType IntOutType=enParamType_Int)
	{
		if(Prepare(szSql)==false)
		{
			return false;
		}
		if(BindParamN(1,Param,Param2,IntOutType)==false)
		{
			return false;
		}

		return Execute();
	}

	template<typename ParamType,typename ParamType2,typename ParamType3>
	bool Execute(const char* szSql,ParamType & Param,ParamType2 & Param2,ParamType3 & Param3,enParamType IntOutType=enParamType_Int)
	{
		if(Prepare(szSql)==false)
		{
			return false;
		}
		if(BindParamN(1,Param,Param2,Param3,IntOutType)==false)
		{
			return false;
		}

		return Execute();
	}

	template<typename ParamType,typename ParamType2,typename ParamType3,typename ParamType4>
	bool Execute(const char* szSql,ParamType & Param,ParamType2 & Param2,ParamType3 & Param3,
		ParamType4 & Param4,enParamType IntOutType=enParamType_Int)
	{
		if(Prepare(szSql)==false)
		{
			return false;
		}
		if(BindParamN(1,Param,Param2,Param3,Param4,IntOutType)==false)
		{
			return false;
		}

		return Execute();
	}

	template<typename ParamType,typename ParamType2,typename ParamType3,typename ParamType4,typename ParamType5>
	bool Execute(const char* szSql,ParamType & Param,ParamType2 & Param2,ParamType3 & Param3,
		ParamType4 & Param4,ParamType5 & Param5,enParamType IntOutType=enParamType_Int)
	{
		if(Prepare(szSql)==false)
		{
			return false;
		}
		if(BindParamN(1,Param,Param2,Param3,Param4,Param5,IntOutType)==false)
		{
			return false;
		}

		return Execute();
	}

	//获取结果
	template<typename BindType>
	int Fetch(BindType * pBindValue,int size=1)
	{
		int count = 0;
		if(pBindValue==NULL) return -1;

		if(BindColN(*pBindValue)==false) return -2;

		if(Fetch()==false)
		{
			return -3;
		}
		count++;

		if(size>1)
		{
			BindType data = *pBindValue;
			for(count=1; count<size;count++)
			{
				if(Fetch()==false)
				{
					break;
				}

				pBindValue[count] = *pBindValue;
			}

			if(count >1)
			{
				*pBindValue = data;
			}
		}

		return count;
	}

	template<typename BindType,typename BindType2>
	int Fetch(BindType * pBindValue,BindType2 * pBindValue2,int size=1)
	{
		int count = 0;
		if(pBindValue==NULL || pBindValue2==NULL) return -1;

		if(BindColN(*pBindValue,*pBindValue2)==false) return -2;

		if(Fetch()==false)
		{
			return -3;
		}

		count=1;

		if(size>1)
		{

			BindType data = *pBindValue;
			BindType2 data2 = *pBindValue2;

			for(; count<size;count++)
			{
				if(Fetch()==false)
				{
					break;
				}

				pBindValue[count] = *pBindValue;
				pBindValue2[count] = *pBindValue2;
			}

			if(count >1)
			{
				*pBindValue = data;
				*pBindValue2 = data2;
			}
		}

		return count;
	}

	template<typename BindType,typename BindType2,typename BindType3>
	int Fetch(BindType * pBindValue,BindType2 * pBindValue2,BindType3 * pBindValue3,int size=1)
	{
		int count = 0;
		if(pBindValue==NULL || pBindValue2==NULL || pBindValue3==NULL) return -1;

		if(BindColN(*pBindValue,*pBindValue2,*pBindValue3)==false) return -2;

		if(Fetch()==false)
		{
			return -3;
		}
		count++;

		if(size>1)
		{

			BindType data = *pBindValue;
			BindType2 data2 = *pBindValue2;
			BindType3 data3 = *pBindValue3;

			for(count=1; count<size;count++)
			{
				if(Fetch()==false)
				{
					break;
				}

				pBindValue[count] = *pBindValue;
				pBindValue2[count] = *pBindValue2;
				pBindValue3[count] = *pBindValue3;
			}

			if(count >1)
			{
				*pBindValue = data;
				*pBindValue2 = data2;
				*pBindValue3 = data3;
			}
		}

		return count;
	}


	template<typename BindType,typename BindType2,typename BindType3,typename BindType4>
	int Fetch(BindType * pBindValue,BindType2 * pBindValue2,BindType3 * pBindValue3,BindType4 * pBindValue4,int size=1)
	{
		int count = 0;
		if(pBindValue==NULL || pBindValue2==NULL || pBindValue3==NULL|| pBindValue4==NULL) return -1;

		if(BindColN(*pBindValue,*pBindValue2,*pBindValue3,*pBindValue4)==false) return -2;

		if(Fetch()==false)
		{
			return -3;
		}
		count++;

		if(size>1)
		{

			BindType data = *pBindValue;
			BindType2 data2 = *pBindValue2;
			BindType3 data3 = *pBindValue3;
			BindType4 data4 = *pBindValue4;

			for(count=1; count<size;count++)
			{
				if(Fetch()==false)
				{
					break;
				}

				pBindValue[count] = *pBindValue;
				pBindValue2[count] = *pBindValue2;
				pBindValue3[count] = *pBindValue3;
				pBindValue4[count] = *pBindValue4;
			}

			if(count >1)
			{
				*pBindValue = data;
				*pBindValue2 = data2;
				*pBindValue3 = data3;
				*pBindValue4 = data4;
			}
		}

		return count;
	}


	template<typename BindType,typename BindType2,typename BindType3,typename BindType4,typename BindType5>
	int Fetch(BindType * pBindValue,BindType2 * pBindValue2,BindType3 * pBindValue3,BindType4 * pBindValue4,BindType5 * pBindValue5,int size=1)
	{
		int count = 0;
		if(pBindValue==NULL || pBindValue2==NULL || pBindValue3==NULL|| pBindValue4==NULL|| pBindValue5==NULL) return -1;

		if(BindColN(*pBindValue,*pBindValue2,*pBindValue3,*pBindValue4,*pBindValue5)==false) return -2;

		if(Fetch()==false)
		{
			return -3;
		}
		count++;

		if(size>1)
		{

			BindType data = *pBindValue;
			BindType2 data2 = *pBindValue2;
			BindType3 data3 = *pBindValue3;
			BindType4 data4 = *pBindValue4;
			BindType5 data5 = *pBindValue5;

			for(count=1; count<size;count++)
			{
				if(Fetch()==false)
				{
					break;
				}

				pBindValue[count] = *pBindValue;
				pBindValue2[count] = *pBindValue2;
				pBindValue3[count] = *pBindValue3;
				pBindValue4[count] = *pBindValue4;
				pBindValue5[count] = *pBindValue5;
			}

			if(count >1)
			{
				*pBindValue = data;
				*pBindValue2 = data2;
				*pBindValue3 = data3;
				*pBindValue4 = data4;
				*pBindValue5 = data5;
			}
		}

		return count;
	}

	bool FreeStmtHandle()
	{
		if(m_pDataBase && m_StmtHandle)
		{
			if(m_pDataBase->FreeStmtHandle(m_StmtHandle)==false)
			{
				return false;
			}
			m_StmtHandle = NULL;
		}

		return true;
	}

	bool FreeResults()
	{
		return m_pDataBase->FreeResults(m_StmtHandle);
	}

	//多结果集
	template<typename InParamType,typename ResultSetType1,typename ResultSetType2>
	bool ExecuteMultiResult( InParamType & DbReq,ResultSetType1 * & pResultSet1,int & len1,ResultSetType2 * & pResultSet2,int & len2)
	{
		if(DbReq.IsStoredProc())
		{
			std::ostringstream ost;

			ost << "CALL " << InParamType::GetProcName() << "(";

			for(int i=0; i< InParamType::size; i++)
		 {
			 if(i>0)
				 ost << ",?";
			 else
				 ost << "?";			
			}

			ost << ")";

			if(Prepare(ost.str().c_str())==false)
			{
				return false;
			}
		}
		else
		{
			if(Prepare(InParamType::GetProcName())==false)
			{
				return false;
			}
		}

		if(BindParam(1,DbReq)==false)
		{
			return false;
		}


		if(Execute()==false)
		{
			return false;
		}

		pResultSet1 = NULL;
		len1 = 0;

		bool bMoveResults = false;

		if(ResultSetType1::size>0)
		{
			int count = GetRowNum();


			if(count>0)
			{
				pResultSet1 = new ResultSetType1[count];

				if((len1=Fetch(pResultSet1,count))<=0)
				{
					delete [] pResultSet1;
					pResultSet1 = NULL;
					if(len1<0)
					{
						len1 = 0;
						return false;
					}		
				}

				bMoveResults = true;
			}


		}

		if(ResultSetType2::size>0)
		{

			if(bMoveResults && m_pDataBase->MoreResults(m_StmtHandle)==false)
			{
				delete [] pResultSet1;
				pResultSet1 = NULL;
				len1 = 0;
				return false;
			}

			int count = GetRowNum();


			if(count>0)
			{
				pResultSet2 = new ResultSetType2[count];

				if((len2=Fetch(pResultSet2,count))<=0)
				{
					delete [] pResultSet2;
					pResultSet2 = NULL;

					if(len2<0)
					{
						len2 = 0;

						delete [] pResultSet1;
						pResultSet1 = NULL;
						len1 = 0;

						return false;
					}		
				}

			}

		}

		if(pResultSet1 || pResultSet2)
		{
			//关闭结果集，防止内存泄漏
			FreeResults();
		}

		return true;
	}



	template <typename InParamType,typename OutParamType,typename ResultSetType>
	bool Execute( InParamType & DbReq,OutParamType & OutParam ,ResultSetType * & pResultSet,int & len)
	{
		std::ostringstream ost;

		ost << "CALL " << InParamType::GetProcName() << "(";

		for(int i=0; i< InParamType::size + OutParamType::size; i++)
		{
			if(i>0)
				ost << ",?";
			else
				ost << "?";			
		}

		ost << ")";

		if(Prepare(ost.str().c_str())==false)
		{
			return false;
		}

		if(BindParam(1,DbReq)==false)
		{
			return false;
		}

		if(BindParam(1+InParamType::size,OutParam,enParamType_Out)==false)
		{
			return false;
		}

		if(Execute()==false)
		{
			return false;
		}

		pResultSet = NULL;
		len = 0;

		bool bMoveResultSet = false; //是否需要移动结果集

		if(ResultSetType::size>0)
		{

			int count = GetRowNum();

			if(count>0)
			{
				pResultSet = new ResultSetType[count];
			}

			if((len=Fetch(pResultSet,count))<=0)
			{
				delete [] pResultSet;
				pResultSet = NULL;
				if(len<0)
				{
					len = 0;
					return false;
				}		
			}

			bMoveResultSet = true;
		}

		if(OutParamType::size>0)
		{
			if(bMoveResultSet)
			{
				if(m_pDataBase->MoreResults(m_StmtHandle)==false)
				{
					delete [] pResultSet;
					pResultSet = NULL;
					len = 0;
					return false;
				}
			}

			if(Fetch(&OutParam)<0)
			{
				delete [] pResultSet;
				pResultSet = NULL;
				len = 0;
				return false;		
			}
		}

		return true;
	}

	template <typename InParamType,typename OutParamType>
	bool Execute( InParamType & DbReq,OutParamType & OutParam)
	{
		ResultSetNull * pResultNull = NULL;

		int len = 0;

		if(Execute(DbReq,OutParam,pResultNull,len)==false)
		{
			return false;
		}

		if(pResultNull!=NULL)
		{
			delete [] pResultNull;
		}

		return true;
	}

	template <typename InParamType>
	bool Execute( InParamType & DbReq)
	{
		OutParamNull OutParam;

		ResultSetNull * pResultNull = NULL;

		int len = 0;

		if(Execute(DbReq,OutParam,pResultNull,len)==false)
		{
			return false;
		}

		if(pResultNull!=NULL)
		{
			delete [] pResultNull;
		}

		return true;
	}

private:

	struct TBindParam
	{	
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value,TDataBase * pDbObj,int begin,enParamType IntOutType) const
		{			
			enParamType type = T::GetIntOutType(INT2TYPE(index));
			return pDbObj->BindParam(begin+index-1,value.GetValue(INT2TYPE(index)),(type==enParamType_Max) ? IntOutType : type );
		}
	};

	template<typename ParamType>
	bool BindParam_i(int index,ParamType & Param,enParamType IntOutType=enParamType_Int,int len = sizeof(ParamType),TypeCategoryKnow=TypeCategoryKnow())
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}
		static int s_len =0;
		s_len = len;
		return m_pDataBase->BindParam(m_StmtHandle,index,IntOutType,(enTypeValue)Type2Int<ParamType>::value,&Param,s_len);
	}
	bool BindParam_i(int index,void* & Param,enParamType IntOutType=enParamType_Int,int len = 0,TypeCategoryKnow=TypeCategoryKnow())
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}
		static int s_len =0;
		s_len = len;
		return m_pDataBase->BindParam(m_StmtHandle,index,IntOutType,enTypeValue_VARBINARY,Param,s_len);
	}

	//特化，参数为结构体
	template<typename ParamType>
	bool BindParam_i(int index,ParamType & Param,enParamType IntOutType=enParamType_Int,int len = sizeof(ParamType),TypeCategoryUnknow=TypeCategoryUnknow())
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}

		return ForEach(Param,TBindParam(),this,index,IntOutType);
	}



	struct TBindCol
	{	
		template<size_t index,typename T> bool  operator () (Int2Type<index>,T &value,TDataBase * pDbObj,int begin) const
		{				
			return pDbObj->BindCol(begin+index-1,value.GetValue(INT2TYPE(index)));
		}
	};

	template<typename BindType>
	bool BindCol_i(int index,BindType & BindValue,int  len = sizeof(BindType),TypeCategoryUnknow=TypeCategoryUnknow())
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}

		return ForEach(BindValue,TBindCol(),this,index);
	}

	template<typename BindType>
	bool BindCol_i(int index,BindType & BindValue,int  len = sizeof(BindType),TypeCategoryKnow=TypeCategoryKnow())
	{
		if(m_pDataBase==NULL || m_StmtHandle==NULL)
		{
			return false;
		}

		static int s_len =0;

		return BindCol(index,BindValue,s_len,len);
	}



protected:
	TDataBase & operator =(TDataBase&);
	TDataBase(TDataBase&);

private:
	IDataBase * m_pDataBase;
	TStmtHandle m_StmtHandle;
};


#endif
