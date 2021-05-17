#ifndef DISTRIBUTE_H_
#define DISTRIBUTE_H_

#include <vector>
using namespace std;

#include "net.h"
#include "threadpool.h"
#include "directory.h"

//密钥分发中心ID
#define KGCID		"Master"

//密钥分发网卡
#define KGCETH		"ens33"

//规定所有用户私钥接收监听端口为2048
#define DISTPORT	2048

//单个用户私钥网络分发
class distribute:public Task_Node{
	private:
		//会话目录
		session_directory *session_dir;
		
		//对端节点信息
		UserInfo *info;
		
		//套接字
		TCP_client *client;

		//协商密钥
		unsigned char keyiv[32];
	public:
		distribute(UserInfo *info, session_directory *session_dir);
		virtual ~distribute();

		void run();
		
		//初始化与用户的TCP连接,KGC作为客户端
		bool InitConnect();

		//KGC与用户进行双向身份认证
		bool Authentication();

		//KGC与用户进行密钥协商
		bool Keyagreement();

		//KGC分发新版私钥
		bool Keydistribute();		
};


#endif 
