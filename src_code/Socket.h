#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "Util.h"

class Socket
{
//需要事先实现 int port = atoi(argv[1]);
public:
    Socket(int port, const char* ip);
    ~Socket();
    
    void bind_fd();
    void listen_fd();

    int connect_fd();//客户端
    int accept_fd();

    void shutdownWrite();

    void setReuseAddr();
    void setKeepAlive();
    void setNoDelay();
    void setReusePort();

private:
    int listenfd;
    int SERVER_PORT;
    const char* SERVER_IP;
};