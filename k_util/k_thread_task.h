#ifndef __K_THREAD_TASK_H__
#define __K_THREAD_TASK_H__

#include <map>
#include <queue>

#include "k_event.h"
#include "k_mutex.h"

struct k_msg
{
	int m_msg_id;
	void* m_data;
};

struct k_add_event
{
	k_socket* m_sock;
	k_handler* m_handler;
	uint32_t m_event_mask;
};

struct k_del_event
{
	k_socket* m_sock;
};

class k_thread_task
{
public:
	k_thread_task();
	virtual ~k_thread_task();

	int init();
	int run();
	int exit();

	int add_event(k_socket* sock, k_handler* handler, uint32_t event_mask);
	int del_event(k_socket* sock);
	int enque_msg(k_msg& msg);
	virtual int process_msg(k_msg& msg);

private:
	class k_mq_notify_handler : public k_handler
	{
	public:
		int handle_read(k_thread_task* mask, k_event* ev, k_socket* sock);
	};

	int init_msg_que_notify();
	int notify_msg_que();
	void clear_events();
	
	void set_process_msg();
	void reset_process_msg();

	int deque_msg(k_msg& msg);
	int process_msg();

	int add_event_impl(k_event* ev);
    int del_event_impl(k_event* ev);

private:
#ifdef WIN32
	k_event* m_mq_event;
#else
	int m_epoll_fd;
	int m_notify_pipe[2];
	k_socket* m_notify_socket;
#endif

	typedef std::map<k_socket*, k_event*> event_map_type;
	event_map_type m_events;

	typedef std::queue<k_msg> msg_que_type;
	msg_que_type m_msg_que;
	k_mutex m_mq_mutex;
	bool m_porcess_msg;
	bool m_exit;
};

#endif
