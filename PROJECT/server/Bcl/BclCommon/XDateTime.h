
#ifndef __BCL_XDATETIME_H__
#define __BCL_XDATETIME_H__

#include "BclHeader.h"
#include "time.h"
#include <string>

//服务器当前虚拟时间
#define CURRENT_TIME() (XDateTime::GetInstance().GetVirtualTime())

class BCL_API XDateTime
{
public:
	      //一些时间常量定义
   enum
   {
          SECOND_OF_MINUTE = 60,         //一分钟有60秒
		  SECOND_OF_HOUR   = 3600,        //每小时3600秒
		  SECOND_OF_DAY    = 86400,      //每天86400秒
		  MINUTE_OF_HOUR   = 60,         //每小时60分
		  MINUTE_OF_DAY    = 1440,       //每天1440分
		  HOUR_OF_DAY      = 24,        //每天24小时
		  DAY_OF_WEEK      = 7,         //每周七天
   };
public:

	XDateTime();

    static XDateTime & GetInstance();

	//是否为同一天
	bool IsSameDay(time_t t1,time_t t2);

		//是否为同一周
	bool IsSameWeek(time_t t1,time_t t2);

	//是否为同一个月
    bool IsSameMonth(time_t t1,time_t t2);

	//UTC时间0时，0分，0秒时对应本地时间的秒数
	time_t ZeroTime();

	//设置虚拟时间,返回旧时间
	time_t SetVirtualTime(time_t t1);

		//获得虚拟时间
	time_t GetVirtualTime();

	//恢复虚拟时间与当前时间一至
	void ResetVirtualTime();

	//获得当天零点
	time_t GetZeroTimeOfDay();

	//格式化时间,szTime格式为 2000/01/01/00/00/00
	time_t FormatTime(const char* szTime);

	//格式化时间,szTime格式为 2000-01-01 00:00:00
	std::string FormatTime(time_t Time);
private:
	
	INT32  m_offsetTime; //虚拟时间与标准时间偏差
};







#endif
