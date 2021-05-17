#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "sm9-KGC.h"

#include "net.h"
#include "threadpool.h"
#include "distribute.h"
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
distribute::distribute(UserInfo *info, session_directory *session_dir)
{
	this->info = info;
	this->session_dir = session_dir;
}

distribute::~distribute()
{
	delete session_dir;
}

//初始化与用户的TCP连接,KGC作为客户端
bool distribute::InitConnect()
{
	//初始化客户端连接
	client = new TCP_client(info->IP, DISTPORT, KGCETH);
	return true;
}

//KGC与用户进行双向身份认证
bool distribute::Authentication()
{
	two_way_auth *auth = new two_way_auth(KGCID, info->MachineID, session_dir);

	chapmessage mess;

	bool res = true;
	
	//接收节点的询问报文
	if(client->Recv((unsigned char *)&mess, sizeof(mess)) == -1)
	{
		res = false;
	}
	
	//对对端节点的询问报文签字
	if(auth->sign(&mess) == false)
	{
		res = false;
	}

	//回传签字报文
	if(client->Send((unsigned char *)&mess, sizeof(mess)) == -1)
	{
		res = false;
	}

	//发送KGC的询问报文
	if(client->Send((unsigned char *)auth->Getchallenge(), sizeof(mess)) == -1)
	{
		res = false;
	}
	
	//接收对端的签字报文
	if(client->Recv((unsigned char *)&mess, sizeof(mess)) == -1)
	{
		res = false;
	}
	
	//对签字报文验签
	if(auth->verify(&mess) == false)
	{
		res = false;
	}

	printf("peer %s is valid !\n", info->MachineID);

	delete auth;
	
	//双向认证成功
	return res;
}

//KGC与用户进行密钥协商
bool distribute::Keyagreement()
{
	bool res = true;

	string Alice = KGCID;
	string Bob = info->MachineID;

	mskmessage *agreemsk = new mskmessage();
	memset((unsigned char *)agreemsk, 0, sizeof(mskmessage));
	
	//接收Bob发来的盐值
	if(client->Recv(agreemsk->rand, 32) == -1)
	{
		res = false;
	}
	
	//Alice生成密钥协商的msk
	agreement_Alice *A = new agreement_Alice(Alice, Bob, agreemsk, session_dir->getdir());

	//ALice对msk加密
	if(A->Init() == false)
	{
		res = false;
	}

	//Alice对盐值和加密后的msk签名
	if(A->sign() == false)
	{
		res = false;
	}
	
	//Alice发送msk
	if(client->Send((unsigned char *)agreemsk, sizeof(mskmessage)) == -1)
	{
		res = false;
	}

	//Alice发送RA
	if(client->Send(A->GetRA(), 65) == -1)
	{
		res = false;
	}

	unsigned char RB[65];
	if(client->Recv(RB, 65) == -1)
	{
		res = false;
	}

	unsigned char SB[32];
	if(client->Recv(SB, 32) == -1)
	{
		res = false;
	}

	if(A->compute_share_key(RB, SB) == false)
	{
		res = false;
	}

	memcpy(keyiv, A->GetSKA(), 32);

	if(client->Send(A->GetSA(), 32) == -1)
	{
		res = false;
	}

	delete A;
	
	return res;
}

//KGC分发新版私钥
bool distribute::Keydistribute()
{
	bool res = true;

	//读取当前工作目录
	string dir = session_dir->getdir();
	
	//文件操作
	File *tempfile;
	//待签字文件名
	string msgfile;
	//签字文件
	string sigfile;

	//对相关数据签名
	msgfile = KGCID + string(info->MachineID) + ".msg";
	sigfile = KGCID + string(info->MachineID) + ".sig";

	//生成待签名文件(需声明目录)
	tempfile = new File();
	tempfile->Write_File(dir + msgfile, (unsigned char *)&(info->skmessage), 32 + (8 + 512) * 4);
	delete tempfile;
	
	//生成签名文件
	//生成私钥报文的数字签名
	sm9_sign *sign_st = new sm9_sign(string(KGCID) + ".ssk", session_dir->getdir());
	res = sign_st->sign(msgfile, sigfile);
	delete sign_st;
	
	//填写签名(需声明目录)
	tempfile = new File();
	tempfile->Read_File(dir + sigfile);
	info->skmessage.siglen = tempfile->Get_filelen();
	memcpy(info->skmessage.sig, tempfile->Get_filedata(), info->skmessage.siglen);
	delete tempfile;

	clearfile(dir + msgfile);
	clearfile(dir + sigfile);

	//对已经签字的私钥报文加密
	UserSK *encsk = new UserSK();
	memset((unsigned char *)encsk, 0, sizeof(UserSK));
	sm4_cbc *sm4 = new sm4_cbc(keyiv, keyiv + 16);
	sm4->sm4_cbc_enc((unsigned char *)&(info->skmessage), (unsigned char *)encsk, sizeof(info->skmessage));

	if(client->Send((unsigned char *)encsk, sizeof(info->skmessage)) <= 0)
	{
		printf("Keydistribute():fail to send\n");
		res = false;
	}
	
	delete sm4;
	delete encsk;
	
	return res;
}
	
void distribute::run()
{
	if(info->distributesk == false)
	{
		//该密钥为初始密钥，需要用密码卡分发
		return;
	}

	//初始化连接
	if(InitConnect() == false)
	{
		return;
	}

	//双向认证
	if(Authentication() == false)
	{
		return;
	}

	//密钥协商
	if(Keyagreement() == false)
	{
		return;
	}
	
	//密钥分发
	if(Keydistribute() == false)
	{
		return;
	}
/*
	//只有成功将密钥分发到客户端后,才将KGC存放的用户新密钥删除，否则采用密码卡分发
	string dir = string(info->MachineID) + "/";
	
	clearfile(dir + info->MachineID + ".ssk");
	clearfile(dir + SM9_SIGN_MSK);
	clearfile(dir + info->MachineID + ".esk");
	clearfile(dir + SM9_ENC_MSK);

	cleardir(dir);
*/
	//分发结束
	return;
}


