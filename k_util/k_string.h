#ifndef __K_STRING_H__
#define __K_STRING_H__

#include <stdint.h>

class k_string
{
public:
	k_string();
	k_string(const char* str);
	k_string(const char* str, int size);

	void assign(const char* str, int size);
	void append(const char* str, int size);
	void append(uint32_t num);

	char* find(const char* str);

	const char* c_str() const;
	int size() const;

	bool operator < (const k_string& r_str) const;
	bool operator == (const k_string& r_str) const;

private:
	enum
	{
		STR_MAX_LEN = 256
	};

	char m_buf[STR_MAX_LEN];
	int m_size;
};

#endif
