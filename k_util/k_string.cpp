#include "k_string.h"
#include <string.h>
#include <stdio.h>

k_string::k_string()
	: m_size(0)
{
	memset(m_buf, 0, STR_MAX_LEN);
}

k_string::k_string(const char* str)
{
	memset(m_buf, 0, STR_MAX_LEN);
	int size = strlen(str);
	if (size < STR_MAX_LEN)
	{
		m_size = size;
		memcpy(m_buf, str, m_size);
	}
	else
	{
		m_size = 0;
	}
}

k_string::k_string(const char* str, int size)
{
	this->assign(str, size);
}

void k_string::assign(const char* str, int size)
{
	memset(m_buf, 0, STR_MAX_LEN);
	if (size < STR_MAX_LEN)
	{
		m_size = size;
		memcpy(m_buf, str, m_size);
	}
	else
	{
		m_size = 0;
	}
}

void k_string::append(const char* str, int size)
{
	if (size + m_size < STR_MAX_LEN)
	{
		memcpy(m_buf + m_size, str, size);
		m_size += size;
		m_buf[m_size] = '\0';
	}
}

void k_string::append(uint32_t num)
{
	char nbuf[16];
	int n = snprintf(nbuf, 16, "%d", num);
	this->append(nbuf, n);
}

char* k_string::find(const char* str)
{
	return strstr(m_buf, str);
}

const char* k_string::c_str() const
{
	return m_buf;
}

int k_string::size() const
{
	return m_size;
}

bool k_string::operator<(const k_string& r_str) const
{
	if (m_size > r_str.size())
	{
		if (memcmp(this->c_str(), r_str.c_str(), r_str.size()) <= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (memcmp(this->c_str(), r_str.c_str(), m_size) >= 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool k_string::operator==(const k_string &r_str) const {
    if(m_size != r_str.size())
		return false;

	return 0 == memcmp(m_buf, r_str.c_str(), m_size);
}

