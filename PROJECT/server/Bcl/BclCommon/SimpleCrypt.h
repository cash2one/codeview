
#ifndef __BCL_SIMPLE_CRYPT_H__
#define __BCL_SIMPLE_CRYPT_H__

#include "BclHeader.h"

class	BCL_API SimpleCryptor
{

public:
	
	SimpleCryptor();

	void	SetKey(const char* key);
	void	Encrypt(UINT8* data, UINT32 len);
	void	Decrypt(UINT8 * data, UINT32 len);
private:

	UINT8	m_efx[256]; //变换表
	UINT8   m_dfx[256];
	UINT8	m_key[128];
	UINT8	m_keysize;


};

#endif

