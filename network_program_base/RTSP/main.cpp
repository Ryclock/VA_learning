#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "rtp.h"
#include <thread>

// #pragma comment(lib, "ws2_32.lib")
// #pragma warning(disable : 4996)

#define H264_FILE_NAME "./data/test.h264"
#define AAC_FILE_NAME "./data/test.aac"
#define SERVER_PORT 8554
#define SERVER_RTP_PORT 55532
#define SERVER_RTCP_PORT 55533
#define BUF_MAX_SIZE (1024 * 1024)

struct ADTS_Header
{
    unsigned int syncword;
    uint8_t id;
    uint8_t layer;
    uint8_t protection_absent;
    uint8_t profile;
    uint8_t sampling_frequent_index;
    uint8_t private_bit;
    uint8_t channel_cfg;
    uint8_t original_copy;
    uint8_t home;
    uint8_t copy_right_identification_bit;
    uint8_t copy_right_identification_start;
    unsigned int aac_frame_len;
    unsigned int adts_buf_fullness;
    uint8_t num_of_raw_data_block_in_frame;
};

int parse_ADTS_header(uint8_t *in, struct ADTS_Header *res)
{
    int frame_num = 0;
    memset(res, 0, sizeof(*res));

    if ((in[0] != 0xFF) || ((in[1] & 0xF0) != 0xF0))
    {
        printf("\nFail to parse ADTS header");
        return -1;
    }

    res->id = ((uint8_t)in[1] & 0x08) >> 3;
    res->layer = ((uint8_t)in[1] & 0x06) >> 1;
    res->protection_absent = ((uint8_t)in[1] & 0x01);
    res->profile = ((uint8_t)in[2] & 0xc0) >> 6;
    res->sampling_frequent_index = ((uint8_t)in[2] & 0x3c) >> 2;
    res->private_bit = ((uint8_t)in[2] & 0x02) >> 1;
    res->channel_cfg = (((uint8_t)in[2] & 0x01) << 2) |
                       (((unsigned int)in[3] & 0xc0) >> 6);
    res->original_copy = ((uint8_t)in[3] & 0x20) >> 5;
    res->home = ((uint8_t)in[3] & 0x10) >> 4;
    res->copy_right_identification_bit = ((uint8_t)in[3] & 0x08) >> 3;
    res->copy_right_identification_start = ((uint8_t)in[3] & 0x04) >> 2;
    res->aac_frame_len = (((unsigned int)in[3] & 0x03) << 11) |
                         (((unsigned int)in[4] & 0xFF) << 3) |
                         (((unsigned int)in[5] & 0xE0) >> 5);
    res->adts_buf_fullness = (((unsigned int)in[5] & 0x1f) << 6) |
                             (((unsigned int)in[6] & 0xfc) >> 2);
    res->num_of_raw_data_block_in_frame = ((uint8_t)in[6] & 0x03);
    return 0;
}

int rtp_send_aac_frame(int client_sock_fd,
                       struct RTP_Packet *rp, uint8_t *frame, uint32_t frame_size)
{
    rp->payload[0] = 0x00;
    rp->payload[1] = 0x10;
    rp->payload[2] = (frame_size & 0x1FE0) >> 5;
    rp->payload[3] = (frame_size & 0x1F) << 3;
    memcpy(rp->payload + 4, frame, frame_size);

    int ret = RTP_send_packet_over_TCP(client_sock_fd, rp, frame_size + 4, 0x02);
    if (ret < 0)
    {
        printf("\nFail to send rtp packet");
        return -1;
    }

    uint32_t delta = 1025; // 44100/1024=43(fps), 44100/43=1025
    rp->rh.seq++;
    rp->rh.timestamp += delta;
    return 0;
}

inline int start_code3(char *buf)
{
    return (buf[0] == 0 && buf[1] == 0 && buf[2] == 1);
}

inline int start_code4(char *buf)
{
    return (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1);
}

char *find_next_start_code(char *buf, int len)
{
    if (len < 3)
    {
        return NULL;
    }

    for (int i = 0; i < len - 3; ++i)
    {
        if (start_code3(buf) || start_code4(buf))
        {
            return buf;
        }
        ++buf;
    }

    if (start_code3(buf))
    {
        return buf;
    }

    return NULL;
}

int get_frame_from_h264_file(FILE *fp, char *frame, int size)
{
    if (fp < 0)
    {
        return -1;
    }

    int r_size = fread(frame, 1, size, fp);
    if (!start_code3(frame) && !start_code4(frame))
    {
        return -1;
    }

    int frame_size;
    char *next_start_code = find_next_start_code(frame + 3, r_size - 3);
    if (!next_start_code)
    {
        // lseek(fd, 0, SEEK_SET);
        // frame_size = r_size;
        return -1;
    }

    frame_size = (next_start_code - frame);
    fseek(fp, frame_size - r_size, SEEK_CUR);
    return frame_size;
}

int rtp_send_h264_frame(int client_sock_fd,
                        struct RTP_Packet *rp, char *frame, uint32_t frame_size)
{
    uint8_t NALU_type;
    int send_bytes = 0;
    int ret;

    NALU_type = frame[0] & 0x1F;
    printf("\nframe_size=%d", frame_size);
    if (frame_size <= RTP_MAX_PKT_SIZE)
    {
        memcpy(rp->payload, frame, frame_size);
        ret = RTP_send_packet_over_TCP(client_sock_fd, rp, frame_size, 0x00);
        if (ret < 0)
        {
            return -1;
        }

        rp->rh.seq++;
        send_bytes += ret;
        if (NALU_type == 7 || NALU_type == 8)
        {
            return send_bytes;
        }
    }
    else
    {
        int pkt_num = frame_size / RTP_MAX_PKT_SIZE;
        int remain_pkt_size = frame_size % RTP_MAX_PKT_SIZE;
        int pos = 1;
        for (int i = 0; i < pkt_num; i++)
        {
            rp->payload[0] = (frame[0] & 0x60) | 28;
            rp->payload[1] = NALU_type;

            if (i == 0)
            {
                rp->payload[1] |= 0x80;
            }
            else if (remain_pkt_size == 0 && i == pkt_num - 1)
            {
                rp->payload[1] |= 0x40;
            }

            memcpy(rp->payload + 2, frame + pos, RTP_MAX_PKT_SIZE);
            ret = RTP_send_packet_over_TCP(client_sock_fd, rp, RTP_MAX_PKT_SIZE + 2, 0x00);
            if (ret < 0)
            {
                return -1;
            }

            rp->rh.seq++;
            send_bytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }

        if (remain_pkt_size > 0)
        {
            rp->payload[0] = (frame[0] & 0x60) | 28;
            rp->payload[1] = NALU_type | 0x40;

            memcpy(rp->payload + 2, frame + pos, remain_pkt_size + 2);
            ret = RTP_send_packet_over_TCP(client_sock_fd, rp, remain_pkt_size + 2, 0x00);
            if (ret < 0)
            {
                return -1;
            }

            rp->rh.seq++;
            send_bytes += ret;
        }
    }
    rp->rh.timestamp += 90000 / 25;
    return send_bytes;
}

int handle_cmd_OPTIONS(char *result, int CSeq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                    "\r\n",
            CSeq);
    return 0;
}

int handle_cmd_DESCRIBE(char *result, int CSeq, char *url)
{
    char sdp[500];
    char local_ip[100];
    sscanf(url, "rtsp://%[^:]:", local_ip);
    sprintf(sdp, "v=0\r\n"
                 "o=- 9%ld 1 IN IP4 %s\r\n"
                 "t=0 0\r\n"
                 "a=control:*\r\n"
                 "m=video 0 RTP/AVP/TCP 96\r\n"
                 "a=rtpmap:96 H264/90000\r\n"
                 "a=control:track0\r\n"
                 "m=audio 1 RTP/AVP 97\r\n"
                 "a=rtpmap:97 mpeg4-generic/44100/2\r\n"
                 "a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=1210;\r\n"
                 "a=control:track1\r\n",
            time(NULL), local_ip);

    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Content-Base: %s\r\n"
                    "Content-type: application/sdp\r\n"
                    "Content-length: %zu\r\n\r\n"
                    "%s",
            CSeq, url, strlen(sdp), sdp);
    return 0;
}

static int handle_cmd_SETUP(char *result, int CSeq)
{
    if (CSeq == 3)
    {
        sprintf(result, "RTSP/1.0 200 OK\r\n"
                        "CSeq: %d\r\n"
                        "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
                        "Session: 66334873\r\n"
                        "\r\n",
                CSeq);
    }
    else if (CSeq == 4)
    {
        sprintf(result, "RTSP/1.0 200 OK\r\n"
                        "CSeq: %d\r\n"
                        "Transport: RTP/AVP/TCP;unicast;interleaved=2-3\r\n"
                        "Session: 66334873\r\n"
                        "\r\n",
                CSeq);
    }
    return 0;
}

int handle_cmd_PLAY(char *result, int CSeq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Range: npt=0.000-\r\n"
                    "Session: 66334873; timeout=10\r\n\r\n",
            CSeq);
    return 0;
}

int create_tcp_socket()
{
    int sock_fd, on = 1;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        return -1;
    }

    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
    return sock_fd;
}

int create_udp_socket()
{
    int sock_fd, on = 1;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        return -1;
    }

    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
    return sock_fd;
}

int bind_socket_addr(int sock_fd, const char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip);

    int bind_res = bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    return bind_res < 0 ? -1 : 0;
}

int accept_client(int sock_fd, char *ip, int *port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int client_fd = accept(sock_fd, (struct sockaddr *)&addr, &len);
    if (client_fd < 0)
    {
        return -1;
    }

    strcpy(ip, inet_ntoa(addr.sin_addr));
    *port = ntohs(addr.sin_port);
    return client_fd;
}

void do_client(int client_sock_fd, const char *client_ip, int client_port)
{
    char method[40], url[100], version[40];
    int CSeq, client_rtp_port, client_rtcp_port, server_rtp_sock_fd = -1, server_rtcp_sock_fd = -1;
    char *r_buf = (char *)malloc(BUF_MAX_SIZE);
    char *s_buf = (char *)malloc(BUF_MAX_SIZE);

    while (true)
    {
        int recv_len = recv(client_sock_fd, r_buf, BUF_MAX_SIZE, 0);
        if (recv_len <= 0)
        {
            break;
        }

        r_buf[recv_len] = '\0';
        printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        printf("\n%s r_buf = %s", __FUNCTION__, r_buf);

        const char *sep = "\n";
        char *line = strtok(r_buf, sep);
        while (line)
        {
            if ((strstr(line, "OPTIONS") ||
                 strstr(line, "DESCRIBE") ||
                 strstr(line, "SETUP") ||
                 strstr(line, "PLAY")) &&
                sscanf(line, "%s %s %s", method, url, version) != 3)
            {
                // error
            }
            else if (strstr(line, "CSeq") &&
                     sscanf(line, "CSeq: %d", &CSeq) != 1)
            {
                // error
            }
            else if (!strncmp(line, "Transport:", strlen("Transport:")) &&
                     sscanf(line, "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n") != 0)
            {
                // error
                printf("\nParse transport error");
            }
            line = strtok(NULL, sep);
        }

        if (!strcmp(method, "OPTIONS"))
        {
            if (handle_cmd_OPTIONS(s_buf, CSeq))
            {
                printf("\nFail to handle options");
                break;
            }
        }
        else if (!strcmp(method, "DESCRIBE"))
        {
            if (handle_cmd_DESCRIBE(s_buf, CSeq, url))
            {
                printf("\nFail to handle describe");
                break;
            }
        }
        else if (!strcmp(method, "SETUP"))
        {
            if (handle_cmd_SETUP(s_buf, CSeq))
            {
                printf("\nFail to handle setup");
                break;
            }
        }
        else if (!strcmp(method, "PLAY"))
        {
            if (handle_cmd_PLAY(s_buf, CSeq))
            {
                printf("\nFail to handle play");
                break;
            }
        }
        else
        {
            printf("\nUndefined method = %s", method);
            break;
        }
        printf("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        printf("\n%s s_buf = %s", __FUNCTION__, s_buf);

        send(client_sock_fd, s_buf, strlen(s_buf), 0);
        if (!strcmp(method, "PLAY"))
        {
            std::thread t1([&]()
                           {                
                FILE *fp = fopen(H264_FILE_NAME, "rb");
                if (!fp)
                {
                    printf("\nFail to read %s", AAC_FILE_NAME);
                    return;
                }

                int frame_size, start_code;
                char *frame = (char *)malloc(500000);
                struct RTP_Packet *rp = (struct RTP_Packet *)malloc(500000);
                RTP_header_init(rp, 0, 0, 0, RTP_VERSION, RTP_PAYLOAD_TYPE_H264, 0, 0, 0, 0x88923423);
                printf("\nstart play");

                while (true)
                {
                    frame_size = get_frame_from_h264_file(fp, frame, 500000);
                    if (frame_size < 0)
                    {
                        printf("\nRead %s break, frame_size=%d", H264_FILE_NAME, frame_size);
                        break;
                    }

                    if (start_code3(frame))
                    {
                        start_code = 3;
                    }
                    else
                    {
                        start_code = 4;
                    }
                    frame_size -= start_code;
                    rtp_send_h264_frame(client_sock_fd, rp, frame + start_code, frame_size);
                    rp->rh.timestamp += 90000 / 25;

                    Sleep(20);
                    // usleep(40000);//1000/25 * 1000
                }

                free(frame);
                free(rp); });
            std::thread t2([&]()
                           {
                FILE *fp = fopen(AAC_FILE_NAME, "rb");
                if (!fp)
                {
                    printf("\nFail to read %s", AAC_FILE_NAME);
                    return;
                }

                int ret;
                uint8_t *frame = (uint8_t *)malloc(5000);
                struct RTP_Packet *rp = (struct RTP_Packet *)malloc(5000);
                struct ADTS_Header ah;
                RTP_header_init(rp, 0, 0, 0, RTP_VERSION, RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);
                printf("\nstart play");

                while (true)
                {
                    ret = fread(frame, 1, 7, fp);
                    if (ret <= 0)
                    {
                        printf("\nFread error");
                        break;
                    }
                    printf("\nFread ret=%d", ret);

                    ret = parse_ADTS_header(frame, &ah);
                    if (ret < 0)
                    {
                        printf("\nParse ADTS header error");
                        break;
                    }

                    ret = fread(frame, 1, ah.aac_frame_len - 7, fp);
                    if (ret <= 0)
                    {
                        printf("\nFread error");
                        break;
                    }

                    rtp_send_aac_frame(client_sock_fd, rp, frame, ah.aac_frame_len - 7);

                    Sleep(23);
                    // usleep(23223);//1000/43.06 * 1000
                }

                free(frame);
                free(rp); });
            t1.join();
            t2.join();
            break;
        }

        memset(method, 0, sizeof(method) / sizeof(char));
        memset(url, 0, sizeof(url) / sizeof(char));
        CSeq = 0;
    }

    closesocket(client_sock_fd);
    free(r_buf);
    free(s_buf);
}

int main(int argc, char *argv[])
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        printf("\nPC server socket start up error");
        return -1;
    }

    int server_sock_fd = create_tcp_socket();
    if (server_sock_fd < 0)
    {
        WSACleanup();
        printf("\nFail to create tcp socket");
        return -1;
    }

    int res = bind_socket_addr(server_sock_fd, "0.0.0.0", SERVER_PORT);
    if (res < 0)
    {
        printf("\nFail to bind addr");
        return -1;
    }

    res = listen(server_sock_fd, 10);
    if (res < 0)
    {
        printf("\nFail to listen");
        return -1;
    }

    printf("\n%s rtsp://127.0.0.1:%d", __FILE__, SERVER_PORT);
    while (true)
    {
        char client_ip[40];
        int client_port;
        int client_sock_fd = accept_client(server_sock_fd, client_ip, &client_port);
        if (client_sock_fd < 0)
        {
            printf("\nFail to accept client");
            return -1;
        }

        printf("\nAccept client; client ip:%s, client port:%d", client_ip, client_port);
        do_client(client_sock_fd, client_ip, client_port);
    }
    closesocket(server_sock_fd);
    return 0;
}