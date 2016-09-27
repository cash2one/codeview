
#include "GameServer.h"

#include <vector>
#include <string>
#include <list>
#include <hash_map>
#include "FileSystem.h"

void main()
{		
	if(GameServer::GetInstance()->Init("./Config.ini")==false)
	{
		printf("GameServer::Init 失败,按任意键退出程序!");
		getchar();
		return;
	}
	
	TRACE("GameServer 启动成功,按 ctrl+c 键退出程序!");

	GameServer::GetInstance()->Run();

	printf("GameServer 退出程序!");
}
