#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

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

    int epfd = epoll_create(1000); // 有关size参数介绍：https://blog.csdn.net/zhoumuyu_yu/article/details/112472419
    if (epfd == -1)
    {
        perror("\nepoll_create");
        exit(0);
    }

    struct epoll_event server_fdEvt;
    server_fdEvt.events = EPOLLIN;
    server_fdEvt.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &server_fdEvt);

    struct epoll_event events[1024];
    int s1 = sizeof(events);
    int s2 = sizeof(events[0]);

    char sendBuf[10000];
    int sendBufLen = 10000;
    memset(sendBuf, 0, sendBufLen);

    while (true)
    {
        int nums = epoll_wait(epfd, events, s1 / s2, 1);
        printf("\nserver_fd=%d, nums=%d", server_fd, nums);

        for (int i = 0; i < nums; ++i)
        {
            int curfd = events[i].data.fd;

            printf("\ncurfd=%d", curfd);
            if (curfd == server_fd)
            {
                struct sockaddr_in conn_addr;
                socklen_t conn_addr_len = sizeof(addr);
                int connfd = accept(server_fd, (struct sockaddr *)&conn_addr, &conn_addr_len);

                printf("\nconnfd=%d", connfd);
                if (connfd == -1)
                {
                    perror("\naccept");
                    exit(0);
                    break;
                }

                server_fdEvt.events = EPOLLIN | EPOLLOUT;
                server_fdEvt.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &server_fdEvt);
            }
            else
            {
                // if(events[i].events & EPOLLOUT)
                // {
                //     continue;
                // }

                if (events[i].events & EPOLLIN)
                {
                    char buf[128];
                    int count = read(curfd, buf, sizeof(buf));
                    if (count <= 0)
                    {
                        printf("\nclient disconnect ...");
                        close(curfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        continue;
                    }

                    printf("\nclient say: %s", buf);
                }

                if (curfd > -1)
                {
                    int size = send(curfd, sendBuf, sendBufLen, 0);
                    if (size < 0)
                    {
                        printf("curfd=%d,send error \n", curfd);
                        close(curfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        continue;
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
