#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <vector>

#include "sql.h"
#include "sm9-KGC.h"

using namespace std;
 
MyDB::MyDB()
{
	//数据库地址
	string host = HOST;

	//用户名
	string user = USER;

	//密码
	string pass = PASS;

	//数据库名称
	string db_name = DB_NAME;

	//表名称
	string tb_name = TB_NAME;
	
	//初始化数据库连接变量
	if(mysql_init(&mysql) == NULL)
	{
		cout << "MyDB::MyDB() Init Error:" << mysql_error(&mysql) << endl;
	}
	
	if(mysql_real_connect(&mysql, host.c_str(), user.c_str(), pass.c_str(), db_name.c_str(), 0, NULL, 0) == NULL)
	{
		cout << "MyDB::MyDB() Connect Error:" << mysql_error(&mysql) << endl;
	}
}
 
MyDB::~MyDB()
{
	//关闭数据库连接
	mysql_close(&mysql);
} 

//加入新设备
bool MyDB::AddMachine(string MachineID, string IP)
{
	string sql = "insert into " + string(TB_NAME) + " values("+ "\"" + MachineID + "\"" + ", " + "\"" + IP + "\"" + ", " + "\"" + "V-.-" + "\"" + ", " + "\"" + string(FALSE) + "\"" + ", " + "NOW()" + ");";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}

//删除设备
bool MyDB::DelMachine(string MachineID)
{
	string sql = "delete from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}

/*
	获取设备IP
	成功:返回IP
	失败:返回空字符串
*/
string MyDB::GetIP(string MachineID)
{
	string res = "";
	//查询节点IP
	string sql = "select IP from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return res;
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		row = mysql_fetch_row(result);
		if(row == NULL)
		{
			mysql_free_result(result);
			return res;
		}
		res = row[0];
		mysql_free_result(result);
	}
	return res;
}

//更改设备IP
bool MyDB::SetIP(string MachineID, string IP)
{
	string sql = "update " + string(TB_NAME) + " set IP = " + "\"" + IP + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}

//获取设备用户私钥版本号
string MyDB::Getversion(string MachineID)
{
	string res = "";
	//查询节点IP
	string sql = "select version from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return res;
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		row = mysql_fetch_row(result);
		if(row == NULL)
		{
			mysql_free_result(result);
			return res;
		}else
		{
			res = row[0];
			mysql_free_result(result);
			return res;
		}
	}
}

//更改设备用户私钥版本号
bool MyDB::Setversion(string MachineID, string version)
{
	string sql = "update " + string(TB_NAME) + " set version = " + "\"" + version + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}

/*
	判断节点是否已分配用户私钥
	成功：返回"true" or "false"
	失败：返回空字符串
*/
string MyDB::Getdistributesk(string MachineID)
{
	string res = "";
	//查询节点是否为新加入节点
	string sql = "select distributesk from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return res;
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		row = mysql_fetch_row(result);
		if(row == NULL)
		{
			mysql_free_result(result);
			return res;
		}else
		{
			res = row[0];
			mysql_free_result(result);
			return res;
		}
	}	
}

//设置设备已经获取私钥
bool MyDB::Setdistributesk(string MachineID)
{
	string sql = "update " + string(TB_NAME) + " set distributesk = " + "\"" + string(TRUE) + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}

//设置设备未被分发私钥
bool MyDB::Set_not_distributesk(string MachineID)
{
	string sql = "update " + string(TB_NAME) + " set distributesk = " + "\"" + string(FALSE) + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return false;
	}
	return true;
}


//更新所有设备私钥版本号，并设置以分配私钥
bool MyDB::UpdateKeyVersion(string version)
{
	//查询所有现存节点
	vector<string> res;
	
	string sql = "select MachineID from " + string(TB_NAME) + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		while(1)
		{
			row = mysql_fetch_row(result);
			if(row == NULL)
			{
				break;
			}
			//读取用户ID
			res.push_back(row[0]);
		}
		mysql_free_result(result);
	}

	//设置用户私钥版本
	for(auto iter : res)
	{
		Setversion(iter, version);
	}
	
	return true;
}

bool MyDB::Updatedistributesk()
{
	//查询所有现存节点
	vector<string> res;
	
	string sql = "select MachineID from " + string(TB_NAME) + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		while(1)
		{
			row = mysql_fetch_row(result);
			if(row == NULL)
			{
				break;
			}
			//读取用户ID
			res.push_back(row[0]);
		}
		mysql_free_result(result);
	}
	
	//设置私钥分发状态
	for(auto iter : res)
	{
		Setdistributesk(iter);
	}
	
	return true;
}


//返回数据库中节点集合
vector<UserInfo *> *MyDB::GetUserInfoList()
{
	vector<UserInfo *> *res = new vector<UserInfo *>();
	//查询节点是否为新加入节点
	string sql = "select * from " + string(TB_NAME) + ";";
	cout << sql << endl;
	if(mysql_query(&mysql, sql.c_str()))
	{
		cout << "Query Error:" << mysql_error(&mysql);
		return res;
	}else
	{
		//获取结果集
		result = mysql_use_result(&mysql);
		while(1)
		{
			row = mysql_fetch_row(result);
			if(row == NULL)
			{
				break;
			}
			//读取用户ID
			string ID = row[0];

			//读取用户IP
			string IP = row[1];

			//读取此次密钥版本号
			string ver = row[2];
			
			//读取用户私钥分配状态
			string distsk = row[3];
			
			UserInfo *tempUser = new UserInfo();
			memset(tempUser, 0, sizeof(UserInfo));

			//填写用户名
			memset(tempUser->MachineID, 0, 32);
			memcpy(tempUser->MachineID, ID.c_str(), ID.size());

			//填写用户IP
			memset(tempUser->IP, 0, 32);
			memcpy(tempUser->IP, IP.c_str(), IP.size());

			//填写密钥版本号
			memset(tempUser->skmessage.version, 0, 32);
			memcpy(tempUser->skmessage.version, ver.c_str(), ver.size());
			
			//填写密钥分配状态
			if(distsk == string(TRUE))
			{
				tempUser->distributesk = true;
			}else
			{
				tempUser->distributesk = false;
			}
			(*res).push_back(tempUser);
		}
		mysql_free_result(result);
	}
	return res;
}

