#pragma once

#include "k_util/k_handler.h"
#include <stdint.h>
#include <stdio.h>

class k_mobile_handler : public k_handler
{
public:
	k_mobile_handler();
	~k_mobile_handler();

	enum
	{
		K_MSG_HEAD_LEN = sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t),
		K_MAGIC = 0x1234,
		K_VIDEO = 1000,
		K_AUDIO = 1001,
		K_LOGIN = 1002,
		K_LOGIN_RSP = 1003
	};

	virtual int handle_read(k_thread_task* task, k_event* ev, k_socket* sock);

	int incoming_msg(uint32_t msg_id, const char* buf, int len, 
		k_thread_task* task, k_event* ev, k_socket* sock);

	int on_login_msg(const char* buf, int len, k_socket* sock, k_thread_task* task);
	int on_media_msg(k_thread_task* task, uint32_t msg_id, uint8_t* buf, int len);

	int ff_h264_handle_frag_packet(const uint8_t *buf, int len,
		int start_bit, const uint8_t *nal_header,
		int nal_header_len);

	int h264_handle_packet_fu_a(const uint8_t *buf, int len);


private:
	char buf[4096];
	char head_buf[K_MSG_HEAD_LEN];

	char* m_rebuf;
	uint32_t m_rebuf_size;
	FILE* m_video_fp;
	FILE* m_auido_fp;
};
