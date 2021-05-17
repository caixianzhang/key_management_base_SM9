#ifndef SM9_ENCRYPT_H_
#define SM9_ENCRYPT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

using namespace std;

class sm9_encrypt{
	private:
		//工作目录
		string dir;
		
		//加密主公钥参数文件名
		string mpk_file;
		
		//加密主公钥参数文件指针
		FILE *mpk_fp;

		//加密主公钥参数结构体
		SM9PublicParameters *mpk;
		
		//加密者ID
		string ID;
	public:
		sm9_encrypt(string mpk_file, string ID, string dir = "");
		virtual ~sm9_encrypt();
		//加密操作
		bool encrypt(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);
		//加密操作(长度大于256)
		bool encrypt_long(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);
};

#endif