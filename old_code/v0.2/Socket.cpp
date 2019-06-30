#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "Socket.h"
#include "Util.h"

// struct linger tmp = { 1, 0 };
// setsockopt( listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

int Createlistenfd()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    setnonblocking(sockfd);
    // SetCloseOnExec(sockfd);
    return sockfd;
}

Socket::Socket(int port, const char* ip): listenfd(Createlistenfd()), SERVER_IP(ip), SERVER_PORT(port) {}

Socket::~Socket()
{
    int ret = close(listenfd);
    assert(ret >= 0);
}

void Socket::bind_fd()
{
    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, SERVER_IP, &address.sin_addr );
    address.sin_port = htons( SERVER_PORT );

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret >= 0 );
}

void Socket::listen_fd()
{
    int ret = listen( listenfd, 5 );
    assert( ret >= 0 );
}

int Socket::accept_fd()
{

}