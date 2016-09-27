
#include "DangLePlatform.h"


 IThirdPart * CreateThirdPartProxy(UINT8 type)
 {
	 switch(type)
	 {
	 case enGamePlatformType_DangLe:
		   return new DangLePlatform();
		 break;
	 default:
		 return 0;
		 break;
	 }
 }
