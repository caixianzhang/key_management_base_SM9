#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

#include "sm9-setup.h"
#include "sm9-keygen.h"

using namespace std;

sm9_keygen::sm9_keygen(enum TYPE type, string msk_file, string ID, string dir)
{
	//填写工作目录
	this->dir = dir;
	
	//确认需要生成的用户密钥类型
	this->type = type;

	//读取主密钥文件名
	this->msk_file = msk_file;

	//读取用户ID
	this->ID = ID;

	if(type == sm9sign)
	{
		//签名用户私钥后缀是 ***.ssk。
		sk_file = ID + ".ssk";
	}else if(type == sm9encrypt)
	{
		//非对称加密用户私钥后缀是 ***.esk。
		sk_file = ID + ".esk";
	}else if(this->type == sm9keyagreement)
	{
		//密钥交换用户私钥后缀是 ***.csk。
		sk_file = ID + ".csk";
	}else{}

	//读取主密钥文件
	msk_fp = fopen((dir + msk_file).c_str(), "r");
	if(msk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_keygen::sm9_keygen():fail to get msk_fp !!!\n");
	}

	//生成主密钥结构体
	msk = d2i_SM9MasterSecret_fp(msk_fp, NULL);
	if(msk == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_keygen::sm9_keygen():fail to get msk !!!\n");
	}

	//由主密钥以及用户ID生成用户私钥结构体
	sk = SM9_extract_private_key(msk, ID.c_str(), ID.size());
	if(sk == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_keygen::sm9_keygen():fail to get sk !!!\n");
	}

	//打开用户私钥文件(需要指定目录)
	sk_fp = fopen((dir + sk_file).c_str(), "w");
	if(sk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_keygen::sm9_keygen():fail to get sk_fp !!!\n");
	}

	//写用户私钥文件
	if(i2d_SM9PrivateKey_fp(sk_fp, sk) == 0)
	{
		fprintf(stderr,"\n");
		printf("sm9_keygen::sm9_keygen():fail to write sk !!!\n");
	}
	
	printf("generate private key file '%s'\n", this->sk_file.c_str());	
}

sm9_keygen::~sm9_keygen()
{
	SM9PrivateKey_free(sk);
	SM9MasterSecret_free(msk);
	fclose(msk_fp);
	fclose(sk_fp);
}
