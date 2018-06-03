#ifndef __K_SOCKADDR_H__
#define __K_SOCKADDR_H__

#include <stdint.h>
#include "k_string.h"
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class k_sockaddr
{
public:
	k_sockaddr();

	int init(int af, k_string& ip, uint16_t port);
	struct sockaddr* get_sockaddr();
	socklen_t get_size();
	socklen_t* get_size_ptr();

private:
	struct sockaddr_in m_sockaddr;
	socklen_t m_size;
};

#endif