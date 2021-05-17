/*
设置密码
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
*/
//进入数据库
sudo su 
mysql

//显示所有数据库
show databases;

//使用数据库
use database_name;

//显示数据库所有表
show tables;

set password for root@localhost = password("123456");
update user set password=password('123456') where user='root' and host='localhost'; 

//创建数据库
create database SM9UserList;
//删除数据库
drop database SM9UserList;
//使用数据库
use SM9UserList;
//创建数据库表
create table MachineStatues(MachineID char(32), IP char(32), version char(32), distributesk char(8), LastModifyTime timestamp);
//删除整张表
drop table MachineStatues;


//插入数据
insert into MachineStatues values("Master", "192.168.136.156", "V1.0", "false", NOW());
insert into MachineStatues values("Alice", "192.168.136.156", "V1.0", "false", NOW());
insert into MachineStatues values("Bob", "192.168.136.156", "V1.0", "false", NOW());

string sql = "insert into " + string(TB_NAME) + " values("+ "\"" + MachineID + "\"" + ", " + "\"" + IP + "\"" + ", " + "\"" + "V-.-" + "\"" + ", " + "\"" + string(FALSE) + "\"" + ", " + "NOW()" + ");";

delete from MachineStatues where MachineID = "Alice";
string sql = "delete from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";
	
update 表名称 set 列名称=新值 where 更新条件;
update MachineStatues set version = "V1.1" where MachineID = "caixianzhang";
string sql = "update " + string(TB_NAME) " set version = " + "\"" + version + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";

update MachineStatues set IP = "192.168.136.137" where MachineID = "caixianzhang";
string sql = "update " + string(TB_NAME) " set IP = " + "\"" + IP + "\"" + " where MachineID = " + "\"" + MachineID + "\"" + ";";

select * from MachineStatues;
select * from MachineStatues where MachineID = 22;

select distributesk from MachineStatues where MachineID = "Alice";
string sql = "select distributesk from " + string(TB_NAME) + " where MachineID = " + "\"" + MachineID + "\"" + ";";