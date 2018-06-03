#pragma once

#include "k_util/k_handler.h"
#include "k_util/k_string.h"
#include <stdint.h>
#include <list>

class k_rtsp_rsp
{
public:
	k_string version;
	k_string result;
	k_string describe;
	k_string session;
	k_string transport;

	k_string content_type;
	k_string content_base;

	k_string cseq;
	k_string rtsp_public;

	std::list<k_string> sdp;
};

class k_rtsp_head
{
public:
	k_string cmd;
	k_string url;
	k_string version;
	k_string cseq;
	k_string session;
	k_string transport;
};

class k_rtsp_handler : public k_handler
{
public:
	k_rtsp_handler();
	~k_rtsp_handler();

	virtual int handle_read(k_thread_task* task, k_event* ev, k_socket* sock);
	virtual void handle_del(k_thread_task* task, k_event* ev, k_socket* sock);

	void on_video(uint8_t* buf, int len);
	void on_audio(uint8_t* buf, int len);

private:
	int parse_rtp_rtcp(uint8_t channel, char* buf, int len);
	int next_sapce(char* p, char*& p_sapce);
	int skip_space(char*& p);
	void next_end(char* p, char*& p_end);
	int parse_head_line(char*& p, k_rtsp_head& head);
	int parse_line(char*& p, k_rtsp_head& head);
	void append_space(char*& p);
	void append_line_end(char*& p);
	int send_rtsp_rsp(k_rtsp_rsp& rsp, k_socket* sock);
	int on_rtsp_head(k_rtsp_head& head, k_thread_task* task, k_socket* sock);

private:
	char m_buf[4096 + 1];
	uint32_t m_session_id;
	k_socket* m_sock;
	char* m_rebuf;
	int m_rebuf_size;
};