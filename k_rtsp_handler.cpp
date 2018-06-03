#include "k_rtsp_handler.h"
#include "k_util/k_socket.h"
#include "k_media_server.h"
#include "k_util/k_util.h"
#include <stdio.h>

#define DEF_CSEQ "CSeq"
#define DEF_TRANSPORT "Transport"
#define DEF_SESSION "Session"

#define DEF_OPTIONS "OPTIONS"
#define DEF_DESCRIBE "DESCRIBE"
#define DEF_SETUP "SETUP"
#define DEF_PLAY "PLAY"
#define DEF_TEARDOWN "TEARDOWN"
#define DEF_GET_PARAMETER "GET_PARAMETER"
#define DEF_SET_PARAMETER "SET_PARAMETER"

k_rtsp_handler::k_rtsp_handler()
	: m_session_id(0)
	, m_sock(NULL)
	, m_rebuf(NULL)
	, m_rebuf_size(0)
{

}

k_rtsp_handler::~k_rtsp_handler()
{
	if (m_rebuf)
	{
		free(m_rebuf);
	}
}

int k_rtsp_handler::next_sapce(char* p, char*& p_sapce)
{
	while (*p != '\0'
		&& *p != ' '
		&& *p != '\r'
		&& *p != '\n')
		++p;

	if (*p != ' ')
	{
		//err
		return -1;
	}

	p_sapce = p;
	return 0;
}

int k_rtsp_handler::skip_space(char*& p)
{
	while (*p == ' ')
		++p;

	if (*p == '\0' || *p == '\r' || *p == '\n')
	{
		//err
		return -1;
	}

	return 0;
}

void k_rtsp_handler::next_end(char* p, char*& p_end)
{
	while (*p != '\0'
		&& *p != '\r'
		&& *p != '\n')
		++p;

	p_end = p;
}

int k_rtsp_handler::parse_head_line(char*& p, k_rtsp_head& head)
{
	char* p_space;
	if (next_sapce(p, p_space))
	{
		return -1;
	}

	head.cmd.assign(p, p_space - p);

	p = p_space;
	if (skip_space(p))
	{
		return -1;
	}

	if (next_sapce(p, p_space))
	{
		return -1;
	}

	head.url.assign(p, p_space - p);

	p = p_space;
	if (skip_space(p))
	{
		return -1;
	}

	next_end(p, p_space);

	head.version.assign(p, p_space - p);

	if (*p_space == '\r')
	{
		p = p_space + 2;
	}
	else
	{
		p = p_space + 1;
	}

	return 0;
}

int k_rtsp_handler::parse_line(char*& p, k_rtsp_head& head)
{
	if (0 == memcmp(p, DEF_CSEQ, strlen(DEF_CSEQ)))
	{
		char* p_end;
		next_end(p, p_end);

		head.cseq.assign(p, p_end - p);
		p = p_end;
	}
	else if (0 == memcmp(p, DEF_TRANSPORT, strlen(DEF_TRANSPORT)))
	{
		char* p_end;
		next_end(p, p_end);

		head.transport.assign(p, p_end - p);
		p = p_end;
	}
	else if (0 == memcmp(p, DEF_SESSION, strlen(DEF_SESSION)))
	{
		char* p_end;
		next_end(p, p_end);

		head.session.assign(p, p_end - p);
		p = p_end;
	}
	else
	{
		next_end(p, p);
	}

	if (*p == '\r')
	{
		p = p + 2;
	}
	else
	{
		p = p + 1;
	}

	return 0;
}

void k_rtsp_handler::append_space(char*& p)
{
	*p = ' ';
	++p;
}

void k_rtsp_handler::append_line_end(char*& p)
{
	*p = '\r';
	++p;
	*p = '\n';
	++p;
}

int k_rtsp_handler::send_rtsp_rsp(k_rtsp_rsp& rsp, k_socket* sock)
{
	char buf[4096];
	char* p = buf;

	memcpy(p, rsp.version.c_str(), rsp.version.size());
	p += rsp.version.size();
	append_space(p);

	memcpy(p, rsp.result.c_str(), rsp.result.size());
	p += rsp.result.size();
	append_space(p);


	memcpy(p, rsp.describe.c_str(), rsp.describe.size());
	p += rsp.describe.size();
	append_line_end(p);

	memcpy(p, rsp.cseq.c_str(), rsp.cseq.size());
	p += rsp.cseq.size();
	append_line_end(p);

	if (rsp.rtsp_public.size())
	{
		memcpy(p, rsp.rtsp_public.c_str(), rsp.rtsp_public.size());
		p += rsp.rtsp_public.size();
		append_line_end(p);
	}

	if (rsp.session.size())
	{
		memcpy(p, rsp.session.c_str(), rsp.session.size());
		p += rsp.session.size();
		append_line_end(p);
	}

	if (rsp.transport.size())
	{
		memcpy(p, rsp.transport.c_str(), rsp.transport.size());
		p += rsp.transport.size();
		append_line_end(p);
	}

	if (rsp.content_type.size())
	{
		memcpy(p, rsp.content_type.c_str(), rsp.content_type.size());
		p += rsp.content_type.size();
		append_line_end(p);
	}

	if (rsp.content_base.size())
	{
		memcpy(p, rsp.content_base.c_str(), rsp.content_base.size());
		p += rsp.content_base.size();
		append_line_end(p);
	}

	if (rsp.sdp.size())
	{
		char sdp[1024];
		char* p_sdp = sdp;
		for (auto it = rsp.sdp.begin(); it != rsp.sdp.end(); ++it)
		{
			memcpy(p_sdp, it->c_str(), it->size());
			p_sdp += it->size();
			append_line_end(p_sdp);
		}

		int sdp_len = p_sdp - sdp;
		int nn = snprintf(p, 32, "Content-Length: %d\r\n\r\n", sdp_len);
		p += nn;

		memcpy(p, sdp, sdp_len);
		p += sdp_len;
	} 
	else
	{
		append_line_end(p);
	}

	int len = p - buf;
	buf[len] = '\0';
	printf("%s\n\n", buf);

	if (len != sock->k_send(buf, len))
	{
		printf("send err\n");
		return -1;
	}

	return 0;
}

int k_rtsp_handler::on_rtsp_head(k_rtsp_head& head, k_thread_task* task, k_socket* sock)
{
	k_rtsp_rsp rsp;

	rsp.version = head.version;
	rsp.cseq = head.cseq;
	rsp.result = "200";
	rsp.describe = "OK";
	rsp.session = head.session;

	if (0 == memcmp(head.cmd.c_str(), DEF_OPTIONS, strlen(DEF_OPTIONS)))
	{
		rsp.rtsp_public = "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, \
PLAY, GET_PARAMETER, SET_PARAMETER";
	}
	else if (0 == memcmp(head.cmd.c_str(), DEF_DESCRIBE, strlen(DEF_DESCRIBE)))
	{
		rsp.content_type = "Content-Type: application/sdp";
		rsp.content_base = "Content-Base: ";
		rsp.content_base.append(head.url.c_str(), head.url.size());
		rsp.content_base.append("/", 1);

		rsp.sdp.push_back("v=0");
		rsp.sdp.push_back("o=LS 0 0 IN IP4 0.0.0.0");
		rsp.sdp.push_back("s=LS RTSP SDP");
		rsp.sdp.push_back("c=IN IP4 0.0.0.0");
		rsp.sdp.push_back("t=0 0");
		rsp.sdp.push_back("a=range:npt=0-");
		rsp.sdp.push_back("a=control:*");

		rsp.sdp.push_back("m=video 0 RTP/AVP 96");
		rsp.sdp.push_back("a=control:track1");
		rsp.sdp.push_back("a=rtpmap:96 H264/90000");
		rsp.sdp.push_back("a=fmtp:96 profile-level-id=;sprop-parameter-sets=;packetization-mode=0");

		rsp.sdp.push_back("m=audio 0 RTP/AVP 97");
		rsp.sdp.push_back("a=control:track2");
		rsp.sdp.push_back("a=rtpmap:97 MPEG4-GENERIC/44100/1");

		k_string aac_fmtp = "a=fmtp:97 profile-level-id=15;mode=AAC-hbr;\
sizelength=13;indexlength=3;indexdeltalength=3;config=";

		static int const samplingFrequencyTable[16] = {
			96000, 88200, 64000, 48000,
			44100, 32000, 24000, 22050,
			16000, 12000, 11025, 8000,
			7350, 0, 0, 0
		};

		uint8_t audioSpecificConfig[2];
		uint8_t audioObjectType = 2;
		uint8_t samplingFrequencyIndex;
		uint8_t channelConfiguration = 1;

		for (samplingFrequencyIndex = 0; samplingFrequencyIndex < 16; ++samplingFrequencyIndex)
		{
			if (samplingFrequencyTable[samplingFrequencyIndex] == 44100)
			{
				break;
			}
		}

		char fConfigStr[16];
		audioSpecificConfig[0] = (audioObjectType << 3) | (samplingFrequencyIndex >> 1);
		audioSpecificConfig[1] = (samplingFrequencyIndex << 7) | (channelConfiguration << 3);
		snprintf(fConfigStr, 16, "%02x%02x", audioSpecificConfig[0], audioSpecificConfig[1]);

		aac_fmtp.append(fConfigStr, strlen(fConfigStr));
		rsp.sdp.push_back(aac_fmtp);
	}
	else if (0 == memcmp(head.cmd.c_str(), DEF_SETUP, strlen(DEF_SETUP)))
	{
		char* p = head.transport.find("interleaved=");
		if (!p)
		{
			rsp.result = "461";
			rsp.describe = "Unsupported transport";
		}
		else
		{
			rsp.transport = head.transport;
			if (rsp.session.size() == 0)
			{
				rsp.session = "Session: ";
				rsp.session.append(++m_session_id);
			}
		}
	}
	else if (0 == memcmp(head.cmd.c_str(), DEF_PLAY, strlen(DEF_PLAY)))
	{
		k_media_server* server = dynamic_cast<k_media_server*>(task);
		server->regist_sink(this);
	}
	else if (0 == memcmp(head.cmd.c_str(), DEF_TEARDOWN, strlen(DEF_TEARDOWN)))
	{
		send_rtsp_rsp(rsp, sock);
		task->del_event(sock);
		return -1;
	}

	return send_rtsp_rsp(rsp, sock);
}

int k_rtsp_handler::handle_read(k_thread_task* task, k_event* ev, k_socket* sock)
{
	m_sock = sock;
	int ret = sock->k_recv(m_buf, 4096);

	if (ret <= 0)
	{
		printf("rev ret:%d\n", ret);
		task->del_event(sock);
		return -1;
	}

	m_buf[ret] = '\0';
	printf("%s\n\n", m_buf);

	char* p = m_buf;

	while (ret > 0)
	{
		if (*p == '$')
		{
			if (ret < 4)
			{
				printf("head lt 4\n");
				return -1;
			}
			uint8_t channel = *(p + 1);
			int len = AV_RB16(p + 2);

			p += 4;
			ret -= 4;

			if (ret < len)
			{
				int32_t need = len - ret;

				if (len > m_rebuf_size)
				{
					if (m_rebuf)
					{
						free(m_rebuf);
					}

					m_rebuf = (char*)malloc(len);
					m_rebuf_size = len;
				}

				memcpy(m_rebuf, p, ret);
				if (sock->k_recv_n(m_rebuf + ret, need))
				{
					printf("recv_n half failed\n");
					return -1;
				}

				p = m_rebuf;
				ret = len;
			}
			
			if (this->parse_rtp_rtcp(channel, p, len))
			{
				return -1;
			}

			p += len;
			ret -= len;
		} 
		else
		{
			char* p_head_end = strstr(p, "\r\n\r\n");
			if (!p_head_end)
			{
				printf("header not complete:%s\n", p);
				return -1;
			}

			ret -= p_head_end - p + strlen("\r\n\r\n");

			bool parsed_head_line = false;
			k_rtsp_head head;

			while(p < p_head_end)
			{
				if (!parsed_head_line)
				{
					if (parse_head_line(p, head))
					{
						return -1;
					}
					parsed_head_line = true;
				}
				else
				{
					parse_line(p, head);
				}
			}

			p = p_head_end + strlen("\r\n\r\n");

			if (on_rtsp_head(head, task, sock))
			{
				return -1;
			}
		}
	}

	return 0;
}

void k_rtsp_handler::handle_del(k_thread_task* task, k_event* ev, k_socket* sock)
{
	k_media_server* server = dynamic_cast<k_media_server*>(task);
	server->unregist_sink(this);

	k_handler::handle_del(task, ev, sock);
}

void k_rtsp_handler::on_video(uint8_t* buf, int len)
{
	buf -= 4;
	*buf++ = '$';
	*buf++ = 0;
	k_util::avio_wb16(buf, len);

	buf -= 4;
	len += 4;

	if (len != m_sock->k_send((char*)buf, len))
	{
		printf("send video err\n");
	}
}

void k_rtsp_handler::on_audio(uint8_t* buf, int len)
{
	buf -= 4;
	*buf++ = '$';
	*buf++ = 2;
	k_util::avio_wb16(buf, len);

	buf -= 4;
	len += 4;

	if (len != m_sock->k_send((char*)buf, len))
	{
		printf("send video err\n");
	}
}

int k_rtsp_handler::parse_rtp_rtcp(uint8_t channel, char* buf, int len)
{
	return 0;
}
