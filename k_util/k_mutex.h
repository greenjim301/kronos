#ifndef __K_MUTEX_H__
#define __K_MUTEX_H__

#ifdef WIN32
#include <windows.h>
#include <synchapi.h>
#define K_MUTEX_T HANDLE
#else
#include <pthread.h>
#define K_MUTEX_T pthread_mutex_t
#endif

class k_mutex
{
public:
	k_mutex();
	~k_mutex();

	int init();
	int acquire();
	void release();

private:
	K_MUTEX_T m_mutex;
};

#endif