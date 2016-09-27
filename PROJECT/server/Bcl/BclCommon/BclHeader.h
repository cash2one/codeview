#ifndef __BCL_BCL_COMMON_HEADER_H__
#define __BCL_BCL_COMMON_HEADER_H__

#ifdef WIN32
#ifdef BCL_DLL
#define BCL_API _declspec(dllexport)
#else
#define BCL_API __declspec(dllimport)
#endif
#else
#define BCL_API
#endif

#ifndef BUILD_BCL_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"Bcl.lib")
#endif
#endif

//数据类型定义
typedef signed char        INT8;
typedef unsigned char      UINT8;
typedef short              INT16;
typedef unsigned short     UINT16;
typedef int                INT32;
typedef unsigned           UINT32;
typedef long               LONG;
typedef unsigned long      ULONG;

typedef  long long          INT64;
typedef  unsigned long long UINT64;


  //定义枚举占用的字节数
#ifdef _MSC_VER
    #define PACKED_ONE  
    #define VC_PACKED_ONE : UINT8
#else
#ifdef __GNUC__ 
  #define PACKED_ONE __attribute__((aligned (1),packed))
  #define VC_PACKED_ONE 
 #else
   #define PACKED_ONE 
   #define VC_PACKED_ONE
 #endif
#endif

#endif
