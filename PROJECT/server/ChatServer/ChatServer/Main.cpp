
#include "stdio.h"
#include "ChatServer.h"

void main()
{
	if(ChatServer::GetInstance().Init("./Config.ini")==false)
	{
		printf("\n\n服务器启动失败，详情请查看ChatServer.log日志，按任意键退出程序!");
		getchar();
		return;
	}


	printf("\n\n服务器启动成功，按ctrl+c键退出程序!");

	ChatServer::GetInstance().Run();
}
