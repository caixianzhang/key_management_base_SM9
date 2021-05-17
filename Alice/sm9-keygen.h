#ifndef SM9_KEYGEN_H_
#define SM9_KEYGEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

#include "sm9-setup.h"

using namespace std;

class sm9_keygen{
	private:
		//工作目录
		string dir;
		
		//密钥类型
		enum TYPE type;

		//主密钥文件名
		string msk_file;

		//用户ID
		string ID;
		
		//用户私钥文件名
		string sk_file;
		
		//主密钥文件指针
		FILE *msk_fp;
		
		//用户私钥文件指针
		FILE *sk_fp;

		//主密钥结构体
		SM9MasterSecret *msk;

		//用户私钥结构体
		SM9PrivateKey *sk;
		
	public:
		sm9_keygen(enum TYPE type, string msk_file, string ID, string dir = "");
		virtual ~sm9_keygen();
};

#endif 
