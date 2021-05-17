#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>

/*
	说明:
	明文输入长度inlen:	16
	密文输出长度outlen: 121
	明文输入长度inlen:	32
	密文输出长度outlen: 138
	加密时候有加盐处理，
	数据长度过长需要进行数据分割。
*/

#include "sm9-decrypt.h"

using namespace std;

/*
	ID:用户ID
	dir:工作目录
*/
sm9_decrypt::sm9_decrypt(string ID, string dir)
{
	this->ID = ID;
	this->dir = dir;
	
	//用户私钥文件名
	sk_file = ID + ".esk";

	//读取用户私钥文件
	sk_fp = fopen((dir + sk_file).c_str(), "r");
	if (sk_fp == NULL) 
	{
		fprintf(stderr,"\n");  
		printf("sm9_decrypt::sm9_decrypt(): fail to open sk_file %s !!!\n", this->sk_file.c_str());
	}

	//生成用户私钥结构体
	sk = d2i_SM9PrivateKey_fp(sk_fp, NULL);
	if (sk == NULL) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_decrypt::sm9_decrypt(): fail to gen sk !!!\n");
	}
}

sm9_decrypt::~sm9_decrypt()
{
	SM9PrivateKey_free(sk);
	fclose(sk_fp);
}
	
/*
	解密操作:
	in:待解密的数据
	inlen:待解密的数据长度
	注意 inlen <= 1024

	out:已解密的数据
	outlen:已解密的数据长度地址，为值结果参数
	注意 *outlen <= 1024
*/
bool sm9_decrypt::decrypt(unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
	if(SM9_decrypt(NID_sm9encrypt_with_sm3_xor, in, inlen, out, outlen, sk) == 0) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_decrypt::decrypt():fail to decrypt\n");
		return false;
	}
	return true;
}

/*
	解密操作
	plainlen:欲生成的明文数据长度
	
	in：待解密的密文数据
	inlen：待解密的密文数据的长度
	注意：这里是针对inlen > 256情况，需要对数据进行分割操作，
		以121字节为分割单位，对末尾剩余字节解密

	
	out：明文数据
	outlen：已解密数据长度地址，为值结果参数
	注意：*outlen    = 分割组数 * 16 + 剩余字节加密长度
*/
bool sm9_decrypt::decrypt_long(size_t plainlen, unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen)
{
	//解密轮数
	int round = (plainlen % 16 == 0) ? plainlen / 16 : plainlen / 16 + 1;
	size_t tempoutlen;
	bool res = true;
	for(int i = 0; i < round; i++)
	{
		if(decrypt(in + i * 121, 121, out + i * 16, &tempoutlen) == false)
		{
			res = false;
		}
	}
	
	/*
		密文总长度为前(round - 1)轮密文长度 + 最后一轮密文长度
	*/
	*outlen	= 16 * (round - 1) + tempoutlen;

	return res;
}