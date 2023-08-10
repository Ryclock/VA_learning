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

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define SERVER_PORT 8554
#define SERVER_RTP_PORT 55532
#define SERVER_RTCP_PORT 55533

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
    int CSeq, client_RTP_port, client_RTCP_port;
    char *r_buf = (char *)malloc(10000);
    char *s_buf = (char *)malloc(10000);

    while (true)
    {
        int recv_len = recv(client_sock_fd, r_buf, 2000, 0);
        if (recv_len <= 0)
        {
            break;
        }

        r_buf[recv_len] = '\0';
        std::string recv_str = r_buf;
        const char *sep = "\n";
        char *line = strtok(r_buf, sep);
        printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        printf("\n%s r_buf = %s", __FUNCTION__, r_buf);
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

            printf("\nstart play\nclient ip:%s\nclient port:%d", client_ip, client_RTP_port);
            while (true)
            {

                Sleep(40);
                // usleep(40000); // 1000/25 * 1000
            }
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