#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

#include "sm9-verify.h"

using namespace std;

sm9_verify::sm9_verify(string mpk_file, string dir)
{
	this->dir = dir;
	
	//填写系统参数文件名
	this->mpk_file = mpk_file;

	//生成摘要结构体
	ctx = EVP_MD_CTX_new();
	if(ctx == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::sm9_verify():fail to malloc ctx !!!\n");
	}

	//初始化摘要结构体
	if(SM9_VerifyInit(ctx, EVP_sm3(), NULL) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::sm9_verify():fail to init ctx !!!\n");
	}
	
	//打开系统参数文件指针
	mpk_fp = fopen((dir + mpk_file).c_str(), "r");
	if(mpk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_verify::sm9_verify():fail to read mpk !!!\n");
	}
	
	//生成系统参数结构体
	mpk = d2i_SM9PublicParameters_fp(mpk_fp, NULL);
	if(mpk == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::sm9_verify():fail to gen mpk !!!\n");
	}
}

sm9_verify::~sm9_verify()
{
	EVP_MD_CTX_free(ctx);
	SM9PublicParameters_free(mpk);
	fclose(mpk_fp);
}

/*
	ID；签名者ID
	msg_file：待验签文件
	sig_file：已生成好的签名文件
*/
bool sm9_verify::verify(string ID, string msg_file, string sig_file)
{
	this->ID = ID;
	this->msg_file = msg_file;
	this->sig_file = sig_file;
	
	//打开待验签文件指针
	msg_fp = fopen((dir + msg_file).c_str(), "r");
	if(msg_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_verify::verify():fail to read msg !!!\n");
		return false;
	}

	//读取待验签文件
	len = fread(buf, 1, sizeof(buf), msg_fp);

	//进行摘要
	if(SM9_SignUpdate(ctx, buf, len) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::verify():fail to sign msg !!!\n");
		return false;
	}

	//打开签名文件指针
	sig_fp = fopen((dir + sig_file).c_str(), "r");
	if(sig_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_verify::verify():fail to read sig !!!\n");
		return false;
	}

	//转化为签名结构体
	sig = d2i_SM9Signature_fp(sig_fp, NULL);
	if(sig == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::verify():fail to sign !!!\n");
		return false;
	}

	//根据系统参数，ID，生成验签公钥结构体
	pk = SM9_extract_public_key(mpk, ID.c_str(), ID.size());
	if(pk == NULL)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::verify():fail to get ID pk !!!\n");
		return false;
	}

	//验证签名
	if(SM9_VerifyFinal(ctx, sig, pk) != 1)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_verify::verify(): verify fail !!!\n");
		return false;
	}

	printf("verify success\n");

	fclose(msg_fp);
	fclose(sig_fp);
	SM9Signature_free(sig);
	SM9PublicKey_free(pk);
	
	return true;
}
