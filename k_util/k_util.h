#ifndef __K_UTIL_H__
#define __K_UTIL_H__

#include <stdint.h>

class k_util
{
public:
	static int init();
	static void cleanup();
	
	static void k_sleep(int seconds);

	static void avio_w8(uint8_t*& s, int b);
	static void avio_wb16(uint8_t*& s, unsigned int val);
	static void avio_wb32(uint8_t*& s, unsigned int val);
};

#define AV_RB16(x)  ((((const uint8_t*)(x))[0] << 8) | ((const uint8_t*)(x))[1])
#define AV_RB32(x)  ((((const uint8_t*)(x))[0] << 24) | \
    (((const uint8_t*)(x))[1] << 16) | \
    (((const uint8_t*)(x))[2] <<  8) | \
    ((const uint8_t*)(x))[3])

#endif