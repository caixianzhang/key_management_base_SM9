#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <libgen.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sms4.h>
#include <openssl/is_gmssl.h>

#include "main.h"
#include "sql.h"
#include "sm9-KGC.h"
#include "sm4-cbc.h"
#include "sm9-encrypt.h"
#include "sm9-decrypt.h"
#include "sm9-setup.h"
#include "sm9-exchange.h"
#include "agreement.h"
#include "rand.h"
#include "clear.h"
#include "directory.h"
#include "two-way-auth.h"
#include "threadpool.h"
#include "rand.h"
#include "distribute.h"


using namespace std;

/*
	使用方法(需切换至root)：

	1.添加新节点(KGC的ID为Master不变)
	root@ubuntu:/home/caixianzhang/KGC# ./KGC add nodename(节点名称) ip(ip地址)

	2.修改节点IP
	root@ubuntu:/home/caixianzhang/KGC# ./KGC config nodename(节点名称) ip(ip地址)

	3.删除节点
	root@ubuntu:/home/caixianzhang/KGC# ./KGC delete nodename(节点名称)

	4.删除所有用户私钥文件
	root@ubuntu:/home/caixianzhang/KGC# ./KGC clear

	5.更新所有节点用户私钥
	root@ubuntu:/home/caixianzhang/KGC# ./KGC update version(版本号)

	6.帮助
	root@ubuntu:/home/caixianzhang/KGC# ./KGC help
*/

void print(unsigned char *buf, int len)
{
	for(int i = 0; i < len; i++)
	{
		if(i % 32 == 0)
		{
			printf("\n");
		}
		printf("%02X ",buf[i]);
	}
	printf("\n");
}

/*
**	测试密钥网络分发
*/
int main(int argc, char *argv[])
{	
	if(argc == 4 && string("add") == string(argv[1]))
	{
		MyDB *db = new MyDB();
		db->AddMachine(argv[2], argv[3]);
		delete db;
	}else if(argc == 4 && string("config") == string(argv[1]))
	{
		MyDB *db = new MyDB();
		db->SetIP(argv[2], argv[3]);
		delete db;
	}else if(argc == 3 && string("delete") == string(argv[1]))
	{
		MyDB *db = new MyDB();
		db->DelMachine(argv[2]);
		delete db;
	}else if(argc == 2 && string("clear") == string(argv[1]))
	{
		//执行清除私钥操作
	}else if(argc == 3 && string("update") == string(argv[1]))
	{
		/*
			更新密钥版本号，生成对应版本密钥, 如果为首次分发，则保留字节密钥，
			否则将密钥删除，并通过网络分发到相应节点。
		*/
		Rand256_Pool *randpool = new Rand256_Pool();
		ThreadPool *threadpool = new ThreadPool();
		MyDB *db = new MyDB();

		//生成密钥版本号
		db->UpdateKeyVersion(argv[2]);
		//读取用户列表
		vector<UserInfo *> *res = db->GetUserInfoList(); 

		//派生新版本密钥
		KGC *kgc = new KGC(res);
		kgc->setup();
		kgc->keygen();

		//读取会话密钥
		read_selfsk(KGCID);

		for(auto iter : *res)
		{
			if(string(iter->MachineID) == string(KGCID))
			{
				continue;
			}else
			{
				session_directory *session_dir = new session_directory(KGCID, randpool->Get_Rand256_Node());
				Task_Node *task = new distribute(iter, session_dir);
				threadpool->Add_Task_To_ThreadPool(task);
			}
		}
		
		sleep(10);
		//刷新发布状态
		db->Updatedistributesk();
		printf("we have distributed sk to all node.\n");
	
		delete kgc;
		delete db;

		while(1);
		
	}else if(argc == 2 && string("help") == string(argv[1]))
	{
		cout << "	使用方法(需切换至root)：\n" << endl;
		
		cout << "	1.添加新节点(KGC的ID为Master不变)" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC add nodename(节点名称) ip(ip地址)\n" << endl;

		cout << "	2.修改节点IP" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC config nodename(节点名称) ip(ip地址)\n" << endl;

		cout << "	3.删除节点" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC delete nodename(节点名称)\n" << endl;

		cout << "	4.删除所有用户私钥文件" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC clear\n" << endl;

		cout << "	5.更新所有节点用户私钥" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC update version(版本号)\n" << endl;

		cout << "	6.帮助" << endl;
		cout << "	e.g. root@ubuntu:/home/caixianzhang/KGC# ./KGC help\n" << endl;
	}else
	{
		cout << "输入错误: try ./KGC help" << endl;
	}	 
	return 0;
}
