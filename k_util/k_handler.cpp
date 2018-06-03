#include <stdio.h>

#include "k_handler.h"
#include "k_socket.h"
#include "k_thread_task.h"

k_handler::k_handler() {

}

k_handler::~k_handler() {

}

void k_handler::handle_del(k_thread_task* task, k_event* ev, k_socket* sock)
{
	printf("delete sock %p\n", sock);
	delete sock;
	delete this;
}

int k_handler::handle_close(k_thread_task* task, k_event* ev, k_socket* sock)
{
	printf("close sock %p\n", sock);
	return task->del_event(sock);
}

int k_handler::handle_write(k_thread_task* task, k_event* ev, k_socket* sock)
{
	return 0;
}
