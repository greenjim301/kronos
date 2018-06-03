#include "k_errno.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <errno.h>
#endif

int k_errno::last_error()
{
#ifdef WIN32
	return 	WSAGetLastError();
#else
	return errno;
#endif
}

bool k_errno::is_retry_error(int err)
{
#ifdef WIN32
	return err == WSAEWOULDBLOCK;
	//return false;
#else
	return err == EINTR;
#endif
}
