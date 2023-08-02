#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#define LOG_INFO(format, ...) fprintf(stdout, "\n[INFO] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(stderr, "\n[INERROR] [%s:%d]:%s() " format "", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

class Server;
class Connection
{
public:
    typedef void (*DisconnectionCallback)(void *, int);

private:
    std::thread *thr = nullptr;
    Server *m_server;
    int m_client_fd;
    DisconnectionCallback m_disconnection_callback = nullptr;
    void *m_arg = nullptr;

    void sendData()
    {
        int size;
        uint64_t total_size = 0;
        time_t t1 = time(NULL);

        while (true)
        {
            char buf[1024];
            memset(buf, 1, sizeof(buf));
            size = ::send(m_client_fd, buf, sizeof(buf), 0);
            if (size < 0)
            {
                LOG_ERROR("Send error: client_fd=%d, error code=%d", m_client_fd, WSAGetLastError());
                m_disconnection_callback(m_arg, m_client_fd);
                break;
            }

            total_size += size;
            if (total_size > 60 * 1024 * 1024)
            {
                time_t t2 = time(NULL);
                if (t2 - t1 > 0)
                {
                    uint64_t speed = total_size / (t2 - t1) / 1024 / 1024;
                    LOG_INFO("client fd=%d, size=%d, total_size=%d, speed=%lluMbps", m_client_fd, size, total_size, speed);
                    total_size = 0;
                    t1 = time(NULL);
                }
            }
        }
    }

public:
    Connection(Server *server, int client_fd) : m_server(server), m_client_fd(client_fd)
    {
        LOG_INFO("");
    }

    ~Connection()
    {
        LOG_INFO("");
        closesocket(m_client_fd);
        if (thr)
        {
            thr->join();
            delete thr;
            thr = nullptr;
        }
    }

    void set_disconncection_callback(DisconnectionCallback dc, void *arg)
    {
        m_disconnection_callback = dc;
        m_arg = arg;
    }

    int start()
    {
        thr = new std::thread(&Connection::sendData, this);
        return 0;
    }

    int get_client_fd()
    {
        return m_client_fd;
    }
};

class Server
{
private:
    const char *m_ip;
    uint16_t m_port;
    int m_sock_fd;
    std::map<int, Connection *> m_conn_map;
    std::mutex m_conn_map_mtx;

    bool add_connection(Connection *conn)
    {
        m_conn_map_mtx.lock();
        if (m_conn_map.find(conn->get_client_fd()) != m_conn_map.end())
        {
            m_conn_map_mtx.unlock();
            return false;
        }

        m_conn_map.insert(std::make_pair(conn->get_client_fd(), conn));
        m_conn_map_mtx.unlock();
        return true;
    }

    Connection *get_connection(int client_fd)
    {
        m_conn_map_mtx.lock();
        std::map<int, Connection *>::iterator it = m_conn_map.find(client_fd);
        if (it == m_conn_map.end())
        {
            m_conn_map_mtx.unlock();
            return nullptr;
        }

        m_conn_map_mtx.unlock();
        return it->second;
    }

    bool remove_connection(int client_fd)
    {
        m_conn_map_mtx.lock();
        std::map<int, Connection *>::iterator it = m_conn_map.find(client_fd);
        if (it == m_conn_map.end())
        {
            m_conn_map_mtx.unlock();
            return false;
        }

        m_conn_map.erase(it);
        m_conn_map_mtx.unlock();
        return true;
    }

public:
    Server(const char *ip, uint16_t port) : m_ip(ip), m_port(port), m_sock_fd(-1)
    {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        {
            LOG_ERROR("WSAStartup error");
            return;
        }
    }

    ~Server()
    {
        if (m_sock_fd > -1)
        {
            closesocket(m_sock_fd);
            m_sock_fd = -1;
        }
        WSACleanup();
    }

    void handle_disconnection(int client_fd)
    {
        LOG_INFO("client_fd=%d", client_fd);
        this->remove_connection(client_fd);
    }

    static void callback_disconnection(void *arg, int client_fd)
    {
        LOG_INFO("client_fd=%d", client_fd);
        Server *server = (Server *)arg;
        server->handle_disconnection(client_fd);
    }

    int start()
    {
        LOG_INFO("TCP Server2 tcp://%s:%d", m_ip, m_port);
        m_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sock_fd < 0)
        {
            LOG_INFO("Create socket error");
            return -1;
        }

        int on = 1;
        setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

        SOCKADDR_IN server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(m_port);

        if (bind(m_sock_fd, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
        {
            LOG_INFO("Socket bind error");
            return -1;
        }
        if (listen(m_sock_fd, 10) < 0)
        {
            LOG_ERROR("socket listen error");
            return -1;
        }

        while (true)
        {
            LOG_INFO("Block to monitor the new connection...");
            int client_fd;
            char client_ip[40] = {0};
            uint16_t client_port;

            socklen_t len = 0;
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            len = sizeof(addr);

            client_fd = accept(m_sock_fd, (struct sockaddr *)&addr, &len);
            if (client_fd < 0)
            {
                LOG_ERROR("socket accept error");
                return -1;
            }

            strcpy(client_ip, inet_ntoa(addr.sin_addr));
            client_port = ntohs(addr.sin_port);

            LOG_INFO("Find new connection: client_ip=%s, client_port=%d", client_ip, client_fd);
            Connection *conn = new Connection(this, client_fd);
            conn->set_disconncection_callback(Server::callback_disconnection, this);
            this->add_connection(conn);
            conn->start();
        }
        return 0;
    }
};

int main()
{
    const char *ip = "127.0.0.1";
    uint16_t port = 8080;

    Server server(ip, port);
    server.start();

    return 0;
}