#ifndef TWO_WAY_AUTH_H_
#define TWO_WAY_AUTU_H_

#include <string>
#include "sm9-sign.h"
#include "sm9-verify.h"
#include "directory.h"

using namespace std;

//挑战应答报文
struct chapmessage{	
	//数字签名长度
	size_t siglen;

	//询问方填写自己的ID
	char id[32];
	//询问方填写应答方待签名的随机数
	unsigned char rand[32];
	
	//数字签名
	unsigned char sig[512];
};

/*
	本模块完成两方身份相互认证
*/
class two_way_auth{
	private:
		//会话目录
		session_directory *session_dir;
		
		//本端节点id
		string selfid;
		
		//对端节点id
		string peerid;

		//挑战数据报
		chapmessage challenge;

		//签名结构体
		sm9_sign *sign_st;

		//验签结构体
		sm9_verify *verify_st;
	public:
		two_way_auth(string selfid, string peerid, session_directory *session_dir);
		virtual ~two_way_auth();
		
		//读取本端需要发出的挑战报文
		chapmessage *Getchallenge();
		
		//对对端发来的挑战报文签字
		bool sign(chapmessage *message);
		//对对端发来的应答报文验签
		bool verify(chapmessage *message);
};

#endif 
