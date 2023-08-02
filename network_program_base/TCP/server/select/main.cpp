#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#define LOG_INFO(format, ...) fprintf(stdout, "\n[INFO] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(stderr, "\n[INERROR] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

static std::string get_time()
{
    const char *time_fmt = "%Y-%m-%d %H:%M:%S";
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), time_fmt);
    return ss.str();
}

int main()
{
    const char *ip = "127.0.0.1";
    uint16_t port = 8080;
    LOG_INFO("TCP Server_select tcp://%s:%d", ip, port);

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG_ERROR("WSAStartup error");
        return -1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        LOG_ERROR("Create socket error");
        return -1;
    }

    int ret;
    unsigned long arg = 1;
    ret = ioctlsocket(server_fd, FIONBIO, (unsigned long *)&arg);
    if (ret == SOCKET_ERROR)
    {
        LOG_ERROR("Set non-blocking fail");
        return -1;
    }

    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        LOG_INFO("Socket bind error");
        return -1;
    }
    if (listen(server_fd, 10) < 0)
    {
        LOG_ERROR("Socket listen error");
        return -1;
    }

    int recv_buf_len = 1000;
    char recv_buf[recv_buf_len] = {0};
    int recv_len = 0;
    int send_buf_len = 10000;
    char send_buf[send_buf_len];
    memset(send_buf, 0, send_buf_len);

    int max_fd = 0;
    fd_set read_fds;
    FD_ZERO(&read_fds);

    FD_SET(server_fd, &read_fds);
    max_fd = std::max(max_fd, server_fd);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    while (true)
    {
        fd_set read_fds_tmp;
        FD_ZERO(&read_fds_tmp);
        read_fds_tmp = read_fds;

        ret = select(max_fd + 1, &read_fds_tmp, nullptr, nullptr, &timeout);
        if (ret < 0)
        {
            LOG_ERROR("No active fd is detected");
        }
        else
        {
            for (int fd = 3; fd < max_fd + 1; fd++)
            {
                if (!FD_ISSET(fd, &read_fds_tmp))
                {
                    continue;
                }

                LOG_INFO("fd=%d event trigger: reading or writing");
                if (fd == server_fd)
                {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd == -1)
                    {
                        LOG_ERROR("Accept error");
                    }

                    LOG_INFO("Find new connection: client_fd=%d", client_fd);
                    FD_SET(client_fd, &read_fds);
                    max_fd = std::max(max_fd, client_fd);
                }
                else
                {
                    recv_len = recv(fd, recv_buf, recv_buf_len, 0);
                    if (recv_len <= 0)
                    {
                        LOG_ERROR("fd=%d, recv_len=%d -> error", fd, recv_len);
                        closesocket(fd);
                        FD_CLR(fd, &read_fds);
                        continue;
                    }

                    LOG_INFO("fd=%d, recv_len=%d -> success", fd, recv_len);
                }
            }
        }

        for (int i = 0; i < read_fds.fd_count; i++)
        {
            int fd = read_fds.fd_array[i];
            if (fd == server_fd)
            {
                continue;
            }

            int size = send(fd, send_buf, send_buf_len, 0);
            if (size < 0)
            {
                LOG_ERROR("Send error: fd=%d, error code=%d", fd, WSAGetLastError());
                continue;
            }
        }
    }

    if (server_fd > -1)
    {
        closesocket(server_fd);
        server_fd = -1;
    }
    WSACleanup();
    return 0;
}