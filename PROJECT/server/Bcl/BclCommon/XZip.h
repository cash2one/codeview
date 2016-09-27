
#ifndef __BCL_ZIP_H__
#define __BCL_ZIP_H__

#include "BclHeader.h"

//通知进度

typedef UINT32 (*LPPROGRESS_UNZIP_ROUTINE)(void* pvParam, int nFilesTotal, int nFilesUnziped, const char* pszFileUnzipping);


class BCL_API XZip
{

public:

	enum	enXZipRatio
	{
		enXZipRatio_None,
		enXZipRatio_Fast,
	};
	//解压缩
	//把pszZipFileName文件(全路径)解压缩到pszPath指定的目录，
	//如果pszPath为空，则在pszZipFileName所在目录，建立一空的同名目录，然后解压缩到该目录,如Unzip("C:/123/456.zip")解压至C:/123/456/
	//压缩至,szPassword为密码,lpProgressRoutine为进度回调函数,pvParam作为回调函数的第一个参数的实参
	static bool Unzip(const char* pszZipFileName, const char* pszPath=NULL, LPPROGRESS_UNZIP_ROUTINE lpProgressRoutine = NULL, void* pvParam = NULL);

	//解压缩
	//把pszZipFileName文件(全路径)解压缩到pszPath指定的目录，
	//如果pszPath为空，则在pszZipFileName所在目录，建立一空的同名目录，然后解压缩到该目录,如Unzip("C:/123/456.zip")解压至C:/123/456/
	//压缩至,szPassword为密码,lpProgressRoutine为进度回调函数,pvParam作为回调函数的第一个参数的实参
	static bool Unzip7z(const char* pszZipFileName, const char* pszPath=NULL, LPPROGRESS_UNZIP_ROUTINE lpProgressRoutine = NULL, void* pvParam = NULL);

/*	static INT32	GetDecompressedDataUpperBound(const char* srcdata, INT32 size, enXZipRatio ratio);

	static bool		Compress(const char* srcdata, INT32 size,  char * dstdata, INT32 & compressedSize, enXZipRatio ratio);

	static bool		Decompress(const char * srcdata, INT32 size, char * outbuffer,INT32  & decompressedSize );*/

	
};


#endif

