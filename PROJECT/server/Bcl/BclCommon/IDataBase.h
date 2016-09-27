
#ifndef __BCL_IDATABASE_H__
#define __BCL_IDATABASE_H__
#include <string>
#include "BclHeader.h"

#include "FieldDef.h"

#ifndef BUILD_DATABASE_SYSTEM_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"DataBaseSystem.lib")
#endif
#endif

//语句句柄
typedef void*  TStmtHandle;

struct IDataBase
{
	//释放对象
	virtual void Release()=0;

	//连接数据库
	virtual bool Connect(const char* szDBName,const char* szUserName,const char* szPassword,const char *szIp=0, unsigned int port=0,const char * szCharacter=0)=0;

	//关闭连接
	virtual bool Close() = 0;

	//线程安全
	virtual bool ThreadInit() = 0;
	virtual bool ThreadEnd() = 0;

		//直接执行，成功返回有效句柄，后续函数调用需要该句柄，失败返回NULL
	virtual TStmtHandle ExecuteDirect(const char* szSql,int len=0) = 0; 

	//准备sql语句,成功返回有效句柄，后续函数调用需要该句柄，失败返回NULL
	virtual TStmtHandle Prepare(const char* szSql,int len=0)=0;

	//绑定参数
	virtual bool BindParam(TStmtHandle handle,int index,enParamType InputOutputType,enTypeValue ValueType,int SqlType,void * pValue,int & len)=0;

	virtual bool BindParam(TStmtHandle handle,int index,enParamType InputOutputType,enTypeValue ValueType,void * pValue,int & len)=0;


	//执行
	virtual bool Execute(TStmtHandle handle) = 0; 

	//绑定列
	virtual bool BindCol(TStmtHandle handle,int index,enTypeValue ValueType,void * pValue,int & len)=0;
	
	//获取结果集
	virtual bool Fetch(TStmtHandle handle)=0;

	virtual bool MoreResults(TStmtHandle handle)=0;

	//获取影响行数
	virtual int GetRowNum(TStmtHandle handle)=0;

	//获取错误信息
	virtual int GetErrorInfo(TStmtHandle handle,std::string & strError)=0;

	//释放句柄，释放后不能再使用
	virtual bool FreeStmtHandle(TStmtHandle handle)=0;

	//release结果集
	virtual bool FreeResults(TStmtHandle handle)=0;
};

enum enDBType
{
	enDBType_ODBC = 0,
	enDBType_MYSQL = 1,
	enDBType_Max,
};

BCL_API IDataBase * CreateDataBase(enDBType type=enDBType_MYSQL);

#endif
