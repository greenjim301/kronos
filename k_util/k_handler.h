#ifndef __K_HANDLER_H__
#define __K_HANDLER_H__

class k_event;
class k_thread_task;
class k_socket;

class k_handler
{
public:
	k_handler();
	virtual ~k_handler();

	virtual int handle_read(k_thread_task* task, k_event* ev, k_socket* sock) = 0;
	
	virtual void handle_del(k_thread_task* task, k_event* ev, k_socket* sock);
	virtual int handle_close(k_thread_task* task, k_event* ev, k_socket* sock);
	virtual int handle_write(k_thread_task* task, k_event* ev, k_socket* sock);
};

#endif