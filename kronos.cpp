// kronos.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#include "stdafx.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "k_util/k_util.h"
#include "k_util/k_socket.h"
#include "k_accept_handler.h"
#include "k_mobile_handler.h"
#include "k_rtsp_handler.h"
#include "k_media_server.h"
#include "k_util/k_errno.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		return -1;
	}

	if (k_util::init())
	{
		return -1;
	}

	k_string server_ip(argv[1]);
	uint16_t mobile_port = atoi(argv[2]);
	k_socket* mobile_sock = new k_socket;
	k_sockaddr mobile_addr;

	mobile_addr.init(AF_INET, server_ip, mobile_port);
	mobile_sock->init(AF_INET, SOCK_STREAM);
	
	int val = 1;
	mobile_sock->k_setopt(SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));
	
	mobile_sock->k_bind(mobile_addr);
	mobile_sock->k_listen();

	printf("mobile listen\n");

	k_media_server* media_server = new k_media_server;
	k_accept_handler<k_mobile_handler>* mobile_handler = new k_accept_handler<k_mobile_handler>;

	media_server->init();
	media_server->add_event(mobile_sock, mobile_handler, k_event::ACCEPT_MASK);

	//rtsp
	k_socket* rtsp_sock = new k_socket;
	rtsp_sock->init(AF_INET, SOCK_STREAM);
	val = 1;
	rtsp_sock->k_setopt(SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));

	uint16_t rtsp_port = 5544;
	k_sockaddr rtsp_addr;
	rtsp_addr.init(AF_INET, server_ip, rtsp_port);

	if (rtsp_sock->k_bind(rtsp_addr))
	{
		printf("bind err:%d\n", k_errno::last_error());
	}
	if (rtsp_sock->k_listen())
	{
		printf("listen err:%d\n", k_errno::last_error());
	}

	k_accept_handler<k_rtsp_handler>* rtsp_handler = new k_accept_handler<k_rtsp_handler>;
	media_server->add_event(rtsp_sock, rtsp_handler, k_event::ACCEPT_MASK);

	while (true)
	{
		k_util::k_sleep(60);
	}
	
	k_util::cleanup();

    return 0;
}

