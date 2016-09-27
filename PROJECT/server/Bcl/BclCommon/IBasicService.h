
#ifndef __BCL_IBASIC_SERVICE_H__
#define __BCL_IBASIC_SERVICE_H__

#include "BclHeader.h"
#include "stdarg.h"
#include "ILogTrace.h"



namespace boost
{
	namespace asio
	{
		class io_service;
	};
};

struct ITimeAxis;
struct ILogTrace;
struct IKeywordFilter;
struct ICryptService;

struct IBasicService
{
	virtual bool			Create(boost::asio::io_service * pIoService=0)=0;
	virtual void			Release()=0;

	virtual boost::asio::io_service * GetIoService() = 0;

	virtual ITimeAxis * GetTimeAxis() = 0;

	virtual ILogTrace * GetLogTrace() = 0;

	virtual IKeywordFilter * GetKeywordFilter() = 0;

	virtual ICryptService * GetCryptService() = 0;

};

BCL_API IBasicService * GetBasicService();

#define TRACE(fm,...) { IBasicService * pBasicService = GetBasicService(); ILogTrace * pLogTrace = NULL; if( pBasicService && (pLogTrace=pBasicService->GetLogTrace())){pLogTrace->TraceLn(fm, ## __VA_ARGS__);}}

#endif

