#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#define LOG_INFO(format, ...) fprintf(stdout, "\n[INFO] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(stderr, "\n[INERROR] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

int main(int argc, char **argv)
{
    const char *server_ip = "127.0.0.1";
    const int server_port = 8080;
    LOG_INFO("TCP Client start, server %s:%d", server_ip, server_port);

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG_ERROR("WSAStartup error");
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        LOG_ERROR("Create socket error");
        WSACleanup();
        return -1;
    }

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    struct in_addr sin_addr;
    sin_addr.S_un.S_addr = inet_addr(server_ip);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr = sin_addr;
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(sockaddr_in)) == -1)
    {
        LOG_ERROR("Socket connect error");
        return -1;
    }
    LOG_INFO("fd=%d connect success", fd);

    char buf[10000];
    int size;
    uint64_t total_size = 0;
    time_t t1 = time(NULL);

    while (true)
    {
        size = recv(fd, buf, sizeof(buf), 0);
        if (size <= 0)
        {
            break;
        }

        total_size += size;
        if (total_size > 60 * 1024 * 1024)
        {
            time_t t2 = time(NULL);
            if (t2 - t1 > 0)
            {
                uint64_t speed = total_size / (t2 - t1) / 1024 / 1024;
                LOG_INFO("fd=%d, size=%d, total_size=%d, speed=%lluMbps\n", fd, size, total_size, speed);
                total_size = 0;
                t1 = time(NULL);
            }
        }
    }

    LOG_INFO("fd=%d disconnect", fd);
    closesocket(fd);
    WSACleanup();
    return 0;
}