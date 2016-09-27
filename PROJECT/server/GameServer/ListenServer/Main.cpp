#include "ListenerLogin.h"

#include "FileSystem.h"

void main()
{
	if( !ListenerLogin::GetInstance()->Init("config.ini")){
		TRACE("ListenerLogin::Init 失败,按任意键退出程序!");
		getchar();
		return;	
	}

	ListenerLogin::GetInstance()->GetBasicServer()->GetIoService()->run();

	TRACE("程序已结束!");
}
