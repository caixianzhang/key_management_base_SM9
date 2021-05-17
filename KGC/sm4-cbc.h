#ifndef SM4_CBC_H_
#define SM4_CBC_H_

#include <libgen.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sms4.h>
#include <openssl/is_gmssl.h>

/*
	SM4 CBC模式加密(这里需要保证加解密的数据为16整数倍)
*/
class sm4_cbc{
	private:
		//加密扩展密钥
		sms4_key_t sms4_enc;

		//解密扩展密钥
		sms4_key_t sms4_dec;
		
		//密钥
		unsigned char key[SMS4_KEY_LENGTH];
		
		//初始化向量
		unsigned char iv[SMS4_IV_LENGTH];
	public:
		sm4_cbc(unsigned char *key, unsigned char *iv);
		virtual ~sm4_cbc();
		//加密
		void sm4_cbc_enc(unsigned char *plain, unsigned char *cipher, int len);
		//解密
		void sm4_cbc_dec(unsigned char *cipher, unsigned char *plain, int len);
};

#endif