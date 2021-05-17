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

#include "net.h"

using namespace std;

TCP_client::TCP_client(string IP, int port, string eth)

{
	//填写IP地址，端口号，通信网卡
	this->IP = IP;
	this->port = port;
	this->eth = eth;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("TCP_client::TCP_client():fail to create sockfd!!! %m \n");
	}
	
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(0);

	inet_pton(AF_INET, Get_Self_IP(), &local.sin_addr);

	if(bind(sockfd, (struct sockaddr *)&local, sizeof(local)) == -1)
	{
		printf("TCP_client::TCP_client():fail to bind eth!!! %m\n");
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	inet_pton(AF_INET, IP.c_str(), &addr.sin_addr);
	
	if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("TCP_client::TCP_client():fail to connect to server!!! %m\n");
	}
}

TCP_client::~TCP_client()

{
	close(sockfd);
}

char *TCP_client::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}

void TCP_client::Set_Sock_Noblock()
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);
}

int TCP_client::Send(unsigned char *send_buffer, int len)
{
	int ret = 0;
	int sendlen = 0;
	while(sendlen < len)
	{
		ret = send(sockfd, send_buffer + sendlen, len - sendlen, MSG_NOSIGNAL);
		if(ret == 0)
		{
			return sendlen;
		}else if(ret < 0)
		{
			if(errno == EAGAIN)
			{
				continue;
			}else
			{
				return -1;
			}
		}else
		{
			sendlen += ret;
		}
	}
	return sendlen;	
}

int TCP_client::Recv(unsigned char *recv_buffer, int len)
{
	int ret = 0;
	int recvlen = 0;
	while(recvlen < len)
	{
		ret = recv(sockfd, recv_buffer + recvlen, len - recvlen, 0);
		if(ret == 0)
		{
			return recvlen;
		}else if(ret < 0)
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			{
				return -1;
			}else
			{
				continue;
			}
		}else
		{
			recvlen += ret;
			if(recvlen == len)
			{
				return recvlen;
			}
		}
	}
	return recvlen;
}

TCP_Server_Listen::TCP_Server_Listen(int port, string eth)
{
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("TCP_Server_Listen::TCP_Server_Listen():fail to create sockfd!!! %m\n");
		close(sockfd);
	}

	//设置地址复用
	reuse = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
	{
		printf("TCP_Server_Listen::TCP_Server_Listen():fail to set sockfd reuse!!! %m\n");
		close(sockfd);
	}

	this->port = port;
	this->eth = eth;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, Get_Self_IP(), &addr.sin_addr);

	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("TCP_Server_Listen::TCP_Server_Listen():fail to set bind sockfd!!! %m\n");
		close(sockfd);
	}

	backlog = 5; 
	if(listen(sockfd, backlog) == -1)
	{
		printf("TCP_Server_Listen::TCP_Server_Listen():fail to set listen sockfd!!! %m\n");
		close(sockfd);
	}
}

TCP_Server_Listen::~TCP_Server_Listen()
{
	close(sockfd);
}

//返回监听套接字
int TCP_Server_Listen::Get_Listen_Fd()
{
	return sockfd;
}

char *TCP_Server_Listen::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}

TCP_Server_Accept::TCP_Server_Accept(int listen_fd)
{
	this->listen_fd = listen_fd;
	char str[INET_ADDRSTRLEN];
	memset(str, 0, INET_ADDRSTRLEN);
	sockfd = accept(listen_fd, (struct sockaddr *)&addr, &len);
	if(sockfd == -1)
	{
		printf("TCP_Server_Accept::TCP_Server_Accept():fail to accept!!!\n");
	}
	inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
	printf("Client: %s Connect !!!\n",str);
}

TCP_Server_Accept::~TCP_Server_Accept()
{
	close(sockfd);
}

void TCP_Server_Accept::Set_Sock_Noblock()
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(sockfd, F_SETFL, flags);
}

int TCP_Server_Accept::Send(unsigned char *send_buffer, int len)
{
	int ret = 0;
	int sendlen = 0;
	while(sendlen < len)
	{
		ret = send(sockfd, send_buffer + sendlen, len - sendlen, MSG_NOSIGNAL);
		if(ret == 0)
		{
			return sendlen;
		}else if(ret < 0)
		{
			if(errno == EAGAIN)
			{
				continue;
			}else
			{
				return -1;
			}
		}else
		{
			sendlen += ret;
		}
	}
	return sendlen;	
}

int TCP_Server_Accept::Recv(unsigned char *recv_buffer, int len)
{
	int ret = 0;
	int recvlen = 0;
	while(recvlen < len)
	{
		ret = recv(sockfd, recv_buffer + recvlen, len - recvlen, 0);
		if(ret == 0)
		{
			return recvlen;
		}else if(ret < 0)
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			{
				return -1;
			}else
			{
				continue;
			}
		}else
		{
			recvlen += ret;
			if(recvlen == len)
			{
				return recvlen;
			}
		}
	}
	return recvlen;
}


UDP_Server::UDP_Server(int port, string eth)
{
	this->port = port;
	this->eth = eth;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		printf("UDP_Server::UDP_Server(): fail to create sockfd !!! %m\n");
	}
	
    reuse = 1;	
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addrlen = sizeof(struct sockaddr);
	inet_pton(AF_INET, Get_Self_IP(), &addr.sin_addr);
	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("UDP_Server::UDP_Server(): bind fail !!! %m\n");
	}
}

UDP_Server::~UDP_Server()
{
	close(sockfd);
}

int UDP_Server::Recv(unsigned char *recv_buffer, int len)
{
	return recvfrom(sockfd, recv_buffer, len, 0, (struct sockaddr *)&addr, &addrlen);
}

char *UDP_Server::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}

UDP_Client::UDP_Client(string IP, int port, string eth)

{
	this->IP = IP;
	this->port = port;
	this->eth = eth;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		printf("UDP_Client::UDP_Client(): fail to create sockfd!!!\n");
	}
	
	addr.sin_family = AF_INET;	
	addr.sin_port = htons(port);
	inet_pton(AF_INET, IP.c_str(), &addr.sin_addr);
	
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(0);
	inet_pton(AF_INET, Get_Self_IP(), &local.sin_addr);

	if(bind(sockfd, (struct sockaddr *)&local, sizeof(local)) == -1)
	{
		printf("UDP_Client::UDP_Client():fail to bind %m\n");
	}
}

UDP_Client::~UDP_Client()
{
	close(sockfd);
}

int UDP_Client::Send(unsigned char *send_buffer, int len)
{
	return sendto(sockfd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(addr));
}

char *UDP_Client::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}

Multicast_Send::Multicast_Send(string IP, int port, string eth, int ttl)

{
	this->IP = IP;
	this->port = port;
	this->eth = eth;
	this->ttl = ttl;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		printf("Multicast_Send::Multicast_Send(): fail to create sockfd!!! %m\n");
	}
	
	setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	inet_pton(AF_INET, this->IP.c_str(), &addr.sin_addr);
	
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(0);
	
	inet_pton(AF_INET, Get_Self_IP(), &local.sin_addr);

	if(bind(sockfd, (struct sockaddr *)&local, sizeof(local)) == -1)
	{
		printf("Multicast_Send::Multicast_Send(): fail to bind !!! %m \n");
	}
}

Multicast_Send::~Multicast_Send()
{
	close(sockfd);
}

int Multicast_Send::Send(unsigned char *send_buffer, int len)
{
	return sendto(sockfd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(addr));
}

char *Multicast_Send::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}


Multicast_Recv::Multicast_Recv(string IP, int port, string eth, int buffersize, int loop)

{
	this->IP= IP;
	this->port = port;
	this->eth = eth;
	this->buffersize = buffersize;
	this->loop = loop;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		printf("Multicast_Recv::Multicast_Recv(): fail to create sockfd !!! %m\n");
	}

	setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &this->loop, sizeof(this->loop));
	
	reuse = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	if(buffersize != 0)
	{
		this->buffersize = buffersize;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &this->buffersize, sizeof(this->buffersize));
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	inet_pton(AF_INET, this->IP.c_str(), &addr.sin_addr);

	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("Multicast_Recv::Multicast_Recv(): fail to bind !!! %m\n");
	}
	
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_interface.s_addr = inet_addr(Get_Self_IP());
	inet_pton(AF_INET, this->IP.c_str(), &mreq.imr_multiaddr);
	setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

Multicast_Recv::~Multicast_Recv()
{
	close(sockfd);
}

int Multicast_Recv::Recv(unsigned char *recv_buffer, int len)
{
	return recvfrom(sockfd, recv_buffer, len, 0, (struct sockaddr *)&addr, &addrlen);
}

char *Multicast_Recv::Get_Self_IP()
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, eth.c_str(), eth.size());
	ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close(fd);
	return inet_ntoa(sin.sin_addr);
}

void Net_print(unsigned char *buff, int len)
{
	int i = 0;
	printf("\n");
	for(i = 0;i < len; i++)
	{
		if(i % 16 == 0)
		{
			printf("\n");
		}
		printf("0x%02X ", buff[i]);
	}
	printf("\n\n");
}

