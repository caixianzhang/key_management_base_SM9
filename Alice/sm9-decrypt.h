#ifndef SM9_DECRYPT_H_
#define SM9_DECRYPT_H_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>

using namespace std;

class sm9_decrypt{
	private:
		//工作目录
		string dir;
		
		//用户ID
		string ID;

		//用户私钥文件名，用户私钥文件名组成：ID.esk
		string sk_file;

		//用户私钥文件的文件指针
		FILE *sk_fp;

		//用户解密私钥
		SM9PrivateKey *sk;
	public:
		sm9_decrypt(string ID, string dir = "");
		virtual ~sm9_decrypt();
		//解密操作
		bool decrypt(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);
		//解密操作(长度大于256)
		bool decrypt_long(size_t plainlen, unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);
};


#endif
