#include "k_mutex.h"

k_mutex::k_mutex()
#ifdef WIN32
	: m_mutex(NULL)
#endif
{

}

k_mutex::~k_mutex()
{
#ifdef WIN32
	if (m_mutex != NULL)
	{
		CloseHandle(m_mutex);
	}
#else
    pthread_mutex_destroy(&m_mutex);
#endif
}

int k_mutex::init()
{
#ifdef WIN32
	m_mutex = CreateMutex(NULL, FALSE, NULL);          
	if (m_mutex == NULL)
	{
		return -1;
	}
#else
    if(pthread_mutex_init(&m_mutex, NULL))
    {
        return -1;
    }
#endif

	return 0;
}

int k_mutex::acquire()
{
#ifdef WIN32
	int ret = WaitForSingleObject(m_mutex, INFINITE); 

	if (ret != WAIT_OBJECT_0)
	{
		return -1;
	}
#else
    if(pthread_mutex_lock(&m_mutex))
    {
        return  -1;
    }
#endif

	return  0;
}

void k_mutex::release()
{
#ifdef WIN32
	ReleaseMutex(m_mutex);
#else
    pthread_mutex_unlock(&m_mutex);
#endif
}
