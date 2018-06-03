#include "k_socket.h"
#include "k_errno.h"

#ifdef WIN32
#define  K_INVALID_SOCKET INVALID_SOCKET
#else
#define  K_INVALID_SOCKET -1
#endif

k_socket::k_socket()
	: m_sock(K_INVALID_SOCKET)
{
}

k_socket::~k_socket()
{
	if (m_sock != K_INVALID_SOCKET)
	{
#ifdef WIN32
		closesocket(m_sock);
#else
		close(m_sock);
#endif
	}
}

void k_socket::set_sock(int sock)
{
	m_sock = sock;
}

int k_socket::get_sock()
{
	return m_sock;
}

int k_socket::init(int af, int type)
{
	m_sock = socket(af, type, 0);
	if (m_sock == K_INVALID_SOCKET)
	{
		return -1;
	}

	return 0;
}

int k_socket::k_bind(k_sockaddr& sock_addr)
{
	int ret = bind(m_sock, sock_addr.get_sockaddr(), sock_addr.get_size());
	if (ret)
	{
		return -1;
	}

	return 0;
}

int k_socket::k_listen()
{
	int ret = listen(m_sock, SOMAXCONN);

	return ret;
}

int k_socket::k_accept(k_sockaddr& sock_addr, k_socket& sock)
{
	while (true)
	{
		int fd = accept(m_sock, sock_addr.get_sockaddr(), sock_addr.get_size_ptr());

		if (fd != K_INVALID_SOCKET)
		{
			sock.set_sock(fd);

			return 0;
		}
		else
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}

			return -1;
		}
	}
}

int k_socket::k_connect(k_sockaddr& sock_addr)
{
	while (true)
	{
		int ret = connect(m_sock, sock_addr.get_sockaddr(), sock_addr.get_size());

		if (ret)
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}
		}

		return ret;
	}
}

int k_socket::k_send(char* buf, int buf_size)
{
	int ret;
	int offset = 0;

	do
	{
		ret = send(m_sock, buf + offset, buf_size - offset, 0);
		if (ret < 0)
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			offset += ret;
		}

	} while (offset != buf_size);

	if (offset != buf_size)
	{
		return ret;
	}
	else
	{
		return offset;
	}
}

int k_socket::k_recv(char* buf, int buf_size)
{
	while (true)
	{
		int ret = recv(m_sock, buf, buf_size, 0);

		if (ret < 0)
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}
		}

		return ret;
	}
}

int k_socket::k_recv_n(char* buf, int buf_size)
{
	int offset = 0;
	int ret;

	while (offset < buf_size)
	{
		ret = this->k_recv(buf + offset, buf_size - offset);
		if (ret <= 0)
		{
			return -1;
		}
		offset += ret;
	}

	return 0;
}

int k_socket::k_setopt(int level, int optname, const char *optval, socklen_t optlen)
{	
	return setsockopt(m_sock, level, optname, optval, optlen);
}

#ifndef WIN32
int k_socket::k_write(char* buf, int buf_size)
{
	return write(m_sock, buf, buf_size);
}

int k_socket::k_read(char* buf, int buf_size)
{
	return read(m_sock, buf, buf_size);
}
#endif

int k_socket::k_recvfrom(char* buf, int buf_size, k_sockaddr& from_addr)
{	
	while (true)
	{
		int ret = recvfrom(m_sock, buf, buf_size, 0,
			from_addr.get_sockaddr(), from_addr.get_size_ptr());

		if (ret < 0)
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}
		}

		return ret;
	}
	
}

int k_socket::k_sendto(char* buf, int buf_size, k_sockaddr& to_addr)
{
	int ret;
	int offset = 0;

	do
	{
		ret = sendto(m_sock, buf + offset, buf_size - offset, 0,
			to_addr.get_sockaddr(), to_addr.get_size());
		if (ret < 0)
		{
			int err = k_errno::last_error();
			if (k_errno::is_retry_error(err))
			{
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			offset += ret;
		}
		
	} while (offset != buf_size);

	if (offset != buf_size)
	{
		return ret;
	} 
	else
	{
		return offset;
	}
}
