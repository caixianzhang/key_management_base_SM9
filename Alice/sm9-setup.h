#ifndef SM9_SETUP_H_
#define SM9_SETUP_H_

#include <libgen.h>
#include <openssl/sm9.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <string>

using namespace std;

/*
	生成的密钥类型
	NID_sm9sign：数字签名密钥
	NID_sm9encrypt：非对称加密密钥
	NID_sm9keyagreement：密钥协商密钥
*/
enum TYPE{sm9sign = NID_sm9sign, sm9encrypt = NID_sm9encrypt, sm9keyagreement = NID_sm9keyagreement};

class sm9_setup{
	private:
		//工作目录
		string dir;
		
		//主密钥文件名
		string msk_file;
		//系统参数文件名
		string mpk_file;
		
		//主密钥结构体
		SM9MasterSecret *msk;
		//系统参数结构体
		SM9PublicParameters *mpk;

		//主密钥文件指针
		FILE *msk_fp;
		//系统参数文件指针
		FILE *mpk_fp;

		//密钥类型		
		enum TYPE type;
	public:
		sm9_setup(enum TYPE type, string msk_file, string mpk_file, string dir = "");
		virtual ~sm9_setup();		
};
#endif 
