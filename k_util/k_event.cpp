#include "k_event.h"

k_event::k_event()
	: m_socket(NULL)
	, m_handler(NULL)
	, m_event_mask(0)
#ifdef WIN32
	, m_event(NULL)
#endif
{
}

k_event::~k_event()
{
#ifdef WIN32
	if (m_event)
	{
		WSACloseEvent(m_event);
	}
#endif
}

int k_event::init(k_socket* sock, k_handler* handler, uint32_t event_mask)
{	
	m_socket = sock;
	m_handler = handler;

#ifdef WIN32
	m_event = WSACreateEvent();

	if (m_event == NULL)
	{
		return -1;
	}
#else
    m_event.data.ptr = this;
#endif
	
	if (m_socket && event_mask)
	{
		if (this->gen_event_mask(event_mask))
		{
			return -1;
		}
	}

	return 0;
}

K_EVENT_T k_event::get_event()
{
	return m_event;
}

void k_event::process(k_thread_task* task, uint32_t mask)
{
    if (mask & READ_MASK)
    {
        m_handler->handle_read(task, this, m_socket);
    }

    if (mask & WRITE_MASK)
    {
        m_handler->handle_write(task, this, m_socket);
    }

    if (mask & ACCEPT_MASK)
    {
        m_handler->handle_read(task, this, m_socket);
    }

    if (mask & CLOSE_MASK)
    {
        m_handler->handle_close(task, this, m_socket);
    }
}

void k_event::on_del(k_thread_task* task)
{
	m_handler->handle_del(task, this, m_socket);
}

int k_event::gen_event_mask(uint32_t event_mask)
{
	if (event_mask & READ_MASK )
	{
#ifdef WIN32
		m_event_mask |= FD_READ;
#else
        m_event_mask |= EPOLLIN;
#endif
	}

	if (event_mask & WRITE_MASK)
	{
#ifdef WIN32
        m_event_mask |= FD_WRITE;
#else
        m_event_mask |= EPOLLOUT;
#endif
	}

	if (event_mask & ACCEPT_MASK)
	{

#ifdef WIN32
        m_event_mask |= FD_ACCEPT;
#else
        m_event_mask |= EPOLLIN;
#endif
	}

#ifdef WIN32
    m_event_mask |= FD_CLOSE;
#else
	m_event_mask |= EPOLLRDHUP;

    m_event.events = m_event_mask;
#endif

	return 0;
}

k_socket *k_event::get_socket() {
    return m_socket;
}

K_EVENT_T *k_event::get_event_ptr() {
    return &m_event;
}

uint32_t k_event::get_event_mask() {
    return m_event_mask;
}

int k_event::parse_event_mask(K_EVENT_T event, uint32_t &mask)
{
#ifdef WIN32
	if (!m_socket)
	{
		mask |= READ_MASK;
		return 0;
	}

    WSANETWORKEVENTS events;
    if (WSAEnumNetworkEvents(m_socket->get_sock(), event, &events))
    {
        return -1;
    }
    
    if (events.lNetworkEvents & FD_READ)
    {
		mask |= READ_MASK;

        if (events.iErrorCode[FD_READ_BIT])
        {
			mask |= CLOSE_MASK;
        }
    }
    
    if (events.lNetworkEvents & FD_WRITE)
    {
        if (events.iErrorCode[FD_WRITE_BIT])
        {
			mask |= CLOSE_MASK;
        }
        else
        {
			mask |= WRITE_MASK;
        }
    }
    
    if (events.lNetworkEvents & FD_ACCEPT)
    {
        if (events.iErrorCode[FD_ACCEPT_BIT])
        {
			mask |= CLOSE_MASK;
        }
        else
        {
			mask |= READ_MASK;
        }
    }
    
    if (events.lNetworkEvents & FD_CLOSE)
    {
		mask |= CLOSE_MASK;

        if (0 == events.iErrorCode[FD_CLOSE_BIT])
        {
			mask |= READ_MASK;
        }
    }
#else
    uint32_t ev = event.events;

    if(ev & EPOLLIN)
    {
        mask |= READ_MASK;
    }

    if(ev & EPOLLOUT)
    {
        mask |= WRITE_MASK;
    }

    if(ev & EPOLLERR)
    {
        mask |= CLOSE_MASK;
    }

    if(ev & EPOLLHUP)
    {
        mask |= CLOSE_MASK;
    }

	if (ev & EPOLLRDHUP)
	{
		mask |= CLOSE_MASK;
	}

#endif
    return 0;
}
