#ifndef SM9_SIGN_H_
#define SM9_SIGN_H_

#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

using namespace std;

class sm9_sign{
	private:	
		//工作目录
		string dir;
		
		//待签名文件名
		string msg_file;

		//用户私钥文件名
		string sk_file;

		//生成的签名数据文件名
		string sig_file;
			
		//待签名文件指针
		FILE *msg_fp;

		//待签名数据
		unsigned char buf[1024 * 1024];

		//待签名数据长度
		int len;
		
		//用户私钥文件指针
		FILE *sk_fp;

		//签名数据文件指针
		FILE *sig_fp;

		//摘要结构体
		EVP_MD_CTX *ctx;
		
		//用户私钥结构体
		SM9PrivateKey *sk;

		//签名数据结构体
		SM9Signature *sig;
	public:
		sm9_sign(string sk_file, string dir = "");
		virtual ~sm9_sign();
		bool sign(string msg_file, string sig_file);
};



#endif 
