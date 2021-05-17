#ifndef AGREEMENT_H_
#define AGREEMENT_H_

#include "sm9-sign.h"
#include "sm9-verify.h"
#include "sm9-exchange.h"
#include "sm9-encrypt.h"
#include "sm9-decrypt.h"

#include <string>

using namespace std;

/*
	系统主密钥报文协议
	盐值由响应方发送给发起方，
	发起方生成主密钥后，利用公钥加密主密钥，
	填入encmsk[256]中去，然后对盐和密文主密钥签名，
	明文主密钥清零。
*/
struct mskmessage{
	//明文主密钥长度
	size_t msklen;

	//加密后的主密钥长度
	size_t encmsklen;
	
	//发起方数字签名长度
	size_t siglen;

	//明文主密钥(固定为139字节)
	unsigned char msk[256];

	//盐值(此处由响应方填写)
	unsigned char rand[32];
	//加密后的主密钥(固定为246字节)
	unsigned char encmsk[256];

	//数字签名(对盐值和加密后的主密钥签名)
	unsigned char sig[512];
};	

/*
	密钥协商发起方
*/
class agreement_Alice{
	private:
		//存放握手文件的目录(需要预先创建)
		string dir;
		
		//系统主密钥(这里规定由Alice生成)
		string mskfile;
		
		//密钥协商发起方id
		string Alice;

		//密钥协商响应方id
		string Bob;

		//密钥协商主密钥报文
		mskmessage *agreemsk;

		//发起方A初始化信息
		sm9_key_exchange *exchange_ini_A;

		sm9_alice *A;

		//共享密钥(32字节)
		unsigned char SKA[32];
		
		unsigned char RA[65];
		unsigned char RB[65];
		unsigned char SA[32];
		unsigned char SB[32];
	public:
		agreement_Alice(string Alice, string Bob, mskmessage *agreemsk, string dir);
		virtual ~agreement_Alice();
		bool Init();
		bool sign();
		unsigned char *GetRA();
		bool compute_share_key(unsigned char RB[65], unsigned char SB[32]);
		unsigned char *GetSA();
		unsigned char *GetSKA();
};

/*
	密钥协商响应方
*/
class agreement_Bob{
	private:
		//存放握手文件的目录(需要预先创建)
		string dir;
		
		//系统主密钥(这里规定由Alice生成)
		string mskfile;
		
		//密钥协商发起方id
		string Alice;

		//密钥协商响应方id
		string Bob;

		//密钥协商主密钥报文
		mskmessage *agreemsk;
	
		sm9_verify *verify_st;

		//发起方A初始化信息
		sm9_key_exchange *exchange_ini_B;

		sm9_bob *B;

		//共享密钥(32字节)
		unsigned char SKB[32];
		
		unsigned char RA[65];
		unsigned char RB[65];
		unsigned char SA[32];
		unsigned char SB[32];
	public:
		agreement_Bob(string Alice, string Bob, mskmessage *agreemsk, string dir);
		virtual ~agreement_Bob();
		bool verify();
		bool Init();
		unsigned char *GetRB();
		bool compute_share_key(unsigned char RA[65]);
		unsigned char *GetSB();
		unsigned char *GetSKB();
		bool checkSA(unsigned char SA[32]);
};

#endif 
