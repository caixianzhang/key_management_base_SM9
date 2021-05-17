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

/*
	参数说明：
	type：需要由主密钥生成的密钥类型
	msk_file；主密钥文件名
	mpk_file: 系统参数文件名
*/
sm9_setup::sm9_setup(enum TYPE type, string msk_file, string mpk_file, string dir)
{
	this->dir = dir;
	this->msk_file = msk_file;
	this->mpk_file = mpk_file;
	this->type = type;
	
	//生成主密钥和系统参数结构体
	if(SM9_setup(NID_sm9bn256v1, type, NID_sm9hash1_with_sm3, &mpk, &msk) == 0) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_setup::sm9_setup():fail to setup mpk msk !!!\n");
	}

	//打开主密钥文件指针
	msk_fp = fopen((dir + msk_file).c_str(), "w");
	if(msk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_setup::sm9_setup():fail to get msk_fp !!!\n");
	}
	
	//打开系统参数文件指针
	mpk_fp = fopen((dir + mpk_file).c_str(), "w");
	if(mpk_fp == NULL)
	{
		fprintf(stderr,"\n");
		printf("sm9_setup::sm9_setup():fail to get mpk_fp !!!\n");
	}
	
	//写主密钥
	if(i2d_SM9MasterSecret_fp(msk_fp, msk) == 0)
	{
		fprintf(stderr,"\n");
		printf("sm9_setup::sm9_setup():fail to write msk !!!\n");
	}
	
	//写系统参数
	if(i2d_SM9PublicParameters_fp(mpk_fp, mpk) == 0)
	{
		fprintf(stderr,"\n");
		printf("sm9_setup::sm9_setup():fail to write mpk !!!\n");
	}
}

sm9_setup::~sm9_setup()
{
	SM9MasterSecret_free(msk);
	SM9PublicParameters_free(mpk);
	fclose(msk_fp);
	fclose(mpk_fp);
}
