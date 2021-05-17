#ifndef SM9_KGC_H_
#define SM9_KGC_H_

#include <vector>

using namespace std; 

//分发给用户的私钥报文
struct UserSK{
	//用户私钥版本号
	unsigned char version[32];

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
	
	//数字签名长度
	size_t siglen;
	//数字签名
	unsigned char sig[512 + 8];
};

//用户相关信息
struct UserInfo{
	//用户ID
	char MachineID[32];

	//用户IP
	char IP[32];
	
	//是否已分配用户私钥.
	bool distributesk;

	//待发送的用户私钥消息报文
	UserSK skmessage;
};


//用户私钥生成中心
class KGC{
	private:
		vector<UserInfo *> *userlist;
	public:
		KGC(vector<UserInfo *> *userlist);
		virtual ~KGC();
		//生成主密钥,和公钥系统参数
		bool setup();
		//根据主密钥生成用户私钥
		bool keygen();
		//生成用户私钥分发报文
		bool genskmessage();
};

#endif
