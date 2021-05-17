#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "directory.h"
#include "sm9-sign.h"
#include "sm9-verify.h"
#include "sm9-encrypt.h"
#include "sm9-decrypt.h"

#include "clear.h"
#include "two-way-auth.h"
#include "rand.h"
#include "file.h"

using namespace std;

/*
	selfid：己方ID
	peerid：对端ID
	session_dir:工作目录
*/
two_way_auth::two_way_auth(string selfid, string peerid, session_directory *session_dir)
{
	this->selfid = selfid;
	this->peerid = peerid;
	this->session_dir = session_dir;
	
	//初始化设备自身发出的挑战报文
	memset((void *)&challenge, 0, sizeof(challenge));
	memcpy(challenge.id, selfid.c_str(), selfid.size());
	memcpy(challenge.rand, session_dir->getrand256(), 32);
	
	//初始化签名结构体
	sign_st = new sm9_sign(selfid + ".ssk", session_dir->getdir());

	//初始化验签结构体
	verify_st = new sm9_verify(SM9_SIGN_MPK, session_dir->getdir());
}

two_way_auth::~two_way_auth()
{
	delete sign_st;
	delete verify_st;
}

chapmessage *two_way_auth::Getchallenge()
{
	return &challenge;
}

//对对端发来的挑战报文签字
bool two_way_auth::sign(chapmessage *message)
{
	string dir = session_dir->getdir();
	
	//文件操作
	File *tempfile;
	//待签字文件名
	string msgfile;
	//签字文件
	string sigfile;

	//对相关数据签名
	msgfile = selfid + peerid + ".msg";
	sigfile = selfid + peerid + ".sig";

	//生成待签名文件(需声明目录)
	tempfile = new File();
	tempfile->Write_File(dir + msgfile, (unsigned char *)(message->id), 64);
	delete tempfile;

	//生成签名文件
	bool res = sign_st->sign(msgfile, sigfile);

	//填写响应签名(需声明目录)
	tempfile = new File();
	tempfile->Read_File(dir + sigfile);
	message->siglen = tempfile->Get_filelen();
	memcpy(message->sig, tempfile->Get_filedata(), message->siglen);
	delete tempfile;

	clearfile(dir + msgfile);
	clearfile(dir + sigfile);

	return res;
}

//对对端发来的应答报文验签
bool two_way_auth::verify(chapmessage *message)
{
	string dir = session_dir->getdir();
	
	//文件操作
	File *tempfile;
	//待验签文件名
	string msgfile;
	//已签字文件
	string sigfile;

	//对相关数据验签
	msgfile = selfid + peerid + ".msg";
	sigfile = selfid + peerid + ".sig";

	//生成待验签文件
	tempfile = new File();
	tempfile->Write_File(dir + msgfile, (unsigned char *)(challenge.id), 64);
	delete tempfile;

	//生成已签名文件
	tempfile = new File();
	tempfile->Write_File(dir + sigfile, message->sig, message->siglen);
	delete tempfile;

	//验证签名
	bool res = verify_st->verify(peerid, msgfile, sigfile);

	clearfile(dir + msgfile);
	clearfile(dir + sigfile);

	return res;
}

