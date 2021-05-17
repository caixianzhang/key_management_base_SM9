#include <stdio.h>
#include <sys/stat.h> 
#include <string>
#include <unistd.h>
#include <string.h>

#include "directory.h"
#include "sm9-KGC.h"
#include "file.h"
#include "clear.h"
#include "rand.h"

/*
	说明:对于KGC或节点
	根目录一直存放会话密钥，
	当需要生成新密钥时候，将密传的新密钥，
	写入用户ID命名的文件夹下。
*/

using namespace std;

static SelfSK *selfsk = NULL;

bool createdir(string dir)
{
	if(mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
	{
		printf("bool createdir(string dir): fail to mkdir %s %m\n", dir.c_str());
		return false;
	}
	return true;
}

bool removedir(string dir)
{
	if(rmdir(dir.c_str()) == -1)
	{
		printf("bool removedir(string dir): fail to rmdir %s %m\n", dir.c_str());
		return false;
	}
	return true;
}

bool cleardir(string dir)
{
	return true;
}

//读取自身的sk(无论节点还是KGC，当前会话密钥均存放在根目录)
bool read_selfsk(string selfid)
{
	selfsk = new SelfSK();
	memset(selfsk, 0, sizeof(SelfSK));
	
	File *tempfile;
			
	tempfile = new File();
	tempfile->Read_File(selfid + ".ssk");
	selfsk->ssklen = tempfile->Get_filelen();
	memcpy(selfsk->ssk, tempfile->Get_filedata(), selfsk->ssklen);
	delete tempfile;
	
	tempfile = new File();
	tempfile->Read_File(SM9_SIGN_MPK);
	selfsk->smpklen = tempfile->Get_filelen();
	memcpy(selfsk->smpk, tempfile->Get_filedata(), selfsk->smpklen);
	delete tempfile;
	
	tempfile = new File();
	tempfile->Read_File(selfid + ".esk");
	selfsk->esklen = tempfile->Get_filelen();
	memcpy(selfsk->esk, tempfile->Get_filedata(), selfsk->esklen);
	delete tempfile;

	tempfile = new File();
	tempfile->Read_File(SM9_ENC_MPK);
	selfsk->empklen = tempfile->Get_filelen();
	memcpy(selfsk->empk, tempfile->Get_filedata(), selfsk->empklen);
	delete tempfile;	
	
	return true;
}

//更新根目录下的会话密钥
bool update_selfsk(string selfid)
{
	//清除根目录下的原始密钥文件
	clearfile(selfid + ".ssk");
	clearfile(SM9_SIGN_MPK);
	clearfile(selfid + ".esk");
	clearfile(SM9_ENC_MPK);

	
	File *tempfile;
	string dir = selfid + "/";
	
	//拷贝新密钥
	tempfile = new File();
	tempfile->Read_File(dir + selfid + ".ssk");
	tempfile->Write_File(selfid + ".ssk", tempfile->Get_filedata(), tempfile->Get_filelen());
	delete tempfile;

	tempfile = new File();
	tempfile->Read_File(dir + SM9_SIGN_MPK);
	tempfile->Write_File(SM9_SIGN_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
	delete tempfile;
	
	tempfile = new File();
	tempfile->Read_File(dir + selfid + ".esk");
	tempfile->Write_File(selfid + ".esk", tempfile->Get_filedata(), tempfile->Get_filelen());
	delete tempfile;

	tempfile = new File();
	tempfile->Read_File(dir + SM9_ENC_MPK);
	tempfile->Write_File(SM9_ENC_MPK, tempfile->Get_filedata(), tempfile->Get_filelen());
	delete tempfile;
	
	return true;
}

//写私钥到会话目录
bool session_directory::gen_selfsk()
{
	File *tempfile;

	tempfile = new File();
	tempfile->Write_File(dir + selfid + ".ssk", selfsk->ssk, selfsk->ssklen);
	delete tempfile;

	tempfile = new File();
	tempfile->Write_File(dir + SM9_SIGN_MPK, selfsk->smpk, selfsk->smpklen);
	delete tempfile;
	
	tempfile = new File();
	tempfile->Write_File(dir + selfid + ".esk", selfsk->esk, selfsk->esklen);
	delete tempfile;

	tempfile = new File();
	tempfile->Write_File(dir + SM9_ENC_MPK, selfsk->empk, selfsk->empklen);
	delete tempfile;
	
	return true;
}
		
//删除会话目录的私钥
bool session_directory::del_selfsk()
{
	clearfile(dir + selfid + ".ssk");
	clearfile(dir + SM9_SIGN_MPK);
	clearfile(dir + selfid + ".esk");
	clearfile(dir + SM9_ENC_MPK);
	return true;
}

session_directory::session_directory(string selfid, Rand256_Node *rand)
{
	this->rand = rand;
	this->selfid = selfid;
	dir = to_string(*(unsigned int *)rand->Get_Rand256()) + "/";
	
	createdir(dir);
	gen_selfsk();
}

session_directory::~session_directory()
{
	delete rand;
	del_selfsk();
	removedir(dir);
}

unsigned char *session_directory::getrand256()
{
	return rand->Get_Rand256();
}

string session_directory::getdir()
{
	return dir;
}

