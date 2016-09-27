
#include "DBProxyServer.h"
#include "ILogTrace.h"
#include <vector>

void main()
{
	
	if(DBProxyServer::Instance()->Init("./Config.ini")==false)
	{
		return;
	}

	TRACE("DBProxyServer Start!");

	DBProxyServer::Instance()->Run();

	TRACE("DBProxyServer Stop!");

}
