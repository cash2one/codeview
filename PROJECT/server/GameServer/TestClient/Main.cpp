#include "TestClient.h"
#include "stdio.h"
#include <string>
#include <vector>

#include "FileSystem.h"

void main()
{

	

	std::vector<int> vect;

	vect.insert(vect.begin(),0);




	int nClientNum = 0;
	printf("请输入要开启的客户端数:");

	if( !scanf("%d",  &nClientNum) || 0 >= nClientNum){
		TRACE("输入参数错误,输入客户端数为:%d！！！",nClientNum);
	}

	if( !TestClient::GetInstance()->Init("Config.ini", nClientNum)){
		TRACE("TestClient::Init 失败,按任意键退出程序!");
		getchar();
		return;	
	}

	TestClient::GetInstance()->GetBasicServer()->GetIoService()->run();

	TRACE("程序已结束!");
}
