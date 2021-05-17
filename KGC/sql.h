#ifndef SQL_H_
#define SQL_H_

#include <string>
#include <mysql/mysql.h>
#include <vector>

using namespace std;

#include "sm9-KGC.h"

//主机名
#define HOST	"localhost"

//用户名
#define USER	"root"

//密码
#define PASS	""

//数据库名称
#define DB_NAME	"SM9UserList"

//表名称
#define TB_NAME "MachineStatues"

#define TRUE	"true"
#define FALSE	"false"

#define UPDATE	"update"
#define ADD		"add"
#define DELETE	"delete"
#define CLEAR	"clear"

class MyDB
{
	private:
		MYSQL mysql;

		//结果集合
		MYSQL_RES *result;

		//单个行数
		MYSQL_ROW row;

		//数据库地址
		string host;

		//用户名
		string user;

		//密码
		string pass;

		//数据库名称
		string db_name;

		//表名称
		string tb_name;
	public:
		MyDB();
		~MyDB();
		//加入新设备
		bool AddMachine(string MachineID, string IP);
		//删除设备
		bool DelMachine(string MachineID);
		//获取设备IP
		string GetIP(string MachineID);
		//更改设备IP
		bool SetIP(string MachineID, string IP);
		//获取设备用户私钥版本号
		string Getversion(string MachineID);
		//更改设备用户私钥版本号
		bool Setversion(string MachineID, string version);
		//判断设备是否已经分配密钥
		string Getdistributesk(string MachineID);
		//设置设备已获取私钥
		bool Setdistributesk(string MachineID);
		//设置设备未被分发密钥
		bool Set_not_distributesk(string MachineID);
		//更新所有设备私钥版本号,并设置以分配私钥
		bool UpdateKeyVersion(string version);
		//设置所有设备已经分配私钥
		bool Updatedistributesk();
		//返回数据库中节点集合
		vector<UserInfo *> *GetUserInfoList();
};

#endif 
