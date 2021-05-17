#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

#include "sm9-encrypt.h"

/*
	说明:
	明文输入长度inlen:	16
	密文输出长度outlen: 121
	明文输入长度inlen:	32
	密文输出长度outlen: 138
	加密时候有加盐处理，
	数据长度过长需要进行数据分割。
*/

using namespace std;

/*
	mpk_file：加密主公钥参数文件名
	ID；加密者ID
*/
sm9_encrypt::sm9_encrypt(string mpk_file, string ID, string dir)
{
	this->dir = dir;
	this->mpk_file = mpk_file;
	this->ID = ID;

	//获取加密主公钥参数文件指针
	mpk_fp = fopen((dir + mpk_file).c_str(), "r");
	if (mpk_fp == NULL) 
	{
		fprintf(stderr,"\n");
		printf("sm9_encrypt::sm9_encrypt():fail to open mpk %s!!!\n", this->mpk_file.c_str());
	}
	
	//生成加密主公钥参数结构体
	mpk = d2i_SM9PublicParameters_fp(mpk_fp, NULL);
	if (mpk == NULL) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_encrypt::sm9_encrypt():parse public parameters failed\n");
	}
}

sm9_encrypt::~sm9_encrypt()
{
	SM9PublicParameters_free(mpk);
	fclose(mpk_fp);
}

/*
	加密操作
	in：待加密数据
	inlen：待加密数据的长度
	注意：inlen <= 256

	
	out：已加密数据
	outlen：已加密数据长度地址，为值结果参数
	注意：*outlen <= 1024
*/		
bool sm9_encrypt::encrypt(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
	if(SM9_encrypt(NID_sm9encrypt_with_sm3_xor, in, inlen, out, outlen, mpk, ID.c_str(), ID.size()) == 0) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_encrypt::encrypt():fail to encrypt\n");
		return false;
	}
	return true;
}

/*
	加密操作
	in：待加密数据
	inlen：待加密数据的长度
	注意：这里是针对inlen > 256情况，需要对数据进行分割操作，
		以16字节为分割单位，对末尾剩余字节加密

	
	out：已加密数据
	outlen：已加密数据长度地址，为值结果参数
	注意：*outlen    = 分割组数 * 121 + 剩余字节加密长度
*/
bool sm9_encrypt::encrypt_long(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{	
	//加密轮数
	int round = (inlen % 16 == 0) ? inlen / 16 : inlen / 16 + 1;
	size_t tempoutlen;
	bool res = true;
	for(int i = 0; i < round; i++)
	{
		if(encrypt(in + i * 16, 16, out + i * 121, &tempoutlen) == false)
		{
			res = false;
		}
	}
	
	/*
		密文总长度为前(round - 1)轮密文长度 + 最后一轮密文长度
	*/
	*outlen	= 121 * (round - 1) + tempoutlen;

	return res;
}


