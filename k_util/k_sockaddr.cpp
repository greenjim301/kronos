#include "k_sockaddr.h"

k_sockaddr::k_sockaddr()
	: m_size(sizeof(m_sockaddr))
{

}

int k_sockaddr::init(int af, k_string& ip, uint16_t port)
{
	m_sockaddr.sin_family = af;
	m_sockaddr.sin_port = htons(port);

	int ret = inet_pton(af, ip.c_str(), &m_sockaddr.sin_addr.s_addr);
	if (ret != 1)
	{
		return -1;
	}

	return 0;
}

struct sockaddr* k_sockaddr::get_sockaddr()
{
	return (struct sockaddr*)&m_sockaddr;
}

socklen_t k_sockaddr::get_size()
{
	return m_size;
}

socklen_t* k_sockaddr::get_size_ptr()
{
	return &m_size;
}

