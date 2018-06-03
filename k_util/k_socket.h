#ifndef __K_SOCKET_H__
#define __K_SOCKET_H__

#include "k_sockaddr.h"

class k_socket
{
public:
	k_socket();
	~k_socket();
	
	void set_sock(int sock);
	int get_sock();

	int init(int af, int type);
	int k_bind(k_sockaddr& sock_addr);
	int k_listen();
	int k_accept(k_sockaddr& sock_addr, k_socket& sock);
	int k_connect(k_sockaddr& sock_addr);
	int k_send(char* buf, int buf_size);
	int k_recv(char* buf, int buf_size);
	int k_recv_n(char* buf, int buf_size);
	int k_setopt(int level, int optname, const char *optval, socklen_t optlen);

#ifndef WIN32
	int k_write(char* buf, int buf_size);
	int k_read(char* buf, int buf_size);
#endif

	//udp½Ó¿Ú
	int k_recvfrom(char* buf, int buf_size, k_sockaddr& from_addr);
	int k_sendto(char* buf, int buf_size, k_sockaddr& to_addr);


private:
	int m_sock;
};

#endif
