#ifndef NET_H_
#define NET_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/tcp.h>

using namespace std;

class TCP_client{
	private:
		//欲连接的服务器IP
		string IP;
		//欲连接的服务端口
		int port;
		//地址结构体
		struct sockaddr_in addr;
		//通信网卡
		string eth;
		//连接成功后的文件描述符
		int sockfd;
	public:
		TCP_client(string IP, int port, string eth);
		~TCP_client();
		char *Get_Self_IP();
		void Set_Sock_Noblock();
		int Send(unsigned char *send_buffer, int len);
		int Recv(unsigned char *recv_buffer, int len);
};

class TCP_Server_Listen{
	private:
		int reuse;
		int port;
		int backlog;
		struct sockaddr_in addr;
		string eth;
		int sockfd;
	public:
		TCP_Server_Listen(int port, string eth);
		~TCP_Server_Listen();
		int Get_Listen_Fd();
		char *Get_Self_IP();
};

class TCP_Server_Accept{
	private:
		int listen_fd;
		int sockfd;
		struct sockaddr_in addr;
		socklen_t len;
	public:
		TCP_Server_Accept(int listen_fd);
		~TCP_Server_Accept();
		void Set_Sock_Noblock();
		int Send(unsigned char *send_buffer, int len);
		int Recv(unsigned char *recv_buffer, int len);
};

class UDP_Server{
	private:
		int reuse;
		int port;
		struct sockaddr_in addr;
		socklen_t addrlen;
		string eth;
		int sockfd;
	public:
		UDP_Server(int port, string eth);
		~UDP_Server();
		int Recv(unsigned char *recv_buffer, int len);
		char *Get_Self_IP();
};

class UDP_Client{
	private:
		string IP;
		int port;
		struct sockaddr_in addr;
		socklen_t addrlen;
		string eth;
		int sockfd;
	public:
		UDP_Client(string IP, int port, string eth);
		~UDP_Client();
		int Send(unsigned char *send_buffer, int len);
		char *Get_Self_IP();
};

class Multicast_Send{
	private:
		int ttl;
		string IP;
		int port;
		struct sockaddr_in addr;
		socklen_t addrlen;
		string eth;
		int sockfd;
	public:
		Multicast_Send(string IP, int port, string eth, int ttl);
		~Multicast_Send();
		int Send(unsigned char *send_buffer, int len);
		char *Get_Self_IP();
};

class Multicast_Recv{
	private:
		int reuse;
		int loop;
		int buffersize;
		string IP;
		int port;
		struct sockaddr_in addr;
		socklen_t addrlen;
		string eth;
		int sockfd;
	public:
		Multicast_Recv(string IP, int port, string eth, int buffersize, int loop);
		~Multicast_Recv();
		int Recv(unsigned char *recv_buffer, int len);
		char *Get_Self_IP();
};

void Net_print(unsigned char *buff, int len);

#endif 
