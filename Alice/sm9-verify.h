#ifndef SM9_VERIFY_H_
#define SM9_VERIFY_H_

#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

using namespace std;

class sm9_verify{
	private:
		//工作目录
		string dir;
		
		//签名的用户ID
		string ID;
		
		//待签名文件名
		string msg_file;

		//待验签文件名
		string sig_file;

		//系统参数文件名
		string mpk_file;
		
		//待验签文件指针
		FILE *msg_fp;

		//生成的签名数据文件指针
		FILE *sig_fp;

		//系统参数文件指针
		FILE *mpk_fp;

		//存放待验签数据
		unsigned char buf[1024 * 1024];
		
		//待验签数据长度
		int len;

		//摘要结构体
		EVP_MD_CTX *ctx;
		
		//签名结构体
		SM9Signature *sig;

		//系统参数结构体
		SM9PublicParameters *mpk;

		//验签公钥
		SM9PublicKey *pk;
	public:
		sm9_verify(string mpk_file, string dir = "");
		virtual ~sm9_verify();
		bool verify(string ID, string msg_file, string sig_file);
};
#endif 
