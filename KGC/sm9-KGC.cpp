#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "sm9-keygen.h"
#include "sm9-setup.h"

#include "sql.h"
#include "clear.h"
#include "file.h"
#include "sm9-KGC.h"
#include "directory.h"
#include "distribute.h"

/*
	KGC模块默认密钥在工作目录下生成
*/
KGC::KGC(vector<UserInfo *> *userlist)
{
	this->userlist = userlist;
}

KGC::~KGC()
{
	//释放用户列表内存空间
	for(auto iter:(*userlist))
	{
		delete iter;
	}
	delete userlist;
}

//生成主密钥,和公钥系统参数(执行初始化过程前，需要清空相关目录)
bool KGC::setup()
{
	//KGC目录下生成密钥对
	string kgc_dir = string(KGCID) + "/";
	createdir(kgc_dir);
	//生成签名主私钥，签名主公钥系统参数 
	delete (new sm9_setup(sm9sign, SM9_SIGN_MSK, SM9_SIGN_MPK, kgc_dir));
	//生成加密主私钥，加密主公钥系统参数 
	delete (new sm9_setup(sm9encrypt, SM9_ENC_MSK, SM9_ENC_MPK, kgc_dir));

	File *tempfile;
	//各个用户的文件夹均分发主密钥对的副本
	for(auto iter:(*userlist))
	{
		if(string(iter->MachineID) != string(KGCID))
		{
			//生成对应的用户目录
			string userdir = string(iter->MachineID) + "/"; 
			createdir(userdir);
			
			//复制签名主私钥
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_SIGN_MSK);
			tempfile->Write_File(userdir + SM9_SIGN_MSK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;

			//复制签名主公钥
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_SIGN_MPK);
			tempfile->Write_File(userdir + SM9_SIGN_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;

			//复制加密主私钥
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_ENC_MSK);
			tempfile->Write_File(userdir + SM9_ENC_MSK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;

			//复制加密主公钥
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_ENC_MPK);
			tempfile->Write_File(userdir + SM9_ENC_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;
		}
	}
	
	return true;
}

//根据主密钥生成用户私钥
bool KGC::keygen()
{
	string kgc_dir = string(KGCID) +  "/";
	//生成用户签名私钥
	delete (new sm9_keygen(sm9sign, SM9_SIGN_MSK, KGCID, kgc_dir));
	//生成用户加密私钥
	delete (new sm9_keygen(sm9encrypt, SM9_ENC_MSK, KGCID, kgc_dir));

	clearfile(kgc_dir + SM9_SIGN_MSK);
	clearfile(kgc_dir + SM9_ENC_MSK);
	
	for(auto iter:(*userlist))
	{
		string userdir = string(iter->MachineID) + "/"; 
		File *tempfile;
		
		if(string(iter->MachineID) != string(KGCID))
		{
			delete (new sm9_keygen(sm9sign, SM9_SIGN_MSK, iter->MachineID, userdir));
			delete (new sm9_keygen(sm9encrypt, SM9_ENC_MSK, iter->MachineID, userdir));

			clearfile(userdir + SM9_SIGN_MSK);
			clearfile(userdir + SM9_ENC_MSK);
		}
		if(string(iter->MachineID) == string(KGCID) && iter->distributesk == false)
		{
			string kgc_dir = string(KGCID) + "/";

			//复制KGC的签名私钥到根目录
			tempfile = new File();
			tempfile->Read_File(kgc_dir + KGCID + ".ssk");
			tempfile->Write_File(string(KGCID) + ".ssk", tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;
			
			//复制KGC的签名公钥到根目录
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_SIGN_MPK);
			tempfile->Write_File(SM9_SIGN_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;
			
			//复制KGC的加密私钥到根目录
			tempfile = new File();
			tempfile->Read_File(kgc_dir + KGCID + ".esk");
			tempfile->Write_File(string(KGCID) + ".esk", tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;
			
			//复制KGC的加密公钥到根目录
			tempfile = new File();
			tempfile->Read_File(kgc_dir + SM9_ENC_MPK);
			tempfile->Write_File(SM9_ENC_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
			delete tempfile;
		}

		//生成密钥分发报文
		tempfile = new File();
		tempfile->Read_File(userdir + iter->MachineID + ".ssk");
		iter->skmessage.ssklen = tempfile->Get_filelen();
		memcpy(iter->skmessage.ssk, tempfile->Get_filedata(), iter->skmessage.ssklen);
		delete tempfile;
		
		tempfile = new File();
		tempfile->Read_File(userdir + SM9_SIGN_MPK);
		iter->skmessage.smpklen = tempfile->Get_filelen();
		memcpy(iter->skmessage.smpk, tempfile->Get_filedata(), iter->skmessage.smpklen);
		delete tempfile;
		
		tempfile = new File();
		tempfile->Read_File(userdir + iter->MachineID + ".esk");
		iter->skmessage.esklen = tempfile->Get_filelen();
		memcpy(iter->skmessage.esk, tempfile->Get_filedata(), iter->skmessage.esklen);
		delete tempfile;
		
		tempfile = new File();
		tempfile->Read_File(userdir + SM9_ENC_MPK);
		iter->skmessage.empklen = tempfile->Get_filelen();
		memcpy(iter->skmessage.empk, tempfile->Get_filedata(), iter->skmessage.empklen);
		delete tempfile;		
	}
	return true;
}

//生成用户私钥分发报文
bool KGC::genskmessage()
{
	File *tempfile;
	string filename;
	for(auto iter:(*userlist))
	{
		if(string(iter->MachineID) != string(KGCID))
		{
			string userdir = string(iter->MachineID) + "/"; 
			//只有已经分发过的密钥才能通过网络分发
			if(iter->distributesk == true)
			{
				//读取签名私钥
				tempfile = new File();
				filename = userdir + string(iter->MachineID) + ".ssk";
				tempfile->Read_File(filename);

				//填写签名私钥长度
				iter->skmessage.ssklen = tempfile->Get_filelen();
				//填写签名私钥
				memcpy(iter->skmessage.ssk, tempfile->Get_filedata(), iter->skmessage.ssklen);
				delete tempfile;

				//读取验签主公钥
				tempfile = new File();
				filename = userdir + SM9_SIGN_MPK;
				tempfile->Read_File(filename);

				//填写验签主公钥长度
				iter->skmessage.smpklen = tempfile->Get_filelen();
				//填写验签主公钥
				memcpy(iter->skmessage.smpk, tempfile->Get_filedata(), iter->skmessage.smpklen);
				delete tempfile;
				
				//读取加密私钥
				tempfile = new File();
				filename = userdir + string(iter->MachineID) + ".esk";
				tempfile->Read_File(filename);

				//填写用户加密私钥长度
				iter->skmessage.esklen = tempfile->Get_filelen();
				//填写用户加密私钥
				memcpy(iter->skmessage.esk, tempfile->Get_filedata(), iter->skmessage.esklen);
				delete tempfile;

				//读取加密主公钥
				tempfile = new File();
				filename = userdir + SM9_ENC_MPK;
				tempfile->Read_File(filename);

				//填写验签主公钥长度
				iter->skmessage.empklen = tempfile->Get_filelen();
				//填写验签主公钥
				memcpy(iter->skmessage.empk, tempfile->Get_filedata(), iter->skmessage.empklen);
				delete tempfile;
			}
		}
	}
	return true;
}

