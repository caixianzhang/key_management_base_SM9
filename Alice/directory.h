#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <string>
#include "rand.h"

using namespace std;

//签名主私钥
#define	SM9_SIGN_MSK	"sm9sign.msk"
//签名主公钥
#define SM9_SIGN_MPK	"sm9sign.mpk"

//加密主私钥
#define SM9_ENC_MSK		"sm9enc.msk"
//加密主公钥
#define SM9_ENC_MPK		"sm9enc.mpk"

struct SelfSK{
	//用户签名私钥长度
	size_t ssklen;
	//用户签名私钥
	unsigned char ssk[512];
	
	//验签主公钥长度
	size_t smpklen;
	//验签主公钥
	unsigned char smpk[512];

	//用户解密私钥长度
	size_t esklen;
	//用户解密私钥
	unsigned char esk[512];

	//加密主公钥长度
	size_t empklen;
	//加密主公钥
	unsigned char empk[512];
};

//初始化个人私钥信息
bool read_selfsk(string selfid);

//更新个人私钥
bool update_selfsk(string selfid);


//创建目录
bool createdir(string dir);

//删除目录
bool removedir(string dir);

//清空目录
bool cleardir(string dir);

//创建临时的回话目录(用于双向认证，密钥协商)
class session_directory{
	private:
		//会话盐值
		Rand256_Node *rand;
		
		//自身id
		string selfid;
		
		//回话目录名(通常根据随机数确定)
		string dir;
		
		//写私钥到会话目录
		bool gen_selfsk();
		
		//删除会话目录的私钥
		bool del_selfsk();
	public:
		session_directory(string selfid, Rand256_Node *rand);
		virtual ~session_directory();
		unsigned char *getrand256();
		string getdir();
};

#endif 

