#include "k_mobile_handler.h"
#include "json/json.h"
#include "k_util/k_socket.h"
#include "k_media_server.h"
#include "k_util/k_util.h"

#define RTP_VERSION 2
#define RTP_FLAG_MARKER 0x2

static const uint8_t start_sequence[] = { 0, 0, 0, 1 };

k_mobile_handler::k_mobile_handler()
	: m_rebuf(NULL)
	, m_rebuf_size(0)
{
	fopen_s(&m_video_fp, "C:/Users/Si_Li/Desktop/tmp/test.264", "wb");
	fopen_s(&m_auido_fp, "C:/Users/Si_Li/Desktop/tmp/test.aac", "wb");
}

k_mobile_handler::~k_mobile_handler()
{
	if (m_rebuf)
	{
		free(m_rebuf);
	}

	fclose(m_video_fp);
	fclose(m_auido_fp);
}

int k_mobile_handler::handle_read(k_thread_task* task, k_event* ev, k_socket* sock)
{
	int ret = sock->k_recv(buf, 4096);

	if (ret <= 0)
	{
		printf("rev failed:%d\n", ret);
		task->del_event(sock);
		return -1;
	}

	char* p = buf;
	uint32_t size = ret;

	while (size > 0)
	{
		if (size < K_MSG_HEAD_LEN)
		{
			int need = K_MSG_HEAD_LEN - size;

			memcpy(head_buf, p, size);
			if (sock->k_recv_n(head_buf + size, need))
			{
				printf("recv_n failed\n");
				task->del_event(sock);
				return -1;
			}

			p = head_buf;
			size = K_MSG_HEAD_LEN;
		}

		uint16_t magic = *(uint16_t*)p;
		magic = ntohs(magic);

		p += sizeof(uint16_t);
		size -= sizeof(uint16_t);

		if (magic != K_MAGIC)
		{
			printf("magic:%d wrong\n", magic);
			return -1;
		}

		uint32_t msg_len = *(uint32_t*)p;
		msg_len = ntohl(msg_len);
		p += sizeof(uint32_t);

		uint32_t msg_id = *(uint32_t*)p;
		msg_id = ntohl(msg_id);
		p += sizeof(uint32_t);

		size -= sizeof(uint32_t) * 2;
		msg_len -= K_MSG_HEAD_LEN;

		if (msg_len > size)
		{
			uint32_t need = msg_len - size;

			if (msg_len > m_rebuf_size)
			{
				if (m_rebuf)
				{
					free(m_rebuf);
				}

				m_rebuf = (char*)malloc(msg_len + 4);
				m_rebuf_size = msg_len;
			}

			memcpy(m_rebuf + 4, p, size);
			if (sock->k_recv_n(m_rebuf + 4 + size, need))
			{
				printf("recv_n 2 failed\n");
				task->del_event(sock);
				return -1;
			}

			p = m_rebuf + 4;
			size = msg_len;
		}

		if (this->incoming_msg(msg_id, p, msg_len, task, ev, sock))
		{
			return -1;
		}

		p += msg_len;
		size -= msg_len;
	}

	return 0;
}

int k_mobile_handler::incoming_msg(uint32_t msg_id, const char* buf, int len, 
	k_thread_task* task, k_event* ev, k_socket* sock)
{
	switch(msg_id)
	{
	case K_LOGIN:
		return this->on_login_msg(buf, len, sock, task);
		break;

	case K_VIDEO:
	case K_AUDIO:
		return this->on_media_msg(task, msg_id, (uint8_t*)buf, len);

	default:
		return -1;
	}
}

int k_mobile_handler::on_login_msg(const char* buf, int len, k_socket* sock, k_thread_task* task)
{
	Json::Reader reader;
	Json::Value root;
	// reader将Json字符串解析到root，root将包含Json里所有子元素  
	if (!reader.parse(buf, buf + len, root))
	{
		printf("json parse failed\n");
		return 0;
	}

	k_string user_id = root["user_id"].asCString();
	k_string password = root["password"].asCString();

	printf("user_id:%s, password:%s\n", user_id.c_str(), password.c_str());

	Json::Value rsp;
	int result = 0;

	rsp["result"] = result;
	rsp["describe"] = "ha ha ha";
	Json::StreamWriterBuilder writer;
	std::string document = Json::writeString(writer, rsp);

	char send_buf[4096];
	uint8_t * p_buf = (uint8_t*)send_buf;
	int tot_len = document.size() + K_MSG_HEAD_LEN;
	int rsp_msg_id = K_LOGIN_RSP;

	k_util::avio_wb16(p_buf, K_MAGIC);//magic
	k_util::avio_wb32(p_buf, tot_len);
	k_util::avio_wb32(p_buf, rsp_msg_id);

	memcpy(p_buf, document.c_str(), document.size());

	int ret = sock->k_send(send_buf, tot_len);
	if (ret != tot_len)
	{
		printf("send failed:%d\n", ret);
		task->del_event(sock);
		printf("send failed\n");
		return -1;
	}

	if (result)
	{
		printf("login error:%d\n", result);
		task->del_event(sock);
		return -1;
	}

	return 0;
}

int k_mobile_handler::ff_h264_handle_frag_packet(const uint8_t *buf, int len,
	int start_bit, const uint8_t *nal_header,
	int nal_header_len)
{
	if (start_bit) {
		fwrite(start_sequence, 1, sizeof(start_sequence), m_video_fp);
		fwrite(nal_header, 1, nal_header_len, m_video_fp);
	}
	fwrite(buf, 1, len, m_video_fp);
	return 0;
}

int k_mobile_handler::h264_handle_packet_fu_a(const uint8_t *buf, int len)
{
	uint8_t fu_indicator, fu_header, start_bit, nal_type, nal;

	if (len < 3) {
		printf("Too short data for FU-A H.264 RTP packet\n");
		return -1;
	}

	fu_indicator = buf[0];
	fu_header = buf[1];
	start_bit = fu_header >> 7;
	nal_type = fu_header & 0x1f;
	nal = fu_indicator & 0xe0 | nal_type;

	// skip the fu_indicator and fu_header
	buf += 2;
	len -= 2;

	return ff_h264_handle_frag_packet(buf, len, start_bit, &nal, 1);
}

int k_mobile_handler::on_media_msg(k_thread_task* task, uint32_t msg_id, uint8_t* buf, int len)
{
	k_media_server* server = dynamic_cast<k_media_server*>(task);
	if (msg_id == K_VIDEO)
	{
		server->on_video(buf, len);
		
	}
	else if (msg_id == K_AUDIO)
	{
		server->on_audio(buf, len);
	}
	return 0;

	if ((buf[0] & 0xc0) != (RTP_VERSION << 6))
	{
		printf("rtp version err\n");
		return -1;
	}

	unsigned int ssrc;
	int payload_type, seq, flags = 0;
	int ext, csrc;
	uint32_t timestamp;

	csrc = buf[0] & 0x0f;
	ext = buf[0] & 0x10;
	payload_type = buf[1] & 0x7f;
	if (buf[1] & 0x80)
		flags |= RTP_FLAG_MARKER;
	seq = AV_RB16(buf + 2);
	timestamp = AV_RB32(buf + 4);
	ssrc = AV_RB32(buf + 8);

	if (buf[0] & 0x20) {
		int padding = buf[len - 1];
		if (len >= 12 + padding)
			len -= padding;
	}

	len -= 12;
	buf += 12;

	len -= 4 * csrc;
	buf += 4 * csrc;

	if (len < 0)
	{
		printf("rtp len err\n");
		return -1;
	}


	/* RFC 3550 Section 5.3.1 RTP Header Extension handling */
	if (ext) {
		if (len < 4)
		{
			printf("rtp ext len err\n");
			return -1;
		}
		/* calculate the header extension length (stored as number
		* of 32-bit words) */
		ext = (AV_RB16(buf + 2) + 1) << 2;

		if (len < ext)
		{
			printf("rtp ext len err2\n");
			return -1;
		}

		// skip past RTP header extension
		len -= ext;
		buf += ext;
	}

	if (!len) {
		printf("rtp len err2\n");
		return -1;
	}

	printf("payload:%d, seq:%d, len:%d, ts:%u\n", payload_type, seq, len, timestamp);

	if (msg_id == K_AUDIO)
	{
		fwrite(buf, 1, len, m_auido_fp);
		return 0;
	}

	uint8_t nal;
	uint8_t type;
	int result = 0;

	nal = buf[0];
	type = nal & 0x1f;

	/* Simplify the case (these are all the NAL types used internally by
	* the H.264 codec). */
	if (type >= 1 && type <= 23)
		type = 1;
	switch (type) {
	case 0:                    // undefined, but pass them through
	case 1:
		fwrite(start_sequence, 1, sizeof(start_sequence), m_video_fp);
		fwrite(buf, 1, len, m_video_fp);
		break;

	case 28:                   // FU-A (fragmented nal)
		result = h264_handle_packet_fu_a(buf, len);
		break;

	default:
		printf("Undefined type (%d)\n", type);
		return -1;
		break;
	}

	return result;
}