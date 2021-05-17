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
#include "update.h"

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
	string selfid = argv[1];
	Rand256_Pool *randpool = new Rand256_Pool();
	
	//读取会话密钥
	read_selfsk(selfid);
	
	session_directory *session_dir = new session_directory(selfid, randpool->Get_Rand256_Node());
	update *up = new update(selfid, session_dir);
	
	up->run();

	delete up;
	
	while(1);
	
	return 0;
}



