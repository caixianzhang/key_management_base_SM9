#ifndef SM9_EXCHANGE_H_
#define SM9_EXCHANGE_H_

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

class  sm9_key_exchange{
	private:
		//工作目录
		string dir;
	
		//需要发送到对端的数据
		unsigned char R[65];
		//需要发送到对端数据的长度(这个值需要初始化为2048，用于告知缓冲区大小)
		size_t Rlen;

		//随机数指针, 为发起方和响应方使用
		BIGNUM *r;

		//发送到对端的结果，grlen固定为384
		unsigned char gr[384];
		size_t grlen;

		//本端ID
		string self_id;
		
		//对端ID
		string peer_id;
		
		//用户私钥文件指针
		FILE *sk_fp;
		
		//本端ID对应的用户私钥
		SM9PrivateKey *sk;

		//形参，这里不使用
		int initiator;
		
	public:
		sm9_key_exchange(string self_id, string peer_id, string msk_file, string dir = "");
		virtual ~sm9_key_exchange();

		//获取RA or RB
		unsigned char *Get_R();
		size_t Get_Rlen(); 

		//获取gr
		unsigned char *Get_gr();
		size_t Get_grlen();
		
		//获取随机数r
		BIGNUM *Get_r();

		//获取对端ID
		string Get_peer_id();

		//获取发起方ID对应的用户私钥
		SM9PrivateKey *Get_sk();
};

//发起方用户A
class sm9_alice{
	private:
		sm9_key_exchange *exchange_ini;
		/*
			采用的摘要算法，NID_sm9kdf_with_sm3 or NID_sm9kdf_with_sha256 ?
			这里主密钥生成时候采用的是NID_sm9kdf_with_sm3, 故type固定填NID_sm9kdf_with_sm3

			源码如下：
			SM9_setup(NID_sm9bn256v1, type, NID_sm9hash1_with_sm3, &mpk, &msk)
		*/
		int type;

		/*
			发起方A生成好的协商密钥SKA, 以及SKA的长度
			注意：SKAlen < 2048
		*/
		unsigned char SKA[2048];
		size_t SKAlen;

		/*
			发起方A生成协商密钥后，确认S1[32] == SB[32],
			则认为协商密钥成功，此时发起方A计算SA,并发送SA到
			响应方B,以供B确认协商密钥成功。
		*/
		unsigned char SA[32];
		
		/*
			发起方A接收响应方B的发出的SB[32]
		*/
		unsigned char SB[32];
		
		//此处的rA对应于sm9_key_exchange()的r
		BIGNUM *rA;

		//发起方A初始化过程计算出的RA(需发送到响应方B)
		unsigned char RA[65];

		//响应方B初始化过程生成的RB, 发起方A接收响应方B的发出的RB[65]
	    unsigned char RB[65];

		//发起方A初始化过程中生成的gr
		unsigned char g1[384];

		//响应方B的ID
		string IDB;

		//发起方A的用户私钥
		SM9PrivateKey *skA;
	public:
		sm9_alice(sm9_key_exchange *exchange_ini, int SKAlen, unsigned char RB[65], unsigned char SB[32]);
		virtual ~sm9_alice();
		//计算共享密钥
		bool compute_share_key();
	
		//获取共享密钥
		unsigned char *Get_SKA();

		//获取SA,已供响应方B确认协商成功
		unsigned char *Get_SA();
};

//响应方用户B
class sm9_bob{
	private:
		sm9_key_exchange *exchange_ini;
		/*
			采用的摘要算法，NID_sm9kdf_with_sm3 or NID_sm9kdf_with_sha256 ?
			这里主密钥生成时候采用的是NID_sm9kdf_with_sm3, 故type固定填NID_sm9kdf_with_sm3

			源码如下：
			SM9_setup(NID_sm9bn256v1, type, NID_sm9hash1_with_sm3, &mpk, &msk)
		*/
		int type;

		/*
			响应方B生成好的协商密钥SKB, 以及SKB的长度
			注意：SKBlen < 2048
		*/
		unsigned char SKB[2048];
		size_t SKBlen;

		/*
			响应方B计算初始化过程中生成的SB(需发送到发起方A)
		*/
		unsigned char SB[32];

		/*
			发起方A计算生成完SKA后计算SA，将SA发送到响应方B,
			与响应方B的S2相比较，如果相同则认为密钥协商成功
		*/
		unsigned char S2[32];

		
		//此处的rB应于sm9_key_exchange()的r
		BIGNUM *rB;

		//响应方B在初始化过程中生成的RB(需要发送给发起方A)
		unsigned char RB[65];

		//发起方A在初始化过程中生成的RA(需要发送给响应方B)
		unsigned char RA[65];
		
		//响应方B在初始化过程中生成的g1
		unsigned char g2[384];

		//发起方A的ID
		string IDA;

		//响应方用户私钥
		SM9PrivateKey *skB;
	public:
		sm9_bob(sm9_key_exchange *exchange_ini, int SKBlen, unsigned char RA[65]);
		virtual ~sm9_bob();

		//计算共享密钥
		bool compute_share_key();

		//获取共享密钥
		unsigned char *Get_SKB();

		//获取RB，以供发起方A计算共享密钥
		unsigned char *Get_RB();

		//获取SB，以供发起方A验证密钥协商是否成功
		unsigned char *Get_SB();

		//获取S2，以供发起方A验证密钥协商是否成功
		unsigned char *Get_S2();

		//校验发起方A计算得出的SA是否正确
		bool check_SA(unsigned char SA[32]);
};

#endif
