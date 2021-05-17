#ifndef UPDATE_H_
#define UPDATE_H_

#include <vector>
using namespace std;

#include "net.h"
#include "threadpool.h"
#include "directory.h"
#include "sm9-verify.h"

//密钥分发中心ID
#define KGCID		"Master"

//密钥接收网卡
#define UPDATEETH	"ens33"

//规定所有用户私钥接收监听端口为2048
#define DISTPORT	2048

//签名主私钥
#define	SM9_SIGN_MSK	"sm9sign.msk"
//签名主公钥
#define SM9_SIGN_MPK	"sm9sign.mpk"

//加密主私钥
#define SM9_ENC_MSK		"sm9enc.msk"
//加密主公钥
#define SM9_ENC_MPK		"sm9enc.mpk"

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

class update{
	private:
		//用户id
		string selfid;
		
		//会话目录
		session_directory *session_dir;

		//服务端监听套接字
		TCP_Server_Listen *server_listen;

		//服务端接收套接字
		TCP_Server_Accept *server;
		
		//协商密钥和初始向量
		unsigned char keyiv[32];
	public:
		update(string selfid, session_directory *session_dir);
		virtual ~update();

		bool run();
		
		//KGC与用户进行双向身份认证
		bool Authentication();

		//KGC与用户进行密钥协商
		bool Keyagreement();

		//接收KGC分发新版私钥
		bool Recvkey();	
		
};

#endif 
