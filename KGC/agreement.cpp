#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "directory.h"
#include "sm9-setup.h"
#include "sm9-exchange.h"
#include "sm9-sign.h"
#include "sm9-verify.h"
#include "sm9-encrypt.h"
#include "sm9-decrypt.h"

#include "file.h"
#include "clear.h"
#include "agreement.h"
#include "rand.h"

agreement_Alice::agreement_Alice(string Alice, string Bob, mskmessage *agreemsk, string dir)
{
	exchange_ini_A = NULL;
	A = NULL;
	
	this->dir = dir;
	this->Alice = Alice;
	this->Bob = Bob;
	this->agreemsk = agreemsk;
	
	mskfile = Alice + Bob + ".msk";
	string mpkfile = Alice + Bob + ".mpk";

	//生成msk
	delete (new sm9_setup(sm9keyagreement, mskfile, mpkfile, dir));
	clearfile(dir + mpkfile);
}

agreement_Alice::~agreement_Alice()
{
	delete agreemsk;
	if(A != NULL)
		delete A;
}

bool agreement_Alice::Init()
{
	File *tempfile;
	bool res = true;
	
	//填写msk
	tempfile = new File();
	tempfile->Read_File(dir + mskfile);
	agreemsk->msklen = tempfile->Get_filelen();
	memcpy(agreemsk->msk, tempfile->Get_filedata(), agreemsk->msklen);
	delete tempfile;

	//加密msk
	sm9_encrypt *enc = new sm9_encrypt(SM9_ENC_MPK, Bob, dir);
	enc->encrypt(agreemsk->msk, agreemsk->msklen, agreemsk->encmsk, &(agreemsk->encmsklen));
	delete enc;
	
	//清空明文区
	memset(agreemsk->msk, 0, 256);

	//初始化发起方A初始化信息
	exchange_ini_A = new sm9_key_exchange(Alice, Bob, mskfile, dir);
	
	//填写RA
	memcpy(RA, exchange_ini_A->Get_R(), 65);

	return res;
}

bool agreement_Alice::sign()
{
	File *tempfile;
	bool res = true;
	
	//生成待签名文件,与签名文件
	string msg_file = Alice + Bob + ".msg";
	string sig_file = Alice + Bob + ".sig";

	tempfile = new File();
	tempfile->Write_File(dir + msg_file, agreemsk->rand, 32 + 256);
	delete tempfile;
	
	//对盐值和加密后的主密钥签名
	sm9_sign *sign_st = new sm9_sign(Alice + ".ssk", dir);
	if(sign_st->sign(msg_file, sig_file) == false)
	{
		printf("agreement_Alice::sign(): sign fail !\n");
		res = false;
	}else
	{
		printf("agreement_Alice::sign(): sign success !\n");
		res = true;
	}
	delete sign_st;
	
	tempfile = new File();
	tempfile->Read_File(dir + sig_file);
	agreemsk->siglen = tempfile->Get_filelen();
	memcpy(agreemsk->sig, tempfile->Get_filedata(), agreemsk->siglen);
	delete tempfile;

	clearfile(dir + msg_file);
	clearfile(dir + sig_file);

	return res;
}

unsigned char *agreement_Alice::GetRA()
{
	return RA;
}

bool agreement_Alice::compute_share_key(unsigned char RB[65], unsigned char SB[32])
{
	memcpy(this->RB, RB, 65);
	memcpy(this->SB, SB, 32);
	
	bool res = true;
	
	A = new sm9_alice(exchange_ini_A, 32, this->RB, this->SB);
	if(A->compute_share_key() == true)
	{
		memcpy(SKA, A->Get_SKA(), 32);
		memcpy(SA, A->Get_SA(), 32);
		res = true;
	}else
	{
		printf("agreement_Alice::compute_share_key():: compute_share_key fail \n");
		res = false;
	}
	return res;
}

unsigned char *agreement_Alice::GetSA()
{
	return SA;
}

unsigned char *agreement_Alice::GetSKA()
{
	return SKA;
}

agreement_Bob::agreement_Bob(string Alice, string Bob, mskmessage *agreemsk, string dir)
{
	exchange_ini_B = NULL;	
	B = NULL;
	
	this->dir = dir;
	this->Alice = Alice;
	this->Bob = Bob;
	this->agreemsk = agreemsk;

	mskfile = Alice + Bob + ".msk";
}

agreement_Bob::~agreement_Bob()
{
	delete agreemsk;
	if(B != NULL)	
		delete B;
}

bool agreement_Bob::verify()
{
	File *tempfile;
	bool res = true;
	
	//生成文件,与签名文件
	string msg_file = Alice + Bob + ".msg";
	string sig_file = Alice + Bob + ".sig";

	tempfile = new File();
	tempfile->Write_File(dir + msg_file, agreemsk->rand, 32 + 256);
	delete tempfile;

	tempfile = new File();
	tempfile->Write_File(dir + sig_file, agreemsk->sig, agreemsk->siglen);
	delete tempfile;
	
	//对盐值和加密后的主密钥验证签名
	sm9_verify *verify_st = new sm9_verify(SM9_SIGN_MPK, dir);
	if(verify_st->verify(Alice, msg_file, sig_file) == true)
	{
		printf("agreement_Bob::verify(): verify right !\n");
		res = true;
	}else
	{
		printf("agreement_Bob::verify(): verify wrong !\n");
		res = false;
	}
	delete verify_st;
	
	clearfile(dir + msg_file);
	clearfile(dir + sig_file);

	return res;
}

/*
	解密msk
*/
bool agreement_Bob::Init()
{
	File *tempfile;
	bool res = true;

	//解密msk
	sm9_decrypt *dec = new sm9_decrypt(Bob, dir);
	dec->decrypt(agreemsk->encmsk, agreemsk->encmsklen, agreemsk->msk, &(agreemsk->msklen));
	delete dec;
	
	//生成mskfile
	tempfile = new File();	
	tempfile->Write_File(dir + mskfile, agreemsk->msk, agreemsk->msklen);
	delete tempfile;

	//响应方B初始化信息
	exchange_ini_B = new sm9_key_exchange(Bob, Alice, mskfile, dir);
	
	//填写RB
	memcpy(RB, exchange_ini_B->Get_R(), 65);

	return res;
}

unsigned char *agreement_Bob::GetRB()
{
	return RB;
}

bool agreement_Bob::compute_share_key(unsigned char RA[65])
{
	B = new sm9_bob(exchange_ini_B, 32, RA);

	bool res = true;

	if(B->compute_share_key() == true)
	{
		//读取共享密钥
		memcpy(SKB, B->Get_SKB(), 32);

		//填写SB
		memcpy(SB, B->Get_SB(), 32);
		
		res = true;
	}else
	{
		printf("agreement_Bob::compute_share_key():: compute_share_key fail \n");
		res = false;
	}
	return res;
}

unsigned char *agreement_Bob::GetSB()
{
	return SB;
}

unsigned char *agreement_Bob::GetSKB()
{
	return SKB;
}

bool agreement_Bob::checkSA(unsigned char SA[32])
{
	bool res = true;
	memcpy(this->SA, SA, 32);
	if(B->check_SA(this->SA) == true)
	{
		printf("agreement_Bob::checkSA(): SA is right !\n");
		res = true;
	}else
	{
		res = false;
	}
	return res;
}
