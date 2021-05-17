#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sms4.h>
#include <openssl/is_gmssl.h>

#include "sm4-cbc.h"

sm4_cbc::sm4_cbc(unsigned char *key, unsigned char *iv)
{
	//填写密钥
	memcpy(this->key, key, SMS4_KEY_LENGTH);

	//填写初始化向量
	memcpy(this->iv, iv, SMS4_IV_LENGTH);
	
	//生成加密轮密钥
	sms4_set_encrypt_key(&sms4_enc, this->key);

	//生成解密轮密钥
	sms4_set_decrypt_key(&sms4_dec, this->key);
}
sm4_cbc::~sm4_cbc(){}

/*
	加密 
	plain:	输入的明文
	cypher:	输出的密文
	len:	数据长度
*/
void sm4_cbc::sm4_cbc_enc(unsigned char *plain, unsigned char *cipher, int len)
{
	unsigned char *pre_block = iv;
	//暂存异或运算的中间结果
	unsigned char tempbuffer[SMS4_BLOCK_SIZE]; 
	for(int i = 0; i < len; i += SMS4_BLOCK_SIZE)
	{
		//前一个密文块与当前明文异或，生成中间数据块
		for(int j = 0; j < SMS4_BLOCK_SIZE; j++)
		{
			tempbuffer[j] = plain[i + j] ^ pre_block[j];
		}
		
		//加密数据块，生成当前密文块
		sms4_encrypt(tempbuffer, cipher + i, &sms4_enc);

		//刷新pre_block
		pre_block = cipher + i;
	}
}

/*
	解密 
	cypher:	输入的密文
	plain:	输出的明文
	len:	数据长度
*/
void sm4_cbc::sm4_cbc_dec(unsigned char *cipher, unsigned char *plain, int len)
{
	unsigned char *pre_block = iv;
	//暂存异或运算的中间结果
	unsigned char tempbuffer[SMS4_BLOCK_SIZE]; 
	for(int i = 0; i < len; i += SMS4_BLOCK_SIZE)
	{
		//解密数据块，生成中间数据块
		sms4_decrypt(cipher + i, tempbuffer, &sms4_dec);
	
		//前一个密文块与当前中间数据块异或，生成明文
		for(int j = 0; j < SMS4_BLOCK_SIZE; j++)
		{
			plain[i + j] = tempbuffer[j] ^ pre_block[j];
		}
		
		//刷新pre_block
		pre_block = cipher + i;
	}
}

