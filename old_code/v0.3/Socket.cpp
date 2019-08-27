#include "Socket.h"
#include <iostream>
using namespace std;
// struct linger tmp = { 1, 0 };
// setsockopt( listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

int Createlistenfd()
{
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int optval = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1){
        return -1;
    }
    assert(setnonblocking(listenfd) >= 0);
    
    return listenfd;
}

Socket::Socket(int port, const char* ip): listenfd(Createlistenfd()), SERVER_PORT(port), SERVER_IP(ip) { }
Socket::Socket(int port): listenfd(Createlistenfd()), SERVER_PORT(port), SERVER_IP(nullptr) { } 

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

    if(SERVER_IP){
        cout << "Setting IP" << endl;
        inet_pton( AF_INET, SERVER_IP, &address.sin_addr );
        address.sin_port = htons( SERVER_PORT );
    }
    else{
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons((unsigned short)SERVER_PORT);
    }
    

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret >= 0 );
}

int Socket::listen_fd()
{
    int ret = listen( listenfd, 5 );
    assert( ret >= 0 );
    return listenfd;
}

int Socket::accept_fd()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof( client_address );
    int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
    if(connfd == -1)
        perror("accept");
    assert(setnonblocking(connfd) >= 0);
    return connfd;
}

// void accept_connection(int listen_fd, int epoll_fd, char* path){
//     struct sockaddr_in client_addr;
//     memset(&client_addr, 0, sizeof(struct sockaddr_in));
//     socklen_t client_addr_len = 0;
//     int accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
//     if(accept_fd == -1)
//         perror("accept");

//     // 设为非阻塞模式
//     int rc = make_socket_non_blocking(accept_fd);

//     // 申请tk_http_request_t类型节点并初始化
//     tk_http_request_t* request = (tk_http_request_t*)malloc(sizeof(tk_http_request_t));
//     tk_init_request_t(request, accept_fd, epoll_fd, path);

//     // 文件描述符可以读，边缘触发(Edge Triggered)模式，保证一个socket连接在任一时刻只被一个线程处理
//     tk_epoll_add(epoll_fd, accept_fd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));

//     // 新增时间信息
//     tk_add_timer(request, TIMEOUT_DEFAULT, tk_http_close_conn);
// }