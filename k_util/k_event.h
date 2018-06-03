#ifndef __K_EVENT_H__
#define __K_EVENT_H__

#ifdef WIN32
#include <WinSock2.h>
#define K_EVENT_T WSAEVENT
#else
#include <sys/epoll.h>
#define K_EVENT_T epoll_event
#endif

#include "k_handler.h"
#include "k_socket.h"

class k_event
{
public:
	enum
	{
		READ_MASK = (1 << 0),
		WRITE_MASK = (1 << 1),
		ACCEPT_MASK = (1 << 3),
        CLOSE_MASK = (1 << 4)
	};

	k_event();
	~k_event();

	int init(k_socket* sock, k_handler* handler, uint32_t event_mask);
    K_EVENT_T get_event();
    K_EVENT_T* get_event_ptr();
    k_socket* get_socket();
    uint32_t get_event_mask();

	void process(k_thread_task* task, uint32_t mask);
	void on_del(k_thread_task* task);

    int parse_event_mask(K_EVENT_T event, uint32_t& mask);

private:
	int gen_event_mask(uint32_t event_mask);

private:
	uint32_t m_event_mask;
    K_EVENT_T m_event;
	k_handler* m_handler;
	k_socket* m_socket;
};

#endif