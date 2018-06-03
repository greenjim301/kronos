#include "k_thread_task.h"
#include "k_errno.h"

#ifdef WIN32
#include <windows.h>
#include <processthreadsapi.h>
#define THREAD_RPOC_RETURN DWORD WINAPI
#else
#include <pthread.h>
#include <stdio.h>
#include <sys/epoll.h>
#define THREAD_RPOC_RETURN void*
#define MAX_EPOLL_EVENTS 1024
#endif


static THREAD_RPOC_RETURN ThreadProc(void* lpParameter)
{
	k_thread_task* task = static_cast<k_thread_task* >(lpParameter);
	
	task->run();
	
	delete task;
	return 0;
}

k_thread_task::k_thread_task()
	: m_porcess_msg(false)
	, m_exit(false)
#ifndef WIN32
	, m_notify_socket(NULL)
#endif
{
}

k_thread_task::~k_thread_task()
{
	this->clear_events();

#ifndef WIN32
	if (m_notify_socket)
	{
		delete m_notify_socket;
	}
#endif
}

int k_thread_task::init()
{
	if (m_mq_mutex.init())
	{
		return -1;
	}

	if (this->init_msg_que_notify())
	{
		return -1;
	}

#ifdef WIN32
	HANDLE hd = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
	if (hd == NULL)
	{
		return -1;
	}
#else
    pthread_t tid;
    if (pthread_create(&tid, NULL, ThreadProc, this))
    {
		close(m_epoll_fd);
        return  -1;
    }
#endif

	return 0;
}

int k_thread_task::run()
{
#ifdef WIN32
	WSAEVENT   event_array[MAXIMUM_WAIT_OBJECTS];
	k_event*   k_event_array[MAXIMUM_WAIT_OBJECTS];
	int event_nums;

	for (;;)
	{
		event_nums = 0;

		for (event_map_type::iterator it = m_events.begin(); it != m_events.end(); ++it)
		{
			k_event* ev = it->second;
			k_event_array[event_nums] = ev;
			event_array[event_nums] = ev->get_event();
			++event_nums;
		}

		int ret = WaitForMultipleObjectsEx(event_nums, event_array, FALSE, INFINITE, FALSE);
		if (ret == WAIT_FAILED)
		{
			return -1;
		}
		else if (ret == WAIT_TIMEOUT)
		{
			continue;
		}
		else if (ret == WAIT_IO_COMPLETION)
		{
			continue;
		}
		else if (ret >= WAIT_ABANDONED_0)
		{
			continue;
		}
		else
		{
			ret -= WAIT_OBJECT_0;
			k_event* ev = k_event_array[ret];

			uint32_t mask = 0;
            if(0 == ev->parse_event_mask(ev->get_event(), mask))
            {
                ev->process(this, mask);
            }
		}	

		if (m_porcess_msg)
		{
			if (this->process_msg())
			{
				break;
			}
		}
	}
#else
    epoll_event event_array[MAX_EPOLL_EVENTS];

    for (;;)
    {
        int ret = epoll_wait(m_epoll_fd, event_array, MAX_EPOLL_EVENTS,  -1);

        if (ret == 0)
        {
            continue;
        }
        else if(ret == -1)
        {
            int err = k_errno::last_error();
            if(k_errno::is_retry_error(err))
            {
                continue;
            }

            break;
        }
        else
        {
            for (int i = 0; i < ret; ++i)
            {
                epoll_event& event = event_array[i];
                k_event* ev = static_cast<k_event*> (event.data.ptr);

                uint32_t mask = 0;
                if(0 == ev->parse_event_mask(event, mask))
                {
                    ev->process(this, mask);
                }
            }
        }

        if (m_porcess_msg)
        {
            if (this->process_msg())
            {
                break;
            }
        }
    }

    close(m_epoll_fd);
#endif

	return 0;
}

int k_thread_task::add_event(k_socket* sock, k_handler* handler, uint32_t event_mask)
{
	k_add_event* add = new k_add_event;
    add->m_sock = sock;
    add->m_handler = handler;
    add->m_event_mask = event_mask;
	k_msg msg = { 1, add };

	if (this->enque_msg(msg))
	{
		delete add;

		return -1;
	}
	
	return 0;
}

int k_thread_task::del_event(k_socket* sock)
{
	k_del_event* del = new k_del_event;
    del->m_sock =  sock ;
	k_msg msg = { 2, del };

	if (this->enque_msg(msg))
	{
		delete del;

		return -1;
	}

	return 0;
}

int k_thread_task::exit()
{
	k_msg msg = { 3, NULL };

	return this->enque_msg(msg);
}

int k_thread_task::enque_msg(k_msg& msg)
{
	int ret = m_mq_mutex.acquire();
	if (ret)
	{
		return -1;
	}

	if (m_exit)
	{
		printf("exiting\n");
		m_mq_mutex.release();
		return -1;
	}

	if (msg.m_msg_id == 3)//exit
	{
		m_exit = true;
	}

	m_msg_que.push(msg);

	if (!m_porcess_msg)
	{
		if (this->notify_msg_que())
		{
			//error
		}
	}

	m_mq_mutex.release();	

	return 0;
}

int k_thread_task::init_msg_que_notify()
{
#ifdef WIN32
    k_mq_notify_handler* handler = new k_mq_notify_handler;
	m_mq_event = new k_event;

	if (m_mq_event->init(NULL, handler, k_event::READ_MASK))
	{
		m_mq_event->on_del(this);
		delete m_mq_event;

		return -1;
	}

    m_events.insert(event_map_type::value_type(NULL, m_mq_event));

#else
	m_epoll_fd = epoll_create(1);

	if (m_epoll_fd == -1)
	{
		return -1;
	}

    if (pipe(m_notify_pipe))
    {
		close(m_epoll_fd);
        return  -1;
    }

    k_socket* sock = new k_socket;
    m_notify_socket = new k_socket;

    sock->set_sock(m_notify_pipe[0]);
	m_notify_socket->set_sock(m_notify_pipe[1]);

    k_mq_notify_handler* handler = new k_mq_notify_handler;
    k_event* ev = new k_event;

	if (ev->init(sock, handler, k_event::READ_MASK))
	{
		ev->on_del(this);
		delete ev;
		delete m_notify_socket;
		close(m_epoll_fd);

		return -1;
	}
    
	if (this->add_event_impl(ev))
	{
		ev->on_del(this);
		delete ev;
		delete m_notify_socket;
		close(m_epoll_fd);

		return -1;
	}

    m_events.insert(event_map_type::value_type(sock, ev));
#endif

	return 0;
}

void k_thread_task::set_process_msg()
{
	m_porcess_msg = true;
}

void k_thread_task::reset_process_msg()
{
	m_porcess_msg = false;
}

int k_thread_task::deque_msg(k_msg& msg)
{
	int ret = m_mq_mutex.acquire();
	if (ret)
	{
		return -1;
	}

	ret = m_msg_que.size();

	if(ret)
	{
		msg = m_msg_que.front();
		m_msg_que.pop();
	} 

	m_mq_mutex.release();

	return ret;
}

int k_thread_task::process_msg()
{
	k_msg msg;
	int ret = 0;
	do
	{
		ret = this->deque_msg(msg);
		if (ret > 0)
		{
			this->process_msg(msg);
		}
	} while (ret > 0);

	this->reset_process_msg();

	return m_exit ? -1 : 0;
}

int k_thread_task::process_msg(k_msg& msg)
{
	switch (msg.m_msg_id)
	{
	case 1:
	{
		k_add_event* add = static_cast<k_add_event*>(msg.m_data);
		k_event* ev = new k_event;

		if(ev->init(add->m_sock, add->m_handler, add->m_event_mask))
        {
			ev->on_del(this);
            delete add;
            delete ev;

            return  -1;
        }

        if (this->add_event_impl(ev))
        {
			ev->on_del(this);
            delete add;
            delete ev;

            return -1;
        }

        m_events.insert(event_map_type::value_type(add->m_sock, ev));

		delete add;
		return 0;
	}
		break;

	case 2:
	{
		k_del_event* del = static_cast<k_del_event*>(msg.m_data);
		k_socket* sock = del->m_sock;
		
		event_map_type::iterator it = m_events.find(sock);
		if (it != m_events.end())
		{
			k_event* ev = it->second;

            this->del_event_impl(ev);
			ev->on_del(this);

			m_events.erase(it);
			delete ev;
		}

		delete del;
		return 0;
	}
	break;

	case 3:
	{
		this->clear_events();
		return 0;
	}
	break;

	default:
		return 0;
	}
}

int k_thread_task::notify_msg_que()
{
#ifdef WIN32
	if (!SetEvent(m_mq_event->get_event()))
	{
		return -1;
	}
#else
	char c = 0;
	if (sizeof(char) != m_notify_socket->k_write(&c, sizeof(char)))
	{
		printf("send notify failed, err:%d\n", k_errno::last_error());
		return -1;
	}
#endif

	return 0;
}

void k_thread_task::clear_events()
{
	std::vector<k_event*> events;
	for (event_map_type::iterator it = m_events.begin(); it != m_events.end(); ++it)
	{
		events.push_back(it->second);
	}

	m_events.clear();

	for (std::vector<k_event*>::iterator it = events.begin();
		it != events.end(); ++it)
	{
		k_event* ev = *it;
		ev->on_del(this);
		delete ev;
	}
}

int k_thread_task::add_event_impl(k_event *ev)
{
#ifdef WIN32
    if(!ev->get_socket())
    {
        return 0;
    }

    if (WSAEventSelect(ev->get_socket()->get_sock(), ev->get_event(),
        ev->get_event_mask()))
	{
		return -1;
	}
#else
    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD,
                        ev->get_socket()->get_sock(), ev->get_event_ptr()))
    {
        return -1;
    }

#endif
    return 0;
}

int k_thread_task::del_event_impl(k_event *ev)
{
#ifdef WIN32
#else
    if(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL,
                 ev->get_socket()->get_sock(), ev->get_event_ptr()))
    {
        return -1;
    }

#endif
    return 0;
}

int k_thread_task::k_mq_notify_handler::handle_read(
	k_thread_task* task, k_event* ev, k_socket* sock)
{
#ifdef WIN32
	ResetEvent(ev->get_event());
#else
	char c;
	sock->k_read(&c, sizeof(char));
#endif

	task->set_process_msg();

	return 0;
}
