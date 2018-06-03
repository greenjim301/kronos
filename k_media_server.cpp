#include "k_media_server.h"

void k_media_server::regist_sink(k_rtsp_handler* handler)
{
	m_rtsp_handler.insert(handler);
}

void k_media_server::unregist_sink(k_rtsp_handler* handler)
{
	m_rtsp_handler.erase(handler);
}

void k_media_server::on_video(uint8_t* buf, int len)
{
	for (auto it = m_rtsp_handler.begin(); it != m_rtsp_handler.end(); ++it)
	{
		(*it)->on_video(buf, len);
	}
}

void k_media_server::on_audio(uint8_t* buf, int len)
{
	for (auto it = m_rtsp_handler.begin(); it != m_rtsp_handler.end(); ++it)
	{
		(*it)->on_audio(buf, len);
	}
}
