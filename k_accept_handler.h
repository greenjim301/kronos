#pragma once

#include "k_util/k_handler.h"

template<typename T>
class k_accept_handler : public k_handler
{
public:
	virtual int handle_read(k_thread_task* task, k_event* ev, k_socket* sock)
	{
		printf("accepted\n");

		k_socket* accept_sock = new k_socket;
		k_sockaddr addr;
		m_read_handler = new T;

		sock->k_accept(addr, *accept_sock);
		task->add_event(accept_sock, m_read_handler, k_event::READ_MASK);

		return 0;
	}

private:
	T* m_read_handler;
};
