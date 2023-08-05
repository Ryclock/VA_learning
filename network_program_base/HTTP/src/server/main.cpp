#include "server/server.h"
#include "utils/log.h"
#include <time.h>

int main(int argc, char **argv) // server
{
#ifdef WIN32
    srand(time(NULL));
#endif
    const char *ip = "127.0.0.1";
    int port = 8080;
    log_info("HTTP Server start http://%s:%d", ip, port);
    HTTP_Server server;
    server.start(ip, port);
    return 0;
}