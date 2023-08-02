#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdint.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#define LOG_INFO(format, ...) fprintf(stdout, "\n[INFO] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(stderr, "\n[INERROR] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

int main()
{
    const char *ip = "127.0.0.1";
    uint16_t port = 8080;
    LOG_INFO("TCP Server1 tcp://%s:%d", ip, port);

    SOCKET server_fd = -1;
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG_ERROR("WSAStartup error");
        return -1;
    }

    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind(server_fd, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        LOG_ERROR("Socket bind error");
        return -1;
    }
    if (listen(server_fd, SOMAXCONN) < 0)
    {
        LOG_ERROR("Socket listen error");
        return -1;
    }

    while (true)
    {
        LOG_INFO("Block to monitor the new connection...");
        int len = sizeof(SOCKADDR);
        SOCKADDR_IN accept_addr;
        int client_fd = accept(server_fd, (SOCKADDR *)&accept_addr, &len);
        if (client_fd == SOCKET_ERROR)
        {
            LOG_ERROR("Accept connection error");
            break;
        }

        LOG_INFO("Find new connection: client_fd=%d", client_fd);
        int size;
        uint64_t total_size = 0;
        time_t t1 = time(NULL);

        while (true)
        {
            char buf[1024];
            memset(buf, 1, sizeof(buf));
            size = ::send(client_fd, buf, sizeof(buf), 0);
            if (size < 0)
            {
                LOG_INFO("Send error: client_fd=%d, error code=%d", client_fd, WSAGetLastError());
                break;
            }

            total_size += size;
            if (total_size > 60 * 1024 * 1024)
            {
                time_t t2 = time(NULL);
                if (t2 - t1 > 0)
                {
                    uint64_t speed = total_size / (t2 - t1) / 1024 / 1024;
                    LOG_INFO("client fd=%d, size=%d, total_size=%d, speed=%lluMbps", client_fd, size, total_size, speed);
                    total_size = 0;
                    t1 = time(NULL);
                }
            }
        }
        LOG_INFO("Close connection: client_fd=%d", client_fd);
    }
    return 0;
}