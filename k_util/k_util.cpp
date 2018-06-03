#include "k_util.h"
#ifdef WIN32
#include <WinSock2.h>
#endif

int k_util::init()
{
#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return -1;
	}
#endif

	return 0;
}

void k_util::cleanup()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void k_util::k_sleep(int seconds)
{
#ifdef WIN32
	Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
}

void k_util::avio_w8(uint8_t*& s, int b)
{
	*s++ = b;
}

void k_util::avio_wb16(uint8_t*& s, unsigned int val)
{
	avio_w8(s, (int)val >> 8);
	avio_w8(s, (uint8_t)val);
}

void k_util::avio_wb32(uint8_t*& s, unsigned int val)
{
	avio_w8(s, val >> 24);
	avio_w8(s, (uint8_t)(val >> 16));
	avio_w8(s, (uint8_t)(val >> 8));
	avio_w8(s, (uint8_t)val);
}

