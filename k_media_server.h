#pragma once

#include "k_util/k_thread_task.h"
#include "k_rtsp_handler.h"
#include <set>

class k_media_server : public k_thread_task
{
public:
	void regist_sink(k_rtsp_handler* handler);
	void unregist_sink(k_rtsp_handler* handler);
	void on_video(uint8_t* buf, int len);
	void on_audio(uint8_t* buf, int len);

private:
	std::set<k_rtsp_handler*> m_rtsp_handler;
};
