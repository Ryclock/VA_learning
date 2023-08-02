#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("\nsocket");
        exit(0);
    }

    struct sockaddr_in addr;
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("\nbind");
        exit(0);
    }

    ret = listen(server_fd, 100);
    if (ret == -1)
    {
        perror("\nlisten");
        exit(0);
    }

    struct pollfd fds[1024];
    for (int i = 0; i < 1024; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = server_fd;

    char sendBuf[10000];
    int sendBufLen = 10000;
    memset(sendBuf, 0, sendBufLen);

    int maxfd = 0;
    while (true)
    {
        printf("\nmaxfd=%d", maxfd);
        ret = poll(fds, maxfd + 1, 1);
        if (ret == -1)
        {
            perror("\nselect");
            break;
        }

        if (fds[0].revents & POLLIN)
        {

            struct sockaddr_in conn_addr;
            socklen_t conn_addr_len = sizeof(addr);
            int connfd = accept(server_fd, (struct sockaddr *)&conn_addr, &conn_addr_len);
            int i;
            for (i = 0; i < 1024; ++i)
            {
                if (fds[i].fd == -1)
                {
                    fds[i].fd = connfd;
                    break;
                }
            }
            maxfd = i > maxfd ? i : maxfd;
        }

        for (int i = 1; i <= maxfd; ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                char buf[128];
                int count = read(fds[i].fd, buf, sizeof(buf));
                if (count <= 0)
                {
                    printf("\nclient disconnect ...");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    continue;
                }
                printf("\nclient say: %s", buf);
                write(fds[i].fd, buf, strlen(buf) + 1);
            }

            if (fds[i].fd > -1)
            {
                printf("\nfds[%d].fd=%d", i, fds[i].fd);
                int size = send(fds[i].fd, sendBuf, sendBufLen, 0);
                if (size < 0)
                {
                    printf("\nfds[%d].fd=%d, send error", i, fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    continue;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
