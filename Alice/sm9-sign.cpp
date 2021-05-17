#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

#include "sm9-sign.h"

using namespace std;

sm9_sign::sm9_sign(string sk_file, string dir)
{
	this->dir = dir;
	this->sk_file = sk_file;
	
	//生成摘要结构体
	ctx = EVP_MD_CTX_new();
	if(ctx == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sm9_sign():fail to malloc ctx !!!\n");
	}

	//初始化摘要结构体
	if(SM9_SignInit(ctx, EVP_sm3(), NULL) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sm9_sign():fail to init ctx !!!\n");
	}

	//打开用户私钥文件
	sk_fp = fopen((dir + sk_file).c_str(), "r");
	if(sk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to read sk !!!\n");
	}
	
	//生成用户私钥结构体
	sk = d2i_SM9PrivateKey_fp(sk_fp, NULL);
	if(sk == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to gen sk !!!\n");
	}
}

sm9_sign::~sm9_sign()
{
	EVP_MD_CTX_free(ctx);
	SM9PrivateKey_free(sk);
	fclose(sk_fp);
}

/*
	进行数字签名
	msg_file：待签名文件
	sig_file：生成的签名文件
*/
bool sm9_sign::sign(string msg_file, string sig_file)
{
	this->msg_file = msg_file;
	this->sig_file = sig_file;
	
	//打开待签名文件
	msg_fp = fopen((dir + msg_file).c_str(), "r");
	if(msg_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to read msg !!!\n");
		return false;
	}

	//读取待签名文件
	len = fread(buf, 1, sizeof(buf), msg_fp);
	
	//进行摘要
	if(SM9_SignUpdate(ctx, buf, len) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to sign msg !!!\n");
		return false;
	}

	//生成数字签名
	sig = SM9_SignFinal(ctx, sk);
	if(sig == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to gen sig !!!\n");
		return false;
	}
	
	//打开签名数据文件指针
	sig_fp = fopen((dir + sig_file).c_str(), "w");
	if(sig_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to read sig !!!\n");
		return false;
	}

	//写入数字签名
	if(i2d_SM9Signature_fp(sig_fp, sig) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_sign::sign():fail to write sig !!!\n");
		return false;
	}
	
	fclose(msg_fp);
	fclose(sig_fp);
	SM9Signature_free(sig);
	
	return true;
}

