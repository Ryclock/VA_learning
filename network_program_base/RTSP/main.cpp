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

// #pragma comment(lib, "ws2_32.lib")
// #pragma warning(disable : 4996)

#define H264_FILE_NAME "./data/test.h264"
#define SERVER_PORT 8554
#define SERVER_RTP_PORT 55532
#define SERVER_RTCP_PORT 55533
#define BUF_MAX_SIZE (1024 * 1024)

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

int RTP_send_h264_frame(int server_RTP_sock_fd, const char *ip, int16_t port, struct RTP_Packet *rp, char *frame, uint32_t frame_size)
{
    uint8_t NALU_type;
    int send_bytes = 0;
    int ret;

    NALU_type = frame[0] & 0x1F;
    printf("\nframe_size=%d", frame_size);
    if (frame_size <= RTP_MAX_PKT_SIZE)
    {
        memcpy(rp->payload, frame, frame_size);
        ret = RTP_send_packet_over_UDP(server_RTP_sock_fd, ip, port, rp, frame_size);
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
            ret = RTP_send_packet_over_UDP(server_RTP_sock_fd, ip, port, rp, RTP_MAX_PKT_SIZE + 2);
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
            ret = RTP_send_packet_over_UDP(server_RTP_sock_fd, ip, port, rp, remain_pkt_size + 2);
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
                 "m=video 0 RTP/AVP 96\r\n"
                 "a=rtpmap:96 H264/90000\r\n"
                 "a=control:track0\r\n",
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

static int handle_cmd_SETUP(char *result, int CSeq, int clientRtpPort)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
                    "Session: 66334873\r\n"
                    "\r\n",
            CSeq, clientRtpPort, clientRtpPort + 1, SERVER_RTP_PORT, SERVER_RTCP_PORT);
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
    int CSeq, client_RTP_port, client_RTCP_port, server_RTP_sock_fd = -1, server_RTCP_sock_fd = -1;
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
                     sscanf(line, "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d",
                            &client_RTP_port, &client_RTCP_port) != 2)
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
                printf("\nFailed to handle options");
                break;
            }
        }
        else if (!strcmp(method, "DESCRIBE"))
        {
            if (handle_cmd_DESCRIBE(s_buf, CSeq, url))
            {
                printf("\nFailed to handle describe");
                break;
            }
        }
        else if (!strcmp(method, "SETUP"))
        {
            if (handle_cmd_SETUP(s_buf, CSeq, client_RTP_port))
            {
                printf("\nFailed to handle setup");
                break;
            }

            server_RTP_sock_fd = create_udp_socket();
            server_RTCP_sock_fd = create_udp_socket();
            if (server_RTP_sock_fd < 0 || server_RTCP_sock_fd < 0)
            {
                printf("\nFailed to create UDP socket");
                break;
            }
            if (bind_socket_addr(server_RTP_sock_fd, "0.0.0.0", SERVER_RTP_PORT) < 0 ||
                bind_socket_addr(server_RTCP_sock_fd, "0.0.0.0", SERVER_RTCP_PORT) < 0)
            {
                printf("\nFailed to bind addr");
                break;
            }
        }
        else if (!strcmp(method, "PLAY"))
        {
            if (handle_cmd_PLAY(s_buf, CSeq))
            {
                printf("\nFailed to handle play");
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
            int frame_size, start_code;
            char *frame = (char *)malloc(500000);
            struct RTP_Packet *rp = (struct RTP_Packet *)malloc(500000);
            FILE *fp = fopen(H264_FILE_NAME, "rb");
            if (!fp)
            {
                printf("\nFail to read %s", H264_FILE_NAME);
                break;
            }

            RTP_header_init(rp, 0, 0, 0, RTP_VERSION, RTP_PAYLOAD_TYPE_H264, 0, 0, 0, 0x88923423);
            printf("\nstart play\nclient ip:%s\nclient port:%d", client_ip, client_RTP_port);
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
                RTP_send_h264_frame(server_RTP_sock_fd, client_ip, client_RTP_port, rp, frame + start_code, frame_size);

                Sleep(40);
                // usleep(40000); // 1000/25 * 1000
            }

            free(frame);
            free(rp);
            break;
        }

        memset(method, 0, sizeof(method) / sizeof(char));
        memset(url, 0, sizeof(url) / sizeof(char));
        CSeq = 0;
    }

    closesocket(client_sock_fd);
    if (server_RTP_sock_fd)
    {
        closesocket(server_RTP_sock_fd);
    }
    if (server_RTCP_sock_fd > 0)
    {
        closesocket(server_RTCP_sock_fd);
    }

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
        printf("\nFailed to create tcp socket");
        return -1;
    }

    int bind_res = bind_socket_addr(server_sock_fd, "0.0.0.0", SERVER_PORT);
    if (bind_res < 0)
    {
        printf("\nFailed to bind addr");
        return -1;
    }

    int listen_res = listen(server_sock_fd, 10);
    if (listen_res < 0)
    {
        printf("\nFailed to listen");
        return -1;
    }

    printf("\n%s rtsp://127.0.0.1:%d", __FILE__, SERVER_PORT);
    while (true)
    {
        int client_sock_fd, client_port;
        char client_ip[40];

        client_sock_fd = accept_client(server_sock_fd, client_ip, &client_port);
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