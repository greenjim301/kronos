#ifndef __K_ERRNO_H__
#define __K_ERRNO_H__

class k_errno
{
public:
	static int last_error();
	static bool is_retry_error(int err);
};


#endif