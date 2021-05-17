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
#include "clear.h"

#include "sm9-exchange.h"

using namespace std;

/*
	self_id:本端ID
	peer_id:对端ID
	msk_file:主密钥文件名
*/
sm9_key_exchange::sm9_key_exchange(string self_id, string peer_id, string msk_file, string dir)
{
	//读取本端ID
	this->self_id = self_id;
	//读取对端ID
	this->peer_id = peer_id;
	
	Rlen = 65;
	grlen = 384;

	memset(R, 0, Rlen);
	memset(gr, 0, grlen);
	
	//初始化随机数
	r = BN_new();

	//由主密钥生成用户私钥，以二进制文件形式存放
	delete (new sm9_keygen(sm9keyagreement, msk_file, self_id, dir));
	//生成用户私钥后可删除主密钥
	clearfile(dir + msk_file);
	
	//读取用户私钥二进制文件
	sk_fp = fopen((dir + self_id + ".csk").c_str(), "r");
	if (sk_fp == NULL) 
	{
		fprintf(stderr,"\n");  
		printf("sm9_key_exchange::sm9_key_exchange():fail to open sk_file !!!\n");
	}

	//生成用户私钥结构体
	sk = d2i_SM9PrivateKey_fp(sk_fp, NULL);
	if (sk == NULL) 
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_key_exchange::sm9_key_exchange():fail to gen sk !!!\n");
	}
	fclose(sk_fp);
	//生成用户私钥结构体后可删除用户私钥
	clearfile(dir + self_id + ".csk");
	
	//计算RA，RB
	if(SM9_generate_key_exchange(R, &Rlen, r, gr, &grlen, peer_id.c_str(), peer_id.size(), sk, initiator) == 0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_key_exchange::sm9_key_exchange(): fail to get R !!!\n");
	}
}

sm9_key_exchange::~sm9_key_exchange()
{	
	SM9PrivateKey_free(sk);
	BN_free(r);
}

unsigned char *sm9_key_exchange::Get_R()
{
	return R;
}

size_t sm9_key_exchange::Get_Rlen()
{
	return Rlen;
}

unsigned char *sm9_key_exchange::Get_gr()
{
	return gr;
}

size_t sm9_key_exchange::Get_grlen()
{
	return  grlen;
}

BIGNUM *sm9_key_exchange::Get_r()
{
	return r;
}

string sm9_key_exchange::Get_peer_id()
{
	return peer_id;
}

SM9PrivateKey *sm9_key_exchange::Get_sk()
{
	return sk;
}

/*
	SKAlen:协商密钥长度
*/
sm9_alice::sm9_alice(sm9_key_exchange *exchange_ini, int SKAlen, unsigned char RB[65], unsigned char SB[32])
{
	this->exchange_ini = exchange_ini;
	//填写摘要类型
	type = NID_sm9kdf_with_sm3;
	
	//读取协商密钥长度
	this->SKAlen = SKAlen;

	//读取随机数r
	rA = exchange_ini->Get_r();

	//发起方A初始化过程计算出的RA
	memcpy(this->RA, exchange_ini->Get_R(), 65);

	//发起方A接收响应方B初始化过程生成的RB
	memcpy(this->RB, RB, 65);

	//发起方A接收响应方B计算共享密钥后生成的SB
	memcpy(this->SB, SB, 32);
	
	//发起方A初始化过程中生成的gr
	memcpy(this->g1, exchange_ini->Get_gr(), 384);
		
	//设置响应方ID
	this->IDB = exchange_ini->Get_peer_id();

	//获取发起方A的私钥
	this->skA = exchange_ini->Get_sk();
	
}
sm9_alice::~sm9_alice()
{
	delete exchange_ini;
}

bool sm9_alice::compute_share_key()
{
	//计算共享密钥
	if(SM9_compute_share_key_A(type, SKA, SKAlen, SA, SB, rA, RA, RB, g1, IDB.c_str(), IDB.size(), skA) ==  0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_alice::compute_share_key():Gen  SKA fail !!!\n");
		return  false;
	}	
	printf("sm9_alice::compute_share_key():Gen  SKA success !!!\n");
	return true;
}

//读取协商后的密钥
unsigned char *sm9_alice::Get_SKA()
{
	return SKA;
}

unsigned char *sm9_alice::Get_SA()
{
	return SA;
}

sm9_bob::sm9_bob(sm9_key_exchange *exchange_ini, int SKBlen, unsigned char RA[65])
{
	this->exchange_ini = exchange_ini;
	//填写摘要类型
	type = NID_sm9kdf_with_sm3;

	//填写协商密钥长度
	this->SKBlen = SKBlen;
		
	//此处的rB应于sm9_key_exchange()的r
	this->rB = exchange_ini->Get_r();

	//填写RA, RB
	memcpy(this->RA, RA, 65);
	
	memcpy(this->RB, exchange_ini->Get_R(), 65);
	
	
	//响应方B在初始化过程中生成的gr
	memcpy(this->g2, exchange_ini->Get_gr(), 384);

	//设置发起方A的ID
	this->IDA = exchange_ini->Get_peer_id();

	//获取响应方B的用户私钥
	this->skB = exchange_ini->Get_sk();
}

sm9_bob::~sm9_bob()
{
	delete exchange_ini;
}

//计算响应方B的共享密钥
bool sm9_bob::compute_share_key()
{	
	//计算共享密钥
	if(SM9_compute_share_key_B(type, SKB, SKBlen, SB, S2, rB, RB, RA, g2, IDA.c_str(), IDA.size(), skB) ==  0)
	{
		ERR_print_errors_fp(stderr);
		fprintf(stderr,"\n");
		printf("sm9_bob::compute_share_key():Gen  SKB fail !!!\n");
		return  false;
	}	
	printf("sm9_bob::compute_share_key():Gen  SKB success !!!\n");
	return true;
}

//获取计算出的共享密钥
unsigned char *sm9_bob::Get_SKB()
{
	return SKB;
}


//获取RB，以供发起方A计算共享密钥
unsigned char *sm9_bob::Get_RB()
{
	return RB;
}

//获取SB，以供发起方A验证密钥协商是否成功
unsigned char *sm9_bob::Get_SB()
{
	return SB;
}

//获取S2，以供发起方A验证密钥协商是否成功
unsigned char *sm9_bob::Get_S2()
{
	return S2;
}

bool sm9_bob::check_SA(unsigned char SA[32])
{
	if(memcmp(SA, S2, 32) == 0)
	{
		//共享密钥有效
		printf("sm9_bob::check_SA():SKB is valid !!!\n");
		return true;
	}else
	{
		//共享密钥无效
		printf("sm9_bob::check_SA():SKB is invalid !!!\n");
		return false;
	}
}

