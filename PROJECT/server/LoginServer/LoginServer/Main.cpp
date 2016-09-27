
#include "stdio.h"
#include "LoginServer.h"

void main()
{
	if(LoginServer::GetInstance().Init("./Config.ini")==false)
	{
		printf("\n\n服务器启动失败，详情请查看LoginServer.log日志，按任意键退出程序!");
		getchar();
		return;
	}


	printf("\n\n服务器启动成功，按ctrl+c键退出程序!");

	LoginServer::GetInstance().Run();
}
