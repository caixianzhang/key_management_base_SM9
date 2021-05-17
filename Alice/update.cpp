#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "directory.h"
#include "net.h"
#include "threadpool.h"
#include "update.h"
#include "agreement.h"
#include "two-way-auth.h"
#include "file.h"
#include "directory.h"
#include "sm4-cbc.h"
#include "file.h"
#include "clear.h"

using namespace std;

/*
	密钥分发前，根目录存放旧版密钥
	KGC目录存放新版密钥，
	密钥分发完成后，根目录下KGC的旧版密钥被更换成新版密钥
*/
update::update(string selfid, session_directory *session_dir)
{
	this->selfid = selfid;
	this->session_dir = session_dir;
	server_listen = new TCP_Server_Listen(DISTPORT, UPDATEETH);
}

update::~update()
{
	delete session_dir;
	delete server_listen;
}

bool update::run()
{
	bool res = true;
	server = new TCP_Server_Accept(server_listen->Get_Listen_Fd());
	if(Authentication() == false)
	{
		printf("auth wrong!\n");
		res = false;
	}else
	{
		printf("auth right!\n");
	}
	
	//进行密钥协商
	if(Keyagreement() == false)
	{
		printf("agree wrong!\n");
		res = false;
	}else
	{
		printf("agree right!\n");
	}
	

	//接收新的分发密钥
	if(Recvkey() == false)
	{
		printf("recv new key fail !\n");
		res = false;
	}else
	{
		printf("recv new key success !\n");
	}
	return res;
}
		
//KGC与用户进行双向身份认证
bool update::Authentication()
{
	two_way_auth *auth = new two_way_auth(selfid, KGCID, session_dir);

	chapmessage mess;

	bool res = true;
	
	//发送询问报文
	if(server->Send((unsigned char *)(auth->Getchallenge()), sizeof(chapmessage)) == -1)
	{
		res = false;
	}
	
	//接收应答报文
	if(server->Recv((unsigned char *)&mess, sizeof(chapmessage)) == -1)
	{
		res = false;
	}

	memcpy(mess.rand, auth->Getchallenge()->rand, 32);

	//验证签名
	if(auth->verify(&mess) == false)
	{
		res = false;
	}

	//接收对端询问报文
	if(server->Recv((unsigned char *)&mess, sizeof(chapmessage)) == -1)
	{
		res = false;
	}

	//对对端询问报文签字
	if(auth->sign(&mess) == false)
	{
		res = false;
	}

	//回传签字报文
	if(server->Send((unsigned char *)&mess, sizeof(chapmessage)) == -1)
	{
		res = false;
	}

	delete auth;

	return res;
}

//KGC与用户进行密钥协商
bool update::Keyagreement()
{
	string Alice = KGCID;
	string Bob = selfid;
	
	bool res = true;

	//发送密钥协商盐值
	if(server->Send(session_dir->getrand256(), 32) == -1)
	{
		res = false;
	}

	mskmessage *agreemsk = new mskmessage();	
	memset((unsigned char *)agreemsk, 0, sizeof(mskmessage));
	
	//Bob接收密钥协商主密钥
	if(server->Recv((unsigned char *)agreemsk, sizeof(mskmessage)) == -1)
	{
		res = false;
	}

	//接收密钥协商RA
	unsigned char RA[65];
	if(server->Recv(RA, 65) == -1)
	{
		res = false;
	}

	//回填随机数 
	memcpy(agreemsk->rand, session_dir->getrand256(), 32);

	//Bob初始化 
	agreement_Bob *B = new agreement_Bob(Alice, Bob, agreemsk, session_dir->getdir());

	//Bob对报文验证签名
	if(B->verify() == false)
	{
		res = false;
	}

	//Bob解密密钥协商的msk, 完成初始化并生成RB 
	if(B->Init() == false)
	{
		res = false;
	}

	//Bob计算共享密钥
	B->compute_share_key(RA);
	printf("SKB:\n");
	memcpy(keyiv, B->GetSKB(), 32);

	//Bob发送RB，SB
	if(server->Send(B->GetRB(), 65) == -1)
	{
		res = false;
	}
	if(server->Send(B->GetSB(), 32) == -1)
	{
		res = false;
	}
	
	//Bob接收SA 验证共享密钥 
	unsigned char SA[32];
	if(server->Recv(SA, 32) == -1)
	{
		res = false;
	}

	//校验SA
	if(B->checkSA(SA) == false)
	{
		res = false;
	}
	
	delete B;
	
	return res;
}

//接收KGC分发新版私钥
bool update::Recvkey()
{
	bool res = true;
	
	UserSK *encsk = new UserSK();
	memset((unsigned char *)encsk, 0, sizeof(UserSK));
	UserSK *decsk = new UserSK();
	memset((unsigned char *)decsk, 0, sizeof(UserSK));
	sm4_cbc *sm4 = new sm4_cbc(keyiv, keyiv + 16);

	if(server->Recv((unsigned char *)encsk, sizeof(UserSK)) <= 0)
	{
		printf("Recvkey():fail to recv\n");
		res = false;
	}
	
	sm4->sm4_cbc_dec((unsigned char *)encsk, (unsigned char *)decsk, sizeof(UserSK));

	//工作目录
	string dir = session_dir->getdir();

	//待验签文件名
	string msg_file = selfid + KGCID + ".msg";
	//已签字文件
	string sig_file = selfid + KGCID + ".sig";
		
	File *tempfile;
	
	tempfile = new File();
	tempfile->Write_File(dir + msg_file, (unsigned char *)decsk, 32 + (8 + 512) * 4);
	delete tempfile;

	tempfile = new File();
	tempfile->Write_File(dir + sig_file, decsk->sig, decsk->siglen);
	delete tempfile;

		 
	//验证签名
	sm9_verify *verify_st = new sm9_verify(SM9_SIGN_MPK, dir);
	if(verify_st->verify(KGCID, msg_file, sig_file) == true)
	{
		printf("Recvkey(): verify right !\n");
		res = true;
	}else
	{
		printf("Recvkey(): verify wrong !\n");
		res = false;
	}

	
	clearfile(dir + msg_file);
	clearfile(dir + sig_file);

	/*
		clearfile(selfid + ".ssk");
		clearfile(SM9_SIGN_MPK);
		clearfile(selfid + ".esk");
		clearfile(SM9_ENC_MPK);
	*/
	printf("New Key version: %s\n", decsk->version);

	dir = selfid + "/";
	createdir(dir);

	//更新签名私钥
	tempfile = new File();
	tempfile->Write_File(dir + selfid + ".ssk", decsk->ssk, decsk->ssklen);
	delete tempfile;

	//更新验签公钥
	tempfile = new File();
	tempfile->Write_File(dir + SM9_SIGN_MPK, decsk->smpk, decsk->smpklen);
	delete tempfile;

	//更新加密私钥
	tempfile = new File();
	tempfile->Write_File(dir + selfid + ".esk", decsk->esk, decsk->esklen);
	delete tempfile;

	//更新加密公钥
	tempfile = new File();
	tempfile->Write_File(dir + SM9_ENC_MPK, decsk->empk, decsk->empklen);
	delete tempfile;	
		
	delete sm4;
	delete encsk;
	delete decsk;
	
	return res;
}

